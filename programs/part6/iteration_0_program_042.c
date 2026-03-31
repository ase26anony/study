/* asan_coverage.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    uint32_t hash;
} ASTNode;

/* Global token array */
static char g_tokens[8][32] = {
    "token_alpha", "token_beta", "token_gamma",
    "token_delta", "token_epsilon", "token_zeta",
    "token_eta", "token_theta"
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_env(void) {
    /* Force initialization of ASAN runtime */
    volatile char init_buf[16];
    __builtin_memset(init_buf, 0, sizeof(init_buf));
    printf("[Constructor] ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_env(void) {
    volatile char cleanup_buf[8];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
    printf("[Destructor] ASAN cleanup completed\n");
}

/* Recursive parser with memory operations */
static ASTNode* build_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data with builtin memcpy */
    int token_idx = depth % 8;
    __builtin_memcpy(node->data, g_tokens[token_idx], 
                     strlen(g_tokens[token_idx]) + 1);
    
    /* Build children recursively */
    node->left = build_ast(depth + 1, max_depth);
    node->right = build_ast(depth + 2, max_depth);
    
    return node;
}

/* Complex memory operation with goto flow control */
static void process_with_goto(ASTNode* dest, ASTNode* src) {
    volatile int use_memmove = 1;
    
    /* Jump into memory operation block */
    if (use_memmove) goto do_memmove;
    
    normal_path:
        __builtin_memcpy(dest->data, src->data, sizeof(dest->data));
        return;
    
    do_memmove:
        /* This tests flow-sensitivity of asan_memfn_rtls retrieval */
        __builtin_memmove(dest->data, src->data, 
                         g_mem_size % sizeof(dest->data));
        
        /* Jump out of block */
        if (dest->hash > 1000) goto normal_path;
        
        /* Additional builtin usage */
        volatile char temp_buf[128];
        __builtin_memset(temp_buf, dest->hash & 0xFF, sizeof(temp_buf));
        
        /* Copy back with overlap */
        __builtin_memmove(dest->data + 32, temp_buf + 64, 64);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes)
    for (i = 0; i < count; i++) {
        volatile size_t local_size = g_mem_size;
        
        if (nodes[i] && nodes[(i + 1) % count]) {
            /* Force ASAN to handle parallel builtins */
            __builtin_memcpy(nodes[i]->data, 
                           nodes[(i + 1) % count]->data,
                           local_size % 128);
            
            /* Clear part of buffer */
            __builtin_memset(nodes[i]->data + 64, i, 32);
            
            /* Overlapping move */
            __builtin_memmove(nodes[i]->data + 32,
                            nodes[i]->data + 16,
                            48);
        }
    }
}

/* Calculate hash for verification */
static uint32_t calculate_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    uint32_t hash = 2166136261u;
    volatile char* data = node->data;
    
    /* Process data with builtin awareness */
    for (size_t i = 0; i < sizeof(node->data) && data[i]; i++) {
        hash ^= data[i];
        hash *= 16777619u;
    }
    
    node->hash = hash;
    return hash + calculate_ast_hash(node->left) 
                + calculate_ast_hash(node->right);
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Build recursive structure */
    ASTNode* root = build_ast(0, 4);
    if (!root) {
        fprintf(stderr, "Failed to build AST\n");
        return 1;
    }
    
    /* Create copy for memmove operations */
    ASTNode* copy = (ASTNode*)malloc(sizeof(ASTNode));
    if (!copy) {
        free(root);
        return 1;
    }
    
    /* Initialize copy with builtin */
    __builtin_memset(copy, 0, sizeof(ASTNode));
    
    /* Test goto flow control with memory ops */
    process_with_goto(copy, root);
    
    /* Create array for parallel operations */
    ASTNode* node_array[8];
    node_array[0] = root;
    node_array[1] = copy;
    
    for (int i = 2; i < 8; i++) {
        node_array[i] = build_ast(i, 3);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(node_array, 8);
    
    /* Calculate and print verification hash */
    uint32_t total_hash = 0;
    for (int i = 0; i < 8; i++) {
        if (node_array[i]) {
            total_hash ^= calculate_ast_hash(node_array[i]);
        }
    }
    
    printf("Verification hash: 0x%08X\n", total_hash);
    printf("Total AST nodes processed: 8\n");
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        free(node_array[i]);
    }
    
    printf("=== Test completed successfully ===\n");
    return 0;
}
