#include <stdlib.h>

#include "list.h"

// ========== 创建与销毁 ==========

// @begin ll_create
LinkedList* ll_create() {
    LinkedList* list = (LinkedList*)malloc(sizeof(LinkedList));
    list->head = list->tail = NULL;
    list->size = 0;
    return list;
}
// @end

// @begin ll_clear
void ll_clear(LinkedList* list) {
    while (list->head) {
        ListNode* next = list->head->next;
        if (list->head->data) {
            free(list->head->data);  // 释放数据
        }
        free(list->head);  // 释放节点
        list->head = next;
    }
    list->head = list->tail = NULL;
    list->size = 0;
}
// @end

// @begin ll_free
void ll_free(LinkedList* list) {
    ll_clear(list);
    free(list);
}
// @end

// ========== 插入操作 ==========

// @begin ll_push_back
void ll_push_back(LinkedList* list, void* data) {
    ListNode* node = (ListNode*)malloc(sizeof(ListNode));
    node->data = data;
    node->next = NULL;
    node->prev = list->tail;
    if (list->tail) list->tail->next = node;
    else list->head = node;
    list->tail = node;
    list->size++;
}
// @end

// @begin ll_push_front
void ll_push_front(LinkedList* list, void* data) {
    ListNode* node = (ListNode*)malloc(sizeof(ListNode));
    node->data = data;
    node->prev = NULL;
    node->next = list->head;
    if (list->head) list->head->prev = node;
    else list->tail = node;
    list->head = node;
    list->size++;
}
// @end

// @begin ll_insert_after
int ll_insert_after(LinkedList* list, int index, void* data) {
    if (index < 0 || index >= list->size) return -1;  // 索引无效

    ListNode* cur = list->head;
    for (int i = 0; i < index; i++) {
        cur = cur->next;
    }

    ListNode* node = (ListNode*)malloc(sizeof(ListNode));
    node->data = data;
    node->prev = cur;
    node->next = cur->next;

    if (cur->next) {
        cur->next->prev = node;
    } else {
        list->tail = node;  // 插入在尾部
    }
    cur->next = node;
    list->size++;
    return 0;  // 成功
}
// @end

// @begin ll_insert_before
int ll_insert_before(LinkedList* list, int index, void* data) {
    if (index < 0 || index > list->size) return -1;

    if (index == 0) {
        ll_push_front(list, data);  // 头部插入
        return 0;
    }

    ListNode* cur = list->head;
    for (int i = 0; i < index; i++) {
        cur = cur->next;
    }

    ListNode* node = (ListNode*)malloc(sizeof(ListNode));
    node->data = data;
    node->prev = cur->prev;
    node->next = cur;

    cur->prev->next = node;
    cur->prev = node;
    list->size++;
    return 0;
}
// @end

// @begin ll_insert_sorted
void ll_insert_sorted(LinkedList* list, void* data, int (*cmp)(void*, void*)) {
    if (list->size == 0 || cmp(data, list->tail->data) >= 0) {
        // 插入尾部 (最大)
        ll_push_back(list, data);
        return;
    }

    if (cmp(data, list->head->data) <= 0) {
        // 插入头部 (最小)
        ll_push_front(list, data);
        return;
    }

    // 找到插入位置
    ListNode* cur = list->head;
    while (cur && cmp(data, cur->data) > 0) {
        cur = cur->next;
    }

    // 插入到 cur 前面
    ListNode* node = (ListNode*)malloc(sizeof(ListNode));
    node->data = data;
    node->prev = cur->prev;
    node->next = cur;

    cur->prev->next = node;
    cur->prev = node;
    list->size++;
}
// @end

// ========== 删除操作 ==========

// @begin ll_pop_back
void* ll_pop_back(LinkedList* list) {
    if (!list->tail) return NULL;
    ListNode* node = list->tail;
    void* data = node->data;
    list->tail = node->prev;
    if (list->tail) list->tail->next = NULL;
    else list->head = NULL;
    free(node);
    list->size--;
    return data;
}
// @end

