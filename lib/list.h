#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

// =============================
//            双向链表
// =============================

// @begin ListNode
typedef struct ListNode {
    void* data;
    struct ListNode* prev;
    struct ListNode* next;
} ListNode;
// @end

// 只接受堆数据
// @begin LinkedList
typedef struct {
    ListNode* head;
    ListNode* tail;
    int size;
} LinkedList;
// @end

// @begin LinkedListIter
typedef struct {
    ListNode* current;
    LinkedList* list;
} LinkedListIter;
// @end

// ========== 内联工具函数 ==========

// 获取链表大小
// @begin ll_size
static inline int ll_size(LinkedList* list) {
    return list->size;
}
// @end

// 判断链表是否为空
// @begin ll_is_empty
static inline int ll_is_empty(LinkedList* list) {
    return list->size == 0;
}
// @end

// 获取头节点
// @begin ll_head
static inline ListNode* ll_head(LinkedList* list) {
    return list->head;
}
// @end

// 获取尾节点
// @begin ll_tail
static inline ListNode* ll_tail(LinkedList* list) {
    return list->tail;
}
// @end

// 获取节点数据
// @begin ll_node_data
static inline void* ll_node_data(ListNode* node) {
    return node->data;
}
// @end

// ========== 创建与销毁 ==========

// 创建链表
LinkedList* ll_create(void);

// 清空链表 (包含堆上数据)
void ll_clear(LinkedList* list);

// 释放链表 (包含堆上数据)
void ll_free(LinkedList* list);

// ========== 插入操作 ==========

// 尾插入
void ll_push_back(LinkedList* list, void* data);

// 头插入
void ll_push_front(LinkedList* list, void* data);

// 在指定索引后插入
int ll_insert_after(LinkedList* list, int index, void* data);

// 在指定索引前插入
int ll_insert_before(LinkedList* list, int index, void* data);

// 顺序插入
void ll_insert_sorted(LinkedList* list, void* data, int (*cmp)(void*, void*));

// ========== 删除操作 ==========

// 尾弹出
void* ll_pop_back(LinkedList* list);

// 头弹出
void* ll_pop_front(LinkedList* list);

// 删除指定索引的节点
void* ll_remove_at(LinkedList* list, int index);

// 删除第一个匹配的元素
int ll_remove_first(LinkedList* list, void* target, int (*cmp)(void*, void*));

// ========== 查找修改 ==========

// 获取指定索引的节点数据
void* ll_get(LinkedList* list, int index);

// 设置指定索引的值
void ll_set(LinkedList* list, int index, void* data);

// 查找第一个匹配的元素
int ll_find(LinkedList* list, void* target, int (*cmp)(void*, void*));

// ========== 高级操作 ==========

// 归并排序
void ll_sort(LinkedList* list, int (*cmp)(void*, void*));

// 链表反转
void ll_reverse(LinkedList* list);

// 遍历回调
/*
void print_int(void* data) {
    int* value = (int*)data;
    printf("%d\n", *value);
}
ll_foreach(list, print_int);
*/
void ll_foreach(LinkedList* list, void (*func)(void* data));

// ========== 环形链表操作 ==========

// 将链表尾部连接到头部 (形成环形链表)
// 注意: 环形链表所有与索引相关的方法无意义, 可能产生未定义行为
void ll_make_cycle(LinkedList* list);

// 将链表尾部连接到指定索引的节点
int ll_make_cycle_index(LinkedList* list, int index);

// ========== 迭代器操作 ==========

// 获取迭代器 (正向)
LinkedListIter ll_iter(LinkedList* list);

// 获取迭代器 (反向)
LinkedListIter ll_iter_rev(LinkedList* list);

// 检查是否有下一个元素 (正向)
int ll_has_next(LinkedListIter* it);

// 检查是否有上一个元素 (反向)
int ll_has_prev(LinkedListIter* it);

// 获取下一个元素并移动迭代器 (正向)
void* ll_next(LinkedListIter* it);

// 获取上一个元素并移动迭代器 (反向)
void* ll_prev(LinkedListIter* it);

// 重置迭代器到头部
void ll_iter_reset(LinkedListIter* it);

// 重置迭代器到尾部
void ll_iter_reset_rev(LinkedListIter* it);

// 移除迭代器当前指向的元素
void* ll_iter_remove(LinkedListIter* it);

#ifdef __cplusplus
}
#endif

#endif // LINKED_LIST_H