/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int depth;
} ASTNode;

/* Global token array */
static char g_token_array[4096];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize token array with pattern */
    for (int i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)((i * 31) & 0xFF);
    }
    
    /* Force early memory operations */
    volatile char buffer[128];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(&g_token_array[0], buffer, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Final memory verification */
    volatile char verify[64];
    __builtin_memset(verify, 0, sizeof(verify));
    __builtin_memcpy(verify, &g_token_array[3840], 64);
}

/* Recursive AST builder with memory operations */
static ASTNode* build_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->depth = depth;
    
    /* Copy data with builtin memcpy */
    size_t copy_len = (depth < 64) ? (size_t)depth : 64;
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Recursive construction with goto for flow control */
    if (depth > 1) {
        int use_left = (depth % 2 == 0);
        
        if (use_left) {
            goto build_left;
        } else {
            goto build_right;
        }
        
    build_left:
        node->left = build_ast(depth - 1, node->data);
        if (depth > 2) {
            goto build_right;
        } else {
            goto finish;
        }
        
    build_right:
        node->right = build_ast(depth - 2, node->data);
        goto finish;
    }
    
finish:
    return node;
}

/* AST copy function with memmove between nodes */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    volatile size_t copy_size = sizeof(dest->data);
    
    /* Use goto to create complex control flow around memmove */
    int use_memmove = (dest->depth % 3 == 0);
    
    if (use_memmove) {
        goto use_memmove_path;
    } else {
        goto use_memcpy_path;
    }
    
use_memmove_path:
    /* Critical: This should trigger the memmove builtin redirection */
    __builtin_memmove(dest->data, src->data, copy_size);
    goto after_copy;
    
use_memcpy_path:
    __builtin_memcpy(dest->data, src->data, copy_size);
    goto after_copy;
    
after_copy:
    /* Propagate to children */
    if (dest->left && src->left) {
        copy_ast_data(dest->left, src->left);
    }
    if (dest->right && src->right) {
        copy_ast_data(dest->right, src->right);
    }
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_operations(void) {
    volatile char buffers[8][256];
    volatile int results[8] = {0};
    
    #pragma omp parallel for
    for (int i = 0; i < 8; i++) {
        /* Each thread uses different builtins */
        switch (i % 3) {
            case 0:
                __builtin_memset(buffers[i], i, sizeof(buffers[i]));
                break;
            case 1:
                if (i > 0) {
                    __builtin_memcpy(buffers[i], buffers[i-1], 
                                   (size_t)(g_mem_size % 256));
                }
                break;
            case 2:
                __builtin_memmove(buffers[i], &buffers[i][128], 128);
                break;
        }
        
        /* Compute verification hash */
        for (int j = 0; j < 256; j++) {
            results[i] += buffers[i][j];
        }
    }
    
    /* Verify parallel results */
    volatile int total = 0;
    for (int i = 0; i < 8; i++) {
        total += results[i];
    }
    
    /* Store result in global array */
    __builtin_memcpy(&g_token_array[2048], &total, sizeof(total));
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Phase 1: Basic builtin calls */
    volatile char test_buf[512];
    volatile char src_buf[512];
    
    /* Initialize source with pattern */
    for (int i = 0; i < 512; i++) {
        src_buf[i] = (char)(i % 256);
    }
    
    /* Force all three builtins */
    __builtin_memset(test_buf, 0xCC, sizeof(test_buf));
    __builtin_memcpy(&test_buf[128], src_buf, 256);
    __builtin_memmove(&test_buf[0], &test_buf[256], 128);
    
    /* Phase 2: Recursive AST operations */
    ASTNode* ast1 = build_ast(5, "AST_Base_Data_String");
    ASTNode* ast2 = build_ast(4, "Another_Base_String");
    
    if (ast1 && ast2) {
        copy_ast_data(ast2, ast1);
        
        /* Verify copy with builtin memcmp */
        volatile int cmp_result = 0;
        for (int i = 0; i < 64; i++) {
            if (ast1->data[i] != ast2->data[i]) {
                cmp_result = 1;
                break;
            }
        }
        
        /* Store comparison result */
        __builtin_memcpy(&g_token_array[1024], &cmp_result, sizeof(cmp_result));
    }
    
    /* Phase 3: OpenMP parallel section */
    parallel_memory_operations();
    
    /* Phase 4: Complex control flow with gotos */
    volatile int operation_selector = 0;
    
operation_start:
    operation_selector = (operation_selector + 1) % 4;
    
    switch (operation_selector) {
        case 0:
            __builtin_memset(&g_token_array[3072], 0xAA, 128);
            goto operation_continue;
        case 1:
            __builtin_memcpy(&g_token_array[3200], &g_token_array[0], 128);
            goto operation_continue;
        case 2:
            __builtin_memmove(&g_token_array[3328], &g_token_array[128], 128);
            goto operation_continue;
        case 3:
            goto operation_final;
        default:
            goto operation_start;
    }
    
operation_continue:
    if (operation_selector < 3) {
        goto operation_start;
    }
    
operation_final:
    /* Final verification hash */
    unsigned long final_hash = 0;
    for (int i = 0; i < sizeof(g_token_array); i++) {
        final_hash = (final_hash * 31) + g_token_array[i];
    }
    
    printf("Test completed. Final hash: %lu\n", final_hash);
    
    /* Cleanup */
    free(ast1);
    free(ast2);
    
    return (final_hash != 0) ? 0 : 1;
}
