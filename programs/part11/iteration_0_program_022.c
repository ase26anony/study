/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* AST-like recursive structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    /* Force early initialization of memory functions */
    char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Create pattern in data using __builtin_memcpy */
    char pattern[64];
    __builtin_memset(pattern, 'A' + (depth % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(pattern));
    
    node->size = sizeof(pattern);
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    return node;
}

/* Function with goto jumps around memory operations */
static void test_goto_memmove(void) {
    char src[128], dst[128];
    volatile int condition = 1;
    
    __builtin_memset(src, 'X', sizeof(src));
    
    /* Jump into memory operation block */
    goto jump_into_block;
    
    /* This label is inside the block with memmove */
    inside_block:
        __builtin_memmove(dst + 32, src + 16, 64);
        goto after_block;
    
    jump_into_block:
        if (condition) {
            goto inside_block;
        }
    
    after_block:
        /* Another memmove after the jump */
        __builtin_memmove(dst, src, 32);
}

/* OpenMP parallel memory operations */
static void parallel_mem_operations(void) {
    const int num_threads = 4;
    char* buffers[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        buffers[tid] = malloc(g_mem_size);
        
        if (buffers[tid]) {
            /* Each thread uses different builtins */
            switch (tid % 3) {
                case 0:
                    __builtin_memset(buffers[tid], tid, g_mem_size);
                    break;
                case 1:
                    if (tid > 0) {
                        __builtin_memcpy(buffers[tid], buffers[tid-1], g_mem_size);
                    }
                    break;
                case 2:
                    __builtin_memmove(buffers[tid], buffers[tid], g_mem_size);
                    break;
            }
        }
        
        #pragma omp barrier
        
        /* Cross-thread memory operation */
        if (tid == 0) {
            for (int i = 1; i < num_threads; i++) {
                if (buffers[i]) {
                    __builtin_memcpy(buffers[0] + i*16, buffers[i], 16);
                }
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_threads; i++) {
        free(buffers[i]);
    }
}

/* Complex memory operation sequence */
static size_t complex_memory_sequence(void) {
    size_t hash = 0;
    char* regions[8];
    
    /* Allocate and initialize regions */
    for (int i = 0; i < 8; i++) {
        regions[i] = malloc(g_mem_size * (i + 1));
        if (regions[i]) {
            __builtin_memset(regions[i], i * 16, g_mem_size * (i + 1));
        }
    }
    
    /* Chain memory operations */
    for (int i = 1; i < 8; i++) {
        if (regions[i] && regions[i-1]) {
            size_t copy_size = g_mem_size * i;
            if (i % 2 == 0) {
                __builtin_memcpy(regions[i], regions[i-1], copy_size);
            } else {
                __builtin_memmove(regions[i], regions[i-1], copy_size);
            }
        }
    }
    
    /* Calculate hash from memory contents */
    for (int i = 0; i < 8; i++) {
        if (regions[i]) {
            for (size_t j = 0; j < g_mem_size && j < 64; j++) {
                hash += (size_t)regions[i][j];
            }
            free(regions[i]);
        }
    }
    
    return hash;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Basic built-in calls */
    char buffer1[256], buffer2[256];
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 128, buffer1, 128);
    
    /* Phase 2: Goto flow control test */
    test_goto_memmove();
    
    /* Phase 3: Recursive AST operations */
    ASTNode* root = create_ast(3);
    if (root) {
        /* Copy between AST nodes */
        if (root->left && root->right) {
            __builtin_memcpy(root->right->data, root->left->data, 
                           root->left->size < root->right->size ? 
                           root->left->size : root->right->size);
        }
        
        /* TODO: Add AST cleanup function */
        free(root);
    }
    
    /* Phase 4: OpenMP parallel operations */
    parallel_mem_operations();
    
    /* Phase 5: Complex sequence */
    size_t final_hash = complex_memory_sequence();
    
    printf("Test completed. Final hash: %zu\n", final_hash);
    printf("Expected coverage:\n");
    printf("1. BUILT_IN_MEMCPY redirection\n");
    printf("2. BUILT_IN_MEMSET redirection\n");
    printf("3. BUILT_IN_MEMMOVE redirection\n");
    printf("4. asan_memfn_rtls cache initialization\n");
    
    return 0;
}
