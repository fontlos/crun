use std::fs;
use std::process::{Command, exit};

use crate::Cli;

pub fn build(cli: &Cli) -> Result<(), &'static str> {
    // 生成可执行文件名
    let exe_name = cli
        .output
        .clone()
        .or_else(|| {
            cli.file
                .file_stem()
                .map(|name| name.to_string_lossy().to_string())
        })
        .ok_or_else(|| "Invalid file name")?;

    // 根据平台添加可执行文件扩展名
    let exe_name = if cfg!(windows) {
        format!("{}.exe", exe_name)
    } else {
        exe_name
    };

    let exe_path = cli.out_dir.join(&exe_name);

    // 编译
    if cli.verbose {
        println!("[Info]: Build {}", cli.file.display());
        println!("[Info]: Output: {}", exe_path.display());
        if !cli.compile_args.is_empty() {
            println!("[Info]: Arguments: {}", cli.compile_args.join(" "));
        }
    }

    // TODO: 扩展可选的不注入直接执行
    // 注入后的源码在 out_dir 同名文件
    let source_file = cli.out_dir.join(cli.file.file_name().unwrap());

    let mut compile_cmd = Command::new("clang");
    compile_cmd
        .arg("-o")
        .arg(&exe_path)
        .arg(&source_file)
        .args(&cli.compile_args);

    if cli.verbose {
        println!("[Info]: Command: {:?}", compile_cmd);
    }

    let compile_status = compile_cmd
        .status()
        .map_err(|_| "Failed to execute clang command")?;

    if !compile_status.success() {
        return Err("Compilation failed");
    }

    if cli.verbose {
        println!("[Info]: Compilation successful");
    }

    // 准备运行环境
    let work_dir = cli.work_dir.as_ref().unwrap_or(&cli.out_dir);

    // 如果指定了工作目录且不存在, 则创建
    if !work_dir.exists() {
        fs::create_dir_all(work_dir).map_err(|_| "Failed to create work directory")?;
        if cli.verbose {
            println!("[Info]: Created work directory: {}", work_dir.display());
        }
    }

    // 获取程序参数（编译参数中在 -- 之后的参数）
    let program_args: Vec<String> = cli
        .compile_args
        .iter()
        .skip_while(|arg| *arg != "--")
        .skip(1)
        .cloned()
        .collect();

    if cli.verbose {
        println!("[Info]: Running: {}", exe_path.display());
        if !program_args.is_empty() {
            println!("[Info]: Arguments: {}", program_args.join(" "));
        }
        println!("[Info]: Work Directory: {}", work_dir.display());
    } else {
        println!("[Info]: Build and run: {}", cli.file.display());
    }

    // 运行程序
    let run_status = Command::new(&exe_path)
        .args(&program_args)
        .current_dir(work_dir)
        .status()
        .map_err(|_| "Failed to run the executable")?;

    // 清理可执行文件
    if !cli.keep && exe_path.exists() {
        if let Err(e) = fs::remove_file(&exe_path) {
            eprintln!(
                "[Warning]: Failed to delete temporary file '{}': {}",
                exe_path.display(),
                e
            );
        } else if cli.verbose {
            println!("[Info]: Deleted temporary file: {}", exe_path.display());
        }
    }

    exit(run_status.code().unwrap_or(0));
}
