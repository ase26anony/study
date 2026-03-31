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
        token_pool[i] = (i % 26) + 'a';
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Clear sensitive data */
    __builtin_memset(token_pool, 0, sizeof(token_pool));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = depth;
    node->left = NULL;
    node->right = NULL;
    
    /* Use volatile to control length */
    int copy_len = volatile_len % 128;
    
    /* Force memcpy redirection with variable length */
    __builtin_memcpy(node->data, &token_pool[token_index], copy_len);
    token_index = (token_index + copy_len) % sizeof(token_pool);
    
    if (depth < max_depth - 1) {
        node->left = create_ast(depth + 1, max_depth);
        node->right = create_ast(depth + 2, max_depth);
        
        /* Copy between nodes using goto for flow control */
        if (node->left && node->right) {
            int use_memmove = volatile_flag;
            
            if (use_memmove) {
                /* Jump into memmove block */
                goto memmove_block;
            } else {
                /* Direct memcpy */
                __builtin_memcpy(node->right->data, node->left->data, 32);
                goto skip_memmove;
            }
            
memmove_block:
            /* This tests the flow-sensitivity of redirection logic */
            __builtin_memmove(node->data, node->left->data, 48);
            /* Jump out of block */
            goto after_copy;
            
skip_memmove:
            /* Alternative path */
            __builtin_memset(node->data + 32, 'X', 16);
            
after_copy:
            ; /* Empty statement for label */
        }
    }
    
    return node;
}

/* Complex memory dispatch with OpenMP */
static void parallel_memory_ops(ASTNode* root) {
    if (!root) return;
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buf[512];
        
        /* Each thread performs different memory operations */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            /* Mix of memory operations */
            switch (i % 3) {
                case 0:
                    __builtin_memcpy(local_buf, root->data, 
                                    volatile_len % sizeof(local_buf));
                    break;
                case 1:
                    __builtin_memset(local_buf, thread_id + '0', 
                                    volatile_len % sizeof(local_buf));
                    break;
                case 2:
                    __builtin_memmove(local_buf, local_buf + 64, 32);
                    break;
            }
            
            /* Modify root data from multiple threads */
            #pragma omp critical
            {
                __builtin_memcpy(root->data + (i % 128), 
                               local_buf, 16);
            }
        }
    }
}

/* Calculate hash of AST */
static unsigned long ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* p = node->data;
    
    /* DJB2 hash algorithm */
    for (int i = 0; i < 256 && *p; i++) {
        hash = ((hash << 5) + hash) + *p++;
    }
    
    return hash + ast_hash(node->left) + ast_hash(node->right);
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear node data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create complex AST */
    ASTNode* root = create_ast(0, 5);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    printf("Created AST with depth 5\n");
    
    /* Perform parallel memory operations */
    parallel_memory_ops(root);
    printf("Completed parallel memory operations\n");
    
    /* Calculate and print verification hash */
    unsigned long hash = ast_hash(root);
    printf("AST verification hash: %lu\n", hash);
    
    /* Additional built-in calls in main */
    char buffer1[256], buffer2[256];
    
    /* Test all three built-ins in sequence */
    __builtin_memset(buffer1, 'A', sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    
    /* Use goto to create interesting control flow */
    int use_memmove = volatile_flag;
    
    if (use_memmove) {
        goto do_memmove;
    }
    
    __builtin_memset(buffer1, 'B', 128);
    goto skip_memmove_main;
    
do_memmove:
    __builtin_memmove(buffer1 + 64, buffer1, 64);
    
skip_memmove_main:
    /* Final verification */
    __builtin_memcpy(root->data, buffer1, 64);
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
