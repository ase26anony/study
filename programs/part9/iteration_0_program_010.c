/* asan_coverage.c - Comprehensive test for ASAN memory built-in redirection */
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

/* Global token array */
static char g_token_pool[4096];
static volatile size_t g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Initialize token pool with pattern */
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i * 13) & 0xFF);
    }
    printf("Constructor: ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Destructor: ASAN environment cleaned up\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_recursive(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with goto for flow control */
    size_t copy_len = (depth * 16) % 256;
    
    /* Goto-based control flow around memcpy */
    if (copy_len > 128) {
        goto large_copy;
    } else {
        goto small_copy;
    }
    
large_copy:
    __builtin_memcpy(node->data, base_data, copy_len);
    goto copy_done;
    
small_copy:
    __builtin_memcpy(node->data, base_data + 128, copy_len);
    goto copy_done;
    
copy_done:
    node->size = copy_len;
    
    /* Recursive creation with memmove between nodes */
    node->left = create_ast_recursive(depth - 1, base_data + 64);
    if (node->left && depth > 2) {
        /* Move data between nodes using __builtin_memmove */
        volatile size_t move_size = (depth % 32) * 8;
        __builtin_memmove(node->data + 32, node->left->data, 
                         move_size < 224 ? move_size : 224);
    }
    
    node->right = create_ast_recursive(depth - 2, base_data + 128);
    
    return node;
}

/* Parallel memory dispatch logic */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buf[512];
        volatile size_t op_size = g_mem_size + thread_id * 16;
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(local_buf, thread_id, op_size % 512);
                break;
            case 1: {
                char src_buf[512];
                __builtin_memset(src_buf, thread_id + 64, 256);
                __builtin_memcpy(local_buf, src_buf, op_size % 256);
                break;
            }
            case 2: {
                char overlap_buf[768];
                __builtin_memset(overlap_buf, thread_id + 128, 384);
                /* Overlapping memmove */
                __builtin_memmove(overlap_buf + 128, overlap_buf, 256);
                __builtin_memcpy(local_buf, overlap_buf + 128, op_size % 256);
                break;
            }
        }
        
        /* Synchronize and combine results */
        #pragma omp barrier
        
        /* Global memory operation with goto */
        volatile int do_global_op = (thread_id == 0);
        if (!do_global_op) goto skip_global;
        
        __builtin_memcpy(g_token_pool + g_token_idx, local_buf, 64);
        g_token_idx = (g_token_idx + 64) % 4096;
        
    skip_global:
        /* Additional memmove with flow control */
        if (thread_id % 2 == 0) {
            goto even_thread;
        } else {
            goto odd_thread;
        }
        
    even_thread:
        __builtin_memmove(local_buf + 128, local_buf, 64);
        goto thread_done;
        
    odd_thread:
        __builtin_memmove(local_buf, local_buf + 64, 64);
        goto thread_done;
        
    thread_done:
        ; /* Empty statement for label */
    }
}

/* Complex token array initialization */
static void initialize_token_array(char* tokens, size_t count) {
    volatile size_t block_size = 32;
    
    for (size_t i = 0; i < count; i += block_size) {
        size_t remaining = count - i;
        size_t op_size = remaining < block_size ? remaining : block_size;
        
        /* Pattern initialization with builtins */
        if (i % 3 == 0) {
            __builtin_memset(tokens + i, (int)(i & 0xFF), op_size);
        } else if (i % 3 == 1) {
            /* Copy from previous block with overlap */
            if (i >= block_size) {
                __builtin_memcpy(tokens + i, tokens + i - block_size, 
                               op_size < block_size ? op_size : block_size);
            }
        } else {
            /* Move within array */
            if (i >= 64) {
                __builtin_memmove(tokens + i, tokens + i - 64, 
                                op_size < 64 ? op_size : 64);
            }
        }
    }
}

/* Compute verification hash */
static unsigned long compute_hash(const char* data, size_t len) {
    unsigned long hash = 5381;
    for (size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + (unsigned char)data[i];
    }
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Initialize complex data structures */
    char* dynamic_buffer = (char*)malloc(2048);
    if (!dynamic_buffer) return 1;
    
    initialize_token_array(dynamic_buffer, 2048);
    
    /* Phase 2: Create recursive AST */
    ASTNode* root = create_ast_recursive(5, dynamic_buffer);
    
    /* Phase 3: Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Phase 4: Additional built-in calls with volatile control */
    volatile int use_memcpy = 1;
    char test_buf1[1024], test_buf2[1024];
    
    __builtin_memset(test_buf1, 0xAA, sizeof(test_buf1));
    
    if (use_memcpy) {
        goto do_copy;
    } else {
        goto do_set;
    }
    
do_copy:
    __builtin_memcpy(test_buf2, test_buf1, g_mem_size % 1024);
    goto mem_ops_done;
    
do_set:
    __builtin_memset(test_buf2, 0x55, g_mem_size % 1024);
    goto mem_ops_done;
    
mem_ops_done:
    /* Overlapping memmove test */
    __builtin_memmove(test_buf1 + 256, test_buf1, 512);
    
    /* Phase 5: Compute verification result */
    unsigned long hash1 = compute_hash(dynamic_buffer, 2048);
    unsigned long hash2 = compute_hash(g_token_pool, 4096);
    unsigned long hash3 = compute_hash(test_buf1, 1024);
    
    printf("Verification hashes:\n");
    printf("  Dynamic buffer: %lu\n", hash1);
    printf("  Token pool: %lu\n", hash2);
    printf("  Test buffer: %lu\n", hash3);
    printf("  Final sum: %lu\n", hash1 + hash2 + hash3);
    
    /* Cleanup */
    free(dynamic_buffer);
    
    /* Free AST recursively */
    ASTNode* nodes[32];
    nodes[0] = root;
    int node_count = 1;
    
    while (node_count > 0) {
        ASTNode* current = nodes[--node_count];
        if (current->left) nodes[node_count++] = current->left;
        if (current->right) nodes[node_count++] = current->right;
        free(current);
    }
    
    printf("ASAN test completed successfully\n");
    return 0;
}
