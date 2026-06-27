use serde::{Deserialize, Serialize};
use walkdir::WalkDir;

use std::collections::{HashMap, HashSet};
use std::fs::File;
use std::io::{BufRead, BufReader, BufWriter};
use std::path::Path;
use std::time::{SystemTime, UNIX_EPOCH};

pub fn load_or_build<P: AsRef<Path>>(path: P) -> Result<Library, Box<dyn std::error::Error>> {
    // 目标目录下查找 lib.json，缺失则重建索引
    let lib_json = path.as_ref().join("lib.json");
    if lib_json.exists() {
        println!("[Info]: Load library index: {}", path.as_ref().display());
        Library::load(&lib_json)
    } else {
        let timestamp = SystemTime::now().duration_since(UNIX_EPOCH)?.as_secs();
        println!("[Warning]: Library index not found, building...");
        let mut library = Library::new(timestamp.to_string().as_str());
        // 第一次遍历：构建符号索引
        library.index(&path)?;
        // 第二次遍历：解析符号依赖
        library.deps()?;
        library.save(&lib_json)?;
        Ok(library)
    }
}

#[derive(Debug, Default, Deserialize, Serialize)]
pub struct Library {
    version: String,
    symbols: HashMap<String, Info>,
}

#[derive(Debug, Default, Deserialize, Serialize)]
pub struct Info {
    pub file: String,
    pub start: usize,
    pub end: usize,
    pub deps: HashSet<String>,
}

impl Library {
    fn new(version: &str) -> Self {
        Library {
            version: version.to_string(),
            symbols: HashMap::new(),
        }
    }

    // 从目录构建索引, 仅收集 @begin/@end 标记
    fn index<P: AsRef<Path>>(&mut self, path: &P) -> Result<(), Box<dyn std::error::Error>> {
        for entry in WalkDir::new(path)
            .into_iter()
            .filter_map(|e| e.ok())
            .filter(|e| {
                e.path()
                    .extension()
                    .map(|ext| ext == "c" || ext == "h")
                    .unwrap_or(false)
            })
        {
            let path = entry.path();
            self.scan(path)?;
        }
        Ok(())
    }

    // 扫描单个文件中的符号
    fn scan(&mut self, path: &Path) -> Result<(), Box<dyn std::error::Error>> {
        let file = File::open(path)?;
        let reader = BufReader::new(file);

        let mut in_block = false;
        let mut block_name = String::new();
        let mut block_start = 0;

        for (line_num, line_result) in reader.lines().enumerate() {
            let line = line_result?;
            let trimmed = line.trim();

            if !in_block && trimmed.starts_with("// @begin") {
                in_block = true;
                block_name = trimmed[9..].trim().to_string();
                block_start = line_num + 1;
            } else if in_block && trimmed.starts_with("// @end") {
                self.symbols.insert(
                    block_name.clone(),
                    Info {
                        file: path.display().to_string(),
                        start: block_start + 1,
                        end: line_num,
                        deps: HashSet::new(),
                    },
                );
                in_block = false;
            }
        }

        if in_block {
            eprintln!(
                "[Warning]: Unclosed block {} in {}",
                block_name,
                path.display()
            );
        }

        Ok(())
    }

    // 解析所有符号的依赖关系
    fn deps(&mut self) -> Result<(), Box<dyn std::error::Error>> {
        let symbol_set: HashSet<String> = self.symbols.keys().cloned().collect();
        let symbols_to_process: Vec<String> = self.symbols.keys().cloned().collect();

        for symbol_name in symbols_to_process {
            let (file_path, start, end) = {
                let info = self.symbols.get(&symbol_name).unwrap();
                (info.file.clone(), info.start, info.end)
            };

            let all_symbols = crate::parse::symbols_in_range(&Path::new(&file_path), start, end)?;

            let deps: HashSet<String> = all_symbols
                .into_iter()
                .filter(|s| symbol_set.contains(s) && s != &symbol_name)
                .collect();

            if let Some(info) = self.symbols.get_mut(&symbol_name) {
                info.deps = deps;
            }
        }

        Ok(())
    }

    // 查找符号信息
    pub fn find_symbol(&self, symbol_name: &str) -> Option<&Info> {
        self.symbols.get(symbol_name)
    }

    // 检查符号是否存在
    pub fn contains_symbol(&self, symbol_name: &str) -> bool {
        self.symbols.contains_key(symbol_name)
    }

    // 保存到文件
    fn save<P: AsRef<Path>>(&self, path: &P) -> Result<(), Box<dyn std::error::Error>> {
        let file = File::create(path)?;
        let writer = BufWriter::new(file);
        serde_json::to_writer_pretty(writer, self)?;
        Ok(())
    }

    // 从文件加载
    fn load<P: AsRef<Path>>(path: &P) -> Result<Self, Box<dyn std::error::Error>> {
        let file = File::open(path)?;
        let reader = BufReader::new(file);
        let library: Library = serde_json::from_reader(reader)?;
        Ok(library)
    }
}
