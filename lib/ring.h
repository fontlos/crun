#ifndef RING_BUFFER_H
#define RING_BUFFER_H

// =============================
//           环形缓冲区
// =============================

// @begin RingBuffer
typedef struct {
    char *data;          // 连续内存缓冲区
    int elem_size;       // 每个元素占用的字节数
    int head;            // 队首索引（读取位置）
    int tail;            // 队尾索引（下一个写入位置）
    int count;           // 当前元素个数
    int capacity;        // 总容量（最多容纳的元素个数）
} RingBuffer;
// @end

// ========== 内联工具函数 ==========

// @begin rb_is_empty
static inline int rb_is_empty(RingBuffer* rb) {
    return rb->count == 0;
}
// @end

// @begin rb_size
static inline int rb_size(RingBuffer* rb) {
    return rb->count;
}
// @end

// @begin rb_capacity
static inline int rb_capacity(RingBuffer* rb) {
    return rb->capacity;
}
// @end

// 创建环形缓冲区
RingBuffer* rb_create(int elem_size, int capacity);

// 扩容
static int rb_reserve(RingBuffer *rb, int new_capacity);

// ========== 队列操作（FIFO） ==========
int rb_push_back(RingBuffer *rb, void *elem);
int rb_pop_front(RingBuffer *rb, void *out);

// ========== 栈操作（LIFO） ==========
int rb_push_front(RingBuffer *rb, void *elem);
int rb_pop_back(RingBuffer *rb, void *out);

// ========== 查看操作 ==========
void* rb_peek_front(RingBuffer *rb);
void* rb_peek_back(RingBuffer *rb);

// 随机访问
void* rb_get(RingBuffer *rb, int index);

// 遍历回调
void rb_foreach(RingBuffer* rb, void (*func)(void*));

// 释放内存
void rb_free(RingBuffer *rb);
void rb_free_deep(RingBuffer *rb, void (*free_elem)(void*));

#endif // RING_BUFFER_H