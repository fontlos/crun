use clap::Parser;
use std::fs;
use std::path::PathBuf;

mod build;
mod inject;
mod library;
pub mod parse;

/// 编译并运行 C 文件
#[derive(Parser)]
#[command(name = "crun")]
#[command(author, version, about, long_about = None)]
struct Cli {
    /// C 源文件路径
    #[arg(value_name = "FILE")]
    file: PathBuf,

    /// 编译选项, 传递给 'clang'
    #[arg(last = true, allow_hyphen_values = true)]
    compile_args: Vec<String>,

    /// 程序运行的工作目录
    #[arg(short, long, value_name = "DIR")]
    work_dir: Option<PathBuf>,

    /// 保留编译后的可执行文件
    #[arg(short, long)]
    keep: bool,

    /// 显示详细输出
    #[arg(short, long)]
    verbose: bool,

    /// 指定输出目录
    #[arg(short, long, value_name = "DIR", default_value = "target")]
    out_dir: PathBuf,

    /// 指定输出文件名, 默认与源文件同名
    #[arg(short, long, value_name = "NAME")]
    output: Option<String>,
}

fn main() {
    let cli = Cli::parse();

    let lib = library::load_or_build("lib").unwrap();

    // 检查 C 文件是否存在
    if !cli.file.exists() {
        eprintln!("[Error]: File '{}' Not Found", cli.file.display());
    }

    // 检查文件扩展名
    let ext = cli.file.extension().and_then(|e| e.to_str());
    if ext != Some("c") && ext != Some("C") {
        eprintln!("[Warning]: File extension is not .c");
    }

    // 创建输出目录
    if let Err(e) = fs::create_dir_all(&cli.out_dir) {
        eprintln!(
            "[Error]: Failed to create output directory '{}': {}",
            cli.out_dir.display(),
            e
        );
    }

    if let Err(e) = inject::inject(&cli, &lib) {
        eprintln!("[Error]: Injection failed: {}", e);
    }

    if let Err(e) = build::build(&cli) {
        eprintln!("[Error]: {}", e);
    }
}
