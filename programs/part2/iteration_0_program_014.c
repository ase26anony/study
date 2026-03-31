/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char global_tokens[4096];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize global buffer with pattern */
    __builtin_memset(global_tokens, 0xAA, sizeof(global_tokens));
    
    /* Force early built-in usage in constructor */
    char local_buf[128];
    __builtin_memset(local_buf, 0xCC, sizeof(local_buf));
    __builtin_memcpy(&global_tokens[512], local_buf, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Final memory operations in destructor */
    char verify_buf[64];
    __builtin_memset(verify_buf, 0xFF, sizeof(verify_buf));
    __builtin_memcpy(&global_tokens[1024], verify_buf, 64);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* src, size_t depth) {
    if (depth > 3) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile to control length */
    size_t copy_len = volatile_len / (depth + 1);
    if (copy_len > 255) copy_len = 255;
    
    /* Built-in memory operations on AST node */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    if (src) {
        /* Conditional memcpy with goto */
        if (volatile_flag) {
            goto do_copy;
        } else {
            node->data[0] = 'X';
            goto skip_copy;
        }
        
    do_copy:
        __builtin_memcpy(node->data, src, copy_len);
    skip_copy:
        /* memmove within the same structure */
        __builtin_memmove(&node->data[128], node->data, copy_len);
    }
    
    node->size = copy_len;
    
    /* Recursive creation with goto jumps */
    if (depth < 2) {
        node->left = create_ast_node(node->data, depth + 1);
        
        /* Jump around memory operation */
        if (volatile_flag % 2) {
            goto skip_right;
        }
        
        node->right = create_ast_node(node->data + 64, depth + 1);
        goto after_right;
        
    skip_right:
        node->right = NULL;
        __builtin_memset(node->data + 192, 0xDD, 32);
    after_right:
        ; /* Empty statement for label */
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Complex memory operation with goto flow */
static void process_with_goto(ASTNode* node1, ASTNode* node2) {
    if (!node1 || !node2) return;
    
    char temp[256];
    
    /* Goto jumping into memory block */
    if (node1->size > 32) {
        goto large_copy;
    } else {
        __builtin_memset(temp, 0x11, 32);
        goto small_op;
    }
    
large_copy:
    /* This block contains builtin_memmove with goto entry */
    __builtin_memmove(temp, node1->data, node1->size);
    
    /* Jump out to different context */
    if (volatile_flag) {
        goto process_data;
    }
    
small_op:
    __builtin_memcpy(temp + 32, node2->data, 16);
    
process_data:
    /* Final memory operation after goto */
    __builtin_memcpy(node2->data + 128, temp, 64);
}

/* OpenMP parallel memory dispatcher */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char thread_buf[512];
        char shared_buf[1024];
        
        /* Initialize with built-ins */
        __builtin_memset(thread_buf, thread_id, sizeof(thread_buf));
        
        #pragma omp barrier
        
        /* Memory operations in parallel region */
        if (thread_id % 2 == 0) {
            __builtin_memcpy(&shared_buf[thread_id * 64], 
                           thread_buf, 
                           volatile_len % 256);
        } else {
            __builtin_memmove(&shared_buf[thread_id * 32],
                            &thread_buf[128],
                            48);
        }
        
        #pragma omp barrier
        
        /* Final memset in parallel */
        __builtin_memset(&thread_buf[256], 0xEE, 128);
    }
}

/* Calculate verification hash */
static unsigned long compute_hash(const char* data, size_t len) {
    unsigned long hash = 5381;
    for (size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + data[i];
    }
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize and create AST structures */
    ASTNode* root = create_ast_node("Initial data for AST construction", 0);
    ASTNode* child = NULL;
    
    if (root) {
        child = create_ast_node(root->data, 1);
    }
    
    /* Phase 2: Execute goto-based memory operations */
    if (root && child) {
        process_with_goto(root, child);
        
        /* Additional memmove with overlapping regions */
        char overlap_buf[512];
        __builtin_memset(overlap_buf, 0x22, sizeof(overlap_buf));
        __builtin_memmove(overlap_buf + 128, overlap_buf, 256);
        __builtin_memcpy(root->data, overlap_buf, 128);
    }
    
    /* Phase 3: Parallel memory operations */
    parallel_memory_ops();
    
    /* Phase 4: Global buffer operations */
    __builtin_memset(&global_tokens[2048], 0x33, 512);
    __builtin_memcpy(&global_tokens[2560], &global_tokens[2048], 256);
    __builtin_memmove(&global_tokens[3072], &global_tokens[0], 512);
    
    /* Phase 5: Variable-length operations using volatile */
    size_t dynamic_len = volatile_len;
    char* dynamic_buf = malloc(dynamic_len * 2);
    if (dynamic_buf) {
        __builtin_memset(dynamic_buf, 0x44, dynamic_len);
        __builtin_memcpy(dynamic_buf + dynamic_len, dynamic_buf, dynamic_len / 2);
        __builtin_memmove(dynamic_buf, dynamic_buf + dynamic_len / 4, dynamic_len);
        free(dynamic_buf);
    }
    
    /* Verification and output */
    unsigned long total_hash = 0;
    
    if (root) {
        total_hash ^= compute_hash(root->data, root->size);
        if (root->left) {
            total_hash ^= compute_hash(root->left->data, root->left->size);
        }
        free(root);
    }
    
    if (child) {
        total_hash ^= compute_hash(child->data, child->size);
        free(child);
    }
    
    total_hash ^= compute_hash(global_tokens, 1024);
    
    printf("Verification hash: 0x%08lx\n", total_hash);
    printf("Test completed. Check for ASAN instrumentation.\n");
    
    return 0;
}
