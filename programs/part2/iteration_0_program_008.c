/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _OPENMP
#include <omp.h>
#endif

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char g_token_array[1024];
static volatile size_t g_token_pos = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Force initialization of memory functions */
    char buffer[32];
    __builtin_memset(buffer, 0xA5, sizeof(buffer));
    __builtin_memcpy(g_token_array, buffer, 32);
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    /* Final memory operation in destructor */
    char final_buffer[16];
    __builtin_memset(final_buffer, 0xFF, sizeof(final_buffer));
}

/* Recursive function with memory operations */
static ASTNode* create_ast_node(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node->data, depth, sizeof(node->data));
    node->size = sizeof(node->data);
    
    /* Copy data between volatile and non-volatile */
    volatile char temp[32];
    __builtin_memcpy((void*)temp, node->data, 32);
    
    /* Create children with goto for flow control */
    if (depth > 1) {
        node->left = create_ast_node(depth - 1);
        node->right = create_ast_node(depth - 1);
        
        /* Copy between child nodes using memmove */
        if (node->left && node->right) {
            /* Use goto to jump into memory operation block */
            goto copy_block;
copy_block:
            __builtin_memmove(node->right->data, 
                            node->left->data, 
                            sizeof(node->data) / 2);
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with complex control flow and builtins */
static void process_ast(ASTNode* root, char* output) {
    if (!root) return;
    
    volatile int use_memcpy = 1;
    char local_buf[128];
    
    /* Control flow with goto around memory operations */
    if (use_memcpy) {
        goto do_copy;
    } else {
        goto do_move;
    }

do_copy:
    __builtin_memcpy(local_buf, root->data, root->size);
    goto after_mem;

do_move:
    __builtin_memmove(local_buf, root->data, root->size);
    goto after_mem;

after_mem:
    /* Process children */
    process_ast(root->left, output);
    process_ast(root->right, output);
    
    /* Accumulate to output */
    for (size_t i = 0; i < sizeof(root->data); i++) {
        output[i % 64] ^= root->data[i];
    }
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char thread_buf[256];
        char src_buf[256];
        
        /* Initialize source with pattern */
        for (int i = 0; i < 256; i++) {
            src_buf[i] = (char)((thread_id * 31 + i) & 0xFF);
        }
        
        /* Use all three builtins in parallel region */
        __builtin_memset(thread_buf, thread_id, sizeof(thread_buf));
        __builtin_memcpy(thread_buf + 64, src_buf + 64, 128);
        __builtin_memmove(thread_buf + 128, thread_buf, 64);
        
        /* Critical section to update global array */
        #pragma omp critical
        {
            __builtin_memcpy(g_token_array + thread_id * 32, 
                           thread_buf, 
                           32);
        }
    }
}

/* Main test driver */
int main(void) {
    /* Initialize token array */
    __builtin_memset(g_token_array, 0, sizeof(g_token_array));
    
    /* Create recursive structure */
    ASTNode* ast_root = create_ast_node(4);
    if (!ast_root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process AST with memory operations */
    char hash_buffer[64] = {0};
    process_ast(ast_root, hash_buffer);
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Additional memory operations with volatile sizes */
    volatile size_t copy_size = g_mem_size;
    if (copy_size > sizeof(g_token_array)) {
        copy_size = sizeof(g_token_array);
    }
    
    char* dynamic_buf = (char*)malloc(copy_size);
    if (dynamic_buf) {
        __builtin_memcpy(dynamic_buf, g_token_array, copy_size);
        __builtin_memset(dynamic_buf + copy_size/2, 0x42, copy_size/4);
        __builtin_memmove(g_token_array, dynamic_buf, copy_size);
        free(dynamic_buf);
    }
    
    /* Compute verification hash */
    uint64_t final_hash = 0;
    for (size_t i = 0; i < sizeof(g_token_array); i++) {
        final_hash = (final_hash * 31) + g_token_array[i];
        final_hash ^= hash_buffer[i % sizeof(hash_buffer)];
    }
    
    /* Cleanup */
    /* Note: In real code, would need recursive free function */
    
    printf("Test completed. Final hash: 0x%016llx\n", 
           (unsigned long long)final_hash);
    
    return 0;
}
