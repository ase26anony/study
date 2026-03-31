/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int g_volatile_len = 64;
static volatile char g_volatile_char = 'A';

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_token_array[256];

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(g_token_array, 0, sizeof(g_token_array));
    
    /* Fill with pattern using builtin memcpy */
    char pattern[32];
    __builtin_memset(pattern, g_volatile_char, sizeof(pattern));
    __builtin_memcpy(g_token_array, pattern, sizeof(pattern));
    
    printf("Constructor: Initialized token array\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Clear sensitive data with builtin memset */
    __builtin_memset(g_token_array, 0, sizeof(g_token_array));
    printf("Destructor: Cleaned up token array\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtin memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy token data with builtin memcpy */
    int copy_len = (g_volatile_len % 32) + 1;
    __builtin_memcpy(node->data, g_token_array, copy_len);
    
    node->id = id;
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_goto = (id % 3) == 0;
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast_node(depth - 1, id * 2);
        node->right = create_ast_node(depth - 1, id * 2 + 1);
        
        if (use_goto) {
            skip_children:
            node->left = NULL;
            node->right = NULL;
        }
        
        return node;
        
        create_children:
        /* Use builtin memmove for overlapping copy */
        char temp[32];
        __builtin_memcpy(temp, node->data, sizeof(node->data));
        __builtin_memmove(node->data + 8, node->data, 24);
        __builtin_memcpy(node->data, temp, 8);
        goto skip_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    return node;
}

/* Process AST with memory operations */
static int process_ast(ASTNode* node, int* sum) {
    if (!node) return 0;
    
    int local_sum = 0;
    
    /* Process data with builtin memory operations */
    for (int i = 0; i < 32; i++) {
        local_sum += node->data[i];
    }
    
    /* Copy between nodes if siblings exist */
    if (node->left && node->right) {
        /* Use builtin memcpy for sibling data transfer */
        __builtin_memcpy(node->right->data + 16, node->left->data, 16);
        
        /* Use builtin memmove for overlapping regions */
        __builtin_memmove(node->left->data + 8, node->left->data, 24);
    }
    
    *sum += local_sum;
    
    process_ast(node->left, sum);
    process_ast(node->right, sum);
    
    return local_sum;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    /* Clear data before freeing with builtin memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char buffer1[128];
        char buffer2[128];
        
        /* Initialize with builtin memset */
        __builtin_memset(buffer1, thread_id + '0', sizeof(buffer1));
        __builtin_memset(buffer2, 0, sizeof(buffer2));
        
        /* Copy between buffers with builtin memcpy */
        int copy_size = (g_volatile_len % 64) + 64;
        __builtin_memcpy(buffer2, buffer1, copy_size);
        
        /* Move data around with builtin memmove */
        __builtin_memmove(buffer1 + 32, buffer1, 96);
        
        #pragma omp critical
        {
            /* Copy to global array */
            __builtin_memcpy(g_token_array + (thread_id * 16), buffer2, 16);
        }
    }
}

/* Main test driver */
int main(void) {
    int total_sum = 0;
    
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Initialize and fill token array */
    volatile int init_len = g_volatile_len;
    __builtin_memset(g_token_array, 'X', init_len % 256);
    
    /* Copy pattern with builtin memcpy */
    char init_pattern[] = "TEST_PATTERN_1234567890";
    __builtin_memcpy(g_token_array + 32, init_pattern, sizeof(init_pattern) - 1);
    
    /* Move data with builtin memmove (overlapping) */
    __builtin_memmove(g_token_array + 16, g_token_array + 32, 20);
    
    /* Phase 2: Create and process AST */
    ASTNode* root = create_ast_node(4, 1);
    if (root) {
        process_ast(root, &total_sum);
        printf("AST processing complete. Sum: %d\n", total_sum);
    }
    
    /* Phase 3: Parallel operations */
    printf("Starting parallel memory operations...\n");
    parallel_memory_operations();
    
    /* Phase 4: Final verification */
    int final_sum = 0;
    for (int i = 0; i < 256; i++) {
        final_sum += g_token_array[i];
    }
    
    printf("Final token array sum: %d\n", final_sum);
    printf("Total operations sum: %d\n", total_sum + final_sum);
    
    /* Cleanup */
    if (root) {
        free_ast(root);
    }
    
    /* Final builtin operations */
    char final_buffer[64];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, g_token_array, 32);
    __builtin_memmove(final_buffer + 16, final_buffer, 32);
    
    printf("=== Test Complete ===\n");
    
    return 0;
}
