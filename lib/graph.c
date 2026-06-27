#include <stdio.h>
#include <stdlib.h>

#include "graph.h"
#include "vec.h"

// =============================
//           底层数据结构
// =============================

// @begin GraphHeader
typedef struct {
    int type;          // 0~7
    int n;             // 顶点数
    bool directed;
    bool weighted;
    bool use_list;     // true=邻接表, false=矩阵
} GraphHeader;
// @end

// 1. 矩阵 + 无权
// @begin GraphMatU
typedef struct {
    GraphHeader header;
    bool **adj;
} GraphMatU;
// @end

// 2. 矩阵 + 有权
// @begin GraphMatW
typedef struct {
    GraphHeader header;
    double **adj;
} GraphMatW;
// @end

// 3. 邻接表 + 无权
// @begin EdgeNode
typedef struct EdgeNode {
    int dest;
    struct EdgeNode *next;
} EdgeNode;
// @end

// @begin GraphListU
typedef struct {
    GraphHeader header;
    EdgeNode **heads;
} GraphListU;
// @end

// 4. 邻接表 + 有权
// @begin WEdgeNode
typedef struct WeightedEdgeNode {
    int dest;
    double weight;
    struct WeightedEdgeNode *next;
} WEdgeNode;
// @end

// @begin GraphListW
typedef struct {
    GraphHeader header;
    WEdgeNode **heads;
} GraphListW;
// @end

// =============================
//           辅助函数
// =============================

// @begin graph_get_header
static GraphHeader* graph_get_header(GraphHandle g) {
    return (GraphHeader*)g;
}
// @end

// @begin graph_encode_type
static int graph_encode_type(bool directed, bool weighted, bool use_list) {
    int type = 0;
    if (use_list) type |= (1 << 0);
    if (weighted)  type |= (1 << 1);
    if (directed) type |= (1 << 2);
    return type;
}
// @end

// =============================
//           基础操作
// =============================

// @begin graph_create
GraphHandle graph_create(bool directed, bool weighted, bool use_list, int vertex_count) {
    if (vertex_count <= 0) return NULL;
    int type = graph_encode_type(directed, weighted, use_list);

    switch(type) {
        case GRAPH_DIR_MATRIX_UNWEIGHTED:
        case GRAPH_UNDIR_MATRIX_UNWEIGHTED: {
            GraphMatU *g = calloc(1, sizeof(GraphMatU));
            g->header.type = type;
            g->header.n = vertex_count;
            g->header.directed = directed;
            g->header.weighted = false;
            g->header.use_list = false;
            g->adj = calloc(vertex_count, sizeof(bool*));
            for(int i=0; i<vertex_count; i++)
                g->adj[i] = calloc(vertex_count, sizeof(bool));
            return g;
        }
        case GRAPH_DIR_MATRIX_WEIGHTED:
        case GRAPH_UNDIR_MATRIX_WEIGHTED: {
            GraphMatW *g = calloc(1, sizeof(GraphMatW));
            g->header.type = type;
            g->header.n = vertex_count;
            g->header.directed = directed;
            g->header.weighted = true;
            g->header.use_list = false;
            g->adj = calloc(vertex_count, sizeof(double*));
            for(int i=0; i<vertex_count; i++) {
                g->adj[i] = malloc(vertex_count * sizeof(double));
                for(int j=0; j<vertex_count; j++)
                    g->adj[i][j] = GRAPH_INF;
            }
            return g;
        }
        case GRAPH_DIR_LIST_UNWEIGHTED:
        case GRAPH_UNDIR_LIST_UNWEIGHTED: {
            GraphListU *g = calloc(1, sizeof(GraphListU));
            g->header.type = type;
            g->header.n = vertex_count;
            g->header.directed = directed;
            g->header.weighted = false;
            g->header.use_list = true;
            g->heads = calloc(vertex_count, sizeof(EdgeNode*));
            return g;
        }
        case GRAPH_DIR_LIST_WEIGHTED:
        case GRAPH_UNDIR_LIST_WEIGHTED: {
            GraphListW *g = calloc(1, sizeof(GraphListW));
            g->header.type = type;
            g->header.n = vertex_count;
            g->header.directed = directed;
            g->header.weighted = true;
            g->header.use_list = true;
            g->heads = calloc(vertex_count, sizeof(WEdgeNode*));
            return g;
        }
        default: return NULL;
    }
}
// @end

// @begin graph_destroy
void graph_destroy(GraphHandle g) {
    if(!g) return;
    GraphHeader *h = graph_get_header(g);
    int n = h->n;
    switch(h->type) {
        case GRAPH_DIR_MATRIX_UNWEIGHTED:
        case GRAPH_UNDIR_MATRIX_UNWEIGHTED: {
            GraphMatU *mg = (GraphMatU*)g;
            for(int i=0; i<n; i++) free(mg->adj[i]);
            free(mg->adj);
            free(mg);
            break;
        }
        case GRAPH_DIR_MATRIX_WEIGHTED:
        case GRAPH_UNDIR_MATRIX_WEIGHTED: {
            GraphMatW *mg = (GraphMatW*)g;
            for(int i=0; i<n; i++) free(mg->adj[i]);
            free(mg->adj);
            free(mg);
            break;
        }
        case GRAPH_DIR_LIST_UNWEIGHTED:
        case GRAPH_UNDIR_LIST_UNWEIGHTED: {
            GraphListU *lg = (GraphListU*)g;
            for(int i=0; i<n; i++) {
                EdgeNode *p = lg->heads[i];
                while(p) { EdgeNode *tmp = p; p = p->next; free(tmp); }
            }
            free(lg->heads);
            free(lg);
            break;
        }
        case GRAPH_DIR_LIST_WEIGHTED:
        case GRAPH_UNDIR_LIST_WEIGHTED: {
            GraphListW *lg = (GraphListW*)g;
            for(int i=0; i<n; i++) {
                WEdgeNode *p = lg->heads[i];
                while(p) { WEdgeNode *tmp = p; p = p->next; free(tmp); }
            }
            free(lg->heads);
            free(lg);
            break;
        }
    }
}
// @end

