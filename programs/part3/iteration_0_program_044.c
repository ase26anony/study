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
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {"memcpy", "memset", "memmove", "test", "data"};
static const int token_count = 5;

/* Constructor/destructor functions */
__attribute__((constructor)) static void init_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
}

__attribute__((destructor)) static void cleanup_globals(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive function with memory operations */
static ASTNode* create_node(int id, const char* src) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->left = node->right = NULL;
    
    /* Force builtin usage with volatile control */
    size_t copy_len = g_mem_size % 64;
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, src, copy_len);
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ASTNode* dest, ASTNode* src) {
    int use_memmove = g_use_memmove;
    
    if (use_memmove) {
        goto use_memmove_block;
    } else {
        goto use_memcpy_block;
    }
    
use_memmove_block:
    /* Jump into block with memmove */
    __builtin_memmove(dest->data, src->data, sizeof(dest->data));
    goto after_operation;
    
use_memcpy_block:
    __builtin_memcpy(dest->data, src->data, sizeof(dest->data));
    goto after_operation;
    
after_operation:
    return;
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count - 1; i++) {
            /* Mix of memory operations */
            if (i % 3 == 0) {
                __builtin_memset(nodes[i]->data, tid, sizeof(nodes[i]->data));
            } else if (i % 3 == 1) {
                __builtin_memcpy(nodes[i]->data, nodes[i+1]->data, 
                                g_mem_size % sizeof(nodes[i]->data));
            } else {
                __builtin_memmove(nodes[i]->data, nodes[i+1]->data,
                                 g_mem_size % sizeof(nodes[i]->data));
            }
        }
    }
}

/* Main execution flow */
int main(void) {
    const int node_count = 8;
    ASTNode* nodes[node_count];
    int result_hash = 0;
    
    printf("Starting ASAN built-in redirection test\n");
    
    /* Create AST nodes with recursive structure */
    for (int i = 0; i < node_count; i++) {
        const char* token = tokens[i % token_count];
        nodes[i] = create_node(i, token);
        
        if (i > 0) {
            nodes[i-1]->left = nodes[i];
        }
    }
    
    /* Test goto flow control */
    if (nodes[0] && nodes[1]) {
        process_with_goto(nodes[0], nodes[1]);
    }
    
    /* Parallel memory operations */
    parallel_memory_ops(nodes, node_count);
    
    /* Additional builtin calls in different contexts */
    char buffer1[256], buffer2[256];
    
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    
    /* Force memmove with overlapping regions */
    __builtin_memmove(buffer1 + 128, buffer1, 128);
    
    /* Calculate verification hash */
    for (int i = 0; i < node_count; i++) {
        for (int j = 0; j < 64; j++) {
            result_hash += nodes[i]->data[j];
        }
        result_hash += nodes[i]->id;
    }
    
    printf("Result hash: %d\n", result_hash);
    
    /* Cleanup */
    for (int i = 0; i < node_count; i++) {
        free(nodes[i]);
    }
    
    return 0;
}
