/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    uint32_t checksum;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 128; /* Force non-constant size */
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: ASAN test completed\n");
}

/* Function with goto jumps around memmove */
static void test_goto_memmove(char* dest, const char* src, size_t n) {
    int use_builtin = 1;
    
    if (n == 0) goto skip_copy;
    
    /* Jump into block with memmove */
    if (use_builtin) goto do_memmove;
    
skip_copy:
    return;
    
do_memmove:
    /* Force builtin memmove with goto context */
    __builtin_memmove(dest, src, n);
    
    /* Jump out of block */
    if (n > 100) goto skip_copy;
    
    /* Additional memmove in same function */
    char temp[64];
    __builtin_memmove(temp, dest, n < 64 ? n : 64);
    
    goto skip_copy;
}

/* Recursive function copying between AST nodes */
static void copy_ast_nodes(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use builtin memcpy for node data */
    __builtin_memcpy(dest->data, src->data, sizeof(dest->data));
    
    /* Recursive copies */
    if (src->left && dest->left) {
        copy_ast_nodes(dest->left, src->left);
    }
    if (src->right && dest->right) {
        copy_ast_nodes(dest->right, src->right);
    }
    
    /* Compute checksum with memset */
    dest->checksum = 0;
    __builtin_memset(&dest->checksum, 0xFF, sizeof(dest->checksum));
}

/* Function using all three builtins in different contexts */
static void test_all_builtins(void) {
    char buffer1[256];
    char buffer2[256];
    volatile size_t size = g_mem_size;
    
    /* 1. memset with volatile size */
    __builtin_memset(buffer1, 0xAA, size);
    
    /* 2. memcpy with conditional size */
    size_t copy_size = (size > 128) ? 128 : size;
    __builtin_memcpy(buffer2, buffer1, copy_size);
    
    /* 3. memmove with overlapping regions */
    __builtin_memmove(buffer1 + 32, buffer1, 64);
    
    /* Test goto with memmove */
    test_goto_memmove(buffer1, buffer2, 32);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    const int num_threads = 4;
    char thread_buffers[4][128];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread uses builtins */
        __builtin_memset(thread_buffers[tid], tid, 128);
        
        #pragma omp barrier
        
        /* Circular shift with memcpy */
        int next_tid = (tid + 1) % num_threads;
        __builtin_memcpy(thread_buffers[next_tid], 
                        thread_buffers[tid], 
                        64);
    }
}

/* Create and manipulate AST */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Fill data with pattern */
    for (int i = 0; i < 31; i++) {
        node->data[i] = 'A' + (depth + i) % 26;
    }
    node->data[31] = '\0';
    
    /* Recursive children */
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    return node;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Basic builtin calls */
    test_all_builtins();
    
    /* Phase 2: AST operations */
    ASTNode* ast1 = create_ast(3);
    ASTNode* ast2 = create_ast(3);
    
    if (ast1 && ast2) {
        copy_ast_nodes(ast2, ast1);
        
        /* Verify copy with memcmp */
        if (__builtin_memcmp(ast1->data, ast2->data, 32) == 0) {
            printf("AST copy successful\n");
        }
    }
    
    /* Phase 3: OpenMP parallel section */
    #ifdef _OPENMP
    parallel_memory_ops();
    printf("OpenMP parallel section executed\n");
    #endif
    
    /* Phase 4: Variable-sized operations */
    char* dyn_buf1 = (char*)malloc(g_mem_size);
    char* dyn_buf2 = (char*)malloc(g_mem_size);
    
    if (dyn_buf1 && dyn_buf2) {
        /* Use all three builtins on dynamic memory */
        __builtin_memset(dyn_buf1, 0xCC, g_mem_size);
        __builtin_memcpy(dyn_buf2, dyn_buf1, g_mem_size / 2);
        __builtin_memmove(dyn_buf1 + 16, dyn_buf1, g_mem_size - 16);
        
        free(dyn_buf1);
        free(dyn_buf2);
    }
    
    /* Cleanup */
    /* ... (AST cleanup would go here) ... */
    
    printf("Test completed successfully\n");
    return 0;
}