// @begin graph_add_vertex
int graph_add_vertex(GraphHandle g) {
    (void)g;
    return -1; // 固定顶点数模式不支持动态添加
}
// @end

// @begin graph_vertex_count
int graph_vertex_count(GraphHandle g) {
    return g ? graph_get_header(g)->n : 0;
}
// @end

// @begin graph_add_edge
void graph_add_edge(GraphHandle g, int u, int v, double weight) {
    if(!g || u<0 || v<0) return;
    GraphHeader *h = graph_get_header(g);
    int n = h->n;
    if(u >= n || v >= n) return;
    bool directed = h->directed;

    switch(h->type) {
        case GRAPH_DIR_MATRIX_UNWEIGHTED:
        case GRAPH_UNDIR_MATRIX_UNWEIGHTED: {
            GraphMatU *mg = (GraphMatU*)g;
            mg->adj[u][v] = true;
            if(!directed) mg->adj[v][u] = true;
            break;
        }
        case GRAPH_DIR_MATRIX_WEIGHTED:
        case GRAPH_UNDIR_MATRIX_WEIGHTED: {
            GraphMatW *mg = (GraphMatW*)g;
            mg->adj[u][v] = weight;
            if(!directed) mg->adj[v][u] = weight;
            break;
        }
        case GRAPH_DIR_LIST_UNWEIGHTED:
        case GRAPH_UNDIR_LIST_UNWEIGHTED: {
            GraphListU *lg = (GraphListU*)g;
            // 去重: 若已存在则忽略
            EdgeNode *p = lg->heads[u];
            while(p) { if(p->dest == v) return; p = p->next; }
            EdgeNode *node = malloc(sizeof(EdgeNode));
            node->dest = v; node->next = lg->heads[u]; lg->heads[u] = node;
            if(!directed) {
                p = lg->heads[v];
                while(p) { if(p->dest == u) return; p = p->next; }
                EdgeNode *node2 = malloc(sizeof(EdgeNode));
                node2->dest = u; node2->next = lg->heads[v]; lg->heads[v] = node2;
            }
            break;
        }
        case GRAPH_DIR_LIST_WEIGHTED:
        case GRAPH_UNDIR_LIST_WEIGHTED: {
            GraphListW *lg = (GraphListW*)g;
            WEdgeNode *p = lg->heads[u];
            while(p) { if(p->dest == v) { p->weight = weight; return; } p = p->next; }
            WEdgeNode *node = malloc(sizeof(WEdgeNode));
            node->dest = v; node->weight = weight; node->next = lg->heads[u]; lg->heads[u] = node;
            if(!directed) {
                p = lg->heads[v];
                while(p) { if(p->dest == u) { p->weight = weight; return; } p = p->next; }
                WEdgeNode *node2 = malloc(sizeof(WEdgeNode));
                node2->dest = u; node2->weight = weight; node2->next = lg->heads[v]; lg->heads[v] = node2;
            }
            break;
        }
    }
}
// @end

// @begin graph_remove_edge
void graph_remove_edge(GraphHandle g, int u, int v) {
    if(!g || u<0 || v<0) return;
    GraphHeader *h = graph_get_header(g);
    int n = h->n;
    if(u >= n || v >= n) return;
    bool directed = h->directed;

    switch(h->type) {
        case GRAPH_DIR_MATRIX_UNWEIGHTED:
        case GRAPH_UNDIR_MATRIX_UNWEIGHTED: {
            GraphMatU *mg = (GraphMatU*)g;
            mg->adj[u][v] = false;
            if(!directed) mg->adj[v][u] = false;
            break;
        }
        case GRAPH_DIR_MATRIX_WEIGHTED:
        case GRAPH_UNDIR_MATRIX_WEIGHTED: {
            GraphMatW *mg = (GraphMatW*)g;
            mg->adj[u][v] = GRAPH_INF;
            if(!directed) mg->adj[v][u] = GRAPH_INF;
            break;
        }
        case GRAPH_DIR_LIST_UNWEIGHTED:
        case GRAPH_UNDIR_LIST_UNWEIGHTED: {
            GraphListU *lg = (GraphListU*)g;
            EdgeNode **p = &lg->heads[u];
            while(*p) { if((*p)->dest == v) { EdgeNode *tmp = *p; *p = (*p)->next; free(tmp); break; } p = &(*p)->next; }
            if(!directed) {
                p = &lg->heads[v];
                while(*p) { if((*p)->dest == u) { EdgeNode *tmp = *p; *p = (*p)->next; free(tmp); break; } p = &(*p)->next; }
            }
            break;
        }
        case GRAPH_DIR_LIST_WEIGHTED:
        case GRAPH_UNDIR_LIST_WEIGHTED: {
            GraphListW *lg = (GraphListW*)g;
            WEdgeNode **p = &lg->heads[u];
            while(*p) { if((*p)->dest == v) { WEdgeNode *tmp = *p; *p = (*p)->next; free(tmp); break; } p = &(*p)->next; }
            if(!directed) {
                p = &lg->heads[v];
                while(*p) { if((*p)->dest == u) { WEdgeNode *tmp = *p; *p = (*p)->next; free(tmp); break; } p = &(*p)->next; }
            }
            break;
        }
    }
}
// @end