// @begin ll_pop_front
void* ll_pop_front(LinkedList* list) {
    if (!list->head) return NULL;
    ListNode* node = list->head;
    void* data = node->data;
    list->head = node->next;
    if (list->head) list->head->prev = NULL;
    else list->tail = NULL;
    free(node);
    list->size--;
    return data;
}
// @end

// @begin ll_remove_at
void* ll_remove_at(LinkedList* list, int index) {
    if (index < 0 || index >= list->size) return NULL;

    if (index == 0) return ll_pop_front(list);
    if (index == list->size - 1) return ll_pop_back(list);

    ListNode* cur = list->head;
    for (int i = 0; i < index; i++) {
        cur = cur->next;
    }

    cur->prev->next = cur->next;
    cur->next->prev = cur->prev;

    void* data = cur->data;
    free(cur);
    list->size--;
    return data;
}
// @end

// @begin ll_remove_first
int ll_remove_first(LinkedList* list, void* target, int (*cmp)(void*, void*)) {
    int index = ll_find(list, target, cmp);
    if (index >= 0) {
        ll_remove_at(list, index);
        return 1;
    }
    return 0;
}
// @end

// ========== 查找修改 ==========

// @begin ll_get
void* ll_get(LinkedList* list, int index) {
    if (index < 0 || index >= list->size) return NULL;

    ListNode* cur = list->head;
    for (int i = 0; i < index; i++) {
        cur = cur->next;
    }
    return cur->data;
}
// @end

// @begin ll_set
void ll_set(LinkedList* list, int index, void* data) {
    if (index < 0 || index >= list->size) return;

    ListNode* cur = list->head;
    for (int i = 0; i < index; i++) {
        cur = cur->next;
    }
    cur->data = data;
}
// @end

// @begin ll_find
int ll_find(LinkedList* list, void* target, int (*cmp)(void*, void*)) {
    ListNode* cur = list->head;
    int index = 0;
    while (cur) {
        if (cmp(cur->data, target) == 0) {
            return index;
        }
        cur = cur->next;
        index++;
    }
    return -1;  // 未找到
}
// @end

// ========== 高级操作 ==========

// 归并排序辅助函数
// @begin ll_merge
ListNode* ll_merge(ListNode* left, ListNode* right, int (*cmp)(void*, void*)) {
    ListNode dummy = {NULL, NULL, NULL};
    ListNode* tail = &dummy;

    while (left && right) {
        if (cmp(left->data, right->data) <= 0) {
            tail->next = left;
            left->prev = tail;
            left = left->next;
        } else {
            tail->next = right;
            right->prev = tail;
            right = right->next;
        }
        tail = tail->next;
    }

    if (left) {
        tail->next = left;
        left->prev = tail;
    }
    if (right) {
        tail->next = right;
        right->prev = tail;
    }

    ListNode* result = dummy.next;
    if (result) result->prev = NULL;
    return result;
}
// @end

// 归并排序辅助函数
// @begin ll_merge_sort
ListNode* ll_merge_sort(ListNode* head, int (*cmp)(void*, void*)) {
    if (!head || !head->next) return head;

    // 找中点 (快慢指针)
    ListNode* slow = head;
    ListNode* fast = head->next;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }

    ListNode* right = slow->next;
    slow->next = NULL;
    if (right) right->prev = NULL;

    // 递归排序
    ListNode* left_sorted = ll_merge_sort(head, cmp);
    ListNode* right_sorted = ll_merge_sort(right, cmp);

    // 合并
    return ll_merge(left_sorted, right_sorted, cmp);
}
// @end

