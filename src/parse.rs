//! 解析 C 文件中的标识符

use std::collections::HashSet;
use std::fs::File;
use std::io::{BufRead, BufReader};
use std::path::Path;

// 简单标识符扫描, 只处理 ASCII 标识符, 不做 C 语法解析
fn is_symbol_char(c: u8) -> bool {
    c.is_ascii_alphanumeric() || c == b'_'
}

// 从行迭代器中提取标识符
fn parse_symbols_from_lines(lines: impl Iterator<Item = String>) -> HashSet<String> {
    let mut symbols = HashSet::new();

    for line in lines {
        let bytes = line.as_bytes();
        let mut i = 0;
        while i < bytes.len() {
            if is_symbol_char(bytes[i]) {
                let start_idx = i;
                i += 1;
                while i < bytes.len() && is_symbol_char(bytes[i]) {
                    i += 1;
                }
                symbols.insert(line[start_idx..i].to_string());
            } else {
                i += 1;
            }
        }
    }
    symbols
}

// 解析文件指定行范围内的符号
pub fn symbols_in_range(
    file_path: &Path,
    start_line: usize,
    end_line: usize,
) -> Result<HashSet<String>, Box<dyn std::error::Error>> {
    let file = File::open(file_path)?;
    let reader = BufReader::new(file);

    let lines = reader
        .lines()
        .enumerate()
        .filter_map(|(line_num, line_result)| {
            let line_num = line_num + 1;
            if line_num >= start_line && line_num <= end_line {
                line_result.ok()
            } else {
                None
            }
        });
    Ok(parse_symbols_from_lines(lines))
}

// 解析整个文件的所有符号
pub fn symbols_in_file(file_path: &Path) -> Result<HashSet<String>, Box<dyn std::error::Error>> {
    let file = File::open(file_path)?;
    let reader = BufReader::new(file);
    let lines = reader.lines().filter_map(|r| r.ok());
    Ok(parse_symbols_from_lines(lines))
}
