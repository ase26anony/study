/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
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
    int id;
} ASTNode;

/* Global token array */
static char g_token_pool[4096];
static volatile int g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i * 7) & 0xFF);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive AST builder with memory operations */
static ASTNode* build_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Fill with pattern using volatile-controlled size */
    size_t fill_size = g_mem_size % sizeof(node->data);
    if (fill_size > 0) {
        __builtin_memset(node->data, node->id, fill_size);
    }
    
    /* Build children recursively */
    node->left = build_ast(depth - 1, counter);
    node->right = build_ast(depth - 1, counter);
    
    return node;
}

/* AST copy function with goto flow control */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    int use_memmove = 0;
    
    /* Goto-based flow control to test edge cases */
    if (dest->id % 3 == 0) {
        goto use_memcpy;
    } else if (dest->id % 3 == 1) {
        goto use_memmove;
    } else {
        goto use_memset;
    }
    
use_memcpy:
    /* Force __builtin_memcpy call */
    __builtin_memcpy(dest->data, src->data, sizeof(dest->data));
    goto done;
    
use_memmove:
    /* Force __builtin_memmove call with overlapping regions */
    use_memmove = 1;
    if (dest == src) {
        /* Self-copy edge case */
        __builtin_memmove(dest->data, dest->data + 16, 32);
    } else {
        __builtin_memmove(dest->data, src->data, sizeof(dest->data));
    }
    goto done;
    
use_memset:
    /* Force __builtin_memset call */
    __builtin_memset(dest->data, dest->id, sizeof(dest->data));
    /* Jump back for additional processing */
    if (use_memmove) {
        goto use_memmove;
    }
    
done:
    return;
}

/* Parallel memory operation dispatcher */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf1[128];
        char local_buf2[128];
        
        /* Initialize with __builtin_memset */
        __builtin_memset(local_buf1, thread_id, sizeof(local_buf1));
        
        /* Copy from global pool with volatile offset */
        volatile int offset = g_token_idx + thread_id * 64;
        __builtin_memcpy(local_buf2, 
                        g_token_pool + (offset % (sizeof(g_token_pool) - 128)),
                        128);
        
        /* Move data around */
        __builtin_memmove(local_buf1 + 32, local_buf2 + 16, 64);
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Verify with another memset */
        __builtin_memset(local_buf2, 0xFF, 32);
    }
}

/* Complex memory pattern generator */
static unsigned long generate_memory_hash(void) {
    unsigned long hash = 0;
    char buffer[512];
    volatile int pattern_seed = 42;
    
    /* Multi-stage memory operations */
    for (int i = 0; i < 8; i++) {
        /* Stage 1: Clear buffer */
        __builtin_memset(buffer, 0, sizeof(buffer));
        
        /* Stage 2: Fill with pattern */
        char fill_char = (char)((pattern_seed + i * 13) & 0xFF);
        __builtin_memset(buffer + i * 64, fill_char, 64);
        
        /* Stage 3: Copy within buffer */
        if (i > 0) {
            __builtin_memcpy(buffer + 256, buffer, 128);
        }
        
        /* Stage 4: Move overlapping regions */
        if (i % 2 == 0) {
            __builtin_memmove(buffer + 128, buffer + 96, 160);
        }
        
        /* Update hash */
        for (int j = 0; j < sizeof(buffer); j++) {
            hash = (hash * 31) + (unsigned char)buffer[j];
        }
        
        /* Volatile update to prevent loop optimization */
        pattern_seed += buffer[i] & 0xF;
    }
    
    return hash;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Build and process AST */
    int counter = 0;
    ASTNode* ast1 = build_ast(3, &counter);
    ASTNode* ast2 = build_ast(3, &counter);
    
    if (ast1 && ast2) {
        /* Copy data between AST nodes */
        copy_ast_data(ast2, ast1);
        
        /* Self-copy edge case */
        copy_ast_data(ast1, ast1);
    }
    
    /* Phase 2: Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Phase 3: Generate and verify memory hash */
    unsigned long final_hash = generate_memory_hash();
    printf("Memory hash: 0x%08lx\n", final_hash);
    
    /* Phase 4: Additional built-in calls in varied contexts */
    {
        char final_buffer[256];
        volatile int dynamic_size = g_mem_size % 128 + 64;
        
        /* Chain of memory operations */
        __builtin_memset(final_buffer, 0xAA, dynamic_size);
        __builtin_memcpy(final_buffer + 64, final_buffer, 64);
        __builtin_memmove(final_buffer, final_buffer + 32, 128);
        __builtin_memset(final_buffer + 128, 0x55, 64);
        
        /* Verify with final memcpy */
        __builtin_memcpy(final_buffer + 192, final_buffer, 64);
    }
    
    /* Cleanup */
    /* Note: In real usage, free AST nodes here */
    
    printf("Test completed successfully\n");
    return 0;
}
