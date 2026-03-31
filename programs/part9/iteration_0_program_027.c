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
    printf("Constructor: Token array initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Use builtin memcpy in destructor */
    char temp[16];
    __builtin_memcpy(temp, token_array, 16);
    printf("Destructor: Cleanup completed\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtin memset */
    __builtin_memset(node->data, id % 256, sizeof(node->data));
    node->id = id;
    
    /* Create left child with goto control flow */
    int use_goto = (id % 3 == 0);
    
    if (use_goto) {
        goto create_left;
    }
    
    node->left = create_ast(depth - 1, id * 2);
    goto skip_left;
    
create_left:
    node->left = create_ast(depth - 1, id * 2 + 1);
    
skip_left:
    /* Right child with different pattern */
    node->right = create_ast(depth - 1, id * 3);
    
    return node;
}

/* Copy data between AST nodes */
static void copy_ast_data(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use builtin memcpy with goto jumps */
    int use_memmove = (dest->id % 2 == 0);
    
    if (use_memmove) {
        goto use_move;
    }
    
    /* Regular memcpy */
    __builtin_memcpy(dest->data, src->data, sizeof(dest->data));
    goto copy_done;
    
use_move:
    /* Use memmove (handles overlapping regions) */
    __builtin_memmove(dest->data, src->data, sizeof(dest->data));
    
copy_done:
    /* Recursive copy */
    copy_ast_data(dest->left, src->left);
    copy_ast_data(dest->right, src->right);
}

/* Process AST and compute hash */
static unsigned long process_ast(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 0;
    
    /* Process data with builtin memory access */
    for (int i = 0; i < sizeof(node->data); i++) {
        hash = hash * 31 + node->data[i];
    }
    
    /* Recursive processing */
    hash += process_ast(node->left);
    hash += process_ast(node->right);
    
    return hash;
}

/* Free AST memory */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[128];
        char local_src[128];
        
        /* Initialize with builtin memset */
        __builtin_memset(local_src, thread_id, sizeof(local_src));
        
        /* Different memory operations based on thread */
        switch (thread_id % 3) {
            case 0:
                __builtin_memcpy(local_buf, local_src, volatile_len % 64);
                break;
            case 1:
                __builtin_memset(local_buf, thread_id + 1, volatile_len % 64);
                break;
            case 2:
                __builtin_memmove(local_buf, local_src, volatile_len % 64);
                break;
        }
        
        /* Copy to global volatile buffer */
        #pragma omp critical
        {
            __builtin_memcpy((void*)volatile_dest + thread_id * 16, 
                           local_buf, 16);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize volatile buffers */
    for (int i = 0; i < sizeof(volatile_src); i++) {
        ((char*)volatile_src)[i] = i % 256;
    }
    
    /* Use all three builtins with volatile variables */
    __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, 
                    volatile_len % 128);
    
    __builtin_memset((void*)volatile_dest + 64, 0xCC, 
                    volatile_len % 64);
    
    /* Overlapping memmove */
    __builtin_memmove((void*)volatile_dest + 32, 
                     (void*)volatile_dest, 64);
    
    /* Phase 2: Create and process AST */
    ASTNode* root = create_ast(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create second AST for copying */
    ASTNode* root2 = create_ast(3, 100);
    
    /* Copy data between ASTs */
    copy_ast_data(root2, root);
    
    /* Phase 3: Parallel operations */
    parallel_memory_ops();
    
    /* Phase 4: Compute and verify results */
    unsigned long hash1 = process_ast(root);
    unsigned long hash2 = process_ast(root2);
    
    printf("AST Hash 1: %lu\n", hash1);
    printf("AST Hash 2: %lu\n", hash2);
    
    /* Verify volatile buffer */
    int sum = 0;
    for (int i = 0; i < 128; i++) {
        sum += ((char*)volatile_dest)[i];
    }
    printf("Volatile buffer sum: %d\n", sum);
    
    /* Cleanup */
    free_ast(root);
    free_ast(root2);
    
    printf("Test completed successfully\n");
    return 0;
}
