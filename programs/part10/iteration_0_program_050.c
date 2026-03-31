/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (i % 26) + 'A';
    }
    
    /* Use builtins in constructor to trigger early initialization */
    __builtin_memset(token_pool, 0, 128);
    volatile char* ptr = token_pool + 256;
    __builtin_memcpy(ptr, "CONSTRUCTOR_INIT", 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Use builtins in destructor */
    char buf[128];
    __builtin_memset(buf, 0xFF, sizeof(buf));
    __builtin_memcpy(buf, "DESTRUCTOR_CLEANUP", 18);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in node data */
    char pattern[32];
    __builtin_memset(pattern, 'A' + (id % 26), 31);
    pattern[31] = '\0';
    __builtin_memcpy(node->data, pattern, 31);
    
    node->id = id;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = volatile_flag;
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        node->right = NULL;
        return node;
        
    create_children:
        node->left = create_ast(depth - 1, id * 2);
        node->right = create_ast(depth - 1, id * 2 + 1);
        
        /* Copy data between nodes using builtin_memmove */
        if (node->left && node->right) {
            __builtin_memmove(node->right->data + 64, 
                            node->left->data, 
                            volatile_len % 128);
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with complex control flow and builtins */
static void process_ast(ASTNode* root) {
    if (!root) return;
    
    char buffer[512];
    int use_goto = volatile_flag & 1;
    
    /* First memory operation */
    __builtin_memset(buffer, 0, sizeof(buffer));
    
    if (use_goto) {
        goto copy_operation;
    }
    
    /* Normal path */
    __builtin_memcpy(buffer, root->data, 
                    volatile_len % sizeof(root->data));
    goto process_children;
    
copy_operation:
    /* Alternative path with memmove */
    __builtin_memmove(buffer, root->data, 
                     volatile_len % sizeof(root->data));
    
process_children:
    /* Process children */
    process_ast(root->left);
    process_ast(root->right);
    
    /* Final memory operation */
    if (root->left && root->right) {
        size_t copy_len = (volatile_len % 128) + 64;
        __builtin_memmove(root->left->data, 
                         root->right->data, 
                         copy_len);
    }
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char local_buf[256];
        char src_buf[256];
        
        /* Initialize source buffer */
        for (int i = 0; i < sizeof(src_buf); i++) {
            src_buf[i] = (i + thread_id) % 256;
        }
        
        /* Use all three builtins in parallel region */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        __builtin_memcpy(local_buf + 64, src_buf, 128);
        
        #pragma omp barrier
        
        __builtin_memmove(local_buf, local_buf + 32, 192);
        
        /* Verify by computing checksum */
        unsigned int checksum = 0;
        for (int i = 0; i < sizeof(local_buf); i++) {
            checksum += (unsigned char)local_buf[i];
        }
        
        #pragma omp critical
        {
            printf("Thread %d checksum: %u\n", thread_id, checksum);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize and create AST */
    ASTNode* root = create_ast(4, 1);
    
    /* Phase 2: Process AST with complex control flow */
    process_ast(root);
    
    /* Phase 3: OpenMP parallel operations */
    printf("\nParallel memory operations:\n");
    parallel_memory_operations();
    
    /* Phase 4: Direct builtin usage with volatile control */
    char final_buffer[1024];
    char source_buffer[1024];
    
    /* Initialize source with pattern */
    for (int i = 0; i < sizeof(source_buffer); i++) {
        source_buffer[i] = (i * 7) % 256;
    }
    
    /* Sequence of builtin calls */
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    
    int use_memmove = volatile_flag & 2;
    if (use_memmove) {
        goto use_memmove_path;
    }
    
    __builtin_memcpy(final_buffer, source_buffer, 
                    volatile_len % sizeof(source_buffer));
    goto compute_result;
    
use_memmove_path:
    __builtin_memmove(final_buffer, source_buffer, 
                     volatile_len % sizeof(source_buffer));
    
compute_result:
    /* Compute final verification hash */
    unsigned long long hash = 0;
    for (int i = 0; i < sizeof(final_buffer); i++) {
        hash = hash * 31 + (unsigned char)final_buffer[i];
    }
    
    printf("\nFinal buffer hash: %llu\n", hash);
    
    /* Phase 5: Cleanup */
    free_ast(root);
    
    /* Final builtin usage */
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    
    printf("Test completed successfully.\n");
    return 0;
}