// 归并排序对外接口
// @begin ll_sort
void ll_sort(LinkedList* list, int (*cmp)(void*, void*)) {
    if (list->size <= 1) return;

    ListNode* new_head = ll_merge_sort(list->head, cmp);

    // 重新连接链表并更新tail
    list->head = new_head;
    ListNode* cur = new_head;
    while (cur->next) {
        cur = cur->next;
    }
    list->tail = cur;
}
// @end

// @begin ll_reverse
void ll_reverse(LinkedList* list) {
    if (list->size <= 1) return;

    ListNode* cur = list->head;
    ListNode* temp = NULL;

    // 交换每个节点的prev和next
    while (cur) {
        temp = cur->prev;
        cur->prev = cur->next;
        cur->next = temp;
        // 因为next和prev交换了, 所以向前移动
        cur = cur->prev;
    }

    // 交换head和tail
    temp = (ListNode*)list->head;
    list->head = list->tail;
    list->tail = temp;
}
// @end

// @begin ll_foreach
void ll_foreach(LinkedList* list, void (*func)(void* data)) {
    ListNode* cur = list->head;
    while (cur) {
        func(cur->data);
        cur = cur->next;
    }
}
// @end

// ========== 环形链表操作 ==========

// @begin ll_make_cycle
void ll_make_cycle(LinkedList* list) {
    if (list->size <= 1) return;

    list->tail->next = list->head;
    list->head->prev = list->tail;
}
// @end

// @begin ll_make_cycle_index
int ll_make_cycle_index(LinkedList* list, int index) {
    if (!list->head || index < 0 || index >= list->size) {
        return -1;  // 无效索引
    }

    // 找到目标节点
    ListNode* target = list->head;
    for (int i = 0; i < index; i++) {
        target = target->next;
    }

    // 将尾部连接到目标节点
    list->tail->next = target;
    target->prev = list->tail;

    return 0;
}
// @end

// ========== 迭代器操作 ==========

// @begin ll_iter
LinkedListIter ll_iter(LinkedList* list) {
    LinkedListIter it;
    it.current = list->head;
    it.list = list;
    return it;
}
// @end

// @begin ll_iter_rev
LinkedListIter ll_iter_rev(LinkedList* list) {
    LinkedListIter it;
    it.current = list->tail;
    it.list = list;
    return it;
}
// @end

// @begin ll_has_next
int ll_has_next(LinkedListIter* it) {
    return it->current != NULL;
}
// @end

// @begin ll_has_prev
int ll_has_prev(LinkedListIter* it) {
    return it->current != NULL;
}
// @end

// @begin ll_next
void* ll_next(LinkedListIter* it) {
    if (!it->current) return NULL;
    void* data = it->current->data;
    it->current = it->current->next;
    return data;
}
// @end

// @begin ll_prev
void* ll_prev(LinkedListIter* it) {
    if (!it->current) return NULL;
    void* data = it->current->data;
    it->current = it->current->prev;
    return data;
}
// @end

// @begin ll_iter_reset
void ll_iter_reset(LinkedListIter* it) {
    it->current = it->list->head;
}
// @end

// @begin ll_iter_reset_rev
void ll_iter_reset_rev(LinkedListIter* it) {
    it->current = it->list->tail;
}
// @end

// @begin ll_iter_remove
void* ll_iter_remove(LinkedListIter* it) {
    if (!it->current) return NULL;

    ListNode* to_remove = it->current;
    void* data = to_remove->data;

    if (it->list->size == 1) {
        // 这是唯一节点
        it->list->head = NULL;
        it->list->tail = NULL;
        it->current = NULL;
    } else {
        // 正常情况
        // 更新迭代器到下一个节点
        it->current = to_remove->next;
        if (to_remove->prev) {
            to_remove->prev->next = to_remove->next;
        } else {
            it->list->head = to_remove->next;
        }
        if (to_remove->next) {
            to_remove->next->prev = to_remove->prev;
        } else {
            it->list->tail = to_remove->prev;
        }
    }

    free(to_remove);
    it->list->size--;

    return data;
}
// @end
