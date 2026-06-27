#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>

// 类型ID编码 (二进制位)
// bit0: 0=邻接矩阵, 1=邻接表
// bit1: 0=无权, 1=有权
// bit2: 0=无向, 1=有向
// @begin GraphHandle
#define GRAPH_UNDIR_MATRIX_UNWEIGHTED 0  // 000
#define GRAPH_UNDIR_MATRIX_WEIGHTED   2  // 010
#define GRAPH_UNDIR_LIST_UNWEIGHTED   1  // 001
#define GRAPH_UNDIR_LIST_WEIGHTED     3  // 011
#define GRAPH_DIR_MATRIX_UNWEIGHTED   4  // 100
#define GRAPH_DIR_MATRIX_WEIGHTED     6  // 110
#define GRAPH_DIR_LIST_UNWEIGHTED     5  // 101
#define GRAPH_DIR_LIST_WEIGHTED       7  // 111
// 哨兵值 Sentinel Value, 表示无边或不可达
#define GRAPH_INF 1e9
// 不透明句柄, 分发具体实现
typedef void* GraphHandle;
// @end

/**
 * 用于表示一条边, 主要供最小生成树算法返回结果
 * 字段: u, v 为两个端点, weight 为边权
 */
// @begin Edge
typedef struct { int u, v; double weight; } Edge;
// @end

// ========== 基础操作 ==========
/**
 * 创建图实例
 * @param directed    true=有向图, false=无向图
 * @param weighted    true=带权图, false=无权图 (权重统一为 1.0)
 * @param use_list    true=邻接表, false=邻接矩阵
 * @param vertex_count 顶点个数 (当前版本固定, 不支持动态增删)
 * @return 成功返回 GraphHandle, 失败返回 NULL
 */
GraphHandle graph_create(bool directed, bool weighted, bool use_list, int vertex_count);

/**
 * 销毁图, 释放所有内存
 * @param g 图句柄
 */
void graph_destroy(GraphHandle g);

// 暂不支持动态扩展, 返回-1
int graph_add_vertex(GraphHandle g);
/**
 * 获取图中顶点总数
 * @param g 图句柄
 * @return 顶点个数
 */
int graph_vertex_count(GraphHandle g);

/**
 * 添加一条边
 * @param g 图句柄
 * @param u 起点
 * @param v 终点
 * @param weight 边权 (无权图可传任意值, 内部统一为 1.0)
 * 
 * 行为说明:
 * - 有向图: 仅添加 u -> v
 * - 无向图: 同时添加 u -> v 和 v -> u
 * - 若边已存在，有权图更新权重，无权图不做任何改变
 */
void graph_add_edge(GraphHandle g, int u, int v, double weight);
void graph_remove_edge(GraphHandle g, int u, int v);
bool graph_has_edge(GraphHandle g, int u, int v);
// 无边返回 INF (1e9)
double graph_get_weight(GraphHandle g, int u, int v);

/**
 * 获取顶点 v 的所有邻居
 * @param g 图句柄
 * @param v 顶点
 * @param out_count 输出参数, 返回邻居数量
 * @return 动态分配的 int 数组, 包含所有邻居顶点编号
 *         调用者必须用 free() 释放该数组
 *         若无邻居, 返回 NULL 且 out_count 置 0
 */
int* graph_get_neighbors(GraphHandle g, int v, int *out_count);

// ========== 属性查询 ==========
// 无向:度, 有向:出度
int graph_degree(GraphHandle g, int v);
int graph_indegree(GraphHandle g, int v);
bool graph_is_directed(GraphHandle g);
bool graph_is_weighted(GraphHandle g);
bool graph_is_matrix(GraphHandle g);
void graph_print(GraphHandle g);

// ========== 核心算法 ==========

/**
 * 深度优先搜索遍历
 * @param g 图句柄
 * @param start 起始顶点
 * @param visited 调用者分配的 bool 数组, 长度 >= 顶点数, 用于标记已访问
 * @param visit 回调函数, 每访问一个顶点时调用, 传入顶点编号
 * 
 * 注意: 递归实现
 */
void graph_dfs(GraphHandle g, int start, bool *visited, void (*visit)(int));
/**
 * 广度优先搜索遍历
 * @param g 图句柄
 * @param start 起始顶点
 * @param visited 调用者分配的 bool 数组, 长度 >= 顶点数, 用于标记已访问
 * @param visit 回调函数, 每访问一个顶点时调用, 传入顶点编号
 */
void graph_bfs(GraphHandle g, int start, bool *visited, void (*visit)(int));

// 最短路径

/**
 * 单源最短路径 Dijkstra 算法 O(V²)
 * @param g 图句柄 (要求边权非负)
 * @param src 源点
 * @param dist 输出数组, 长度 >= 顶点数, 返回 src 到各顶点的最短距离
 * @param prev 输出数组, 长度 >= 顶点数, 返回前驱节点 (用于重构路径)
 *             若不需要可传 NULL。
 * 
 * 注意: 无权图请优先使用 BFS 求最短路径, 性能更优
 */
void graph_dijkstra(GraphHandle g, int src, double *dist, int *prev);
/**
 * 所有点对最短路径 Floyd-Warshall 算法 O(V³)
 * @param g 图句柄（仅支持邻接矩阵模式）
 * @param dist 输出矩阵 (调用者分配), 大小 n * n, 返回所有点对最短距离
 *             调用者负责分配和释放
 * 
 * 注意: 仅支持邻接矩阵模式
 */
void graph_floyd(GraphHandle g, double **dist);

/**
 * 最小生成树 Kruskal 算法, 仅限无向图
 * @param g 图句柄 (必须为无向图)
 * @param out_edge_count 输出参数, 返回生成树边数
 * @return 动态分配的 Edge 数组, 包含最小生成树的所有边
 *         调用者必须用 free() 释放
 *         若图不是无向图或无边, 返回 NULL 且 out_edge_count 置 0
 */
Edge* graph_kruskal(GraphHandle g, int *out_edge_count);
/**
 * 最小生成树 Prim 算法, 仅限无向图
 * @param g 图句柄 (必须为无向图)
 * @param start 起始顶点
 * @param mst_edges 输出数组, 调用者分配, 大小至少为 (顶点数 - 1)
 * @param out_count 输出参数, 实际返回的边数
 */
void graph_prim(GraphHandle g, int start, Edge *mst_edges, int *out_count);

/**
 * 拓扑排序 Kahn 算法, 仅限有向无环图
 * @param g 图句柄 (必须为有向图)
 * @param out_count 输出参数, 返回排序顶点数
 * @return 动态分配的 int 数组, 包含拓扑序列
 *         调用者必须用 free() 释放
 *         若图不是 DAG (有环), 返回 NULL 且 out_count 置 0
 */
int* graph_topological_sort(GraphHandle g, int *out_count);

/**
 * 检测图是否存在环
 * @param g 图句柄
 * @return true=有环, false=无环
 * 
 * 说明:
 * - 无向图: 使用并查集 (DFS 检测回边)
 * - 有向图: 使用三色 DFS
 */
bool graph_has_cycle(GraphHandle g);

#endif // GRAPH_H