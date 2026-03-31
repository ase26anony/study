/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    int data;
    char buffer[32];
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_globals(void) {
    g_init_flag = 1;
    printf("Constructor: Global initialization complete\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup(void) {
    printf("Destructor: Cleanup complete\n");
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_tree(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->data = depth;
    
    /* Fill buffer with pattern using __builtin_memcpy */
    char pattern[32];
    for (int i = 0; i < 32; i++) pattern[i] = 'A' + (i % 26);
    __builtin_memcpy(node->buffer, pattern, 32);
    
    /* Recursive creation */
    node->left = create_tree(depth - 1);
    node->right = create_tree(depth - 1);
    
    return node;
}

/* Complex memory operation with goto flow control */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    volatile int use_goto = 1;
    
    if (use_goto) goto copy_block;
    
    normal_path:
        /* Standard memory move */
        __builtin_memmove(dst->buffer, src->buffer, 32);
        return;
    
    copy_block:
        /* Jump into memory operation block */
        if (src && dst) {
            /* Force ASAN to handle this memmove */
            __builtin_memmove(dst->buffer + 16, src->buffer + 8, 16);
        }
        goto normal_path;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            volatile size_t local_size = g_mem_size;
            
            /* Each thread uses builtins with volatile sizes */
            if (nodes[i]) {
                /* Test all three builtins in parallel */
                __builtin_memset(nodes[i]->buffer, tid + '0', 16);
                
                if (i > 0) {
                    __builtin_memcpy(nodes[i]->buffer + 16, 
                                   nodes[i-1]->buffer, 16);
                }
                
                /* Circular shift within buffer */
                char temp[32];
                __builtin_memcpy(temp, nodes[i]->buffer, 32);
                __builtin_memmove(nodes[i]->buffer, temp + 8, 24);
                __builtin_memcpy(nodes[i]->buffer + 24, temp, 8);
            }
        }
    }
}

/* Multi-stage initialization with memory builtins */
static void initialize_data(char* dest, const char* src, size_t len) {
    volatile size_t adjusted_len = len;
    
    /* Stage 1: Clear destination */
    __builtin_memset(dest, 0, adjusted_len);
    
    /* Stage 2: Copy source data */
    __builtin_memcpy(dest, src, adjusted_len);
    
    /* Stage 3: Move data within buffer */
    if (adjusted_len > 16) {
        __builtin_memmove(dest + 8, dest, adjusted_len - 8);
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Create complex token array */
    const char* tokens[] = {"alpha", "beta", "gamma", "delta", "epsilon"};
    char token_buffer[256];
    volatile size_t buffer_size = sizeof(token_buffer);
    
    /* Initialize using all three builtins */
    __builtin_memset(token_buffer, 0, buffer_size);
    
    size_t offset = 0;
    for (int i = 0; i < 5; i++) {
        size_t len = strlen(tokens[i]);
        __builtin_memcpy(token_buffer + offset, tokens[i], len);
        offset += len + 1;
    }
    
    /* Create recursive tree structure */
    ASTNode* root = create_tree(3);
    if (!root) {
        fprintf(stderr, "Failed to create tree\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    ASTNode* nodes[8];
    nodes[0] = root;
    for (int i = 1; i < 8; i++) {
        nodes[i] = create_tree(2);
    }
    
    /* Test goto flow control with memory operations */
    if (root->left && root->right) {
        process_with_goto(root->left, root->right);
    }
    
    /* Execute parallelized memory dispatch */
    parallel_memory_ops(nodes, 8);
    
    /* Multi-stage initialization test */
    char test_buffer[128];
    initialize_data(test_buffer, "TestPattern", 64);
    
    /* Compute verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < 256 && i < buffer_size; i++) {
        hash = (hash * 31) + token_buffer[i];
    }
    
    for (int i = 0; i < 8 && nodes[i]; i++) {
        for (int j = 0; j < 32; j++) {
            hash = (hash * 31) + nodes[i]->buffer[j];
        }
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real ASAN, memory leaks would be reported */
    
    return 0;
}
