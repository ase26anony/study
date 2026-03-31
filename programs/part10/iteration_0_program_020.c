/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* AST-like recursive structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static volatile size_t token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize token pool with pattern */
    for (size_t i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)(i % 256);
    }
    
    /* Use builtins in constructor */
    __builtin_memset(token_pool + 1024, 0xAA, 128);
    __builtin_memcpy(token_pool + 1152, token_pool, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    /* Final builtin usage in destructor */
    volatile char cleanup_buf[128];
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
    __builtin_memcpy(cleanup_buf, token_pool + 2048, 32);
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast_node(size_t depth) {
    if (depth == 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy pattern data */
    size_t copy_len = (depth * 16) % sizeof(node->data);
    if (copy_len > 0) {
        __builtin_memcpy(node->data, token_pool + token_index, copy_len);
        token_index = (token_index + copy_len) % sizeof(token_pool);
    }
    
    node->size = copy_len;
    node->left = create_ast_node(depth - 1);
    node->right = create_ast_node(depth - 1);
    
    return node;
}

/* Function with goto jumps around memmove */
static void goto_memmove_test(char* dest, char* src, size_t len) {
    int use_memmove = volatile_flag;
    
    if (use_memmove) {
        goto do_copy;
    } else {
        goto skip_copy;
    }
    
do_copy:
    {
        char temp_buf[256];
        /* Force memmove with overlapping regions */
        __builtin_memmove(temp_buf, src, len);
        __builtin_memmove(dest, temp_buf, len);
    }
    goto after_copy;
    
skip_copy:
    __builtin_memset(dest, 0, len);
    
after_copy:
    /* Additional builtin after label */
    if (len > 32) {
        __builtin_memcpy(dest + 16, src + 8, 16);
    }
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        char local_buf[512];
        size_t local_len = volatile_len % 256;
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, 0, sizeof(local_buf));
        
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            char temp[64];
            size_t op_len = (i * 7) % 64;
            
            /* Mix of builtins in loop */
            if (i % 3 == 0) {
                __builtin_memcpy(temp, token_pool + i * 8, op_len);
            } else if (i % 3 == 1) {
                __builtin_memset(temp, i, op_len);
            } else {
                __builtin_memmove(temp, temp + 4, op_len - 4);
            }
            
            /* Copy back to shared pool */
            #pragma omp critical
            {
                __builtin_memcpy(token_pool + (i * 8) % sizeof(token_pool), 
                               temp, op_len);
            }
        }
    }
}

/* Complex memory operation sequence */
static size_t execute_memory_sequence(void) {
    size_t hash = 0;
    char sequence_buf[1024];
    
    /* Stage 1: memset */
    __builtin_memset(sequence_buf, 0xCC, sizeof(sequence_buf));
    
    /* Stage 2: memcpy with volatile length */
    size_t copy_len = volatile_len % 512;
    __builtin_memcpy(sequence_buf + 128, token_pool, copy_len);
    
    /* Stage 3: memmove with overlap */
    __builtin_memmove(sequence_buf + 256, sequence_buf + 128, 128);
    
    /* Stage 4: goto-based memmove test */
    goto_memmove_test(sequence_buf + 384, sequence_buf + 256, 64);
    
    /* Calculate verification hash */
    for (size_t i = 0; i < sizeof(sequence_buf); i++) {
        hash = (hash * 31) + sequence_buf[i];
    }
    
    return hash;
}

int main(void) {
    size_t final_hash = 0;
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: AST operations */
    ASTNode* root = create_ast_node(4);
    if (root) {
        /* Copy between AST nodes */
        if (root->left && root->right) {
            size_t copy_size = root->left->size < root->right->size ? 
                              root->left->size : root->right->size;
            __builtin_memcpy(root->left->data, root->right->data, copy_size);
        }
        
        /* Recursive cleanup would go here */
        free(root);
    }
    
    /* Phase 2: Parallel operations */
    parallel_memory_ops();
    
    /* Phase 3: Complex sequence */
    final_hash = execute_memory_sequence();
    
    /* Final builtin calls to ensure cache initialization */
    char final_buf[256];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
    __builtin_memcpy(final_buf + 64, token_pool + 512, 128);
    __builtin_memmove(final_buf + 128, final_buf + 64, 64);
    
    /* Add final buffer to hash */
    for (size_t i = 0; i < sizeof(final_buf); i++) {
        final_hash = (final_hash * 31) + final_buf[i];
    }
    
    printf("Test completed. Final hash: %zu\n", final_hash);
    printf("Built-in calls executed: memcpy, memset, memmove\n");
    
    return 0;
}
