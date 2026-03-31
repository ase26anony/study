/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ast_node {
    struct ast_node* left;
    struct ast_node* right;
    char data[64];
    int value;
} ast_node_t;

/* Global token array */
static char g_token_buffer[4096];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token buffer with pattern */
    for (size_t i = 0; i < sizeof(g_token_buffer); i++) {
        g_token_buffer[i] = (char)((i * 31) & 0xFF);
    }
    printf("Constructor: Initialized token buffer\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive parser with memory operations */
static ast_node_t* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ast_node_t* node = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    
    /* Copy data using __builtin_memcpy with volatile size */
    volatile size_t copy_size = 32;
    if (copy_size > sizeof(node->data)) copy_size = sizeof(node->data);
    __builtin_memcpy(node->data, base_data, copy_size);
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 3) {
        /* Jump to skip left creation */
        goto skip_left;
    }
    
create_left_label:
    node->left = create_ast(depth - 1, node->data);
    
skip_left:
    if (!create_left) {
        /* Jump back if needed */
        create_left = 1;
        goto create_right;
    }
    
    node->right = create_ast(depth - 1, node->data + 16);
    
create_right:
    node->value = depth * 1000 + g_token_index++;
    
    return node;
}

/* Complex memory operation with goto jumps */
static void process_with_goto(ast_node_t* node, char* dest) {
    if (!node) return;
    
    volatile int use_memmove = 1;
    volatile size_t op_size = 48;
    
    /* Jump into memory operation block */
    if (use_memmove) {
        goto do_memmove;
    }
    
    /* This block will be jumped into */
do_memmove:
    {
        char temp[64];
        /* Use __builtin_memmove with overlapping regions */
        __builtin_memcpy(temp, node->data, op_size);
        __builtin_memmove(dest, temp, op_size);
        
        /* Jump out of block */
        if (node->value > 2000) {
            goto after_ops;
        }
    }
    
    /* Alternative path with memcpy */
    __builtin_memcpy(dest + 16, node->data + 16, 16);
    
after_ops:
    /* Process children */
    process_with_goto(node->left, dest + 32);
    process_with_goto(node->right, dest + 64);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    const int num_threads = 4;
    char thread_buffers[4][256];
    int results[4] = {0};
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread uses builtins with different patterns */
        volatile size_t size = 64 + (tid * 16);
        
        /* Initialize buffer */
        __builtin_memset(thread_buffers[tid], tid, size);
        
        /* Copy between buffers with overlap */
        if (tid > 0) {
            __builtin_memcpy(thread_buffers[tid], 
                           thread_buffers[tid-1], 
                           size - 16);
        }
        
        /* Move data within buffer */
        __builtin_memmove(thread_buffers[tid] + 32,
                         thread_buffers[tid],
                         size - 32);
        
        /* Compute checksum */
        for (size_t i = 0; i < size; i++) {
            results[tid] += thread_buffers[tid][i];
        }
    }
    
    /* Verify parallel execution */
    int total = 0;
    for (int i = 0; i < num_threads; i++) {
        total += results[i];
    }
    printf("Parallel ops checksum: %d\n", total);
}

/* Multi-stage interaction function */
static void complex_memory_dispatch(void) {
    char stage1[512];
    char stage2[512];
    char stage3[512];
    
    /* Stage 1: Initialize with memset */
    volatile size_t init_size = 256;
    __builtin_memset(stage1, 0xAA, init_size);
    
    /* Stage 2: Copy with memcpy */
    __builtin_memcpy(stage2, stage1, init_size);
    
    /* Stage 3: Move with overlapping regions */
    __builtin_memmove(stage3, stage2, init_size);
    __builtin_memmove(stage3 + 128, stage3, 128);
    
    /* Mix operations */
    for (int i = 0; i < 4; i++) {
        volatile int offset = i * 64;
        __builtin_memcpy(stage1 + offset, 
                        stage3 + offset, 
                        32);
        __builtin_memset(stage2 + offset, i, 32);
    }
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Create recursive AST */
    ast_node_t* root = create_ast(5, "BaseASTNodeData0123456789ABCDEF");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process with goto flow control */
    char process_buffer[1024];
    process_with_goto(root, process_buffer);
    
    /* Execute parallel operations */
    parallel_memory_ops();
    
    /* Complex dispatch */
    complex_memory_dispatch();
    
    /* Compute final verification hash */
    uint32_t hash = 0;
    for (size_t i = 0; i < sizeof(process_buffer); i++) {
        hash = (hash * 31) + process_buffer[i];
    }
    
    /* Also hash token buffer */
    for (size_t i = 0; i < sizeof(g_token_buffer); i += 64) {
        hash ^= g_token_buffer[i];
    }
    
    printf("Final verification hash: 0x%08X\n", hash);
    printf("Token operations: %d\n", g_token_index);
    
    /* Cleanup */
    /* Note: In real ASAN, this would detect leaks */
    
    return 0;
}
