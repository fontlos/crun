#ifndef STDSTR_H
#define STDSTR_H

#include <stdio.h>

#include "vec.h"

// =============================
//           C字符字符串
// =============================

// 字符串类型（本质是 char 数组）

// @begin String
typedef Vector String;
// @end

/**
 * 创建字符串
 * @return 成功返回 String 指针，失败返回 NULL
 */
String* string_create(void);

/**
 * 添加单个字符到字符串末尾
 * @param s 字符串指针
 * @param c 字符
 * @return 成功返回 0，失败返回 -1
 */
int string_push_char(String *s, char c);

/**
 * 添加 C 字符串到末尾
 * @param s 字符串指针
 * @param cstr C 字符串
 * @return 成功返回 0，失败返回 -1
 */
int string_push(String *s, const char *cstr);

/**
 * 获取 C 风格字符串（以 '\0' 结尾）
 * @param s 字符串指针
 * @return C 字符串指针，失败返回 NULL
 */
const char* string_cstr(String *s);

/**
 * 获取字符串长度
 * @param s 字符串指针
 * @return 字符串长度
 */
int string_length(const String *s);

/**
 * 清空字符串
 * @param s 字符串指针
 */
void string_clear(String *s);

/**
 * 释放字符串内存
 * @param s 字符串指针
 */
void string_free(String *s);

/**
 * 从文件读取一行
 * @param s 字符串指针
 * @param fp 文件指针
 * @param keep_newline 是否保留换行符（1:保留, 0:不保留）
 * @return 成功返回 0，失败或 EOF 返回 -1
 */
int string_readline(String *s, FILE *fp, int keep_newline);

#endif // STDSTR_H