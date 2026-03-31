/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Forward declarations for recursive structures */
struct ast_node;
typedef struct ast_node ast_node_t;

/* Complex AST-like structure with volatile members */
struct ast_node {
    volatile int type;
    volatile char *data;
    volatile size_t len;
    ast_node_t *left;
    ast_node_t *right;
    volatile uint8_t padding[32];  /* For redzone testing */
};

/* Global volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_init_flag = 0;
volatile char g_global_buf[1024];

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force early initialization of memory functions */
    volatile char local_buf[64];
    __builtin_memset(local_buf, 0xAA, sizeof(local_buf));
    __builtin_memcpy(g_global_buf, local_buf, 64);
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Final memory operations */
    volatile char cleanup_buf[128];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive AST creation with memory operations */
static ast_node_t* create_ast(int depth, volatile size_t size) {
    if (depth <= 0) return NULL;
    
    ast_node_t *node = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    
    node->type = depth;
    node->len = size;
    node->data = (char*)malloc(size);
    
    if (node->data) {
        /* Pattern initialization */
        for (volatile size_t i = 0; i < size; i++) {
            node->data[i] = (char)(i % 256);
        }
        
        /* Copy pattern to global buffer with goto control flow */
        volatile size_t copy_len = size > 64 ? 64 : size;
        
        if (copy_len > 32) {
            goto copy_block;
        } else {
            /* Small copy path */
            __builtin_memcpy(g_global_buf, node->data, copy_len);
            goto skip_large;
        }
        
copy_block:
        /* This goto tests flow sensitivity */
        __builtin_memmove(g_global_buf + 32, node->data, copy_len - 16);
        
skip_large:
        /* Continue with normal flow */
        node->left = create_ast(depth - 1, size / 2);
        node->right = create_ast(depth - 1, size / 2);
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void process_with_goto(ast_node_t *a, ast_node_t *b) {
    if (!a || !b || !a->data || !b->data) return;
    
    volatile size_t min_len = a->len < b->len ? a->len : b->len;
    volatile char temp_buf[256];
    
    /* Jump into memory operation block */
    if (min_len > 128) {
        goto large_op;
    }
    
    /* Small operation path */
    __builtin_memcpy(temp_buf, a->data, min_len);
    goto after_copy;
    
large_op:
    /* Large operation with memmove */
    __builtin_memmove(temp_buf, a->data, min_len);
    
    /* Jump out to different operation */
    if (min_len > 192) {
        goto complex_mix;
    }
    
after_copy:
    /* Continue with memset */
    __builtin_memset(b->data, 0xCC, b->len > 64 ? 64 : b->len);
    return;
    
complex_mix:
    /* Mixed operations */
    __builtin_memcpy(b->data, temp_buf, 64);
    __builtin_memset(temp_buf, 0xDD, sizeof(temp_buf));
}

/* OpenMP parallel memory dispatcher */
static uint64_t parallel_memory_ops(ast_node_t **nodes, int count) {
    volatile uint64_t hash = 0;
    
    #pragma omp parallel reduction(+:hash)
    {
        int tid = omp_get_thread_num();
        volatile char thread_buf[512];
        
        /* Each thread uses builtins */
        __builtin_memset(thread_buf, tid, sizeof(thread_buf));
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i] && nodes[i]->data) {
                /* Mix of memory operations */
                volatile size_t op_len = nodes[i]->len > 512 ? 512 : nodes[i]->len;
                
                if (i % 3 == 0) {
                    __builtin_memcpy(thread_buf, nodes[i]->data, op_len);
                } else if (i % 3 == 1) {
                    __builtin_memmove(thread_buf + 128, nodes[i]->data, op_len);
                } else {
                    __builtin_memset(nodes[i]->data, 0xEE, op_len);
                }
                
                /* Compute simple hash */
                for (volatile size_t j = 0; j < op_len && j < 64; j++) {
                    hash += (uint64_t)thread_buf[j] * (i + 1);
                }
            }
        }
        
        /* Final thread-local operation */
        __builtin_memcpy(g_global_buf + tid * 64, thread_buf, 64);
    }
    
    return hash;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize AST structures */
    const int ast_depth = 5;
    const int node_count = 8;
    ast_node_t *nodes[node_count];
    
    for (int i = 0; i < node_count; i++) {
        volatile size_t size = g_mem_size * (i + 1);
        nodes[i] = create_ast(ast_depth, size);
    }
    
    /* Phase 2: Process with goto control flow */
    for (int i = 0; i < node_count - 1; i++) {
        process_with_goto(nodes[i], nodes[i + 1]);
    }
    
    /* Phase 3: Parallel operations */
    uint64_t result_hash = parallel_memory_ops(nodes, node_count);
    
    /* Phase 4: Final verification operations */
    volatile char verify_buf[1024];
    __builtin_memset(verify_buf, 0, sizeof(verify_buf));
    
    /* Chain of memory operations */
    for (int i = 0; i < node_count; i++) {
        if (nodes[i] && nodes[i]->data) {
            volatile size_t len = nodes[i]->len > 256 ? 256 : nodes[i]->len;
            __builtin_memcpy(verify_buf + i * 32, nodes[i]->data, len);
            __builtin_memmove(nodes[i]->data, verify_buf, len);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < node_count; i++) {
        if (nodes[i]) {
            if (nodes[i]->data) free(nodes[i]->data);
            free(nodes[i]);
        }
    }
    
    printf("Test completed. Hash: %llu\n", (unsigned long long)result_hash);
    printf("Global init flag: %d\n", (int)g_init_flag);
    
    return (result_hash != 0) ? 0 : 1;
}
