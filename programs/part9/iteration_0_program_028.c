/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
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
    g_mem_size = 256 + (rand() % 768);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Function with goto jumps around memmove */
static void test_goto_memmove(char* dest, char* src, size_t n) {
    int use_copy = 1;
    
    if (n == 0) goto skip_copy;
    
    /* Jump into block with memmove */
    if (use_copy) goto do_copy;
    
skip_copy:
    return;
    
do_copy:
    /* Force builtin memmove with goto context */
    __builtin_memmove(dest, src, n);
    
    /* Jump out */
    goto skip_copy;
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_tree(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->size = g_mem_size % 256;
    
    /* Fill data with pattern */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        node->data[i] = (char)((i * 37) & 0xFF);
    }
    
    if (depth > 1) {
        node->left = create_tree(depth - 1);
        node->right = create_tree(depth - 1);
        
        /* Copy data between nodes if both exist */
        if (node->left && node->right) {
            size_t copy_size = node->left->size < node->right->size ? 
                              node->left->size : node->right->size;
            __builtin_memcpy(node->right->data, node->left->data, copy_size);
        }
    }
    
    return node;
}

/* Complex memory dispatch with OpenMP */
static void parallel_memory_ops(void) {
    const int num_buffers = 16;
    char* buffers[num_buffers];
    size_t sizes[num_buffers];
    
    /* Initialize buffers with volatile sizes */
    for (int i = 0; i < num_buffers; i++) {
        sizes[i] = (g_mem_size + i * 32) % 512;
        buffers[i] = (char*)malloc(sizes[i] + 64); /* Extra for redzones */
        if (!buffers[i]) continue;
        
        /* Pattern initialization with memset */
        __builtin_memset(buffers[i], i, sizes[i]);
    }
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < num_buffers - 1; i++) {
            if (buffers[i] && buffers[i+1]) {
                size_t copy_len = sizes[i] < sizes[i+1] ? sizes[i] : sizes[i+1];
                
                /* Alternate between memcpy and memmove based on thread */
                if (thread_id % 2 == 0) {
                    __builtin_memcpy(buffers[i+1], buffers[i], copy_len);
                } else {
                    /* Overlapping regions for memmove */
                    size_t overlap = copy_len / 2;
                    __builtin_memmove(buffers[i] + overlap, buffers[i], copy_len - overlap);
                }
            }
        }
        
        /* Thread-specific memset */
        #pragma omp single
        {
            for (int i = 0; i < num_buffers; i += 4) {
                if (buffers[i]) {
                    __builtin_memset(buffers[i] + 16, thread_id, 32);
                }
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
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Basic built-in calls */
    char buffer1[512];
    char buffer2[512];
    volatile size_t copy_size = g_mem_size % 256;
    
    /* Force all three builtins */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, copy_size);
    __builtin_memmove(buffer1 + 128, buffer1, copy_size);
    
    /* Phase 2: Goto edge cases */
    test_goto_memmove(buffer1 + 64, buffer2, 128);
    
    /* Phase 3: Recursive structure with memory ops */
    ASTNode* root = create_tree(4);
    
    if (root) {
        /* Traverse and verify */
        unsigned long hash = 0;
        ASTNode* stack[32];
        int top = 0;
        stack[top++] = root;
        
        while (top > 0) {
            ASTNode* node = stack[--top];
            
            /* Compute simple hash */
            for (size_t i = 0; i < node->size && i < sizeof(node->data); i++) {
                hash = (hash * 31 + node->data[i]) & 0xFFFFFFFF;
            }
            
            if (node->right) stack[top++] = node->right;
            if (node->left) stack[top++] = node->left;
            
            /* Free with cleanup */
            __builtin_memset(node, 0xDD, sizeof(ASTNode));
            free(node);
        }
        
        printf("Tree hash: %lu\n", hash);
    }
    
    /* Phase 4: OpenMP parallel memory operations */
    parallel_memory_ops();
    
    /* Phase 5: Variable-sized operations */
    for (int i = 0; i < 8; i++) {
        size_t size = (g_mem_size + i * 64) % 384;
        char* dyn_buf = (char*)malloc(size);
        
        if (dyn_buf) {
            /* Mix of all three builtins */
            __builtin_memset(dyn_buf, i, size);
            
            if (i % 3 == 0) {
                __builtin_memcpy(buffer1, dyn_buf, size < 512 ? size : 512);
            } else if (i % 3 == 1) {
                __builtin_memmove(dyn_buf + size/2, dyn_buf, size/2);
            }
            
            free(dyn_buf);
        }
    }
    
    /* Final verification */
    int sum = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        sum += buffer1[i];
    }
    
    printf("Final buffer sum: %d\n", sum);
    printf("Test completed successfully\n");
    
    return 0;
}
