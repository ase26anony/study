/* asan_coverage.c - Comprehensive test for ASAN/HWASAN built-in redirection */
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
        token_pool[i] = (i % 26) + 'a';
    }
    
    /* Use builtins in constructor */
    __builtin_memset(token_pool + 1024, 0xAA, 128);
    __builtin_memcpy(token_pool + 1152, token_pool, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Final builtin usage in destructor */
    __builtin_memset(token_pool, 0, 256);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy with volatile length */
    int copy_len = (volatile_len % 128) + 64;
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Fill remainder with memset */
    __builtin_memset(node->data + copy_len, depth, sizeof(node->data) - copy_len);
    
    node->id = depth;
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        char new_data[256];
        __builtin_memcpy(new_data, node->data, 128);
        
        /* Goto block for testing flow sensitivity */
        if (volatile_flag) {
            goto create_left;
        }
        
        node->left = NULL;
        node->right = NULL;
        return node;
        
    create_left:
        node->left = create_ast(depth - 1, new_data);
        
        /* Memmove between buffers */
        __builtin_memmove(new_data + 64, new_data, 128);
        node->right = create_ast(depth - 2, new_data + 64);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Complex memory dispatch with OpenMP */
static void parallel_memory_operations(ASTNode* root) {
    if (!root) return;
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buffer[512];
        
        /* Each thread performs memory operations */
        #pragma omp for
        for (int i = 0; i < 8; i++) {
            /* Use all three builtins with varying patterns */
            __builtin_memset(local_buffer, thread_id + i, sizeof(local_buffer));
            
            /* Conditional builtin usage */
            if (i % 2 == 0) {
                __builtin_memcpy(local_buffer + 128, root->data, 
                                (volatile_len % 256) + 64);
            } else {
                __builtin_memmove(local_buffer, local_buffer + 64, 256);
            }
            
            /* Nested memory operation */
            char temp[128];
            __builtin_memcpy(temp, local_buffer + 192, 64);
            __builtin_memset(local_buffer + 192, 0xFF, 64);
            __builtin_memcpy(local_buffer + 256, temp, 64);
        }
        
        /* Critical section with memory operation */
        #pragma omp critical
        {
            static char shared_buffer[1024];
            __builtin_memcpy(shared_buffer + thread_id * 64, 
                           local_buffer, 64);
        }
    }
}

/* AST traversal with goto jumps */
static int traverse_ast_with_goto(ASTNode* node, int* sum) {
    if (!node) return 0;
    
    int local_sum = 0;
    
    /* Forward goto */
    if (node->id % 3 == 0) {
        goto process_data;
    }
    
    /* Normal processing */
    for (int i = 0; i < 32; i++) {
        local_sum += node->data[i];
    }
    
    goto skip_memmove;
    
process_data:
    /* Block with memmove accessed via goto */
    char temp[128];
    __builtin_memcpy(temp, node->data, 128);
    __builtin_memmove(node->data, node->data + 64, 128);
    __builtin_memcpy(node->data + 64, temp, 64);
    
    for (int i = 64; i < 96; i++) {
        local_sum += node->data[i] * 2;
    }
    
skip_memmove:
    /* Backward goto */
    if (node->left && (node->id % 4 == 0)) {
        goto add_left;
    }
    
    *sum += local_sum;
    return 1;
    
add_left:
    traverse_ast_with_goto(node->left, sum);
    goto skip_memmove;  /* Jump back */
}

/* Main execution flow */
int main(void) {
    int total_sum = 0;
    
    /* Phase 1: Initialize and create AST */
    printf("Initializing ASAN test structures...\n");
    ASTNode* root = create_ast(5, token_pool);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Parallel memory operations */
    printf("Executing parallel memory operations...\n");
    parallel_memory_operations(root);
    
    /* Phase 3: Traversal with goto control flow */
    printf("Traversing AST with goto jumps...\n");
    traverse_ast_with_goto(root, &total_sum);
    
    /* Additional builtin calls in main */
    char final_buffer[1024];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, root->data, 256);
    __builtin_memmove(final_buffer + 512, final_buffer, 256);
    
    /* Calculate verification hash */
    unsigned int hash = 0;
    for (int i = 0; i < 512; i++) {
        hash = (hash * 31) + final_buffer[i];
    }
    
    printf("Verification hash: %u\n", hash);
    printf("Total sum from AST: %d\n", total_sum);
    
    /* Cleanup */
    free(root);
    
    return 0;
}
