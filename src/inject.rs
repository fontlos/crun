// inject.rs
use crate::Cli;
use crate::library::Library;
use crate::parse;
use std::collections::{HashMap, HashSet, VecDeque};
use std::fs::File;
use std::io::{BufRead, BufReader, BufWriter, Write};
use std::path::Path;

pub fn inject(cli: &Cli, lib: &Library) -> Result<(), Box<dyn std::error::Error>> {
    // 1. 解析目标文件中的符号，只保留库中存在的符号
    let needed: Vec<String> = parse::symbols_in_file(&cli.file)?
        .into_iter()
        .filter(|s| lib.contains_symbol(s))
        .collect();

    if needed.is_empty() {
        println!("[Info]: No library symbols found in file, skipping injection");
        // 直接将源文件复制到输出目录
        let output_path = cli.out_dir.join(cli.file.file_name().unwrap());
        std::fs::copy(&cli.file, &output_path)?;
        return Ok(());
    }

    println!("[Info]: Found {} library symbols: {:?}", needed.len(), needed);

    // 2. 构建依赖子图并排序，保证依赖在前
    let graph = subgraph(lib, &needed);
    let order = topo(&graph)?;
    println!("[Info]: Injection order: {:?}", order);

    // 3. 生成依赖块并写入新文件
    let dep_blocks = blocks(lib, &order);
    let output_path = cli.out_dir.join(cli.file.file_name().unwrap());
    write_injected(&cli.file, &output_path, &dep_blocks)?;

    println!(
        "[Info]: Injected {} dependencies into {}",
        order.len(),
        output_path.display()
    );

    Ok(())
}

// 构建依赖子图（只包含需要的符号及其传递依赖）
fn subgraph(lib: &Library, needed: &[String]) -> HashMap<String, HashSet<String>> {
    let mut graph = HashMap::with_capacity(needed.len());
    let mut queue: VecDeque<String> = needed.iter().cloned().collect();
    let mut visited = HashSet::with_capacity(needed.len());

    while let Some(symbol) = queue.pop_front() {
        if !visited.insert(symbol.clone()) {
            continue;
        }

        if let Some(info) = lib.find_symbol(&symbol) {
            let mut deps: HashSet<String> = HashSet::new();
            for dep in &info.deps {
                if lib.contains_symbol(dep) {
                    deps.insert(dep.clone());
                }
            }

            for dep in deps.iter() {
                if !visited.contains(dep) {
                    queue.push_back(dep.clone());
                }
            }

            graph.insert(symbol, deps);
        }
    }

    graph
}

// 拓扑排序 (Kahn算法)
fn topo(
    graph: &HashMap<String, HashSet<String>>,
) -> Result<Vec<String>, Box<dyn std::error::Error>> {
    let mut in_degree: HashMap<String, usize> = HashMap::new();
    let mut reverse_adj: HashMap<String, Vec<String>> = HashMap::new();

    // 初始化节点
    for node in graph.keys() {
        in_degree.entry(node.clone()).or_insert(0);
    }

    // 依赖关系: node -> dep, 需要转成 dep -> node 才能保证依赖在前
    for (node, deps) in graph {
        for dep in deps {
            reverse_adj.entry(dep.clone()).or_default().push(node.clone());
            *in_degree.entry(node.clone()).or_insert(0) += 1;
        }
    }

    // 找到入度为0的节点 (没有依赖)
    let mut queue: VecDeque<String> = in_degree
        .iter()
        .filter(|(_, deg)| **deg == 0)
        .map(|(node, _)| node.clone())
        .collect();

    let mut result = Vec::new();

    while let Some(node) = queue.pop_front() {
        result.push(node.clone());

        if let Some(dependents) = reverse_adj.get(&node) {
            for dependent in dependents {
                if let Some(deg) = in_degree.get_mut(dependent) {
                    *deg = deg.saturating_sub(1);
                    if *deg == 0 {
                        queue.push_back(dependent.clone());
                    }
                }
            }
        }
    }

    if result.len() != graph.len() {
        return Err("[Error]: Circular dependency detected".into());
    }

    Ok(result)
}

// 生成依赖块代码
fn blocks(lib: &Library, order: &[String]) -> Vec<String> {
    let mut blocks = Vec::new();

    for symbol in order {
        if let Some(info) = lib.find_symbol(symbol) {
            if let Ok(body) = read_block(&Path::new(&info.file), info.start, info.end) {
                let block = format!("// @begin {}\n{}\n// @end {}", symbol, body, symbol);
                blocks.push(block);
            } else {
                eprintln!("[Warning]: Failed to read function body for {}", symbol);
            }
        }
    }

    blocks
}

// 读取函数体
fn read_block(
    file_path: &Path,
    start: usize,
    end: usize,
) -> Result<String, Box<dyn std::error::Error>> {
    let file = File::open(file_path)?;
    let reader = BufReader::new(file);
    let mut body = Vec::new();

    for (line_num, line_result) in reader.lines().enumerate() {
        let line_num = line_num + 1;
        if line_num >= start && line_num <= end {
            body.push(line_result?);
        }
    }

    Ok(body.join("\n"))
}

// 注入依赖到目标文件
fn write_injected(
    input_path: &Path,
    output_path: &Path,
    dep_blocks: &[String],
) -> Result<(), Box<dyn std::error::Error>> {
    let input_file = File::open(input_path)?;
    let reader = BufReader::new(input_file);

    let mut last_std_include: Option<usize> = None;
    for (idx, line_result) in reader.lines().enumerate() {
        let line = line_result?;
        if let Some(first) = include_first_char(&line) {
            if first != '"' {
                last_std_include = Some(idx);
            }
        }
    }

    let input_file = File::open(input_path)?;
    let reader = BufReader::new(input_file);
    let output_file = File::create(output_path)?;
    let mut writer = BufWriter::new(output_file);

    if last_std_include.is_none() {
        for block in dep_blocks {
            writeln!(writer, "{}", block)?;
            writeln!(writer)?;
        }
    }

    for (idx, line_result) in reader.lines().enumerate() {
        let line = line_result?;
        if include_first_char(&line) == Some('"') {
            continue;
        }

        writeln!(writer, "{}", line)?;

        if let Some(last_idx) = last_std_include {
            if idx == last_idx {
                writeln!(writer)?;
                writeln!(writer, "// ========== INJECTED DEPENDENCIES START ==========")?;
                for block in dep_blocks {
                    writeln!(writer, "{}", block)?;
                    writeln!(writer)?;
                }
                writeln!(writer, "// ========== INJECTED DEPENDENCIES END ==========")?;
            }
        }
    }

    writer.flush()?;
    Ok(())
}

fn include_first_char(line: &str) -> Option<char> {
    let trimmed = line.trim_start();
    if !trimmed.starts_with("#include") {
        return None;
    }
    let rest = trimmed["#include".len()..].trim_start();
    rest.chars().next()
}
