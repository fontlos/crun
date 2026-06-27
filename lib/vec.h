#ifndef VECTOR_H
#define VECTOR_H

// @begin Vector
typedef struct {
    char *data;          // 连续内存, 缓存友好
    int elem_size;       // 每个元素占用的字节数
    int size;            // 当前元素个数
    int capacity;        // 当前容量
} Vector;
// @end

// ========== 内联工具函数 ==========

// 只重置数量不释放内存, 直接覆盖
// @begin vec_clear
static inline void vec_clear(Vector *v) {
    if (v) v->size = 0;
}
// @end

/**
 * 判断向量是否为空
 * @param v 向量指针
 * @return 空返回 1（真），非空返回 0（假）
 */
// @begin vec_is_empty
static inline int vec_is_empty(Vector *v) {
    return v->size == 0;
}
// @end

/**
 * 获取元素个数
 * @param v 向量指针
 * @return 元素个数
 */
// @begin vec_size
static inline int vec_size(Vector *v) {
    return v->size;
}
// @end

// ========== 创建与销毁 ==========

/**
 * 创建向量
 * @param elem_size 每个元素占用的字节数（必须 > 0）
 * @param capacity 初始容量（<=0 时使用默认容量 8）
 * @return 成功返回 Vector 指针，失败返回 NULL
 */
Vector* vec_create(int elem_size, int capacity);

/**
 * 窃取向量内部数据（转移所有权）
 * @param v 向量指针
 * @return 返回 data 指针，调用者负责 free，同时向量变为空且 data 置 NULL
 */
void* vec_steal(Vector *v);

/**
 * 释放向量内存（不释放元素指向的动态内存）
 * @param v 向量指针
 */
void vec_free(Vector *v);

/**
 * 深度释放向量内存（释放元素指向的动态内存）
 * @param v 向量指针
 * @param free_elem 元素释放函数，用于释放每个元素指向的动态内存
 */
void vec_free_deep(Vector *v, void (*free_elem)(void*));

// ========== 容量操作 ==========

/**
 * 重整大小（扩容）
 * @param v 向量指针
 * @param new_capacity 新容量
 * @return 成功返回 0，失败返回 -1
 */
int vec_reserve(Vector *v, int new_capacity);

// ========== 栈操作 ==========

/**
 * 查看栈顶元素（返回内部指针）
 * @param v 向量指针
 * @return 栈顶元素指针，空栈返回 NULL
 */
void* vec_peek(Vector *v);

/**
 * 弹出栈顶元素
 * @param v 向量指针
 * @param out 输出缓冲区（可为 NULL，表示不保存弹出值）
 * @return 成功返回 0，空栈返回 -1
 */
int vec_pop(Vector *v, void *out);

/**
 * 压入元素到栈顶（尾部）
 * @param v 向量指针
 * @param elem 要压入的元素指针
 * @return 成功返回 0，失败返回 -1
 */
int vec_push(Vector *v, void *elem);

// ========== 随机访问 ==========

/**
 * 获取指定索引元素的指针
 * @param v 向量指针
 * @param idx 索引（0 <= idx < size）
 * @return 元素指针，索引无效返回 NULL
 */
void* vec_get(Vector *v, int idx);

/**
 * 获取指定索引元素的值（复制到 out）
 * @param v 向量指针
 * @param idx 索引（0 <= idx < size）
 * @param out 输出缓冲区，用于保存元素值
 * @return 成功返回 0，索引无效返回 -1
 */
int vec_get_copy(Vector *v, int idx, void *out);

/**
 * 设置指定索引元素
 * @param v 向量指针
 * @param idx 索引（0 <= idx < size）
 * @param elem 新元素指针
 * @param out 输出缓冲区（可为 NULL），用于保存旧值
 * @return 成功返回 0，失败返回 -1
 */
int vec_set(Vector *v, int idx, void *elem, void *out);

/**
 * 删除指定位置元素
 * @param v 向量指针
 * @param idx 索引（0 <= idx < size）
 * @param out 输出缓冲区（可为 NULL），用于保存被删除的值
 * @return 成功返回 0，失败返回 -1
 */
int vec_remove(Vector *v, int idx, void *out);

/**
 * 插入元素到指定位置
 * @param v 向量指针
 * @param idx 索引（0 <= idx <= size）
 * @param elem 要插入的元素指针
 * @return 成功返回 0，失败返回 -1
 */
int vec_insert(Vector *v, int idx, void *elem);

// ========== 遍历与查找 ==========

/**
 * 遍历向量（对每个元素调用回调函数）
 * @param v 向量指针
 * @param func 回调函数
 */
void vec_foreach(Vector *v, void (*func)(void*));

/**
 * 查找元素
 * @param v 向量指针
 * @param elem 要查找的元素指针
 * @param cmp 比较函数
 * @return 找到返回索引，未找到返回 -1
 */
int vec_find(Vector *v, void *elem, int (*cmp)(void*, void*));

// ========== 交换与反转 ==========

/**
 * 交换两个位置的元素（使用变长数组，C99）
 * @param v 向量指针
 * @param i 索引1
 * @param j 索引2
 */
void vec_swap(Vector *v, int i, int j);

/**
 * 反转数组区间
 * @param v 向量指针
 * @param start 起始索引
 * @param end 结束索引（不包含）
 */
void vec_reverse_range(Vector *v, int start, int end);

/**
 * 反转整个数组
 * @param v 向量指针
 */
void vec_reverse(Vector *v);

/**
 * 对向量进行排序
 * @param v 向量指针
 * @param cmp 比较函数
 */
void vec_sort(Vector *v, int (*cmp)(const void*, const void*));

/**
 * 有序插入 (保持向量有序)
 * @param v 向量指针 (假设已有序)
 * @param elem 要插入的元素指针
 * @param cmp 比较函数
 * @return 成功返回插入位置索引，失败返回 -1
 */
int vec_insert_sorted(Vector *v, void *elem, int (*cmp)(const void*, const void*));

/**
 * 折半查找
 * @param v 向量指针 (假设已有序)
 * @param key 要查找的键
 * @param cmp 比较函数
 * @return 找到返回索引，未找到返回 -1
 */
int vec_binary_search(Vector *v, void *key, int (*cmp)(const void*, const void*));

#endif // VECTOR_H