// @begin graph_has_edge
bool graph_has_edge(GraphHandle g, int u, int v) {
    if(!g || u<0 || v<0) return false;
    GraphHeader *h = graph_get_header(g);
    int n = h->n;
    if(u >= n || v >= n) return false;
    switch(h->type) {
        case GRAPH_DIR_MATRIX_UNWEIGHTED:
        case GRAPH_UNDIR_MATRIX_UNWEIGHTED:
            return ((GraphMatU*)g)->adj[u][v];
        case GRAPH_DIR_MATRIX_WEIGHTED:
        case GRAPH_UNDIR_MATRIX_WEIGHTED:
            return ((GraphMatW*)g)->adj[u][v] < GRAPH_INF;
        case GRAPH_DIR_LIST_UNWEIGHTED:
        case GRAPH_UNDIR_LIST_UNWEIGHTED: {
            EdgeNode *p = ((GraphListU*)g)->heads[u];
            while(p) { if(p->dest == v) return true; p = p->next; }
            return false;
        }
        case GRAPH_DIR_LIST_WEIGHTED:
        case GRAPH_UNDIR_LIST_WEIGHTED: {
            WEdgeNode *p = ((GraphListW*)g)->heads[u];
            while(p) { if(p->dest == v) return true; p = p->next; }
            return false;
        }
        default: return false;
    }
}
// @end

// @begin graph_get_weight
double graph_get_weight(GraphHandle g, int u, int v) {
    if(!g || u<0 || v<0) return GRAPH_INF;
    GraphHeader *h = graph_get_header(g);
    int n = h->n;
    if(u >= n || v >= n) return GRAPH_INF;
    switch(h->type) {
        case GRAPH_DIR_MATRIX_UNWEIGHTED:
        case GRAPH_UNDIR_MATRIX_UNWEIGHTED:
            return ((GraphMatU*)g)->adj[u][v] ? 1.0 : GRAPH_INF;
        case GRAPH_DIR_MATRIX_WEIGHTED:
        case GRAPH_UNDIR_MATRIX_WEIGHTED:
            return ((GraphMatW*)g)->adj[u][v];
        case GRAPH_DIR_LIST_UNWEIGHTED:
        case GRAPH_UNDIR_LIST_UNWEIGHTED: {
            EdgeNode *p = ((GraphListU*)g)->heads[u];
            while(p) { if(p->dest == v) return 1.0; p = p->next; }
            return GRAPH_INF;
        }
        case GRAPH_DIR_LIST_WEIGHTED:
        case GRAPH_UNDIR_LIST_WEIGHTED: {
            WEdgeNode *p = ((GraphListW*)g)->heads[u];
            while(p) { if(p->dest == v) return p->weight; p = p->next; }
            return GRAPH_INF;
        }
        default: return GRAPH_INF;
    }
}
// @end

// @begin graph_get_neighbors
int* graph_get_neighbors(GraphHandle g, int v, int *out_count) {
    if(!g || !out_count || v < 0 || v >= graph_get_header(g)->n) {
        *out_count = 0; 
        return NULL;
    }
    
    GraphHeader *h = graph_get_header(g);
    int n = h->n;
    Vector *vec = vec_create(sizeof(int), 8);
    if (!vec) {
        *out_count = 0;
        return NULL;
    }

    // 收集邻居
    switch(h->type) {
        case GRAPH_DIR_MATRIX_UNWEIGHTED:
        case GRAPH_UNDIR_MATRIX_UNWEIGHTED: {
            bool **adj = ((GraphMatU*)g)->adj;
            for(int i = 0; i < n; i++) {
                if(adj[v][i]) {
                    vec_push(vec, &i);
                }
            }
            break;
        }
        case GRAPH_DIR_MATRIX_WEIGHTED:
        case GRAPH_UNDIR_MATRIX_WEIGHTED: {
            double **adj = ((GraphMatW*)g)->adj;
            for(int i = 0; i < n; i++) {
                if(adj[v][i] < GRAPH_INF) {
                    vec_push(vec, &i);
                }
            }
            break;
        }
        case GRAPH_DIR_LIST_UNWEIGHTED:
        case GRAPH_UNDIR_LIST_UNWEIGHTED: {
            EdgeNode *p = ((GraphListU*)g)->heads[v];
            while(p) {
                vec_push(vec, &p->dest);
                p = p->next;
            }
            break;
        }
        case GRAPH_DIR_LIST_WEIGHTED:
        case GRAPH_UNDIR_LIST_WEIGHTED: {
            WEdgeNode *p = ((GraphListW*)g)->heads[v];
            while(p) {
                vec_push(vec, &p->dest);
                p = p->next;
            }
            break;
        }
        default: {
            vec_free(vec);
            *out_count = 0;
            return NULL;
        }
    }
    
    *out_count = vec_size(vec);
    
    // 空向量处理
    if (*out_count == 0) {
        vec_free(vec);
        return NULL;
    }
    
    // 窃取数据所有权, 调用者负责 free(result)
    int *result = (int*)vec_steal(vec);
    vec_free(vec);  // 释放 Vector 外壳, data 已被置 NULL
    
    // 理论上 vec_steal 不应该失败, vec 有效, 但防御性检查
    if (!result) {
        *out_count = 0;
        return NULL;
    }
    
    return result;
}
// @end

