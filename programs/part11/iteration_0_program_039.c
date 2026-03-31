/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
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
static char token_pool[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Force initialization of memory builtins early */
    char buffer[128];
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, "constructor_init", 16);
    
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)(i % 256);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Use all three builtins in destructor */
    char cleanup_buf[256];
    __builtin_memset(cleanup_buf, 0xFF, 256);
    __builtin_memcpy(cleanup_buf, token_pool, 128);
    __builtin_memmove(cleanup_buf + 128, cleanup_buf, 64);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile to control length */
    int len = volatile_len % 128 + 64;
    
    /* Test all three builtins in AST creation */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy from token pool with varying lengths */
    __builtin_memcpy(node->data, token_pool + token_index, len);
    token_index = (token_index + len) % sizeof(token_pool);
    
    node->id = id;
    
    /* Create children with goto for flow control */
    if (depth > 1) {
        goto create_children;
    } else {
        node->left = node->right = NULL;
        goto finish_node;
    }
    
create_children:
    /* Use memmove in goto block */
    char temp[128];
    __builtin_memcpy(temp, node->data, 64);
    __builtin_memmove(node->data + 64, temp, 64);
    
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
finish_node:
    return node;
}

/* Function with complex control flow using goto */
static void process_with_goto(ASTNode* node) {
    if (!node) return;
    
    char buffer[512];
    int use_memmove = 0;
    
    /* Jump into memory operation block */
    if (node->id % 3 == 0) {
        goto use_memmove_block;
    } else if (node->id % 3 == 1) {
        goto use_memcpy_block;
    } else {
        goto use_memset_block;
    }
    
use_memmove_block:
    /* This tests the BUILT_IN_MEMMOVE case */
    __builtin_memcpy(buffer, node->data, 128);
    __builtin_memmove(buffer + 128, buffer, 64);
    use_memmove = 1;
    goto process_children;
    
use_memcpy_block:
    /* Test BUILT_IN_MEMCPY */
    __builtin_memcpy(buffer, node->data, 256);
    goto process_children;
    
use_memset_block:
    /* Test BUILT_IN_MEMSET */
    __builtin_memset(buffer, node->id, 256);
    __builtin_memcpy(buffer, node->data, 128);
    goto process_children;
    
process_children:
    /* Process children with potential jumps back */
    if (node->left && use_memmove) {
        __builtin_memmove(node->left->data, buffer, 64);
    }
    
    process_with_goto(node->left);
    process_with_goto(node->right);
}

/* Parallel processing function */
static void parallel_memory_operations(void) {
    int i;
    char parallel_buffers[8][1024];
    
    #pragma omp parallel for private(i)
    for (i = 0; i < 8; i++) {
        /* Each thread uses all three builtins */
        __builtin_memset(parallel_buffers[i], i, sizeof(parallel_buffers[i]));
        
        /* Copy from token pool with thread-specific offset */
        int offset = (i * 512) % sizeof(token_pool);
        __builtin_memcpy(parallel_buffers[i] + 256, 
                        token_pool + offset, 
                        256);
        
        /* Move data within buffer */
        __builtin_memmove(parallel_buffers[i], 
                         parallel_buffers[i] + 256, 
                         128);
    }
    
    /* Verify parallel operations */
    #pragma omp parallel for reduction(+:token_index)
    for (i = 0; i < 8; i++) {
        for (int j = 0; j < 128; j++) {
            token_index += parallel_buffers[i][j];
        }
    }
}

/* Multi-stage processing */
static unsigned long long process_ast(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long long hash = 0;
    char process_buffer[384];
    
    /* Stage 1: Copy node data */
    __builtin_memcpy(process_buffer, node->data, 256);
    
    /* Stage 2: Clear part of buffer */
    __builtin_memset(process_buffer + 256, 0, 128);
    
    /* Stage 3: Move data around */
    __builtin_memmove(process_buffer + 128, process_buffer, 128);
    
    /* Calculate hash from processed data */
    for (int i = 0; i < 256; i++) {
        hash = (hash * 31 + process_buffer[i]) % 1000000007;
    }
    
    /* Recursively process children */
    hash += process_ast(node->left);
    hash += process_ast(node->right);
    
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Create recursive AST */
    ASTNode* root = create_ast(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Process with goto control flow */
    process_with_goto(root);
    
    /* Phase 3: Parallel memory operations */
    parallel_memory_operations();
    
    /* Phase 4: Multi-stage AST processing */
    unsigned long long result = process_ast(root);
    
    /* Phase 5: Additional builtin calls in main */
    char final_buffer[1024];
    volatile int final_len = volatile_flag ? 512 : 256;
    
    __builtin_memset(final_buffer, 0xAA, final_len);
    __builtin_memcpy(final_buffer + 256, root->data, 256);
    __builtin_memmove(final_buffer, final_buffer + 128, 384);
    
    /* Calculate final verification hash */
    unsigned long long final_hash = 0;
    for (int i = 0; i < 512; i++) {
        final_hash = (final_hash * 17 + final_buffer[i]) % 1000000009;
    }
    
    result = (result + final_hash) % 1000000007;
    
    printf("Test completed. Result hash: %llu\n", result);
    printf("Token index: %d\n", token_index);
    
    /* Cleanup */
    /* Note: In real code, you'd want to free the AST properly */
    
    return 0;
}
