/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_array[1024];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(token_array, 0xAA, sizeof(token_array));
    printf("Constructor: Initialized token array\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use builtin memcpy in destructor */
    char temp[64];
    __builtin_memcpy(temp, token_array, sizeof(temp));
    printf("Destructor: Cleaned up %d bytes\n", (int)sizeof(temp));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtin memset */
    __builtin_memset(node->data, id % 256, sizeof(node->data));
    node->id = id;
    
    /* Create children with different patterns */
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    /* Copy data between nodes if both children exist */
    if (node->left && node->right) {
        /* Use goto for control flow edge case */
        if (id % 3 == 0) goto copy_block;
        
        /* Normal path */
        __builtin_memcpy(node->data, node->left->data, 16);
        
        copy_block:
        /* Jump target with memmove */
        __builtin_memmove(node->right->data, node->data, 16);
        
        /* Jump back out */
        if (id % 5 == 0) goto skip_additional;
    }
    
    skip_additional:
    return node;
}

/* Complex memory operation with goto patterns */
static void complex_memory_ops(void) {
    char buffer1[128];
    char buffer2[128];
    int i;
    
    /* Initialize with volatile-controlled length */
    int len = volatile_len;
    
    /* Pattern 1: goto into memcpy block */
    if (len > 32) goto pattern2;
    
    __builtin_memset(buffer1, 0xCC, len);
    
    pattern2:
    /* Use builtin memcpy with goto */
    for (i = 0; i < 3; i++) {
        if (i == 1) goto do_memcpy;
        
        __builtin_memset(buffer2, i, 64);
        continue;
        
        do_memcpy:
        __builtin_memcpy(buffer1, buffer2, 32);
    }
    
    /* Pattern 3: nested goto with memmove */
    if (len < 100) goto pattern3_end;
    
    {
        char temp[64];
        __builtin_memmove(temp, buffer1, 48);
        goto pattern3_skip;
        
        pattern3_end:
        __builtin_memmove(buffer2, buffer1, 24);
        
        pattern3_skip:
        /* Empty but reachable */
    }
}

/* OpenMP parallel section */
static void parallel_memory_dispatch(void) {
    int i;
    char local_buffers[8][64];
    
    #pragma omp parallel for
    for (i = 0; i < 8; i++) {
        /* Each thread uses builtins */
        __builtin_memset(local_buffers[i], i + 'A', 32);
        
        /* Conditional memcpy */
        if (i % 2 == 0) {
            __builtin_memcpy(local_buffers[i] + 32, 
                           local_buffers[(i + 1) % 8], 16);
        } else {
            __builtin_memmove(local_buffers[i] + 16,
                            local_buffers[i], 16);
        }
        
        /* Update token array (shared memory) */
        #pragma omp critical
        {
            __builtin_memcpy(token_array + i * 32,
                           local_buffers[i], 32);
            token_index += 32;
        }
    }
}

/* Calculate hash of token array */
static unsigned int calculate_hash(void) {
    unsigned int hash = 0;
    int i;
    
    for (i = 0; i < token_index && i < sizeof(token_array); i++) {
        hash = (hash * 31) + (unsigned char)token_array[i];
    }
    
    return hash;
}

/* Main execution flow */
int main(void) {
    ASTNode* root = NULL;
    unsigned int final_hash;
    
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Stage 1: Initialize with volatile operations */
    {
        volatile int init_len = volatile_len % 128 + 32;
        __builtin_memset((void*)volatile_dest, 0x5A, init_len);
        __builtin_memcpy((void*)volatile_src, volatile_dest, init_len);
        __builtin_memmove((void*)volatile_dest + 16, 
                         volatile_src, init_len / 2);
    }
    
    /* Stage 2: Create recursive AST */
    printf("Creating recursive AST structure...\n");
    root = create_ast(4, 1);
    
    /* Stage 3: Complex control flow with gotos */
    printf("Executing complex memory operations...\n");
    complex_memory_ops();
    
    /* Stage 4: OpenMP parallel dispatch */
    printf("Dispatching parallel memory operations...\n");
    #ifdef _OPENMP
    parallel_memory_dispatch();
    #else
    printf("OpenMP not available, skipping parallel section\n");
    #endif
    
    /* Stage 5: Process AST with memory operations */
    if (root) {
        ASTNode* nodes[16];
        int node_count = 0;
        
        /* Collect nodes */
        nodes[node_count++] = root;
        if (root->left) nodes[node_count++] = root->left;
        if (root->right) nodes[node_count++] = root->right;
        
        /* Perform memory operations between nodes */
        for (int i = 0; i < node_count - 1; i++) {
            __builtin_memcpy(nodes[i]->data + 8,
                           nodes[i + 1]->data, 8);
            __builtin_memmove(nodes[i + 1]->data + 16,
                            nodes[i]->data, 8);
        }
        
        /* Cleanup */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    /* Final calculation and verification */
    final_hash = calculate_hash();
    printf("Token array hash: 0x%08X\n", final_hash);
    printf("Total bytes processed: %d\n", token_index);
    
    /* One final builtin call in main */
    char final_check[32];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
    
    printf("=== Test Complete ===\n");
    return (final_hash != 0) ? 0 : 1;
}
