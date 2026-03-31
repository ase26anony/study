/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor attribute for early initialization */
__attribute__((constructor))
static void init_asan_early(void) {
    printf("Constructor: Initializing ASAN/HWASAN environment\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    printf("Destructor: ASAN/HWASAN cleanup complete\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Fill with pattern using volatile size */
    for (size_t i = 0; i < g_mem_size && i < sizeof(node->data); i++) {
        node->data[i] = (char)((id + i) % 256);
    }
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int use_copy = 1;
    
    if (src == NULL || dst == NULL) {
        goto cleanup;
    }
    
    /* Jump into memory operation block */
    if (use_copy) {
        goto do_copy;
    }
    
    skip_copy:
    printf("Skipped copy operation\n");
    return;
    
    do_copy:
    /* Force memmove redirection with volatile control */
    if (g_use_memmove) {
        __builtin_memmove(dst->data, src->data, g_mem_size);
    } else {
        __builtin_memcpy(dst->data, src->data, g_mem_size);
    }
    
    /* Jump out of block */
    goto skip_copy;
    
    cleanup:
    printf("Invalid nodes\n");
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    #pragma omp parallel for
    for (int i = 0; i < count; i++) {
        if (nodes[i] != NULL) {
            /* Create temporary buffer */
            char temp[256];
            
            /* Use all three builtins in parallel region */
            __builtin_memset(temp, i, g_mem_size);
            __builtin_memcpy(nodes[i]->data, temp, g_mem_size);
            
            /* Conditional memmove */
            if (i > 0 && nodes[i-1] != NULL) {
                __builtin_memmove(nodes[i-1]->data + 32, 
                                 nodes[i]->data, 
                                 g_mem_size / 2);
            }
        }
    }
}

/* Main execution flow */
int main(void) {
    const int NUM_NODES = 8;
    const int AST_DEPTH = 3;
    
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create AST structures */
    ASTNode* nodes[NUM_NODES];
    for (int i = 0; i < NUM_NODES; i++) {
        nodes[i] = create_ast(AST_DEPTH, i + 1);
    }
    
    /* Test goto flow with memmove */
    printf("\nTesting goto flow control:\n");
    for (int i = 0; i < NUM_NODES - 1; i++) {
        process_with_goto(nodes[i], nodes[i + 1]);
    }
    
    /* Execute parallel memory operations */
    printf("\nExecuting parallel memory operations:\n");
    parallel_memory_ops(nodes, NUM_NODES);
    
    /* Verify results with checksum */
    printf("\nVerifying memory operations:\n");
    unsigned long long checksum = 0;
    for (int i = 0; i < NUM_NODES; i++) {
        if (nodes[i] != NULL) {
            for (size_t j = 0; j < g_mem_size && j < sizeof(nodes[i]->data); j++) {
                checksum += (unsigned char)nodes[i]->data[j];
            }
            
            /* Recursive cleanup */
            free(nodes[i]->left);
            free(nodes[i]->right);
            free(nodes[i]);
        }
    }
    
    printf("Final checksum: %llu\n", checksum);
    printf("Test completed successfully\n");
    
    return 0;
}