// =============================
//           属性查询
// =============================

// @begin graph_degree
int graph_degree(GraphHandle g, int v) {
    if(!g || v<0 || v>=graph_get_header(g)->n) return -1;
    int cnt;
    int *neighbors = graph_get_neighbors(g, v, &cnt);
    free(neighbors);
    return cnt;
}
// @end

// @begin graph_indegree
int graph_indegree(GraphHandle g, int v) {
    if(!g || v<0 || v>=graph_get_header(g)->n) return -1;
    GraphHeader *h = graph_get_header(g);
    int n = h->n;
    int cnt = 0;
    if(!h->directed) return graph_degree(g, v); // 无向图入度=度
    switch(h->type) {
        case GRAPH_DIR_MATRIX_UNWEIGHTED: {
            bool **adj = ((GraphMatU*)g)->adj;
            for(int i=0; i<n; i++) if(adj[i][v]) cnt++;
            break;
        }
        case GRAPH_DIR_MATRIX_WEIGHTED: {
            double **adj = ((GraphMatW*)g)->adj;
            for(int i=0; i<n; i++) if(adj[i][v] < GRAPH_INF) cnt++;
            break;
        }
        case GRAPH_DIR_LIST_UNWEIGHTED: {
            for(int i=0; i<n; i++) {
                EdgeNode *p = ((GraphListU*)g)->heads[i];
                while(p) { if(p->dest == v) cnt++; p = p->next; }
            }
            break;
        }
        case GRAPH_DIR_LIST_WEIGHTED: {
            for(int i=0; i<n; i++) {
                WEdgeNode *p = ((GraphListW*)g)->heads[i];
                while(p) { if(p->dest == v) cnt++; p = p->next; }
            }
            break;
        }
        default: return -1;
    }
    return cnt;
}
// @end

// @begin graph_is_directed
bool graph_is_directed(GraphHandle g) { return g ? graph_get_header(g)->directed : false; }
// @end

// @begin graph_is_weighted
bool graph_is_weighted(GraphHandle g) { return g ? graph_get_header(g)->weighted : false; }
// @end

// @begin graph_is_matrix
bool graph_is_matrix(GraphHandle g)  { return g ? !graph_get_header(g)->use_list : false; }
// @end

// @begin graph_print
void graph_print(GraphHandle g) {
    if(!g) return;
    GraphHeader *h = graph_get_header(g);
    int n = h->n;
    printf("Graph: %s, %s, %s, vertices=%d\n",
            h->directed ? "directed" : "undirected",
            h->weighted ? "weighted" : "unweighted",
            h->use_list ? "adjacency list" : "adjacency matrix",
            n);
    for(int u=0; u<n; u++) {
        int cnt; int *nei = graph_get_neighbors(g, u, &cnt);
        printf("%d: ", u);
        for(int i=0; i<cnt; i++) {
            double w = graph_get_weight(g, u, nei[i]);
            printf("%d(w=%.1f) ", nei[i], w);
        }
        printf("\n");
        free(nei);
    }
}
// @end

// =============================
//           核心算法
// =============================

// 辅助: 轻量级的获取权重, 依赖一定假设
// @begin graph_get_w
static double graph_get_w(GraphHandle g, int u, int v) {
    GraphHeader *h = graph_get_header(g);
    switch(h->type) {
        case GRAPH_DIR_MATRIX_UNWEIGHTED:
        case GRAPH_UNDIR_MATRIX_UNWEIGHTED: return ((GraphMatU*)g)->adj[u][v] ? 1.0 : GRAPH_INF;
        case GRAPH_DIR_MATRIX_WEIGHTED:
        case GRAPH_UNDIR_MATRIX_WEIGHTED: return ((GraphMatW*)g)->adj[u][v];
        case GRAPH_DIR_LIST_UNWEIGHTED:
        case GRAPH_UNDIR_LIST_UNWEIGHTED: {
            EdgeNode *p = ((GraphListU*)g)->heads[u];
            while(p) { if(p->dest == v) return 1.0; p = p->next; } return GRAPH_INF;
        }
        case GRAPH_DIR_LIST_WEIGHTED:
        case GRAPH_UNDIR_LIST_WEIGHTED: {
            WEdgeNode *p = ((GraphListW*)g)->heads[u];
            while(p) { if(p->dest == v) return p->weight; p = p->next; } return GRAPH_INF;
        }
        default: return GRAPH_INF;
    }
}
// @end

