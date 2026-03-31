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
    size_t depth;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    printf("ASAN Test Constructor: Initializing...\n");
    g_mem_size = 256 + (rand() % 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("ASAN Test Destructor: Cleaning up...\n");
}

/* Recursive AST creation with memory operations */
static ASTNode* create_ast(size_t depth) {
    if (depth == 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->depth = depth;
    
    /* Create pattern in data using __builtin_memcpy */
    char pattern[64];
    for (int i = 0; i < 64; i++) {
        pattern[i] = (char)((depth + i) % 256);
    }
    __builtin_memcpy(node->data, pattern, 64);
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_left = (depth % 2 == 0);
        
        if (use_left) {
            node->left = create_ast(depth - 1);
            goto skip_right;
        }
        
        node->right = create_ast(depth - 1);
        
        skip_right:
        /* Use __builtin_memmove to shift data */
        if (node->left && node->right) {
            __builtin_memmove(node->left->data + 16, 
                            node->right->data, 
                            32);
        }
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void complex_memory_ops(void* dest, void* src, size_t size) {
    volatile int flag = 1;
    char* d = (char*)dest;
    char* s = (char*)src;
    
    start_block:
    if (flag) {
        /* First __builtin_memmove with goto into block */
        goto do_memmove;
    }
    
    mid_block:
    /* __builtin_memset after jump */
    __builtin_memset(d + size/2, 0xAA, size/4);
    goto end_block;
    
    do_memmove:
    /* This tests flow-sensitivity */
    __builtin_memmove(d, s, size/2);
    flag = 0;
    goto mid_block;
    
    end_block:
    /* Final __builtin_memcpy */
    __builtin_memcpy(d + size/2, s + size/2, size/2);
}

/* OpenMP parallel memory operations */
static void parallel_memory_dispatch(void) {
    const int num_chunks = 8;
    char* buffers[num_chunks];
    size_t chunk_size = g_mem_size / num_chunks;
    
    /* Allocate buffers */
    for (int i = 0; i < num_chunks; i++) {
        buffers[i] = (char*)malloc(chunk_size);
        __builtin_memset(buffers[i], i, chunk_size);
    }
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < num_chunks; i++) {
            /* Each thread performs memory operations */
            char* src = buffers[i];
            char* dest = buffers[(i + 1) % num_chunks];
            
            /* Mix of builtins */
            __builtin_memcpy(dest, src, chunk_size/2);
            __builtin_memset(src + chunk_size/2, thread_id, chunk_size/4);
            __builtin_memmove(dest + chunk_size/2, 
                            src + chunk_size/4, 
                            chunk_size/4);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_chunks; i++) {
        free(buffers[i]);
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Basic built-in calls */
    char buffer1[512];
    char buffer2[512];
    
    __builtin_memset(buffer1, 0xCC, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 128, buffer2, 256);
    
    /* Phase 2: AST operations */
    ASTNode* root = create_ast(4);
    if (root) {
        /* Copy between AST nodes */
        if (root->left && root->right) {
            __builtin_memcpy(root->left->data, 
                           root->right->data, 
                           sizeof(root->left->data));
        }
        
        /* Complex memory ops with goto */
        complex_memory_ops(root->data, buffer1, 64);
        
        /* Free AST recursively */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    /* Phase 3: OpenMP parallel operations */
    #ifdef _OPENMP
    parallel_memory_dispatch();
    #endif
    
    /* Phase 4: Variable-sized operations */
    volatile size_t dynamic_size = g_mem_size;
    char* dyn_buf1 = (char*)malloc(dynamic_size);
    char* dyn_buf2 = (char*)malloc(dynamic_size);
    
    if (dyn_buf1 && dyn_buf2) {
        __builtin_memset(dyn_buf1, 0x55, dynamic_size);
        __builtin_memcpy(dyn_buf2, dyn_buf1, dynamic_size);
        
        /* Overlapping memmove */
        __builtin_memmove(dyn_buf1 + dynamic_size/4, 
                         dyn_buf1, 
                         dynamic_size/2);
        
        free(dyn_buf1);
        free(dyn_buf2);
    }
    
    /* Verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        hash = (hash * 31) + buffer1[i];
    }
    
    printf("Test completed. Verification hash: %lu\n", hash);
    printf("Expected ASAN coverage:\n");
    printf("  - BUILT_IN_MEMCPY redirection\n");
    printf("  - BUILT_IN_MEMSET redirection\n");
    printf("  - BUILT_IN_MEMMOVE redirection\n");
    printf("  - asan_memfn_rtls cache initialization\n");
    
    return 0;
}
