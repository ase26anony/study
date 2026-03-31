/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
};

/* Global token array */
static char g_tokens[][32] = {
    "token1", "token2", "token3", "token4",
    "token5", "token6", "token7", "token8"
};

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_env(void) {
    volatile char buffer[128];
    /* Force __builtin_memset initialization */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    printf("[constructor] Initialized ASAN environment\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_env(void) {
    volatile char buffer[64];
    /* Force __builtin_memcpy in destructor */
    char src[] = "cleanup_data";
    __builtin_memcpy(buffer, src, sizeof(src));
    printf("[destructor] Cleaned up ASAN environment\n");
}

/* Recursive function with memory operations */
static struct ASTNode* build_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    struct ASTNode* node = malloc(sizeof(struct ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset for initialization */
    __builtin_memset(node, 0, sizeof(struct ASTNode));
    
    node->id = (*counter)++;
    
    /* Copy token data using __builtin_memcpy */
    int token_idx = node->id % 8;
    __builtin_memcpy(node->data, g_tokens[token_idx], 
                     sizeof(g_tokens[token_idx]));
    
    /* Recursive calls */
    node->left = build_ast(depth - 1, counter);
    node->right = build_ast(depth - 1, counter);
    
    return node;
}

/* Function with goto statements around __builtin_memmove */
static void memmove_with_goto(char* dest, char* src, size_t n) {
    int use_memmove = g_use_memmove;
    
    if (use_memmove) {
        goto do_memmove;
    } else {
        goto do_memcpy;
    }

do_memmove:
    /* This should trigger the BUILT_IN_MEMMOVE case */
    __builtin_memmove(dest, src, n);
    goto finish;
    
do_memcpy:
    __builtin_memcpy(dest, src, n);
    goto finish;
    
finish:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char buffer1[256];
        char buffer2[256];
        
        /* Each thread uses different builtins */
        switch (tid % 3) {
            case 0:
                __builtin_memset(buffer1, tid, sizeof(buffer1));
                break;
            case 1:
                __builtin_memcpy(buffer1, g_tokens[tid % 8], 32);
                break;
            case 2:
                __builtin_memmove(buffer1, buffer2, 128);
                break;
        }
        
        #pragma omp barrier
        
        /* Cross-thread memory operation */
        if (tid == 0) {
            for (int i = 1; i < omp_get_num_threads(); i++) {
                /* Simulate inter-thread data transfer */
                __builtin_memcpy(buffer2, buffer1, 64);
            }
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Initialize and build AST */
    int counter = 0;
    struct ASTNode* root = build_ast(3, &counter);
    
    if (!root) {
        fprintf(stderr, "Failed to build AST\n");
        return 1;
    }
    
    /* Phase 2: Test memory operations between AST nodes */
    char temp_buffer[512];
    size_t op_size = g_mem_size;
    
    /* Test all three builtins with volatile size */
    __builtin_memset(temp_buffer, 0xCC, op_size);
    
    if (root->left) {
        __builtin_memcpy(root->right->data, root->left->data, 256);
    }
    
    /* Test memmove with goto */
    memmove_with_goto(temp_buffer + 128, temp_buffer, op_size / 2);
    
    /* Phase 3: Parallel operations */
    printf("Launching parallel memory operations\n");
    parallel_mem_ops();
    
    /* Phase 4: Compute verification hash */
    unsigned long hash = 0;
    struct ASTNode* nodes[100];
    int node_count = 0;
    
    /* Collect nodes for verification */
    nodes[node_count++] = root;
    if (root->left) nodes[node_count++] = root->left;
    if (root->right) nodes[node_count++] = root->right;
    if (root->left && root->left->left) nodes[node_count++] = root->left->left;
    
    /* Compute simple hash from node data */
    for (int i = 0; i < node_count; i++) {
        for (int j = 0; j < 256 && nodes[i]->data[j]; j++) {
            hash = (hash * 31 + nodes[i]->data[j]) % 1000000007;
        }
        hash = (hash + nodes[i]->id * 17) % 1000000007;
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(root->left->left);
    free(root->left->right);
    free(root->right->left);
    free(root->right->right);
    free(root->left);
    free(root->right);
    free(root);
    
    return 0;
}