// 辅助函数: DFS 递归实现
// @begin graph_dfs_rec
static void graph_dfs_rec(GraphHandle g, int v, bool *visited, void (*visit)(int)) {
    visited[v] = true;
    if(visit) visit(v);
    GraphHeader *h = graph_get_header(g);
    int n = h->n;
    // 手动展开邻居遍历
    switch(h->type) {
        case GRAPH_DIR_MATRIX_UNWEIGHTED:
        case GRAPH_UNDIR_MATRIX_UNWEIGHTED: {
            bool **adj = ((GraphMatU*)g)->adj;
            for(int i=0; i<n; i++) if(adj[v][i] && !visited[i]) graph_dfs_rec(g, i, visited, visit);
            break;
        }
        case GRAPH_DIR_MATRIX_WEIGHTED:
        case GRAPH_UNDIR_MATRIX_WEIGHTED: {
            double **adj = ((GraphMatW*)g)->adj;
            for(int i=0; i<n; i++) if(adj[v][i] < GRAPH_INF && !visited[i]) graph_dfs_rec(g, i, visited, visit);
            break;
        }
        case GRAPH_DIR_LIST_UNWEIGHTED:
        case GRAPH_UNDIR_LIST_UNWEIGHTED: {
            EdgeNode *p = ((GraphListU*)g)->heads[v];
            while(p) { if(!visited[p->dest]) graph_dfs_rec(g, p->dest, visited, visit); p = p->next; }
            break;
        }
        case GRAPH_DIR_LIST_WEIGHTED:
        case GRAPH_UNDIR_LIST_WEIGHTED: {
            WEdgeNode *p = ((GraphListW*)g)->heads[v];
            while(p) { if(!visited[p->dest]) graph_dfs_rec(g, p->dest, visited, visit); p = p->next; }
            break;
        }
    }
}
// @end

// @begin graph_dfs
void graph_dfs(GraphHandle g, int start, bool *visited, void (*visit)(int)) {
    if(!g || !visited || start<0 || start>=graph_get_header(g)->n) return;
    graph_dfs_rec(g, start, visited, visit);
}
// @end

// @begin graph_bfs
void graph_bfs(GraphHandle g, int start, bool *visited, void (*visit)(int)) {
    if(!g || !visited || start < 0 || start >= graph_get_header(g)->n) return;
    
    GraphHeader *h = graph_get_header(g);
    int n = h->n;
    Vector *queue = vec_create(sizeof(int), n > 0 ? n : 8);  // 防止n=0
    if (!queue) return;
    
    visited[start] = true;
    vec_push(queue, &start);
    
    int front = 0;
    while (front < vec_size(queue)) {
        // 使用 vec_get 零拷贝读取
        int *p_v = (int*)vec_get(queue, front);
        if (!p_v) break;
        int v = *p_v;
        front++;
        
        if (visit) visit(v);
        
        // 遍历邻居 (switch逻辑不变)
        switch(h->type) {
            case GRAPH_DIR_MATRIX_UNWEIGHTED:
            case GRAPH_UNDIR_MATRIX_UNWEIGHTED: {
                bool **adj = ((GraphMatU*)g)->adj;
                for(int i = 0; i < n; i++) {
                    if(adj[v][i] && !visited[i]) {
                        visited[i] = true;
                        vec_push(queue, &i);
                    }
                }
                break;
            }
            case GRAPH_DIR_MATRIX_WEIGHTED:
            case GRAPH_UNDIR_MATRIX_WEIGHTED: {
                double **adj = ((GraphMatW*)g)->adj;
                for(int i = 0; i < n; i++) {
                    if(adj[v][i] < GRAPH_INF && !visited[i]) {
                        visited[i] = true;
                        vec_push(queue, &i);
                    }
                }
                break;
            }
            case GRAPH_DIR_LIST_UNWEIGHTED:
            case GRAPH_UNDIR_LIST_UNWEIGHTED: {
                EdgeNode *p = ((GraphListU*)g)->heads[v];
                while(p) {
                    if(!visited[p->dest]) {
                        visited[p->dest] = true;
                        vec_push(queue, &p->dest);
                    }
                    p = p->next;
                }
                break;
            }
            case GRAPH_DIR_LIST_WEIGHTED:
            case GRAPH_UNDIR_LIST_WEIGHTED: {
                WEdgeNode *p = ((GraphListW*)g)->heads[v];
                while(p) {
                    if(!visited[p->dest]) {
                        visited[p->dest] = true;
                        vec_push(queue, &p->dest);
                    }
                    p = p->next;
                }
                break;
            }
            default: break;
        }
    }
    
    vec_free(queue);
}
// @end

// ----- 最短路径 -----
// Dijkstra (O(V^2))
// @begin graph_dijkstra
void graph_dijkstra(GraphHandle g, int src, double *dist, int *prev) {
    if(!g || !dist || src<0 || src>=graph_get_header(g)->n) return;
    GraphHeader *h = graph_get_header(g);
    int n = h->n;
    bool *visited = calloc(n, sizeof(bool));
    for(int i=0; i<n; i++) { dist[i] = GRAPH_INF; if(prev) prev[i] = -1; }
    dist[src] = 0;

    for(int cnt=0; cnt<n; cnt++) {
        int u = -1; double min_d = GRAPH_INF;
        for(int i=0; i<n; i++) if(!visited[i] && dist[i] < min_d) { min_d = dist[i]; u = i; }
        if(u == -1) break;
        visited[u] = true;
        // 松弛邻居
        switch(h->type) {
            case GRAPH_DIR_MATRIX_UNWEIGHTED:
            case GRAPH_UNDIR_MATRIX_UNWEIGHTED: {
                bool **adj = ((GraphMatU*)g)->adj;
                for(int v=0; v<n; v++) if(adj[u][v] && !visited[v] && dist[u] + 1 < dist[v]) {
                    dist[v] = dist[u] + 1; if(prev) prev[v] = u;
                }
                break;
            }
            case GRAPH_DIR_MATRIX_WEIGHTED:
            case GRAPH_UNDIR_MATRIX_WEIGHTED: {
                double **adj = ((GraphMatW*)g)->adj;
                for(int v=0; v<n; v++) if(adj[u][v] < GRAPH_INF && !visited[v] && dist[u] + adj[u][v] < dist[v]) {
                    dist[v] = dist[u] + adj[u][v]; if(prev) prev[v] = u;
                }
                break;
            }
            case GRAPH_DIR_LIST_UNWEIGHTED:
            case GRAPH_UNDIR_LIST_UNWEIGHTED: {
                EdgeNode *p = ((GraphListU*)g)->heads[u];
                while(p) { int v=p->dest; if(!visited[v] && dist[u] + 1 < dist[v]) { dist[v]=dist[u]+1; if(prev) prev[v]=u; } p=p->next; }
                break;
            }
            case GRAPH_DIR_LIST_WEIGHTED:
            case GRAPH_UNDIR_LIST_WEIGHTED: {
                WEdgeNode *p = ((GraphListW*)g)->heads[u];
                while(p) { int v=p->dest; if(!visited[v] && dist[u] + p->weight < dist[v]) { dist[v]=dist[u]+p->weight; if(prev) prev[v]=u; } p=p->next; }
                break;
            }
        }
    }
    free(visited);
}
// @end

