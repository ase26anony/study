/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char global_tokens[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(global_tokens, 'A', sizeof(global_tokens));
    printf("Constructor: Initialized global tokens\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_destructor(void) {
    /* Use builtin memcpy in destructor */
    char temp[64];
    __builtin_memcpy(temp, global_tokens, 64);
    printf("Destructor: Cleaned up %d bytes\n", 64);
}

/* Recursive parser with goto control flow */
static ASTNode* parse_recursive(int depth, char* buffer) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = depth;
    node->left = NULL;
    node->right = NULL;
    
    /* Initialize node data with builtin memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    if (depth > 0) {
        /* Use goto for control flow edge case */
        if (depth % 2 == 0) {
            goto even_depth;
        }
        
        /* Normal path with builtin memcpy */
        char temp[128];
        __builtin_memcpy(temp, buffer, 128);
        __builtin_memcpy(node->data, temp, 128);
        
        even_depth:
        /* Jump target with builtin memmove */
        char* ptr = node->data + 64;
        __builtin_memmove(ptr, buffer, 64);
        
        /* Recursive calls */
        node->left = parse_recursive(depth - 1, buffer + 64);
        if (depth > 1) {
            node->right = parse_recursive(depth - 2, buffer + 128);
        }
    }
    
    return node;
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_ops(ASTNode* nodes[], int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Complex memory operations that can't be optimized away */
            char buffer[512];
            int len = volatile_len;
            
            /* Use all three builtins with volatile control */
            if (volatile_flag) {
                __builtin_memset(buffer, i, len);
                __builtin_memcpy(nodes[i]->data, buffer, len);
            } else {
                __builtin_memmove(nodes[i]->data + 128, nodes[i]->data, len);
            }
            
            /* Additional builtin usage in conditional */
            if (i % 3 == 0) {
                __builtin_memcpy(buffer, global_tokens + i * 64, 64);
            } else if (i % 3 == 1) {
                __builtin_memset(buffer + 32, 0xFF, 32);
            } else {
                __builtin_memmove(buffer, buffer + 16, 48);
            }
        }
    }
}

/* Tree traversal with memory operations */
static int traverse_and_hash(ASTNode* node, int* sum) {
    if (!node) return 0;
    
    int local_sum = 0;
    
    /* Process node data with builtins */
    char temp[256];
    __builtin_memcpy(temp, node->data, sizeof(node->data));
    
    /* Compute simple hash */
    for (int i = 0; i < 256; i++) {
        local_sum += temp[i];
    }
    
    /* Recursive traversal */
    local_sum += traverse_and_hash(node->left, sum);
    local_sum += traverse_and_hash(node->right, sum);
    
    *sum += local_sum;
    return local_sum;
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Initialize and parse */
    char init_buffer[1024];
    __builtin_memset(init_buffer, 'X', sizeof(init_buffer));
    
    ASTNode* root = parse_recursive(4, init_buffer);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Create node array for parallel processing */
    ASTNode* nodes[8];
    nodes[0] = root;
    for (int i = 1; i < 8; i++) {
        nodes[i] = parse_recursive(3, init_buffer + i * 128);
    }
    
    /* Phase 3: Execute parallel memory operations */
    printf("Executing parallel memory operations...\n");
    parallel_memory_ops(nodes, 8);
    
    /* Phase 4: Traverse and compute result */
    printf("Traversing AST structure...\n");
    int total_sum = 0;
    traverse_and_hash(root, &total_sum);
    
    /* Phase 5: Additional builtin usage in main */
    char final_buffer[256];
    __builtin_memcpy(final_buffer, root->data, 256);
    __builtin_memset(final_buffer + 128, 0, 64);
    __builtin_memmove(final_buffer, final_buffer + 32, 224);
    
    /* Compute verification hash */
    int verify_hash = 0;
    for (int i = 0; i < 256; i++) {
        verify_hash += final_buffer[i];
    }
    
    printf("Results:\n");
    printf("  AST Hash Sum: %d\n", total_sum);
    printf("  Final Buffer Hash: %d\n", verify_hash);
    printf("  Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real code, you'd want to properly free all nodes */
    
    return 0;
}
