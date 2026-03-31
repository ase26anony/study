/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char *data;
    size_t len;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_global_data(void) {
    g_init_flag = 1;
    printf("Constructor: Global data initialized\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor)) 
static void cleanup_global_data(void) {
    printf("Destructor: Program cleanup complete\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(const char *token, size_t depth) {
    if (depth == 0) return NULL;
    
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->len = g_mem_size / (depth + 1);
    node->data = (char*)malloc(node->len);
    
    /* Use __builtin_memset to initialize node data */
    if (node->data) {
        __builtin_memset(node->data, 0, node->len);
        __builtin_memcpy(node->data, token, 
                        (node->len < strlen(token)) ? node->len : strlen(token));
    }
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_left = (depth % 2 == 0);
        
        if (use_left) {
            node->left = create_ast("LEFT", depth - 1);
            goto skip_right;
        }
        
        node->right = create_ast("RIGHT", depth - 1);
        skip_right:
        node->left = create_ast("BOTH", depth - 1);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ASTNode *src, ASTNode *dst) {
    if (!src || !dst || !src->data || !dst->data) return;
    
    size_t copy_len = (src->len < dst->len) ? src->len : dst->len;
    
    /* Jump into memory operation */
    if (copy_len > 0) {
        goto do_copy;
    }
    
    return;
    
do_copy:
    /* This tests flow-sensitivity of asan_memfn_rtls retrieval */
    __builtin_memmove(dst->data, src->data, copy_len);
    
    /* Jump out */
    goto finish;
    
finish:
    return;
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_ops(ASTNode **nodes, size_t count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (size_t i = 0; i < count; i++) {
            if (nodes[i] && nodes[i]->data) {
                /* Force built-in usage in parallel context */
                char *temp = (char*)malloc(g_mem_size);
                if (temp) {
                    __builtin_memset(temp, tid, g_mem_size);
                    __builtin_memcpy(nodes[i]->data, temp, 
                                   (nodes[i]->len < g_mem_size) ? nodes[i]->len : g_mem_size);
                    
                    /* Complex goto pattern inside parallel region */
                    if (tid % 2 == 0) {
                        goto parallel_copy;
                    }
                    
                    free(temp);
                    continue;
                    
                parallel_copy:
                    __builtin_memmove(temp, nodes[i]->data, 
                                     (nodes[i]->len < g_mem_size) ? nodes[i]->len : g_mem_size);
                    free(temp);
                }
            }
        }
    }
}

/* Compute hash from AST structure */
static size_t compute_ast_hash(ASTNode *node) {
    if (!node) return 0;
    
    size_t hash = 0;
    if (node->data) {
        for (size_t i = 0; i < node->len && i < 64; i++) {
            hash = hash * 31 + node->data[i];
        }
    }
    
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

/* Free AST memory */
static void free_ast(ASTNode *node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    if (node->data) free(node->data);
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Initialize token array */
    const char *tokens[] = {"TOKEN1", "TOKEN2", "TOKEN3", "TOKEN4"};
    size_t token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Create AST structures */
    ASTNode *ast_nodes[4];
    for (size_t i = 0; i < 4; i++) {
        ast_nodes[i] = create_ast(tokens[i], 3);
    }
    
    /* Perform memory operations with gotos */
    for (size_t i = 0; i < 3; i++) {
        process_with_goto(ast_nodes[i], ast_nodes[i + 1]);
    }
    
    /* Force initialization of asan_memfn_rtls cache with all built-ins */
    char buffer1[256], buffer2[256];
    volatile size_t op_size = g_mem_size;
    
    /* Explicit calls to all three built-ins */
    __builtin_memset(buffer1, 0xAA, op_size);
    __builtin_memcpy(buffer2, buffer1, op_size);
    __builtin_memmove(buffer1, buffer2, op_size);
    
    /* Additional volatile-controlled operations */
    volatile char *volatile_ptr1 = buffer1;
    volatile char *volatile_ptr2 = buffer2;
    __builtin_memcpy((void*)volatile_ptr2, (void*)volatile_ptr1, op_size / 2);
    
    /* Parallel processing */
    parallel_memory_ops(ast_nodes, 4);
    
    /* Compute and print verification result */
    size_t total_hash = 0;
    for (size_t i = 0; i < 4; i++) {
        total_hash += compute_ast_hash(ast_nodes[i]);
    }
    
    printf("Verification hash: %zu\n", total_hash);
    printf("Built-in operations completed\n");
    
    /* Cleanup */
    for (size_t i = 0; i < 4; i++) {
        free_ast(ast_nodes[i]);
    }
    
    return 0;
}