// Floyd-Warshall (仅矩阵模式)
// @begin graph_floyd
void graph_floyd(GraphHandle g, double **dist) {
    if(!g || !dist) return;
    GraphHeader *h = graph_get_header(g);
    if(h->use_list) { printf("Floyd only supports matrix mode.\n"); return; }
    int n = h->n;
    // 初始化 dist 为权重矩阵
    for(int i=0; i<n; i++) for(int j=0; j<n; j++) dist[i][j] = graph_get_w(g, i, j);
    for(int i=0; i<n; i++) dist[i][i] = 0;

    for(int k=0; k<n; k++)
        for(int i=0; i<n; i++)
            for(int j=0; j<n; j++)
                if(dist[i][k] + dist[k][j] < dist[i][j])
                    dist[i][j] = dist[i][k] + dist[k][j];
}
// @end

// ----- Kruskal (需要提取所有边) -----
// @begin RawEdge
typedef struct { int u, v; double w; } RawEdge;
// @end

// @begin cmp_raw_edge
static int cmp_raw_edge(const void *a, const void *b) {
    double diff = ((RawEdge*)a)->w - ((RawEdge*)b)->w;
    return (diff > 0) - (diff < 0);
}
// @end

// 辅助函数: 并查集 DisjointSet 实现

// 查找: log2(N), 递归深度通常不超过 10
// @begin ds_find
static int ds_find(int *parent, int x) { return parent[x]==x ? x : (parent[x]=ds_find(parent, parent[x])); }
// @end

// 合并
// @begin ds_union
static void ds_union(int *parent, int *rank, int a, int b) {
    a=ds_find(parent,a); b=ds_find(parent,b);
    if(a==b) return;
    if(rank[a]<rank[b]) { int t=a; a=b; b=t; }
    parent[b]=a; if(rank[a]==rank[b]) rank[a]++;
}
// @end

// @begin graph_kruskal
Edge* graph_kruskal(GraphHandle g, int *out_edge_count) {
    if(!g || !out_edge_count) return NULL;
    GraphHeader *h = graph_get_header(g);
    if(h->directed) { *out_edge_count = 0; return NULL; }
    
    int n = h->n;
    
    // 顶点数 <= 1 时, 没有边可加入MST
    if (n <= 1) {
        *out_edge_count = 0;
        return NULL;
    }
    
    Vector *edges = vec_create(sizeof(RawEdge), 16);
    if (!edges) { *out_edge_count = 0; return NULL; }

    // 1. 收集所有边
    switch(h->type) {
        case GRAPH_UNDIR_MATRIX_UNWEIGHTED: {
            bool **adj = ((GraphMatU*)g)->adj;
            for(int i = 0; i < n; i++) {
                for(int j = i + 1; j < n; j++) {
                    if(adj[i][j]) {
                        RawEdge e = {i, j, 1.0};
                        vec_push(edges, &e);
                    }
                }
            }
            break;
        }
        case GRAPH_UNDIR_MATRIX_WEIGHTED: {
            double **adj = ((GraphMatW*)g)->adj;
            for(int i = 0; i < n; i++) {
                for(int j = i + 1; j < n; j++) {
                    if(adj[i][j] < GRAPH_INF) {
                        RawEdge e = {i, j, adj[i][j]};
                        vec_push(edges, &e);
                    }
                }
            }
            break;
        }
        case GRAPH_UNDIR_LIST_UNWEIGHTED: {
            GraphListU *lg = (GraphListU*)g;
            for(int i = 0; i < n; i++) {
                EdgeNode *p = lg->heads[i];
                while(p) {
                    if(p->dest > i) {
                        RawEdge e = {i, p->dest, 1.0};
                        vec_push(edges, &e);
                    }
                    p = p->next;
                }
            }
            break;
        }
        case GRAPH_UNDIR_LIST_WEIGHTED: {
            GraphListW *lg = (GraphListW*)g;
            for(int i = 0; i < n; i++) {
                WEdgeNode *p = lg->heads[i];
                while(p) {
                    if(p->dest > i) {
                        RawEdge e = {i, p->dest, p->weight};
                        vec_push(edges, &e);
                    }
                    p = p->next;
                }
            }
            break;
        }
        default: {
            vec_free(edges);
            *out_edge_count = 0;
            return NULL;
        }
    }
    
    int edge_cnt = vec_size(edges);
    if (edge_cnt == 0) {
        vec_free(edges);
        *out_edge_count = 0;
        return NULL;
    }

    // 2. 窃取边数组所有权
    RawEdge *edge_array = (RawEdge*)vec_steal(edges);
    vec_free(edges);  // 释放Vector外壳
    
    if (!edge_array) {
        *out_edge_count = 0;
        return NULL;
    }

    // 3. 排序
    qsort(edge_array, edge_cnt, sizeof(RawEdge), cmp_raw_edge);

    // 4. 分配并查集
    int *parent = malloc(n * sizeof(int));
    int *rank = calloc(n, sizeof(int));
    if (!parent || !rank) {
        free(parent);
        free(rank);
        free(edge_array);
        *out_edge_count = 0;
        return NULL;
    }
    
    for(int i = 0; i < n; i++) parent[i] = i;

    // 5. 分配MST结果
    Edge *mst = malloc((n - 1) * sizeof(Edge));
    if (!mst) {
        free(parent);
        free(rank);
        free(edge_array);
        *out_edge_count = 0;
        return NULL;
    }
    
    int mst_cnt = 0;
    for(int i = 0; i < edge_cnt && mst_cnt < n - 1; i++) {
        int ru = ds_find(parent, edge_array[i].u);
        int rv = ds_find(parent, edge_array[i].v);
        if(ru != rv) {
            parent[ru] = rv;
            mst[mst_cnt++] = (Edge){edge_array[i].u, edge_array[i].v, edge_array[i].w};
        }
    }
    
    free(parent);
    free(rank);
    free(edge_array);  // 释放边数组
    
    *out_edge_count = mst_cnt;
    return mst;
}
// @end

