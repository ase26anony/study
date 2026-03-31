/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function with __attribute__ */
__attribute__((constructor)) 
static void init_asan_test(void) {
    printf("ASAN Test Constructor: Initializing...\n");
}

/* Destructor function */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("ASAN Test Destructor: Cleaning up...\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with volatile size */
    size_t copy_size = (g_mem_size % 64) + 1;
    __builtin_memcpy(node->data, base_data, copy_size);
    node->size = copy_size;
    
    /* Recursive creation with goto for control flow */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    node->left = create_ast(depth - 1, "left_branch");
    node->right = create_ast(depth - 1, "right_branch");
    
done:
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    volatile int flag = 1;
    
    if (flag) {
        goto mem_operation;
    }
    
    /* Normal path */
    printf("Normal path\n");
    return;
    
mem_operation:
    /* Jump into memory operation */
    if (src && dst) {
        size_t op_size = g_mem_size % sizeof(ASTNode);
        __builtin_memmove(dst, src, op_size);
    }
    
    /* Jump out */
    goto cleanup;
    
cleanup:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        volatile size_t local_size = g_mem_size;
        
        if (nodes[i]) {
            /* Create temporary buffer */
            char buffer[128];
            
            /* Use all three built-ins in parallel region */
            __builtin_memset(buffer, 0xAA, sizeof(buffer));
            
            if (i % 3 == 0) {
                __builtin_memcpy(buffer, nodes[i]->data, 
                               nodes[i]->size < sizeof(buffer) ? 
                               nodes[i]->size : sizeof(buffer));
            } else if (i % 3 == 1) {
                __builtin_memmove(nodes[i]->data, buffer, 
                                nodes[i]->size < sizeof(buffer) ? 
                                nodes[i]->size : sizeof(buffer));
            }
            
            /* Force memory barrier */
            #pragma omp barrier
        }
    }
}

/* Multi-stage initialization */
static void initialize_token_array(char** tokens, int token_count) {
    const char* base_tokens[] = {"memcpy", "memset", "memmove", "asan", "hwasan"};
    volatile int idx = 0;
    
    for (int i = 0; i < token_count; i++) {
        tokens[i] = (char*)malloc(32);
        if (tokens[i]) {
            /* Mix of memory operations */
            __builtin_memset(tokens[i], 0, 32);
            
            idx = i % 5;
            size_t len = strlen(base_tokens[idx]) + 1;
            __builtin_memcpy(tokens[i], base_tokens[idx], len);
            
            /* Occasionally use memmove */
            if (i % 7 == 0) {
                char temp[32];
                __builtin_memmove(temp, tokens[i], len);
                __builtin_memmove(tokens[i], temp, len);
            }
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize complex data structures */
    const int NODE_COUNT = 8;
    ASTNode* nodes[NODE_COUNT];
    
    for (int i = 0; i < NODE_COUNT; i++) {
        char base_data[32];
        snprintf(base_data, sizeof(base_data), "node_data_%d", i);
        nodes[i] = create_ast(3, base_data);
    }
    
    /* Phase 2: Test goto control flow with memory operations */
    for (int i = 0; i < NODE_COUNT - 1; i++) {
        process_with_goto(nodes[i], nodes[i + 1]);
    }
    
    /* Phase 3: OpenMP parallel operations */
    parallel_memory_ops(nodes, NODE_COUNT);
    
    /* Phase 4: Token array processing */
    const int TOKEN_COUNT = 16;
    char* tokens[TOKEN_COUNT];
    initialize_token_array(tokens, TOKEN_COUNT);
    
    /* Phase 5: Compute verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < NODE_COUNT; i++) {
        if (nodes[i]) {
            for (size_t j = 0; j < nodes[i]->size && j < sizeof(nodes[i]->data); j++) {
                hash = (hash * 31) + nodes[i]->data[j];
            }
            
            /* Additional memory operation for coverage */
            char temp[64];
            __builtin_memcpy(temp, nodes[i]->data, nodes[i]->size);
            __builtin_memset(nodes[i]->data + nodes[i]->size / 2, 0, 8);
            __builtin_memmove(nodes[i]->data, temp, nodes[i]->size);
            
            free(nodes[i]);
        }
    }
    
    /* Cleanup tokens */
    for (int i = 0; i < TOKEN_COUNT; i++) {
        if (tokens[i]) {
            free(tokens[i]);
        }
    }
    
    printf("Test completed. Final hash: %lu\n", hash);
    printf("Expected: ASAN should have redirected all built-in memory functions\n");
    
    return (hash != 0) ? 0 : 1;
}
