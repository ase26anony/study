/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
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

/* Constructor and destructor functions */
__attribute__((constructor)) static void init_asan_test(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (i % 256) ^ 0x55;
    }
}

__attribute__((destructor)) static void cleanup_asan_test(void) {
    /* Clear sensitive data */
    __builtin_memset(token_pool, 0, sizeof(token_pool));
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtin memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy pattern using builtin memcpy */
    int copy_len = volatile_len % 128;
    __builtin_memcpy(node->data, token_pool + (id * 32) % 4096, copy_len);
    
    node->id = id;
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    return node;
}

/* Complex function with goto and memory operations */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int state = 0;
    
    if (!src || !dst) return;
    
    /* Jump table simulation with goto */
    switch (state) {
        case 0: goto copy_block;
        case 1: goto move_block;
        default: goto end;
    }
    
copy_block:
    /* Force memcpy redirection */
    __builtin_memcpy(dst->data, src->data, 
                     volatile_len % sizeof(src->data));
    state = 1;
    if (volatile_flag) goto move_block;
    
move_block:
    /* Force memmove redirection with overlapping regions */
    char temp[256];
    __builtin_memcpy(temp, dst->data, 128);
    __builtin_memmove(dst->data + 64, dst->data, 128);
    __builtin_memcpy(dst->data, temp, 128);
    state = 2;
    
end:
    /* Final memset */
    __builtin_memset(temp, 0, sizeof(temp));
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[512];
        char shared_buf[1024];
        
        /* Initialize with builtin memset */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        /* Copy between buffers using different builtins */
        if (thread_id % 2 == 0) {
            __builtin_memcpy(shared_buf + thread_id * 64, 
                           local_buf, 64);
        } else {
            __builtin_memmove(shared_buf + thread_id * 64,
                            local_buf, 64);
        }
        
        #pragma omp barrier
        
        /* Verify with memset pattern */
        __builtin_memset(local_buf, 0, sizeof(local_buf));
    }
}

/* Multi-stage processing with varied memory operations */
static unsigned long process_tokens(void) {
    unsigned long hash = 0xDEADBEEF;
    char stage1[1024];
    char stage2[1024];
    char stage3[1024];
    
    /* Stage 1: Direct token processing */
    int len = volatile_len % 512;
    __builtin_memcpy(stage1, token_pool, len);
    
    /* Stage 2: Overlapping move */
    __builtin_memmove(stage1 + 256, stage1, 256);
    
    /* Stage 3: Pattern initialization */
    __builtin_memset(stage2, 0xAA, sizeof(stage2));
    
    /* Stage 4: Mixed operations */
    for (int i = 0; i < 4; i++) {
        if (i % 3 == 0) {
            __builtin_memcpy(stage3 + i * 128, 
                           stage1 + i * 64, 128);
        } else if (i % 3 == 1) {
            __builtin_memmove(stage3 + i * 128,
                            stage2 + i * 64, 128);
        } else {
            __builtin_memset(stage3 + i * 128, i, 128);
        }
    }
    
    /* Compute hash */
    for (int i = 0; i < sizeof(stage3); i++) {
        hash = (hash * 31) + stage3[i];
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create AST structures */
    ASTNode* ast1 = create_ast(3, 1);
    ASTNode* ast2 = create_ast(3, 2);
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Test goto-based control flow */
    process_with_goto(ast1, ast2);
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Process tokens with multiple memory builtins */
    unsigned long result_hash = process_tokens();
    
    /* Additional edge cases */
    char overlap_buf[1024];
    
    /* Self-overlapping memmove */
    __builtin_memset(overlap_buf, 0xCC, sizeof(overlap_buf));
    __builtin_memmove(overlap_buf + 256, overlap_buf, 512);
    
    /* Small and large operations */
    __builtin_memcpy(overlap_buf, "test", 5);
    __builtin_memset(overlap_buf + 512, 0xFF, 256);
    
    /* Cleanup */
    free(ast1);
    free(ast2);
    
    printf("Test completed. Hash: 0x%08lX\n", result_hash);
    printf("All memory builtins should have been redirected by ASAN\n");
    
    return 0;
}
