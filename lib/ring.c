#include <stdlib.h>
#include <string.h>

#include "ring.h"

// =============================
//           环形缓冲区
// =============================

// 创建环形缓冲区
// @begin rb_create
RingBuffer* rb_create(int elem_size, int capacity) {
    if (elem_size <= 0) return NULL;
    RingBuffer *rb = (RingBuffer*)malloc(sizeof(RingBuffer));
    rb->elem_size = elem_size;
    rb->capacity = capacity > 0 ? capacity : 8;
    rb->data = (char*)malloc(rb->capacity * elem_size);
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    return rb;
}
// @end

// 扩容
// @begin rb_reserve
static int rb_reserve(RingBuffer *rb, int new_capacity) {
    if (new_capacity <= rb->capacity) return 0;
    char *new_data = (char*)malloc(new_capacity * rb->elem_size);
    if (!new_data) return -1;
    // 重新排列元素到新缓冲区开头
    for (int i = 0; i < rb->count; i++) {
        int src_idx = (rb->head + i) % rb->capacity;
        memcpy(new_data + i * rb->elem_size,
            rb->data + src_idx * rb->elem_size,
            rb->elem_size);
    }
    free(rb->data);
    rb->data = new_data;
    rb->head = 0;
    rb->tail = rb->count;
    rb->capacity = new_capacity;
    return 0;
}
// @end

// ========== 队列操作（FIFO） ==========
// @begin rb_push_back
int rb_push_back(RingBuffer *rb, void *elem) {
    if (rb->count >= rb->capacity) {
        if (rb_reserve(rb, rb->capacity * 2) != 0) return -1;
    }
    memcpy(rb->data + rb->tail * rb->elem_size, elem, rb->elem_size);
    rb->tail = (rb->tail + 1) % rb->capacity;
    rb->count++;
    return 0;
}
// @end

// @begin rb_pop_front
int rb_pop_front(RingBuffer *rb, void *out) {
    if (rb->count == 0) return -1;
    char *src = rb->data + rb->head * rb->elem_size;
    if (out) memcpy(out, src, rb->elem_size);
    rb->head = (rb->head + 1) % rb->capacity;
    rb->count--;
    // 可选缩容策略
    return 0;
}
// @end

// ========== 栈操作（LIFO） ==========
// @begin rb_push_front
int rb_push_front(RingBuffer *rb, void *elem) {
    if (rb->count >= rb->capacity) {
        if (rb_reserve(rb, rb->capacity * 2) != 0) return -1;
    }
    rb->head = (rb->head - 1 + rb->capacity) % rb->capacity;
    char *dest = rb->data + rb->head * rb->elem_size;
    memcpy(dest, elem, rb->elem_size);
    rb->count++;
    return 0;
}
// @end

// @begin rb_pop_back
int rb_pop_back(RingBuffer *rb, void *out) {
    if (rb->count == 0) return -1;
    rb->tail = (rb->tail - 1 + rb->capacity) % rb->capacity;
    char *src = rb->data + rb->tail * rb->elem_size;
    if (out) memcpy(out, src, rb->elem_size);
    rb->count--;
    return 0;
}
// @end

// ========== 查看操作 ==========
// @begin rb_peek_front
void* rb_peek_front(RingBuffer *rb) {
    if (rb->count == 0) return NULL;
    return rb->data + rb->head * rb->elem_size;
}
// @end

// @begin rb_peek_back
void* rb_peek_back(RingBuffer *rb) {
    if (rb->count == 0) return NULL;
    int last = (rb->tail - 1 + rb->capacity) % rb->capacity;
    return rb->data + last * rb->elem_size;
}
// @end

// 随机访问
// @begin rb_get
void* rb_get(RingBuffer *rb, int index) {
    if (index < 0 || index >= rb->count) return NULL;
    int pos = (rb->head + index) % rb->capacity;
    return rb->data + pos * rb->elem_size;
}
// @end

// 遍历回调
// @begin rb_foreach
void rb_foreach(RingBuffer* rb, void (*func)(void*)) {
    for (int i = 0; i < rb_size(rb); i++) {
        func(rb_get(rb, i));
    }
}
// @end

// 释放内存
// @begin rb_free
void rb_free(RingBuffer *rb) {
    if (!rb) return;
    free(rb->data);
    free(rb);
}
// @end

// @begin rb_free_deep
void rb_free_deep(RingBuffer *rb, void (*free_elem)(void*)) {
    if (!rb) return;
    if (free_elem) {
        for (int i = 0; i < rb->count; i++) {
            int idx = (rb->head + i) % rb->capacity;   // 实际存储位置
            free_elem(rb->data + idx * rb->elem_size);
        }
    }
    free(rb->data);
    free(rb);
}
// @end
