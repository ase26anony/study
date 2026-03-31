/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
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
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Recursive function using builtins with goto */
static void process_ast_recursive(ASTNode* node, int depth) {
    if (!node || depth <= 0) return;
    
    volatile int use_memmove = depth % 2;
    
    /* Goto label before builtin */
    if (use_memmove) {
        goto use_memmove_block;
    }
    
    /* Use __builtin_memset */
    __builtin_memset(node->data, depth, node->size);
    
    /* Jump over memmove block */
    goto skip_memmove;
    
use_memmove_block:
    /* This block tests goto into builtin context */
    if (node->left && node->right) {
        __builtin_memmove(node->left->data, node->right->data, 
                         node->size < 128 ? node->size : 128);
    }
    
skip_memmove:
    /* Use __builtin_memcpy */
    char temp[256];
    __builtin_memcpy(temp, node->data, node->size);
    
    /* Recursive calls */
    process_ast_recursive(node->left, depth - 1);
    process_ast_recursive(node->right, depth - 1);
}

/* Function with varied builtin usage patterns */
static size_t hash_memory_regions(char* regions[], int count) {
    size_t hash = 0;
    volatile size_t block_size = g_mem_size;
    
    for (int i = 0; i < count; i++) {
        char buffer[256];
        
        /* Pattern 1: Direct builtin calls */
        __builtin_memset(regions[i], i, block_size);
        
        /* Pattern 2: Builtin in conditional */
        if (i % 3 == 0) {
            __builtin_memcpy(buffer, regions[i], block_size);
        } else if (i % 3 == 1) {
            __builtin_memmove(regions[i], buffer, block_size);
        }
        
        /* Compute simple hash */
        for (size_t j = 0; j < block_size && j < 256; j++) {
            hash += (size_t)regions[i][j];
        }
    }
    
    return hash;
}

/* OpenMP parallel section with builtins */
static void parallel_memory_operations(void) {
    const int num_threads = 4;
    char thread_buffers[4][256];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        volatile size_t local_size = g_mem_size + tid * 16;
        
        /* Each thread uses different builtins */
        switch (tid % 3) {
            case 0:
                __builtin_memset(thread_buffers[tid], tid, local_size);
                break;
            case 1:
                __builtin_memcpy(thread_buffers[tid], 
                               thread_buffers[(tid + 1) % 4],
                               local_size);
                break;
            case 2:
                __builtin_memmove(thread_buffers[tid],
                                thread_buffers[(tid + 3) % 4],
                                local_size);
                break;
        }
        
        /* Barrier to ensure all builtins are processed */
        #pragma omp barrier
        
        /* Cross-thread memory operation */
        if (tid == 0) {
            __builtin_memcpy(thread_buffers[1],
                           thread_buffers[2],
                           g_mem_size);
        }
    }
}

/* Complex control flow with nested gotos */
static void complex_control_flow_test(void) {
    char buffer1[512], buffer2[512];
    volatile int mode = 0;
    
    /* Initialize with memset */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    
    /* Goto-based state machine */
    state_machine_start:
    mode = (mode + 1) % 4;
    
    switch (mode) {
        case 0:
            goto use_memcpy;
        case 1:
            goto use_memset;
        case 2:
            goto use_memmove;
        default:
            goto finish;
    }
    
use_memcpy:
    __builtin_memcpy(buffer2, buffer1, g_mem_size);
    goto state_machine_start;
    
use_memset:
    __builtin_memset(buffer1, mode, g_mem_size * 2);
    goto state_machine_start;
    
use_memmove:
    __builtin_memmove(buffer1, buffer2, g_mem_size);
    goto state_machine_start;
    
finish:
    /* Final builtin to ensure all paths covered */
    __builtin_memcpy(buffer1, buffer2, sizeof(buffer1));
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN builtin redirection test\n");
    
    /* Create AST structure */
    ASTNode* root = calloc(1, sizeof(ASTNode));
    ASTNode* left = calloc(1, sizeof(ASTNode));
    ASTNode* right = calloc(1, sizeof(ASTNode));
    
    if (!root || !left || !right) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    root->size = g_mem_size;
    left->size = g_mem_size / 2;
    right->size = g_mem_size / 2;
    
    root->left = left;
    root->right = right;
    
    /* Test 1: Recursive AST processing */
    printf("Test 1: Recursive AST processing\n");
    process_ast_recursive(root, 3);
    
    /* Test 2: Memory region hashing */
    printf("Test 2: Memory region hashing\n");
    char* regions[4];
    for (int i = 0; i < 4; i++) {
        regions[i] = malloc(g_mem_size);
        if (!regions[i]) {
            fprintf(stderr, "Region allocation failed\n");
            return 1;
        }
    }
    
    size_t hash = hash_memory_regions(regions, 4);
    printf("Memory hash: %zu\n", hash);
    
    /* Test 3: OpenMP parallel operations */
    printf("Test 3: OpenMP parallel operations\n");
    parallel_memory_operations();
    
    /* Test 4: Complex control flow */
    printf("Test 4: Complex control flow\n");
    complex_control_flow_test();
    
    /* Force multiple builtin calls with different sizes */
    printf("Test 5: Varied builtin calls\n");
    char final_buffer[1024];
    volatile size_t sizes[] = {16, 32, 64, 128, 256};
    
    for (int i = 0; i < 5; i++) {
        volatile size_t current_size = sizes[i];
        
        __builtin_memset(final_buffer, i, current_size);
        
        if (i % 2 == 0) {
            char temp[1024];
            __builtin_memcpy(temp, final_buffer, current_size);
            __builtin_memmove(final_buffer, temp, current_size);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        free(regions[i]);
    }
    free(left);
    free(right);
    free(root);
    
    printf("ASAN builtin redirection test completed successfully\n");
    return 0;
}
