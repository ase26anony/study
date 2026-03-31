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
static char token_pool[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)((i * 13) & 0xFF);
    }
    /* Use builtin memset in constructor */
    __builtin_memset(token_pool + 2048, 0xAA, 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Cleanup with builtin memset */
    __builtin_memset(token_pool, 0, 256);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = depth;
    node->left = NULL;
    node->right = NULL;
    
    /* Use builtin memcpy to copy data */
    int copy_len = (depth * 16) % 256;
    if (copy_len > 0) {
        __builtin_memcpy(node->data, base_data, copy_len);
        node->data[copy_len] = '\0';
    }
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = volatile_flag & 1;
        
        if (use_goto) {
            goto create_left;
        }
        
        node->left = create_ast(depth - 1, base_data + 16);
        
        create_left:
        node->right = create_ast(depth - 2, base_data + 32);
        
        /* Copy between nodes using builtin memmove */
        if (node->left && node->right) {
            int move_len = (depth * 8) % 128;
            __builtin_memmove(node->right->data, node->left->data, move_len);
        }
    }
    
    return node;
}

/* Complex memory dispatch with OpenMP */
static void parallel_memory_ops(ASTNode* root) {
    if (!root) return;
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread operates on different memory regions */
        char local_buf[512];
        int offset = thread_id * 64;
        
        /* Use volatile length to prevent folding */
        int len = volatile_len;
        if (len > 512) len = 512;
        
        /* Force builtin memcpy with variable length */
        __builtin_memcpy(local_buf, token_pool + offset, len);
        
        /* Conditional memset based on thread ID */
        if (thread_id % 2 == 0) {
            __builtin_memset(local_buf + 128, thread_id, 64);
        }
        
        /* Process AST nodes in parallel */
        #pragma omp for
        for (int i = 0; i < 8; i++) {
            ASTNode* current = root;
            int steps = i;
            
            /* Navigate tree with goto jumps */
            navigate_tree:
            if (current) {
                char temp[256];
                int copy_size = (i * 32) % 256;
                
                /* Use builtin memcpy with goto context */
                __builtin_memcpy(temp, current->data, copy_size);
                
                /* Conditional jump back */
                if (steps-- > 0 && current->left) {
                    current = current->left;
                    goto navigate_tree;
                }
            }
        }
        
        /* Final memory consolidation with memmove */
        if (thread_id == 0) {
            char consolidate[1024];
            __builtin_memmove(consolidate, local_buf, 256);
        }
    }
}

/* Calculate hash from AST structure */
static unsigned long calculate_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    /* Simple DJB2 hash */
    while (*ptr) {
        hash = ((hash << 5) + hash) + *ptr++;
    }
    
    /* Process children with goto for flow control */
    if (node->left) {
        hash ^= calculate_ast_hash(node->left);
        if (node->right) {
            goto hash_right;
        }
    }
    
    if (node->right) {
        hash_right:
        hash ^= calculate_ast_hash(node->right);
    }
    
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create complex AST structure */
    ASTNode* root = create_ast(5, token_pool);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Perform parallel memory operations */
    parallel_memory_ops(root);
    
    /* Calculate and print verification hash */
    unsigned long hash = calculate_ast_hash(root);
    printf("Verification hash: %lu\n", hash);
    
    /* Additional builtin usage in cleanup */
    char final_buf[1024];
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf, root->data, 128);
    __builtin_memmove(final_buf + 128, token_pool, 256);
    
    /* Free AST recursively */
    /* Note: In real code, implement proper tree freeing */
    
    printf("Test completed successfully.\n");
    return 0;
}
