/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

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

/* Function with goto jumps around memmove */
static void test_goto_memmove(char* dest, char* src) {
    int use_copy = 1;
    
    if (use_copy) {
        goto copy_block;
    }
    
    /* This block should be jumped over */
    __builtin_memset(dest, 0, g_mem_size);
    
copy_block:
    /* Jump target with memmove */
    __builtin_memmove(dest, src, g_mem_size);
    
    if (dest[0] == 'X') {
        goto skip_clear;
    }
    
    __builtin_memset(dest + 32, 0, 16);
    
skip_clear:
    /* Jump out of block */
    return;
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast_node(const char* data, size_t depth) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy for initialization */
    __builtin_memcpy(node->data, data, strlen(data) + 1);
    node->size = strlen(data) + 1;
    
    if (depth > 0) {
        char child_data[256];
        __builtin_snprintf(child_data, sizeof(child_data), 
                          "%s-%zu", data, depth);
        
        node->left = create_ast_node(child_data, depth - 1);
        node->right = create_ast_node(child_data, depth - 1);
        
        /* Copy between child nodes using builtins */
        if (node->left && node->right) {
            __builtin_memcpy(node->right->data, 
                           node->left->data, 
                           node->left->size);
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Complex memory dispatch with OpenMP */
static void parallel_memory_operations(void) {
    const int num_buffers = 8;
    char* buffers[num_buffers];
    size_t sizes[num_buffers];
    
    /* Initialize buffers with volatile sizes */
    for (int i = 0; i < num_buffers; i++) {
        sizes[i] = g_mem_size + (i * 16);
        buffers[i] = (char*)malloc(sizes[i]);
        if (buffers[i]) {
            __builtin_memset(buffers[i], i + 'A', sizes[i]);
        }
    }
    
    /* OpenMP parallel region with memory operations */
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < num_buffers - 1; i++) {
            if (buffers[i] && buffers[i + 1]) {
                /* Force different builtin usage patterns */
                switch (thread_id % 3) {
                    case 0:
                        __builtin_memcpy(buffers[i], 
                                       buffers[i + 1], 
                                       sizes[i] < sizes[i + 1] ? 
                                       sizes[i] : sizes[i + 1]);
                        break;
                    case 1:
                        __builtin_memset(buffers[i] + 16, 
                                       thread_id, 
                                       sizes[i] - 32);
                        break;
                    case 2:
                        __builtin_memmove(buffers[i], 
                                        buffers[i] + 8, 
                                        sizes[i] - 8);
                        break;
                }
            }
        }
        
        /* Barrier to ensure all memory ops complete */
        #pragma omp barrier
        
        /* Additional memmove with goto in parallel region */
        #pragma omp single
        {
            char temp[256];
            __builtin_memset(temp, 0, sizeof(temp));
            
            int do_move = 1;
            if (do_move) {
                goto perform_move;
            }
            
            __builtin_memcpy(temp, "skip", 5);
            
        perform_move:
            __builtin_memmove(temp, "moved_data", 11);
            
            if (buffers[0]) {
                __builtin_memcpy(buffers[0], temp, 11);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_buffers; i++) {
        free(buffers[i]);
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Test 1: Direct builtin calls with volatile sizes */
    char buffer1[256];
    char buffer2[256];
    volatile size_t copy_size = g_mem_size;
    
    __builtin_memset(buffer1, 'X', sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, copy_size);
    __builtin_memmove(buffer1 + 32, buffer1, 64);
    
    /* Test 2: Goto flow control */
    test_goto_memmove(buffer2, buffer1);
    
    /* Test 3: Recursive AST operations */
    ASTNode* root = create_ast_node("root", 3);
    if (root) {
        /* Perform memory operations between tree nodes */
        if (root->left && root->right) {
            size_t copy_len = root->left->size < root->right->size ? 
                            root->left->size : root->right->size;
            __builtin_memcpy(root->right->data, 
                           root->left->data, 
                           copy_len);
        }
        
        /* TODO: Add recursive free function */
        free(root);
    }
    
    /* Test 4: OpenMP parallel memory operations */
    parallel_memory_operations();
    
    /* Test 5: Mixed builtin usage in loops */
    char pattern[512];
    for (int i = 0; i < 10; i++) {
        volatile int offset = i * 32;
        if (i % 2 == 0) {
            __builtin_memset(pattern + offset, i, 32);
        } else {
            __builtin_memcpy(pattern + offset, 
                           pattern + offset - 32, 
                           32);
        }
    }
    
    /* Final verification */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(pattern); i++) {
        hash = hash * 31 + pattern[i];
    }
    
    printf("Test completed. Hash: %lu\n", hash);
    printf("Expected ASAN coverage:\n");
    printf("  - BUILT_IN_MEMCPY/MEMSET/MEMMOVE redirection\n");
    printf("  - asan_memfn_rtls[] cache initialization\n");
    printf("  - __asan_ vs __hwasan_ branch selection\n");
    printf("  - Flow-sensitive RTL modification logic\n");
    
    return 0;
}
