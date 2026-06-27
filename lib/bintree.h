#ifndef BINARY_TREE_H
#define BINARY_TREE_H

#ifdef __cplusplus
extern "C" {
#endif

// =============================
//             二叉树
// =============================

// @begin BinTreeNode
typedef struct BinTreeNode {
    void* data;
    struct BinTreeNode* left;
    struct BinTreeNode* right;
    struct BinTreeNode* parent;
} BinTreeNode;
// @end

// @begin BinaryTree
typedef struct BinaryTree {
    BinTreeNode* root;
    int size;
    int (*compare)(void*, void*);  // 比较函数，用于二叉搜索树
} BinaryTree;
// @end

// ========== 内联工具函数 ==========

// 获取树的大小
// @begin bt_size
static inline int bt_size(BinaryTree* tree) {
    return tree->size;
}
// @end

// 判断树是否为空
// @begin bt_is_empty
static inline int bt_is_empty(BinaryTree* tree) {
    return tree->size == 0;
}
// @end

// 获取根节点
// @begin bt_root
static inline BinTreeNode* bt_root(BinaryTree* tree) {
    return tree->root;
}
// @end

// 获取节点数据
// @begin bt_node_data
static inline void* bt_node_data(BinTreeNode* node) {
    return node->data;
}
// @end

// 获取左子节点
// @begin bt_node_left
static inline BinTreeNode* bt_node_left(BinTreeNode* node) {
    return node->left;
}
// @end

// 获取右子节点
// @begin bt_node_right
static inline BinTreeNode* bt_node_right(BinTreeNode* node) {
    return node->right;
}
// @end

// 获取父节点
// @begin bt_node_parent
static inline BinTreeNode* bt_node_parent(BinTreeNode* node) {
    return node->parent;
}
// @end

// ========== 创建与销毁 ==========

// 创建二叉树
// compare: 比较函数，返回值 <0 表示第一个参数小于第二个，=0 表示相等，>0 表示大于
BinaryTree* bt_create(int (*compare)(void*, void*));

// 创建节点（内部使用，也可外部调用）
BinTreeNode* bt_create_node(void* data);

// 释放整个树
// free_data: 可选，用于释放节点中存储的数据
void bt_free(BinaryTree* tree, void (*free_data)(void*));

// ========== 基本操作 ==========

// 插入节点（作为二叉搜索树插入）
void bt_insert(BinaryTree* tree, void* data);

// ========== 遍历 ==========

// 前序遍历（递归版本，使用栈）
void bt_pre_order(BinaryTree* tree, void (*visit)(void*));

// 中序遍历（递归版本，使用栈）
void bt_in_order(BinaryTree* tree, void (*visit)(void*));

// 后序遍历（递归版本，使用栈）
void bt_post_order(BinaryTree* tree, void (*visit)(void*));

// 层序遍历（使用队列）
void bt_level_order(BinaryTree* tree, void (*visit)(void*));

// ========== 节点操作辅助函数 ==========

// 查找节点（二叉搜索树查找）
// 返回第一个比较结果为 0 的节点
BinTreeNode* bt_find(BinaryTree* tree, void* data);

// 查找最小值节点
BinTreeNode* bt_find_min(BinTreeNode* root);

// 查找最大值节点
BinTreeNode* bt_find_max(BinTreeNode* root);

// 删除节点（二叉搜索树删除）
// out: 可选，用于返回被删除节点的数据
// 返回值: 0 成功，-1 失败
int bt_remove(BinaryTree* tree, void* data, void* out);

// 获取节点高度（递归）
int bt_node_height(BinTreeNode* node);

// 获取树的高度
static inline int bt_height(BinaryTree* tree) {
    return bt_node_height(tree->root);
}

#ifdef __cplusplus
}
#endif

#endif // BINARY_TREE_H