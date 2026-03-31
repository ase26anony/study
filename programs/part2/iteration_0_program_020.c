/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
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
static char global_tokens[1024];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize global tokens with pattern */
    for (int i = 0; i < sizeof(global_tokens); i++) {
        global_tokens[i] = (i % 26) + 'a';
    }
    
    /* Use builtins in constructor */
    __builtin_memset(global_tokens + 512, 'X', 128);
    volatile_flag = 0;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Verify memory was properly handled */
    __builtin_memset(global_tokens, 0, 64);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy for initialization */
    __builtin_memcpy(node->data, base_data, 
                     volatile_len < 256 ? volatile_len : 256);
    
    node->id = depth;
    node->left = node->right = NULL;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        char child_data[256];
        
        /* Label for goto testing */
        create_left_child:
        __builtin_memcpy(child_data, node->data, 128);
        child_data[127] = 'L';
        node->left = create_ast(depth - 1, child_data);
        
        if (volatile_flag) {
            /* Jump back to test flow sensitivity */
            goto create_left_child;
        }
        
        /* Use memmove for overlapping regions */
        __builtin_memmove(child_data + 64, child_data, 128);
        child_data[191] = 'R';
        node->right = create_ast(depth - 1, child_data);
    }
    
    return node;
}

/* Complex memory operation with OpenMP */
static void parallel_memory_operations(ASTNode* root) {
    if (!root) return;
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buffer[512];
        
        /* Each thread performs different memory operations */
        #pragma omp for
        for (int i = 0; i < 4; i++) {
            switch (i) {
                case 0:
                    /* Test memcpy redirection */
                    __builtin_memcpy(local_buffer, root->data, 
                                     volatile_len % 256);
                    break;
                case 1:
                    /* Test memset redirection */
                    __builtin_memset(local_buffer + thread_id * 64, 
                                     'T' + thread_id, 32);
                    break;
                case 2:
                    /* Test memmove redirection with overlap */
                    if (root->left) {
                        __builtin_memmove(root->left->data + 32,
                                         root->left->data, 96);
                    }
                    break;
                case 3:
                    /* Mixed operations */
                    __builtin_memcpy(local_buffer + 256, 
                                     global_tokens + thread_id * 128, 128);
                    __builtin_memset(local_buffer + 384, 0, 64);
                    break;
            }
        }
        
        /* Synchronization point */
        #pragma omp barrier
        
        /* Additional memory operations after barrier */
        #pragma omp single
        {
            /* Use goto to jump into memory operation block */
            if (volatile_flag) {
                goto memory_block;
            }
            
            memory_block:
            __builtin_memmove(global_tokens, global_tokens + 256, 256);
        }
    }
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    /* Clear data before freeing */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize with volatile-controlled size */
    volatile_len = 128;
    
    /* Create recursive AST structure */
    ASTNode* root = create_ast(4, "BaseASTNodeDataForTestingBuiltinFunctions");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Perform parallel memory operations */
    parallel_memory_operations(root);
    
    /* Additional sequential memory operations */
    char buffer[1024];
    
    /* Test all three builtins in sequence */
    __builtin_memset(buffer, 'A', 256);
    __builtin_memcpy(buffer + 256, root->data, 128);
    
    /* Create overlapping region for memmove */
    __builtin_memmove(buffer + 128, buffer, 384);
    
    /* Complex goto pattern around memory operations */
    int use_goto = 1;
    
    if (use_goto) {
        goto perform_memcpy;
    }
    
    skipped_memcpy:
    __builtin_memset(buffer + 512, 'B', 128);
    goto after_operations;
    
    perform_memcpy:
    __builtin_memcpy(buffer + 384, global_tokens, 128);
    goto skipped_memcpy;
    
    after_operations:
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < 768; i++) {
        hash = (hash * 31) + buffer[i];
    }
    
    printf("Memory operations completed. Hash: %lu\n", hash);
    
    /* Cleanup */
    free_ast(root);
    
    /* Final memory operation in main */
    __builtin_memset(buffer, 0, sizeof(buffer));
    
    printf("Test completed successfully.\n");
    return 0;
}
