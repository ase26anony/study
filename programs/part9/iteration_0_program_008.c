/* asan_coverage.c - Comprehensive test for ASAN memory built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    int id;
    uint32_t checksum;
} ASTNode;

/* Global token array */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", 
    "test", "coverage", "builtin", "instrumentation"
};
static const int g_num_tokens = sizeof(g_tokens) / sizeof(g_tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    printf("Initializing ASAN test environment...\n");
    /* Force initialization of memory builtins */
    char buf[32];
    __builtin_memset(buf, 0, sizeof(buf));
    __builtin_memcpy(buf, "init", 5);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Cleaning up ASAN test environment...\n");
    /* Final memory operations */
    volatile char final_buf[16];
    __builtin_memset((void*)final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Copy token data with goto-controlled flow */
    int token_idx = id % g_num_tokens;
    
    /* Use goto to jump into memory operation block */
    if (depth > 2) {
        goto mem_op_block;
    }
    
    normal_path:
    __builtin_memcpy(node->data, g_tokens[token_idx], 
                    strlen(g_tokens[token_idx]) + 1);
    goto after_mem_op;
    
    mem_op_block:
    /* This tests flow-sensitivity of ASAN logic */
    __builtin_memmove(node->data, g_tokens[token_idx],
                     strlen(g_tokens[token_idx]) + 1);
    
    after_mem_op:
    
    /* Recursive creation with varied memory sizes */
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    /* Calculate checksum using memory operations */
    uint32_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint8_t)node->data[i];
    }
    node->checksum = sum;
    
    return node;
}

/* Function with complex control flow and gotos */
static void process_ast_with_gotos(ASTNode* node) {
    if (!node) return;
    
    volatile int mode = node->id % 3;
    
    switch (mode) {
        case 0: {
            /* Jump into memset block */
            goto memset_block;
        }
        case 1: {
            /* Jump into memcpy block */
            goto memcpy_block;
        }
        default: {
            /* Jump into memmove block */
            goto memmove_block;
        }
    }
    
    memset_block: {
        char temp[64];
        __builtin_memset(temp, node->id, sizeof(temp));
        __builtin_memcpy(node->data, temp, sizeof(temp));
        goto after_ops;
    }
    
    memcpy_block: {
        ASTNode dummy;
        __builtin_memset(&dummy, 0, sizeof(dummy));
        __builtin_memcpy(&dummy, node, sizeof(ASTNode));
        /* Jump out to memmove */
        goto memmove_block;
    }
    
    memmove_block: {
        char buffer[128];
        __builtin_memset(buffer, 0, sizeof(buffer));
        __builtin_memmove(buffer + 32, node->data, 64);
        __builtin_memmove(node->data, buffer + 32, 64);
        goto after_ops;
    }
    
    after_ops:
    
    /* Recursive processing */
    process_ast_with_gotos(node->left);
    process_ast_with_gotos(node->right);
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    const int num_threads = 4;
    volatile size_t local_size = g_mem_size;
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        char* buffers[3];
        
        /* Allocate thread-local buffers */
        for (int i = 0; i < 3; i++) {
            buffers[i] = (char*)malloc(local_size);
            if (buffers[i]) {
                /* Force builtin usage in parallel region */
                __builtin_memset(buffers[i], tid + i, local_size);
            }
        }
        
        #pragma omp barrier
        
        /* Perform memory operations between buffers */
        if (buffers[0] && buffers[1]) {
            __builtin_memcpy(buffers[1], buffers[0], local_size / 2);
        }
        
        if (buffers[1] && buffers[2]) {
            __builtin_memmove(buffers[2], buffers[1], local_size / 4);
        }
        
        #pragma omp barrier
        
        /* Verify with memset */
        if (buffers[0]) {
            __builtin_memset(buffers[0], 0, local_size);
        }
        
        /* Cleanup */
        for (int i = 0; i < 3; i++) {
            free(buffers[i]);
        }
    }
}

/* Multi-stage initialization with memory builtins */
static uint64_t execute_memory_test_suite(void) {
    uint64_t total_hash = 0;
    
    /* Stage 1: Direct builtin calls */
    {
        char stage1_buf[512];
        volatile size_t len = 256;
        
        __builtin_memset(stage1_buf, 0xAA, len);
        __builtin_memcpy(stage1_buf + 128, stage1_buf, len / 2);
        __builtin_memmove(stage1_buf, stage1_buf + 64, len / 4);
        
        for (size_t i = 0; i < len; i++) {
            total_hash += (uint8_t)stage1_buf[i];
        }
    }
    
    /* Stage 2: AST operations */
    ASTNode* root = create_ast(4, 1);
    if (root) {
        process_ast_with_gotos(root);
        
        /* Calculate hash from AST */
        ASTNode* stack[32];
        int top = 0;
        stack[top++] = root;
        
        while (top > 0) {
            ASTNode* current = stack[--top];
            total_hash += current->id + current->checksum;
            
            if (current->right) stack[top++] = current->right;
            if (current->left) stack[top++] = current->left;
        }
        
        /* Cleanup AST */
        free(root);
    }
    
    /* Stage 3: Parallel operations */
    parallel_memory_operations();
    
    return total_hash;
}

int main(void) {
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Force initialization of all three builtins early */
    volatile char init_buf[3][64];
    __builtin_memset(init_buf[0], 0, 64);
    __builtin_memcpy(init_buf[1], init_buf[0], 64);
    __builtin_memmove(init_buf[2], init_buf[1], 64);
    
    /* Execute comprehensive test suite */
    uint64_t result = execute_memory_test_suite();
    
    /* Final verification with all three builtins */
    char final_buf[1024];
    volatile size_t final_size = 512;
    
    __builtin_memset(final_buf, 0, final_size);
    __builtin_memcpy(final_buf + 256, g_tokens[0], strlen(g_tokens[0]));
    __builtin_memmove(final_buf, final_buf + 128, 128);
    
    /* Add final buffer to result */
    for (size_t i = 0; i < 256; i++) {
        result += (uint8_t)final_buf[i];
    }
    
    printf("Test completed. Result hash: %llu\n", 
           (unsigned long long)result);
    
    return (result != 0) ? 0 : 1;
}
