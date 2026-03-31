/* asan_coverage.c - Comprehensive test for ASAN memory builtin redirection */
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
    int id;
} ASTNode;

/* Global token array */
static char g_token_pool[4096];
static volatile int g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i * 13) & 0xFF);
    }
    
    /* Force early builtin usage in constructor */
    volatile char local_buf[128];
    __builtin_memset(local_buf, 0xAA, sizeof(local_buf));
    __builtin_memcpy(&g_token_pool[1024], local_buf, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_globals(void) {
    /* Final memory operation in destructor */
    volatile char final_buf[32];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with goto control flow */
static ASTNode* create_ast(int depth, int* node_id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*node_id)++;
    node->left = NULL;
    node->right = NULL;
    
    /* Initialize node data with builtin memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Pattern fill using volatile-controlled size */
    size_t fill_size = g_mem_size % 128;
    for (size_t i = 0; i < fill_size && i < sizeof(node->data); i++) {
        node->data[i] = (char)((node->id + i) & 0xFF);
    }
    
    /* Goto-based control flow around memmove */
    int use_memmove = (depth % 3 == 0);
    
    if (use_memmove) {
        goto memmove_block;
    } else {
        goto normal_block;
    }
    
memmove_block:
    {
        /* Create overlapping memory regions for memmove */
        char temp_buf[512];
        volatile int offset = 32;
        
        __builtin_memcpy(temp_buf, node->data, sizeof(node->data));
        
        /* Critical: goto jumps into memmove context */
        if (depth > 2) {
            goto overlapping_move;
        }
        
        __builtin_memmove(node->data + offset, node->data, 
                         sizeof(node->data) - offset);
        goto after_memmove;
        
    overlapping_move:
        /* Overlapping move with goto entry */
        __builtin_memmove(node->data, node->data + 16, 
                         sizeof(node->data) - 16);
    }
    
after_memmove:
    /* Jump back to normal flow */
    goto normal_block;

normal_block:
    /* Recursive creation with varied memory operations */
    node->left = create_ast(depth - 1, node_id);
    node->right = create_ast(depth - 2, node_id);
    
    /* Copy between nodes if both exist */
    if (node->left && node->right) {
        volatile size_t copy_len = g_mem_size % 128;
        if (copy_len > sizeof(node->left->data)) {
            copy_len = sizeof(node->left->data);
        }
        
        __builtin_memcpy(node->right->data, node->left->data, copy_len);
        
        /* Another goto jump */
        if (node->id % 7 == 0) {
            goto extra_memset;
        }
    }
    
    return node;
    
extra_memset:
    /* Additional memset via goto */
    __builtin_memset(node->data + 64, 0xCC, 32);
    return node;
}

/* Parallel memory dispatch function */
static void parallel_memory_ops(int iterations) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char local_buffer[256];
        volatile int buffer_size = (thread_id * 32 + 64) % 256;
        
        /* Each thread uses all three builtins */
        __builtin_memset(local_buffer, thread_id, buffer_size);
        
        #pragma omp for
        for (int i = 0; i < iterations; i++) {
            char src[128], dst[128];
            volatile int op_size = (i * 17 + thread_id * 13) % 128;
            
            /* Pattern initialization */
            for (int j = 0; j < op_size; j++) {
                src[j] = (char)((i + j + thread_id) & 0xFF);
            }
            
            /* Use all three memory builtins */
            __builtin_memcpy(dst, src, op_size);
            
            if (i % 3 == 0) {
                __builtin_memset(dst + op_size/2, 0xAA, op_size/4);
            }
            
            if (i % 5 == 0) {
                /* Create overlap for memmove */
                __builtin_memmove(dst + 8, dst, op_size - 8);
            }
            
            /* Copy back to global pool (thread-safe region) */
            #pragma omp critical
            {
                size_t copy_pos = (g_token_idx++) % 
                                 (sizeof(g_token_pool) - op_size);
                __builtin_memcpy(&g_token_pool[copy_pos], dst, op_size);
            }
        }
        
        /* Final per-thread memmove */
        char thread_final[64];
        __builtin_memset(thread_final, 0xEE, sizeof(thread_final));
        __builtin_memmove(local_buffer, thread_final, 32);
    }
}

/* Hash calculation for verification */
static unsigned long calculate_hash(const char* data, size_t len) {
    unsigned long hash = 5381;
    for (size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + (unsigned char)data[i];
    }
    return hash;
}

int main(void) {
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Phase 1: Recursive AST creation */
    int node_id = 1;
    ASTNode* root = create_ast(5, &node_id);
    
    /* Phase 2: Parallel memory operations */
    parallel_memory_ops(100);
    
    /* Phase 3: AST traversal and memory operations */
    unsigned long total_hash = 0;
    ASTNode* nodes[64];
    int node_count = 0;
    
    /* Collect nodes for processing */
    nodes[node_count++] = root;
    for (int i = 0; i < node_count && i < 63; i++) {
        if (nodes[i]->left) nodes[node_count++] = nodes[i]->left;
        if (nodes[i]->right) nodes[node_count++] = nodes[i]->right;
        
        /* Process node data */
        volatile size_t proc_len = g_mem_size % sizeof(nodes[i]->data);
        char temp_buf[256];
        
        __builtin_memcpy(temp_buf, nodes[i]->data, proc_len);
        
        /* Conditional memmove with goto */
        if (nodes[i]->id % 4 == 0) {
            goto do_memmove;
        }
        
        __builtin_memset(temp_buf + proc_len/2, nodes[i]->id, 16);
        goto after_conditional;
        
    do_memmove:
        __builtin_memmove(temp_buf + 8, temp_buf, proc_len - 8);
        
    after_conditional:
        total_hash += calculate_hash(temp_buf, proc_len);
    }
    
    /* Phase 4: Global pool processing */
    volatile size_t pool_samples = 8;
    for (size_t i = 0; i < pool_samples; i++) {
        size_t offset = (i * 511) % (sizeof(g_token_pool) - 64);
        total_hash += calculate_hash(&g_token_pool[offset], 64);
    }
    
    /* Cleanup */
    for (int i = 0; i < node_count; i++) {
        free(nodes[i]);
    }
    
    printf("Test completed. Total hash: %lu\n", total_hash);
    printf("Expected non-zero hash verification: %s\n", 
           total_hash != 0 ? "PASS" : "FAIL");
    
    return total_hash == 0 ? 1 : 0;
}
