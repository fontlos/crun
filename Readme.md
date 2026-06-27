# crun

A simple C single-file execution tool that bundles a common data structure library and enables direct execution of `.c` files through static code injection.

Based on Clang. Just for homework: [judge.buaa.edu.cn](judge.buaa.edu.cn)

## Usage

```bash
Usage: crun [OPTIONS] <FILE> [-- <COMPILE_ARGS>...]

Arguments:
  <FILE>             C 源文件路径
  [COMPILE_ARGS]...  编译选项, 传递给 'clang'

Options:
  -w, --work-dir <DIR>  程序运行的工作目录
  -k, --keep            保留编译后的可执行文件
  -v, --verbose         显示详细输出
  -o, --out-dir <DIR>   指定输出目录, [default: target]
  -o, --output <NAME>   指定输出文件名, 默认与源文件同名
  -h, --help            Print help
  -V, --version         Print version
```

## Use case

Directory Structure

```
├── lib/            # Bundled C data structure library copy from repo
│   ├── bintree.c/h
│   ├── graph.c/h
│   ├── list.c/h
│   ├── ring.c/h
│   ├── str.c/h
│   └── vec.c/h
├── main.c
└── crun.exe
```

### `main.c`

```c
#include <stdio.h>
#include "./lib/vec.h"

int main() {
    Vector *v1 = vec_create(sizeof(int), 0);
    int x = 42, y = 100;
    vec_push(v1, &x);
    vec_push(v1, &y);

    int *p = vec_get(v1, 0);
    printf("%d\n", *p);  // 42

    int z = 999;
    vec_set(v1, 0, &z, NULL);
    int *p2 = vec_get(v1, 0);
    printf("%d\n", *p2);  // 999

    int top;
    vec_pop(v1, &top);
    printf("%d\n", top);  // 100
    vec_free(v1);

    return 0;
}
```

### Execution

```bash
$ crun main.c
```

### What Happens

1. `crun` parses `main.c`'s identifies
2. Recursively collects dependencies from `lib/`
3. Generates `target/main.c` with all dependencies injected
4. Compiles `target/main.c` with clang
5. Runs the executable

### Generated `target/main.c` (simplified)

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ========== INJECTED DEPENDENCIES START ==========
typedef struct {
    char *data;
    int elem_size;
    int size;
    int capacity;
} Vector;

Vector* vec_create(int elem_size, int capacity) {
    // implementation
}

void vec_free(Vector *v) {
    // implementation
}

// ... other implementations

// ========== INJECTED DEPENDENCIES START ==========

// You original code
int main() {
    Vector *v1 = vec_create(sizeof(int), 0);
    // ...
}
```
