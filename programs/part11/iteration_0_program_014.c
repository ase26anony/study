/* asan_coverage.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char *volatile_dest = NULL;
static volatile char *volatile_src = NULL;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode *left;
    struct ASTNode *right;
    int id;
} ASTNode;

/* Global token array */
static char token_array[256];

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_tokens(void) {
    /* Initialize with pattern */
    for (int i = 0; i < 256; i++) {
        token_array[i] = (char)((i * 13) & 0xFF);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup(void) {
    /* Verify operations by printing checksum */
    unsigned int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += (unsigned char)token_array[i];
    }
    printf("Final checksum: %u\n", sum);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[32];
    for (int i = 0; i < 32; i++) {
        pattern[i] = (char)((id + i) & 0xFF);
    }
    __builtin_memcpy(node->data, pattern, 32);
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (id % 2 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        node->right = create_ast(depth - 1, id * 2 + 1);
        
        create_children:
        /* Jump target with __builtin_memmove */
        if (node->left && node->right) {
            /* Move data between children */
            __builtin_memmove(node->left->data + 16, 
                            node->right->data, 16);
        }
    }
    
    return node;
}

/* Function with complex control flow using goto */
static void process_with_goto(ASTNode *node) {
    if (!node) return;
    
    volatile char buffer[128];
    volatile int offset = 0;
    
    /* Jump into block with memory operation */
    if (node->id % 3 == 0) {
        goto memcpy_block;
    }
    
    /* Normal path */
    __builtin_memset(buffer, 0xA5, sizeof(buffer));
    offset = 32;
    
memcpy_block:
    /* Target label with __builtin_memcpy */
    __builtin_memcpy((char*)buffer + offset, node->data, 32);
    
    /* Jump out of block */
    if (offset > 64) {
        goto finish;
    }
    
    /* More operations */
    __builtin_memmove(buffer, buffer + 16, 64);
    
finish:
    /* Final operation */
    volatile_len = sizeof(buffer);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char local_buf[256];
        char src_buf[256];
        
        /* Initialize source with thread-specific pattern */
        for (int i = 0; i < 256; i++) {
            src_buf[i] = (char)((thread_id * 31 + i) & 0xFF);
        }
        
        /* Use all three builtins in parallel region */
        __builtin_memset(local_buf, thread_id, 256);
        __builtin_memcpy(local_buf + 64, src_buf, 128);
        __builtin_memmove(local_buf, local_buf + 32, 192);
        
        /* Update global token array (with potential races for ASAN to detect) */
        #pragma omp critical
        {
            __builtin_memcpy(token_array + thread_id * 16, 
                           local_buf, 16);
        }
    }
}

/* Multi-stage initialization */
static void initialize_buffers(char **dest, char **src, size_t size) {
    *dest = (char*)malloc(size);
    *src = (char*)malloc(size);
    
    if (*dest && *src) {
        /* Force initialization through builtins */
        __builtin_memset(*dest, 0xCC, size);
        __builtin_memset(*src, 0xAA, size);
        
        /* Partial copy */
        __builtin_memcpy(*dest + size/4, *src, size/2);
    }
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Stage 1: Initialize with volatile variables */
    char *dest, *src;
    size_t size = (size_t)volatile_len * 2;
    
    initialize_buffers(&dest, &src, size);
    
    /* Stage 2: Create recursive AST */
    ASTNode *root = create_ast(4, 1);
    
    /* Stage 3: Process with goto control flow */
    if (root) {
        process_with_goto(root);
        
        /* Copy between AST nodes */
        if (root->left && root->right) {
            __builtin_memcpy(root->right->data, 
                           root->left->data, 32);
        }
    }
    
    /* Stage 4: Parallel operations */
    parallel_memory_ops();
    
    /* Stage 5: Final memory operations with volatile */
    if (dest && src) {
        volatile_dest = dest;
        volatile_src = src;
        
        /* Use all three builtins with volatile pointers */
        __builtin_memcpy((char*)volatile_dest, 
                        (char*)volatile_src, 
                        (size_t)volatile_len);
        
        __builtin_memset((char*)volatile_dest + volatile_len/2, 
                        0x55, 
                        (size_t)volatile_len/2);
        
        __builtin_memmove((char*)volatile_dest, 
                         (char*)volatile_dest + volatile_len/4, 
                         (size_t)volatile_len * 3/4);
    }
    
    /* Cleanup */
    if (root) {
        /* Recursive free */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    free(dest);
    free(src);
    
    printf("Test completed successfully\n");
    return 0;
}
