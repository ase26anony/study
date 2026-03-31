/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
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
    size_t size;
} ASTNode;

/* Global token array */
static char global_tokens[1024];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(global_tokens, 'A', sizeof(global_tokens));
    printf("Constructor: Initialized global tokens\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use builtin memcpy in destructor */
    char temp[1024];
    __builtin_memcpy(temp, global_tokens, sizeof(global_tokens));
    printf("Destructor: Cleaned up %zu bytes\n", sizeof(global_tokens));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy for node initialization */
    size_t copy_len = (size_t)(volatile_len % 256);
    __builtin_memcpy(node->data, base_data, copy_len);
    node->data[copy_len] = '\0';
    node->size = copy_len;
    
    /* Recursive creation with goto for flow control */
    int create_left = volatile_flag;
    
    if (create_left) {
        node->left = create_ast(depth - 1, "LEFT");
        goto skip_right_init;
    }
    
    node->left = NULL;
    
skip_right_init:
    /* Use builtin memmove with goto */
    char temp[256];
    __builtin_memcpy(temp, node->data, node->size);
    
    if (depth > 2) {
        goto move_operation;
    }
    
    node->right = create_ast(depth - 1, "RIGHT");
    return node;
    
move_operation:
    /* Builtin memmove with overlapping regions */
    __builtin_memmove(node->data + 10, node->data, node->size - 10);
    node->right = create_ast(depth - 1, "MOVED");
    
    return node;
}

/* Complex memory dispatch with OpenMP */
static void parallel_memory_operations(void) {
    int i;
    const int num_ops = 100;
    char* buffers[100];
    
    /* Allocate buffers */
    for (i = 0; i < num_ops; i++) {
        buffers[i] = (char*)malloc(volatile_len + i);
    }
    
    #pragma omp parallel for
    for (i = 0; i < num_ops; i++) {
        /* Force builtin usage in parallel region */
        if (i % 3 == 0) {
            __builtin_memset(buffers[i], i, volatile_len + i);
        } else if (i % 3 == 1) {
            __builtin_memcpy(buffers[i], global_tokens, 
                           (volatile_len + i) < 1024 ? volatile_len + i : 1024);
        } else {
            /* Use goto with memmove */
            if (i > 50) {
                goto do_memmove;
            }
            __builtin_memset(buffers[i], 0xFF, volatile_len + i);
            continue;
            
        do_memmove:
            __builtin_memmove(buffers[i] + 10, buffers[i], 
                            volatile_len + i - 10);
        }
    }
    
    /* Cleanup */
    for (i = 0; i < num_ops; i++) {
        free(buffers[i]);
    }
}

/* Function with goto jumping into memory operation block */
static void goto_memory_test(void) {
    char buffer1[512];
    char buffer2[512];
    int use_memmove = volatile_flag;
    
    __builtin_memset(buffer1, 'X', sizeof(buffer1));
    
    if (!use_memmove) {
        goto copy_operation;
    }
    
    /* Jump into memmove block */
    goto memmove_block;
    
copy_operation:
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    goto end_test;
    
memmove_block:
    /* This is the target of the goto */
    __builtin_memmove(buffer2, buffer1, sizeof(buffer1));
    
end_test:
    /* Verify with another builtin */
    __builtin_memset(buffer1, 0, sizeof(buffer1));
}

/* Main execution flow */
int main(void) {
    ASTNode* root = NULL;
    unsigned long hash = 0;
    int i;
    
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Initialize and parse */
    root = create_ast(4, "ROOT");
    
    /* Phase 2: Parallel memory operations */
    parallel_memory_operations();
    
    /* Phase 3: Goto flow control test */
    goto_memory_test();
    
    /* Phase 4: Direct builtin calls with volatile control */
    char dynamic_buffer[2048];
    volatile int dynamic_size = 512;
    
    for (i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            __builtin_memset(dynamic_buffer, i, dynamic_size);
        } else {
            __builtin_memcpy(dynamic_buffer + 100, dynamic_buffer, dynamic_size - 100);
        }
        
        /* Force memmove on certain iterations */
        if (i == 5) {
            __builtin_memmove(dynamic_buffer, dynamic_buffer + 50, dynamic_size - 50);
        }
    }
    
    /* Calculate verification hash */
    for (i = 0; i < (int)sizeof(dynamic_buffer); i++) {
        hash += (unsigned long)dynamic_buffer[i];
    }
    
    /* Also hash global tokens */
    for (i = 0; i < 1024; i++) {
        hash += (unsigned long)global_tokens[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real code, you'd want to free the AST recursively */
    
    return 0;
}
