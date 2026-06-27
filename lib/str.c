#include <stdio.h>

#include "str.h"

// =============================
//           C字符字符串
// =============================

// @begin string_create
String* string_create(void) {
    return vec_create(sizeof(char), 0);
}
// @end

// @begin string_push_char
int string_push_char(String *s, char c) {
    return vec_push(s, &c);
}
// @end

// @begin string_push
int string_push(String *s, const char *cstr) {
    while (*cstr) {
        if (vec_push(s, &cstr) != 0) return -1;
        cstr++;
    }
    return 0;
}
// @end

// @begin string_cstr
const char* string_cstr(String *s) {
    // 确保有空间存放 '\0'
    if (s->size >= s->capacity) {
        if (vec_reserve(s, s->capacity + 1) != 0) return NULL;
    }
    s->data[s->size] = '\0';   // 在末尾添加空字符，不增加 size
    return s->data;
}
// @end

// @begin string_length
int string_length(const String *s) {
    return s->size;
}
// @end

// @begin string_clear
void string_clear(String *s) {
    s->size = 0;
}
// @end

// @begin string_free
void string_free(String *s) {
    vec_free(s);
}
// @end

// @begin string_readline
int string_readline(String *s, FILE *fp, int keep_newline) {
    if (!s || !fp) return -1;
    int c;
    string_clear(s);   // 清空原有内容
    while ((c = fgetc(fp)) != EOF) {
        if (c == '\n') {
            if (keep_newline) {
                if (vec_push(s, &(char){c}) != 0) return -1;
            }
            return 0;   // 成功读取一行（可能没有字符，但返回 0）
        }
        if (vec_push(s, &(char){c}) != 0) return -1;
    }
    // 遇到 EOF
    if (s->size == 0) return -1;   // 无任何数据
    return 0;   // 最后一行没有换行符
}
// @end