// ----- Prim (仅无向) -----
// @begin graph_prim
void graph_prim(GraphHandle g, int start, Edge *mst_edges, int *out_count) {
    if(!g || !mst_edges || !out_count) return;
    GraphHeader *h = graph_get_header(g);
    if(h->directed) { *out_count = 0; return; }
    int n = h->n;
    double *min_w = malloc(n * sizeof(double));
    int *parent = malloc(n * sizeof(int));
    bool *in_mst = calloc(n, sizeof(bool));
    for(int i=0; i<n; i++) { min_w[i] = GRAPH_INF; parent[i] = -1; }
    min_w[start] = 0;

    for(int cnt=0; cnt<n; cnt++) {
        int u = -1; double min_d = GRAPH_INF;
        for(int i=0; i<n; i++) if(!in_mst[i] && min_w[i] < min_d) { min_d = min_w[i]; u = i; }
        if(u == -1) break;
        in_mst[u] = true;
        // 更新邻居
        switch(h->type) {
            case GRAPH_UNDIR_MATRIX_UNWEIGHTED: {
                bool **adj = ((GraphMatU*)g)->adj;
                for(int v=0; v<n; v++) if(adj[u][v] && !in_mst[v] && 1.0 < min_w[v]) { min_w[v]=1.0; parent[v]=u; }
                break;
            }
            case GRAPH_UNDIR_MATRIX_WEIGHTED: {
                double **adj = ((GraphMatW*)g)->adj;
                for(int v=0; v<n; v++) if(adj[u][v] < GRAPH_INF && !in_mst[v] && adj[u][v] < min_w[v]) { min_w[v]=adj[u][v]; parent[v]=u; }
                break;
            }
            case GRAPH_UNDIR_LIST_UNWEIGHTED: {
                EdgeNode *p = ((GraphListU*)g)->heads[u];
                while(p) { int v=p->dest; if(!in_mst[v] && 1.0 < min_w[v]) { min_w[v]=1.0; parent[v]=u; } p=p->next; }
                break;
            }
            case GRAPH_UNDIR_LIST_WEIGHTED: {
                WEdgeNode *p = ((GraphListW*)g)->heads[u];
                while(p) { int v=p->dest; if(!in_mst[v] && p->weight < min_w[v]) { min_w[v]=p->weight; parent[v]=u; } p=p->next; }
                break;
            }
            default: break;
        }
    }
    int idx = 0;
    for(int i=0; i<n; i++) {
        if(i != start && parent[i] != -1) {
            mst_edges[idx++] = (Edge){parent[i], i, min_w[i]};
        }
    }
    *out_count = idx;
    free(min_w); free(parent); free(in_mst);
}
// @end

// ----- 拓扑排序 (Kahn, 仅DAG) -----
// @begin graph_topological_sort
int* graph_topological_sort(GraphHandle g, int *out_count) {
    if(!g || !out_count) return NULL;
    GraphHeader *h = graph_get_header(g);
    if(!h->directed) { *out_count = 0; return NULL; }
    int n = h->n;
    int *indeg = malloc(n * sizeof(int));
    for(int i=0; i<n; i++) indeg[i] = graph_indegree(g, i);
    int *queue = malloc(n * sizeof(int));
    int front=0, rear=0;
    for(int i=0; i<n; i++) if(indeg[i]==0) queue[rear++] = i;

    int *topo = malloc(n * sizeof(int));
    int idx = 0;
    while(front < rear) {
        int u = queue[front++];
        topo[idx++] = u;
        // 减少邻居入度
        switch(h->type) {
            case GRAPH_DIR_MATRIX_UNWEIGHTED: {
                bool **adj = ((GraphMatU*)g)->adj;
                for(int v=0; v<n; v++) if(adj[u][v] && --indeg[v] == 0) queue[rear++] = v;
                break;
            }
            case GRAPH_DIR_MATRIX_WEIGHTED: {
                double **adj = ((GraphMatW*)g)->adj;
                for(int v=0; v<n; v++) if(adj[u][v] < GRAPH_INF && --indeg[v] == 0) queue[rear++] = v;
                break;
            }
            case GRAPH_DIR_LIST_UNWEIGHTED: {
                EdgeNode *p = ((GraphListU*)g)->heads[u];
                while(p) { int v=p->dest; if(--indeg[v] == 0) queue[rear++] = v; p=p->next; }
                break;
            }
            case GRAPH_DIR_LIST_WEIGHTED: {
                WEdgeNode *p = ((GraphListW*)g)->heads[u];
                while(p) { int v=p->dest; if(--indeg[v] == 0) queue[rear++] = v; p=p->next; }
                break;
            }
            default: break;
        }
    }
    free(indeg); free(queue);
    if(idx != n) { free(topo); *out_count = 0; return NULL; }
    *out_count = n;
    return topo;
}
// @end

