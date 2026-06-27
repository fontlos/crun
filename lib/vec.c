#include <stdlib.h>
#include <string.h>

#include "vec.h"

// ========== 创建与销毁 ==========

// @begin vec_create
Vector* vec_create(int elem_size, int capacity) {
    if (elem_size <= 0) return NULL;
    Vector *v = (Vector*)malloc(sizeof(Vector));
    v->elem_size = elem_size;
    v->size = 0;
    v->capacity = capacity > 0 ? capacity : 8;
    v->data = (char*)malloc(v->capacity * elem_size);
    return v;
}
// @end

// @begin vec_steal
void* vec_steal(Vector *v) {
    if (!v) return NULL;
    void *data = v->data;
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
    // 注意: 不释放 Vector 本身，调用者仍需 vec_free(v)
    return data;
}
// @end

// @begin vec_free
void vec_free(Vector *v) {
    if (!v) return;
    free(v->data);
    free(v);
}
// @end

// @begin vec_free_deep
void vec_free_deep(Vector *v, void (*free_elem)(void*)) {
    if (!v) return;
    if (free_elem) {
        for (int i = 0; i < v->size; i++) {
            free_elem(v->data + i * v->elem_size);
        }
    }
    free(v->data);
    free(v);
}
// @end

// ========== 容量操作 ==========

// @begin vec_reserve
int vec_reserve(Vector *v, int new_capacity) {
    if (new_capacity <= v->capacity) return 0;
    char *new_data = (char*)realloc(v->data, new_capacity * v->elem_size);
    if (!new_data) return -1;
    v->data = new_data;
    v->capacity = new_capacity;
    return 0;
}
// @end

// ========== 栈操作 ==========

// @begin vec_peek
void* vec_peek(Vector *v) {
    if (v->size == 0) return NULL;
    return v->data + (v->size - 1) * v->elem_size;
}
// @end

// @begin vec_pop
int vec_pop(Vector *v, void *out) {
    if (v->size == 0) return -1;
    v->size--;
    if (out) memcpy(out, v->data + v->size * v->elem_size, v->elem_size);
    return 0;
}
// @end

// @begin vec_push
int vec_push(Vector *v, void *elem) {
    if (v->size >= v->capacity) {
        int new_cap = v->capacity * 2;
        if (vec_reserve(v, new_cap) != 0) return -1;
    }
    memcpy(v->data + v->size * v->elem_size, elem, v->elem_size);
    v->size++;
    return 0;
}
// @end

// ========== 随机访问 ==========

// @begin vec_get
void* vec_get(Vector *v, int idx) {
    if (!v || idx < 0 || idx >= v->size) return NULL;
    return v->data + idx * v->elem_size;
}
// @end

// @begin vec_get_copy
int vec_get_copy(Vector *v, int idx, void *out) {
    if (!v || idx < 0 || idx >= v->size) return -1;
    if (out) memcpy(out, v->data + idx * v->elem_size, v->elem_size);
    return 0;
}
// @end

// @begin vec_set
int vec_set(Vector *v, int idx, void *elem, void *out) {
    if (!v || idx < 0 || idx >= v->size) return -1;
    void *target = v->data + idx * v->elem_size;
    if (out) memcpy(out, target, v->elem_size);
    memcpy(target, elem, v->elem_size);
    return 0;
}
// @end

// @begin vec_remove
int vec_remove(Vector *v, int idx, void *out) {
    if (!v || idx < 0 || idx >= v->size) return -1;
    void *removed = v->data + idx * v->elem_size;
    if (out) memcpy(out, removed, v->elem_size);
    memmove(removed, removed + v->elem_size, (v->size - idx - 1) * v->elem_size);
    v->size--;
    return 0;
}
// @end

// @begin vec_insert
int vec_insert(Vector *v, int idx, void *elem) {
    if (idx < 0 || idx > v->size) return -1;
    if (v->size >= v->capacity) {
        int new_cap = v->capacity * 2;
        if (vec_reserve(v, new_cap) != 0) return -1;
    }
    void *dest = v->data + (idx + 1) * v->elem_size;
    void *src  = v->data + idx * v->elem_size;
    memmove(dest, src, (v->size - idx) * v->elem_size);
    memcpy(src, elem, v->elem_size);
    v->size++;
    return 0;
}
// @end

// ========== 遍历与查找 ==========

// @begin vec_foreach
void vec_foreach(Vector *v, void (*func)(void*)) {
    for (int i = 0; i < v->size; i++) {
        func(v->data + i * v->elem_size);
    }
}
// @end

// @begin vec_find
int vec_find(Vector *v, void *elem, int (*cmp)(void*, void*)) {
    for (int i = 0; i < v->size; i++) {
        if (cmp(v->data + i * v->elem_size, elem) == 0) return i;
    }
    return -1;
}
// @end

// ========== 交换与反转 ==========

// @begin vec_swap
void vec_swap(Vector *v, int i, int j) {
    if (i < 0 || i >= v->size || j < 0 || j >= v->size) return;
    char *pi = v->data + i * v->elem_size;
    char *pj = v->data + j * v->elem_size;
    char tmp[v->elem_size];
    memcpy(tmp, pi, v->elem_size);
    memcpy(pi, pj, v->elem_size);
    memcpy(pj, tmp, v->elem_size);
}
// @end

// @begin vec_reverse_range
void vec_reverse_range(Vector *v, int start, int end) {
    if (start < 0) start = 0;
    if (end > v->size) end = v->size;
    if (start >= end) return;
    for (int left = start, right = end - 1; left < right; ++left, --right) {
        vec_swap(v, left, right);
    }
}
// @end

// @begin vec_reverse
void vec_reverse(Vector *v) {
    if (v->size <= 1) return;
    vec_reverse_range(v, 0, v->size);
}
// @end

// ========== 排序与查找 ==========

// @begin vec_sort
void vec_sort(Vector *v, int (*cmp)(const void*, const void*)) {
    if (v->size <= 1) return;
    // 标准库 qsort
    qsort(v->data, v->size, v->elem_size, cmp);
}
// @end

// @begin vec_insert_sorted
int vec_insert_sorted(Vector *v, void *elem, int (*cmp)(const void*, const void*)) {
    if (!v || !elem || !cmp) return -1;
    
    // 使用 bsearch 查找插入位置
    void *found = bsearch(elem, v->data, v->size, v->elem_size, cmp);
    if (found) {
        // 如果元素已存在，插入到相同元素的后面（保持稳定性）
        int pos = (char*)found - v->data;
        pos /= v->elem_size;
        // 跳过所有相等的元素
        while (pos < v->size && cmp(v->data + pos * v->elem_size, elem) == 0) {
            pos++;
        }
        // 插入到该位置
        if (vec_insert(v, pos, elem) == 0) {
            return pos;
        }
        return -1;
    }
    
    // 未找到, 使用二分查找确定插入位置
    int left = 0, right = v->size;
    while (left < right) {
        int mid = left + (right - left) / 2;
        if (cmp(v->data + mid * v->elem_size, elem) < 0) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    
    // 插入到 left 位置
    if (vec_insert(v, left, elem) == 0) {
        return left;
    }
    return -1;
}
// @end

// @begin vec_binary_search
int vec_binary_search(Vector *v, void *key, int (*cmp)(const void*, const void*)) {
    if (!v || !key || !cmp || v->size == 0) return -1;
    
    // 使用标准库 bsearch
    void *found = bsearch(key, v->data, v->size, v->elem_size, cmp);
    if (!found) return -1;
    
    // 返回索引
    int idx = (char*)found - v->data;
    return idx / v->elem_size;
}
// @end