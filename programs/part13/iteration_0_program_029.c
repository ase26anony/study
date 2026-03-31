/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    size_t size;
} ASTNode;

/* Global token array */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "test"
};
static const size_t g_num_tokens = sizeof(g_tokens) / sizeof(g_tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_env(void) {
    volatile char buffer[256];
    /* Force early built-in usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 128, g_tokens[0], strlen(g_tokens[0]));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_env(void) {
    volatile int cleanup_flag = 1;
    __builtin_memset(&cleanup_flag, 0, sizeof(cleanup_flag));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(size_t depth, const char* token) {
    if (depth == 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with built-in memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data with built-in memcpy */
    size_t token_len = strlen(token);
    __builtin_memcpy(node->data, token, token_len < 63 ? token_len : 63);
    node->size = token_len;
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_left = (depth % 2 == 0);
        
        if (use_left) {
            goto create_left;
        } else {
            goto create_right;
        }
        
    create_left:
        node->left = create_ast(depth - 1, g_tokens[(depth + 0) % g_num_tokens]);
        goto skip_right;
        
    create_right:
        node->right = create_ast(depth - 1, g_tokens[(depth + 1) % g_num_tokens]);
        
    skip_right:
        /* Built-in memmove between nodes */
        if (node->left && node->right) {
            volatile size_t move_size = sizeof(node->left->data);
            __builtin_memmove(node->left->data, node->right->data, 
                            move_size < 64 ? move_size : 64);
        }
    }
    
    return node;
}

/* Complex memory dispatch with OpenMP */
static void parallel_memory_ops(ASTNode** nodes, size_t count) {
    volatile int thread_id = 0;
    
    #pragma omp parallel private(thread_id)
    {
        thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (size_t i = 0; i < count; i++) {
            volatile char temp_buffer[512];
            size_t buffer_size = g_mem_size % 512;
            
            /* Mixed built-in usage pattern */
            if (i % 3 == 0) {
                __builtin_memset(temp_buffer, thread_id, buffer_size);
                if (nodes[i]) {
                    __builtin_memcpy(nodes[i]->data, temp_buffer, 
                                   buffer_size < 64 ? buffer_size : 64);
                }
            } else if (i % 3 == 1) {
                if (nodes[i] && i + 1 < count && nodes[i + 1]) {
                    __builtin_memmove(nodes[i]->data, nodes[i + 1]->data, 32);
                }
            } else {
                /* Nested memory operations */
                volatile char inner_buf[256];
                __builtin_memset(inner_buf, 0xFF, 128);
                __builtin_memcpy(temp_buffer, inner_buf, 128);
                __builtin_memmove(temp_buffer + 128, temp_buffer, 128);
            }
            
            /* Goto for additional control flow */
            if (thread_id % 2 == 0) {
                goto even_thread;
            } else {
                goto odd_thread;
            }
            
        even_thread:
            __builtin_memset(temp_buffer + 256, 0xCC, 64);
            continue;
            
        odd_thread:
            __builtin_memcpy(temp_buffer + 320, temp_buffer, 64);
            continue;
        }
    }
}

/* Calculate hash from AST */
static uint64_t compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    uint64_t hash = 5381;
    volatile char* data = node->data;
    
    /* Process data with built-in awareness */
    for (size_t i = 0; i < node->size && i < 64; i++) {
        hash = ((hash << 5) + hash) + data[i];
    }
    
    /* Recursive hash computation */
    uint64_t left_hash = compute_ast_hash(node->left);
    uint64_t right_hash = compute_ast_hash(node->right);
    
    /* Combine with memory operation */
    volatile uint64_t combined[3] = {hash, left_hash, right_hash};
    __builtin_memmove(&hash, combined, sizeof(uint64_t));
    
    return hash ^ left_hash ^ right_hash;
}

/* Free AST with memory sanitization */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear memory before free */
    volatile char* data = node->data;
    __builtin_memset(data, 0, sizeof(node->data));
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    free(node);
}

int main(void) {
    const size_t num_nodes = 8;
    ASTNode* nodes[num_nodes];
    uint64_t final_hash = 0;
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Create AST nodes with recursive parsing */
    for (size_t i = 0; i < num_nodes; i++) {
        const char* token = g_tokens[i % g_num_tokens];
        nodes[i] = create_ast(3 + (i % 3), token);
        
        /* Immediate memory operation on creation */
        volatile char init_buf[128];
        __builtin_memset(init_buf, i, sizeof(init_buf));
        if (nodes[i]) {
            __builtin_memcpy(nodes[i]->data + 32, init_buf, 32);
        }
    }
    
    /* Phase 2: Parallel memory operations */
    parallel_memory_ops(nodes, num_nodes);
    
    /* Phase 3: Cross-node memory transfers with goto */
    for (size_t i = 0; i < num_nodes - 1; i++) {
        if (i % 4 == 0) {
            goto block_transfer;
        } else if (i % 4 == 1) {
            goto overlapping_transfer;
        } else {
            goto normal_transfer;
        }
        
    block_transfer:
        if (nodes[i] && nodes[i + 1]) {
            __builtin_memcpy(nodes[i]->data, nodes[i + 1]->data, 48);
        }
        continue;
        
    overlapping_transfer:
        if (nodes[i] && nodes[i + 1]) {
            __builtin_memmove(nodes[i]->data + 16, nodes[i]->data, 32);
        }
        continue;
        
    normal_transfer:
        if (nodes[i] && nodes[i + 1]) {
            volatile char temp[64];
            __builtin_memset(temp, 0xAB, sizeof(temp));
            __builtin_memcpy(nodes[i]->data, temp, 32);
        }
        continue;
    }
    
    /* Phase 4: Compute final result */
    for (size_t i = 0; i < num_nodes; i++) {
        if (nodes[i]) {
            uint64_t node_hash = compute_ast_hash(nodes[i]);
            volatile uint64_t hash_buffer[2] = {final_hash, node_hash};
            __builtin_memmove(&final_hash, hash_buffer, sizeof(uint64_t));
            final_hash ^= node_hash;
        }
    }
    
    /* Final memory operation */
    volatile uint64_t result_buffer[4];
    __builtin_memset(result_buffer, 0, sizeof(result_buffer));
    __builtin_memcpy(result_buffer, &final_hash, sizeof(final_hash));
    __builtin_memmove(result_buffer + 1, result_buffer, sizeof(uint64_t) * 3);
    
    printf("Final hash: 0x%016llx\n", (unsigned long long)final_hash);
    printf("Test completed. Cleaning up...\n");
    
    /* Cleanup */
    for (size_t i = 0; i < num_nodes; i++) {
        free_ast(nodes[i]);
    }
    
    /* Final built-in calls */
    volatile int exit_code = 0;
    __builtin_memset(&exit_code, 0, sizeof(exit_code));
    
    return (int)(final_hash & 0x7FFFFFFF);
}
