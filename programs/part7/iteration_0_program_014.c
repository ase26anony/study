/* ISO C99-compliant test program for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
struct ast_node {
    int type;
    char data[64];
    struct ast_node *left;
    struct ast_node *right;
    struct ast_node *next;
};

/* Constructor function to force early initialization */
__attribute__((constructor)) static void init_globals(void) {
    printf("Constructor: Initializing global state\n");
    g_mem_size = 256;
}

/* Destructor function for cleanup verification */
__attribute__((destructor)) static void cleanup_globals(void) {
    printf("Destructor: Cleaning up\n");
}

/* Function with goto jumping into memory operation block */
static void test_goto_memmove(void *dst, const void *src, size_t n) {
    int use_builtin = 1;
    
    if (n > 100) {
        goto use_libc;
    }
    
use_builtin:
    /* This block will be entered via goto */
    __builtin_memmove(dst, src, n);
    goto done;
    
use_libc:
    /* Force compiler to consider both paths */
    if (use_builtin) {
        goto use_builtin;
    }
    memmove(dst, src, n);
    
done:
    return;
}

/* Recursive function operating on AST structures */
static struct ast_node* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    struct ast_node *node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    node->type = depth;
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    node->next = NULL;
    
    /* Use volatile to prevent constant folding */
    volatile int data_size = sizeof(node->data);
    
    /* Built-in memset with non-foldable size */
    __builtin_memset(node->data, depth, data_size);
    
    return node;
}

/* Function copying between AST nodes */
static void copy_ast_data(struct ast_node *dst, const struct ast_node *src) {
    if (!dst || !src) return;
    
    /* Built-in memcpy between node data */
    volatile size_t copy_size = sizeof(dst->data);
    __builtin_memcpy(dst->data, src->data, copy_size);
    
    /* Recursive copy */
    if (src->left && dst->left) {
        copy_ast_data(dst->left, src->left);
    }
    if (src->right && dst->right) {
        copy_ast_data(dst->right, src->right);
    }
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    const int num_threads = 4;
    char *buffers[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread allocates and operates on its own buffer */
        buffers[tid] = malloc(g_mem_size);
        if (buffers[tid]) {
            /* Built-in memset in parallel region */
            __builtin_memset(buffers[tid], tid, g_mem_size);
            
            /* Built-in memcpy between thread buffers */
            if (tid > 0) {
                __builtin_memcpy(buffers[tid], buffers[tid-1], g_mem_size);
            }
            
            /* Built-in memmove with overlapping regions */
            if (g_mem_size > 32) {
                __builtin_memmove(buffers[tid] + 16, buffers[tid], g_mem_size - 32);
            }
        }
        
        #pragma omp barrier
        
        /* Test goto with memmove in parallel context */
        if (buffers[tid]) {
            test_goto_memmove(buffers[tid] + 64, buffers[tid], 32);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_threads; i++) {
        if (buffers[i]) {
            free(buffers[i]);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Basic built-in calls with volatile sizes */
    char buffer1[512];
    char buffer2[512];
    volatile size_t op_size = g_mem_size;
    
    __builtin_memset(buffer1, 0xAA, op_size);
    __builtin_memcpy(buffer2, buffer1, op_size);
    __builtin_memmove(buffer1 + 128, buffer1, op_size - 128);
    
    /* Phase 2: AST structure operations */
    struct ast_node *ast1 = create_ast(3);
    struct ast_node *ast2 = create_ast(3);
    
    if (ast1 && ast2) {
        copy_ast_data(ast2, ast1);
        
        /* Additional built-in memcpy on extracted data */
        char temp[64];
        __builtin_memcpy(temp, ast1->data, sizeof(temp));
        __builtin_memcpy(ast2->data, temp, sizeof(temp));
    }
    
    /* Phase 3: Parallel memory operations */
    parallel_memory_ops();
    
    /* Phase 4: Mixed memory operations in complex control flow */
    {
        char *dynamic_buf = malloc(1024);
        if (dynamic_buf) {
            for (int i = 0; i < 10; i++) {
                volatile int offset = i * 64;
                
                switch (i % 3) {
                    case 0:
                        __builtin_memset(dynamic_buf + offset, i, 64);
                        break;
                    case 1:
                        __builtin_memcpy(dynamic_buf + offset, buffer1, 64);
                        break;
                    case 2:
                        __builtin_memmove(dynamic_buf + offset, 
                                         dynamic_buf + offset - 32, 
                                         64);
                        break;
                }
            }
            free(dynamic_buf);
        }
    }
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        hash = (hash * 31) + buffer1[i];
    }
    
    printf("Test completed. Verification hash: %lu\n", hash);
    
    /* Cleanup AST */
    /* Note: In real ASAN, this would detect leaks if present */
    
    return 0;
}
