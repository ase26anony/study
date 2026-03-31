/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ast_node {
    int type;
    char data[64];
    struct ast_node* left;
    struct ast_node* right;
    struct ast_node* parent;
} ast_node_t;

/* Global token array */
static char g_token_buffer[4096];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Force initialization of ASAN runtime */
    __builtin_memset(g_token_buffer, 0xAA, sizeof(g_token_buffer));
    g_token_index = 42; /* Arbitrary non-zero value */
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Final memory operation in destructor */
    volatile char final_buf[128];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ast_node_t* parse_expression(int depth) {
    if (depth <= 0) return NULL;
    
    ast_node_t* node = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    
    /* Fill data with pattern using memcpy */
    char pattern[64];
    __builtin_memset(pattern, 'A' + (depth % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(node->data));
    
    node->type = depth;
    
    /* Recursive calls */
    node->left = parse_expression(depth - 1);
    node->right = parse_expression(depth - 2);
    
    /* Copy parent reference if exists */
    if (node->left) {
        node->left->parent = node;
        /* Use memmove for overlapping regions */
        if (g_use_memmove) {
            __builtin_memmove(&node->left->type, &node->type, 
                            sizeof(node->type));
        }
    }
    
    return node;
}

/* Complex memory dispatch with goto flow control */
static void dispatch_memory_operations(void) {
    volatile char src[512];
    volatile char dst[512];
    volatile char temp[512];
    int use_memmove = 0;
    
    /* Initialize source with pattern */
    for (size_t i = 0; i < sizeof(src); i++) {
        src[i] = (char)(i % 256);
    }
    
    /* Label for goto jumps */
    mem_ops_start:
    
    /* First memcpy */
    __builtin_memcpy(dst, src, g_mem_size);
    
    if (use_memmove) {
        /* Jump to memmove section */
        goto do_memmove;
    }
    
    /* memset section */
    __builtin_memset(temp, 0xCC, g_mem_size);
    
    /* Conditional jump */
    if (g_token_index > 100) {
        use_memmove = 1;
        goto mem_ops_start; /* Loop back */
    }
    
    do_memmove:
    /* Overlapping memmove */
    __builtin_memmove((char*)dst + 128, dst, 256);
    
    /* Jump out of block */
    goto mem_ops_end;
    
    /* Unreachable in normal flow */
    __builtin_memset(dst, 0xFF, sizeof(dst));
    
    mem_ops_end:
    /* Final verification copy */
    __builtin_memcpy(temp, dst, 128);
}

/* OpenMP parallel section */
static void parallel_memory_operations(void) {
    volatile char parallel_buf[1024];
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        char local_buf[256];
        
        /* Use builtins with thread-specific patterns */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp critical
        {
            /* Copy to shared buffer */
            __builtin_memcpy((char*)parallel_buf + thread_id * 256, 
                           local_buf, 256);
            
            /* Move data within shared buffer */
            if (thread_id > 0) {
                __builtin_memmove((char*)parallel_buf + (thread_id-1)*256,
                                (char*)parallel_buf + thread_id*256,
                                128);
            }
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize and parse */
    ast_node_t* ast_root = parse_expression(5);
    
    /* Phase 2: Dispatch memory operations */
    dispatch_memory_operations();
    
    /* Phase 3: Parallel operations */
    parallel_memory_operations();
    
    /* Phase 4: Complex node copying */
    if (ast_root && ast_root->left && ast_root->right) {
        /* Copy between nodes using all three builtins */
        __builtin_memset(ast_root->left->data, 0xDD, 
                        sizeof(ast_root->left->data));
        
        __builtin_memcpy(ast_root->right->data, 
                        ast_root->left->data,
                        sizeof(ast_root->left->data));
        
        /* Overlapping copy within same node */
        __builtin_memmove(ast_root->data + 32,
                         ast_root->data,
                         32);
    }
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    if (ast_root) {
        for (size_t i = 0; i < sizeof(g_token_buffer); i++) {
            hash = (hash * 31) + g_token_buffer[i];
        }
        hash += ast_root->type;
        
        /* Cleanup */
        free(ast_root->left);
        free(ast_root->right);
        free(ast_root);
    }
    
    printf("Test completed. Hash: %lu\n", hash);
    return (hash != 0) ? 0 : 1;
}