// ----- 检测环 (无向用并查集, 有向用DFS三色) -----
// @begin dfs_cycle_dir
static bool dfs_cycle_dir(GraphHandle g, int v, int *color) {
    color[v] = 1; // 访问中
    GraphHeader *h = graph_get_header(g);
    int n = h->n;
    switch(h->type) {
        case GRAPH_DIR_MATRIX_UNWEIGHTED: {
            bool **adj = ((GraphMatU*)g)->adj;
            for(int i=0; i<n; i++) if(adj[v][i]) {
                if(color[i]==1) return true;
                if(color[i]==0 && dfs_cycle_dir(g, i, color)) return true;
            } break;
        }
        case GRAPH_DIR_MATRIX_WEIGHTED: {
            double **adj = ((GraphMatW*)g)->adj;
            for(int i=0; i<n; i++) if(adj[v][i] < GRAPH_INF) {
                if(color[i]==1) return true;
                if(color[i]==0 && dfs_cycle_dir(g, i, color)) return true;
            } break;
        }
        case GRAPH_DIR_LIST_UNWEIGHTED: {
            EdgeNode *p = ((GraphListU*)g)->heads[v];
            while(p) { if(color[p->dest]==1) return true; if(color[p->dest]==0 && dfs_cycle_dir(g, p->dest, color)) return true; p=p->next; } break;
        }
        case GRAPH_DIR_LIST_WEIGHTED: {
            WEdgeNode *p = ((GraphListW*)g)->heads[v];
            while(p) { if(color[p->dest]==1) return true; if(color[p->dest]==0 && dfs_cycle_dir(g, p->dest, color)) return true; p=p->next; } break;
        }
        default: break;
    }
    color[v] = 2;
    return false;
}
// @end

// @begin has_cycle_undirected
static bool has_cycle_undirected(GraphHandle g) {
    GraphHeader *h = graph_get_header(g);
    int n = h->n;
    int *parent = malloc(n * sizeof(int));
    int *rank = calloc(n, sizeof(int));
    for(int i=0; i<n; i++) parent[i] = i;
    bool cycle = false;
    switch(h->type) {
        case GRAPH_UNDIR_MATRIX_UNWEIGHTED: {
            bool **adj = ((GraphMatU*)g)->adj;
            for(int i=0; i<n && !cycle; i++) for(int j=i+1; j<n; j++) if(adj[i][j]) {
                if(ds_find(parent, i) == ds_find(parent, j)) { cycle = true; break; }
                ds_union(parent, rank, i, j);
            } break;
        }
        case GRAPH_UNDIR_MATRIX_WEIGHTED: {
            double **adj = ((GraphMatW*)g)->adj;
            for(int i=0; i<n && !cycle; i++) for(int j=i+1; j<n; j++) if(adj[i][j] < GRAPH_INF) {
                if(ds_find(parent, i) == ds_find(parent, j)) { cycle = true; break; }
                ds_union(parent, rank, i, j);
            } break;
        }
        case GRAPH_UNDIR_LIST_UNWEIGHTED: {
            GraphListU *lg = (GraphListU*)g;
            for(int i=0; i<n && !cycle; i++) {
                EdgeNode *p = lg->heads[i];
                while(p) { if(p->dest > i) {
                    if(ds_find(parent, i) == ds_find(parent, p->dest)) { cycle = true; break; }
                    ds_union(parent, rank, i, p->dest);
                } p=p->next; }
            } break;
        }
        case GRAPH_UNDIR_LIST_WEIGHTED: {
            GraphListW *lg = (GraphListW*)g;
            for(int i=0; i<n && !cycle; i++) {
                WEdgeNode *p = lg->heads[i];
                while(p) { if(p->dest > i) {
                    if(ds_find(parent, i) == ds_find(parent, p->dest)) { cycle = true; break; }
                    ds_union(parent, rank, i, p->dest);
                } p=p->next; }
            } break;
        }
        default: break;
    }
    free(parent); free(rank);
    return cycle;
}
// @end

// @begin graph_has_cycle
bool graph_has_cycle(GraphHandle g) {
    if(!g) return false;
    GraphHeader *h = graph_get_header(g);
    if(h->directed) {
        int n = h->n;
        int *color = calloc(n, sizeof(int));
        bool found = false;
        for(int i=0; i<n; i++) if(color[i]==0 && dfs_cycle_dir(g, i, color)) { found = true; break; }
        free(color);
        return found;
    } else {
        return has_cycle_undirected(g);
    }
}
// @end
