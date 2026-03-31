/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char *data;
    size_t len;
    struct ASTNode *left;
    struct ASTNode *right;
    unsigned char redzone[32]; /* Simulate ASAN redzone */
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    printf("Constructor: Initializing ASAN environment\n");
    /* Force early initialization of memory builtins */
    char buf1[64], buf2[64];
    __builtin_memset(buf1, 0xAA, sizeof(buf1));
    __builtin_memcpy(buf2, buf1, sizeof(buf1));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Destructor: Cleaning up ASAN environment\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char *base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->len = g_mem_size % 128 + 64;
    node->data = (char*)malloc(node->len);
    
    /* Use builtins with volatile-controlled sizes */
    __builtin_memset(node->data, depth, node->len);
    
    if (base_data) {
        size_t copy_len = node->len < strlen(base_data) ? node->len : strlen(base_data);
        __builtin_memcpy(node->data, base_data, copy_len);
    }
    
    /* Recursive creation with goto for flow control */
    int use_left = depth % 2;
    
    if (use_left) {
        node->left = create_ast(depth - 1, "LeftBranch");
        node->right = NULL;
    } else {
        node->left = NULL;
        node->right = create_ast(depth - 1, "RightBranch");
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ASTNode *src, ASTNode *dst) {
    if (!src || !dst || !src->data || !dst->data) return;
    
    size_t copy_size = src->len < dst->len ? src->len : dst->len;
    
    /* Jump into memory operation */
    goto perform_copy;
    
perform_copy:
    if (g_use_memmove) {
        /* Force memmove path */
        __builtin_memmove(dst->data, src->data, copy_size);
    } else {
        /* Force memcpy path */
        __builtin_memcpy(dst->data, src->data, copy_size);
    }
    
    /* Jump out */
    goto after_copy;
    
after_copy:
    /* Additional memset */
    __builtin_memset(src->data + copy_size/2, 0xFF, copy_size/4);
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(ASTNode **nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i] && nodes[i]->data) {
                /* Each thread uses different builtins */
                switch (tid % 3) {
                    case 0:
                        __builtin_memset(nodes[i]->data, tid, nodes[i]->len / 2);
                        break;
                    case 1:
                        if (i > 0) {
                            __builtin_memcpy(nodes[i]->data, 
                                           nodes[i-1]->data, 
                                           nodes[i]->len < nodes[i-1]->len ? 
                                           nodes[i]->len : nodes[i-1]->len);
                        }
                        break;
                    case 2:
                        if (i > 0 && i < count-1) {
                            __builtin_memmove(nodes[i]->data, 
                                            nodes[i+1]->data, 
                                            nodes[i]->len < nodes[i+1]->len ? 
                                            nodes[i]->len : nodes[i+1]->len);
                        }
                        break;
                }
            }
        }
    }
}

/* Compute hash of AST structure */
static unsigned long compute_ast_hash(ASTNode *node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char *ptr = node->data;
    
    /* Hash the data */
    for (size_t i = 0; i < node->len && i < 256; i++) {
        hash = ((hash << 5) + hash) + ptr[i];
    }
    
    /* Recursive hash computation */
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Create AST structures */
    ASTNode *ast1 = create_ast(4, "RootNode1");
    ASTNode *ast2 = create_ast(3, "RootNode2");
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST structures\n");
        return 1;
    }
    
    /* Test goto flow control with memory operations */
    process_with_goto(ast1, ast2);
    
    /* Toggle memmove flag */
    g_use_memmove = 0;
    process_with_goto(ast2, ast1);
    
    /* Create array for parallel operations */
    ASTNode *node_array[6];
    node_array[0] = ast1;
    node_array[1] = ast2;
    for (int i = 2; i < 6; i++) {
        node_array[i] = create_ast(2, "ParallelNode");
    }
    
    /* Execute parallel memory operations */
    parallel_memory_operations(node_array, 6);
    
    /* Compute and print verification hash */
    unsigned long total_hash = 0;
    for (int i = 0; i < 6; i++) {
        if (node_array[i]) {
            total_hash ^= compute_ast_hash(node_array[i]);
        }
    }
    
    printf("Verification hash: %lu\n", total_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    for (int i = 2; i < 6; i++) {
        if (node_array[i]) {
            free(node_array[i]->data);
            free(node_array[i]);
        }
    }
    free(ast1->data);
    free(ast1);
    free(ast2->data);
    free(ast2);
    
    return 0;
}
