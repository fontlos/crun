#include <stdlib.h>
#include <string.h>

#include "bintree.h"
#include "vec.h"
#include "ring.h"

// ========== 创建与销毁 ==========

// @begin bt_create
BinaryTree* bt_create(int (*compare)(void*, void*)) {
    BinaryTree* tree = (BinaryTree*)malloc(sizeof(BinaryTree));
    if (!tree) return NULL;
    tree->root = NULL;
    tree->size = 0;
    tree->compare = compare;
    return tree;
}
// @end

// @begin bt_create_node
BinTreeNode* bt_create_node(void* data) {
    BinTreeNode* node = (BinTreeNode*)malloc(sizeof(BinTreeNode));
    if (!node) return NULL;
    node->data = data;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    return node;
}
// @end

// @begin bt_free_node
void bt_free_node(BinTreeNode* node, void (*free_data)(void*)) {
    if (node == NULL) return;
    bt_free_node(node->left, free_data);
    bt_free_node(node->right, free_data);
    if (free_data) free_data(node->data);
    free(node);
}
// @end

// @begin bt_free
void bt_free(BinaryTree* tree, void (*free_data)(void*)) {
    if (!tree) return;
    bt_free_node(tree->root, free_data);
    free(tree);
}
// @end

// ========== 基本操作 ==========

// @begin bt_insert
void bt_insert(BinaryTree* tree, void* data) {
    if (!tree) return;

    BinTreeNode* new_node = bt_create_node(data);
    if (!new_node) return;

    if (tree->root == NULL) {
        tree->root = new_node;
        tree->size++;
        return;
    }

    BinTreeNode* current = tree->root;
    while (1) {
        int cmp = tree->compare(data, current->data);
        if (cmp < 0) {
            if (current->left == NULL) {
                current->left = new_node;
                new_node->parent = current;
                break;
            }
            current = current->left;
        } else {
            if (current->right == NULL) {
                current->right = new_node;
                new_node->parent = current;
                break;
            }
            current = current->right;
        }
    }
    tree->size++;
}
// @end

// ========== 查找 ==========

// @begin bt_find
BinTreeNode* bt_find(BinaryTree* tree, void* data) {
    if (!tree || !tree->root) return NULL;

    BinTreeNode* current = tree->root;
    while (current) {
        int cmp = tree->compare(data, current->data);
        if (cmp == 0) return current;
        if (cmp < 0) current = current->left;
        else current = current->right;
    }
    return NULL;
}
// @end

// @begin bt_find_min
BinTreeNode* bt_find_min(BinTreeNode* root) {
    if (!root) return NULL;
    while (root->left) root = root->left;
    return root;
}
// @end

// @begin bt_find_max
BinTreeNode* bt_find_max(BinTreeNode* root) {
    if (!root) return NULL;
    while (root->right) root = root->right;
    return root;
}
// @end

// ========== 删除 ==========

// @begin bt_remove
int bt_remove(BinaryTree* tree, void* data, void* out) {
    if (!tree || !tree->root) return -1;

    BinTreeNode* target = bt_find(tree, data);
    if (!target) return -1;

    // 保存要删除的数据
    if (out) memcpy(out, target->data, sizeof(void*));  // 注意：这里需要根据实际数据类型调整

    BinTreeNode* parent = target->parent;
    BinTreeNode* replacement = NULL;

    // 情况1：没有子节点
    if (!target->left && !target->right) {
        replacement = NULL;
    }
    // 情况2：只有右子节点
    else if (!target->left) {
        replacement = target->right;
        replacement->parent = parent;
    }
    // 情况3：只有左子节点
    else if (!target->right) {
        replacement = target->left;
        replacement->parent = parent;
    }
    // 情况4：有两个子节点，找到右子树的最小节点
    else {
        BinTreeNode* successor = bt_find_min(target->right);
        // 交换数据
        void* temp = target->data;
        target->data = successor->data;
        successor->data = temp;

        // 删除后继节点
        BinTreeNode* succ_parent = successor->parent;
        BinTreeNode* succ_child = successor->right;

        if (succ_parent == target) {
            succ_parent->right = succ_child;
        } else {
            succ_parent->left = succ_child;
        }
        if (succ_child) succ_child->parent = succ_parent;

        free(successor);
        tree->size--;
        return 0;
    }

    // 更新父节点的链接
    if (!parent) {
        tree->root = replacement;
    } else if (parent->left == target) {
        parent->left = replacement;
    } else {
        parent->right = replacement;
    }

    free(target);
    tree->size--;
    return 0;
}
// @end

// ========== 高度 ==========

// @begin bt_node_height
int bt_node_height(BinTreeNode* node) {
    if (!node) return -1;
    int left_height = bt_node_height(node->left);
    int right_height = bt_node_height(node->right);
    return (left_height > right_height ? left_height : right_height) + 1;
}
// @end

// ========== 遍历实现 ==========

// @begin bt_pre_order
void bt_pre_order(BinaryTree* tree, void (*visit)(void*)) {
    if (!tree || !tree->root || !visit) return;

    Vector* stack = vec_create(sizeof(BinTreeNode*), 16);
    vec_push(stack, &tree->root);

    while (!vec_is_empty(stack)) {
        BinTreeNode* node;
        vec_pop(stack, &node);  // 修正：添加 out 参数
        visit(node->data);

        if (node->right) vec_push(stack, &node->right);
        if (node->left) vec_push(stack, &node->left);
    }

    vec_free(stack);
}
// @end

// @begin bt_in_order
void bt_in_order(BinaryTree* tree, void (*visit)(void*)) {
    if (!tree || !tree->root || !visit) return;

    Vector* stack = vec_create(sizeof(BinTreeNode*), 16);
    BinTreeNode* curr = tree->root;

    while (curr || !vec_is_empty(stack)) {
        while (curr) {
            vec_push(stack, &curr);
            curr = curr->left;
        }
        vec_pop(stack, &curr);  // 修正：添加 out 参数
        visit(curr->data);
        curr = curr->right;
    }

    vec_free(stack);
}
// @end

// @begin bt_post_order
void bt_post_order(BinaryTree* tree, void (*visit)(void*)) {
    if (!tree || !tree->root || !visit) return;

    Vector* stack1 = vec_create(sizeof(BinTreeNode*), 16);
    Vector* stack2 = vec_create(sizeof(BinTreeNode*), 16);

    vec_push(stack1, &tree->root);

    while (!vec_is_empty(stack1)) {
        BinTreeNode* node;
        vec_pop(stack1, &node);  // 修正：添加 out 参数
        vec_push(stack2, &node);

        if (node->left) vec_push(stack1, &node->left);
        if (node->right) vec_push(stack1, &node->right);
    }

    while (!vec_is_empty(stack2)) {
        BinTreeNode* node;
        vec_pop(stack2, &node);  // 修正：添加 out 参数
        visit(node->data);
    }

    vec_free(stack1);
    vec_free(stack2);
}
// @end

// @begin bt_level_order
void bt_level_order(BinaryTree* tree, void (*visit)(void*)) {
    if (!tree || !tree->root || !visit) return;

    RingBuffer* queue = rb_create(sizeof(BinTreeNode*), 16);
    rb_push_back(queue, &tree->root);

    while (!rb_is_empty(queue)) {
        BinTreeNode* node;
        rb_pop_front(queue, &node);  // 修正：添加 out 参数
        visit(node->data);

        if (node->left) rb_push_back(queue, &node->left);
        if (node->right) rb_push_back(queue, &node->right);
    }

    rb_free(queue);
}
// @end
