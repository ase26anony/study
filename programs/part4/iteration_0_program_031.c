/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor attribute function */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 256 + (rand() % 768);
}

/* Destructor attribute function */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    printf("Destructor: ASAN test completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with volatile size */
    size_t copy_size = (g_mem_size % 64) + 1;
    __builtin_memcpy(node->data, base_data, copy_size);
    node->size = copy_size;
    
    /* Create children recursively */
    node->left = create_ast(depth - 1, base_data);
    node->right = create_ast(depth - 1, base_data);
    
    return node;
}

/* Function with goto and __builtin_memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int use_memmove = 1;
    
    /* Jump into memory operation block */
    if (src->size > 32) {
        goto do_memmove;
    }
    
    /* Normal path */
    __builtin_memcpy(dst->data, src->data, src->size);
    return;
    
do_memmove:
    /* Overlapping memory operation with goto */
    char buffer[128];
    __builtin_memcpy(buffer, src->data, src->size);
    
    /* Jump out of block */
    if (use_memmove) {
        goto apply_memmove;
    }
    
    __builtin_memset(dst->data, 0, sizeof(dst->data));
    return;
    
apply_memmove:
    /* Force __builtin_memmove usage */
    __builtin_memmove(dst->data, buffer, src->size);
    
    /* Jump back */
    if (dst->size > src->size) {
        goto do_memmove;
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Volatile-controlled memory operations */
            volatile size_t local_size = g_mem_size % 64;
            
            /* Mix of memory built-ins */
            if (i % 3 == 0) {
                __builtin_memset(nodes[i]->data, i, local_size);
            } else if (i % 3 == 1) {
                if (i > 0) {
                    __builtin_memcpy(nodes[i]->data, nodes[i-1]->data, local_size);
                }
            } else {
                char temp[64];
                __builtin_memcpy(temp, nodes[i]->data, local_size);
                __builtin_memmove(nodes[i]->data + 10, temp, local_size - 10);
            }
        }
    }
}

/* Multi-stage initialization */
static void initialize_token_array(char tokens[][64], int rows) {
    for (int i = 0; i < rows; i++) {
        /* Use all three built-ins in initialization */
        __builtin_memset(tokens[i], 0, 64);
        
        char pattern[32];
        __builtin_memset(pattern, 'A' + (i % 26), 32);
        
        __builtin_memcpy(tokens[i], pattern, 32);
        
        if (i > 0) {
            __builtin_memmove(tokens[i] + 16, tokens[i-1], 16);
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Stage 1: Initialize complex data structures */
    char token_array[8][64];
    initialize_token_array(token_array, 8);
    
    /* Stage 2: Create recursive AST */
    ASTNode* root = create_ast(4, "BaseASTData");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Stage 3: Process with goto flow control */
    ASTNode* copy_root = (ASTNode*)malloc(sizeof(ASTNode));
    if (copy_root) {
        __builtin_memset(copy_root, 0, sizeof(ASTNode));
        process_with_goto(root, copy_root);
    }
    
    /* Stage 4: Create node array for parallel processing */
    ASTNode* node_array[16];
    node_array[0] = root;
    for (int i = 1; i < 16; i++) {
        node_array[i] = create_ast(2, token_array[i % 8]);
    }
    
    /* Stage 5: Execute OpenMP parallel memory operations */
    #ifdef _OPENMP
    parallel_memory_ops(node_array, 16);
    #endif
    
    /* Stage 6: Compute verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 64; j++) {
            hash = (hash * 31 + token_array[i][j]) % 1000000007;
        }
    }
    
    if (root) {
        for (int i = 0; i < 64; i++) {
            hash = (hash * 31 + root->data[i]) % 1000000007;
        }
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real ASAN, memory leaks would be reported */
    
    return 0;
}
