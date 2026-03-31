#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    char data[32];
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 32;
volatile int volatile_flag = 1;

/* Global token array */
static char global_tokens[256];
static volatile int token_index = 0;

/* Constructor function */
__attribute__((constructor)) 
static void init_asan_hooks(void) {
    /* Force initialization of memory builtins */
    char buffer[64];
    __builtin_memset(buffer, 0, sizeof(buffer));
    
    /* Copy initialization string */
    const char *init_str = "ASAN_INIT";
    __builtin_memcpy(global_tokens, init_str, strlen(init_str) + 1);
    
    printf("Constructor: ASAN hooks initialized\n");
}

/* Destructor function */
__attribute__((destructor))
static void cleanup_asan(void) {
    /* Clear sensitive data */
    __builtin_memset(global_tokens, 0, sizeof(global_tokens));
    printf("Destructor: ASAN cleanup completed\n");
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = depth * 10;
    
    /* Fill data with pattern using memcpy */
    char pattern[32];
    __builtin_memset(pattern, 'A' + (depth % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, volatile_len % sizeof(node->data));
    
    /* Recursive calls with goto for flow control */
    if (depth > 1) {
        int use_goto = volatile_flag;
        
        if (use_goto) {
            goto parse_left;
        } else {
            node->left = parse_expression(depth - 1);
            goto skip_left;
        }
        
    parse_left:
        node->left = parse_expression(depth - 1);
        
    skip_left:
        /* Use memmove for overlapping regions */
        if (node->left) {
            __builtin_memmove(&node->value, &node->left->value, 
                            sizeof(node->value));
        }
        
        node->right = parse_expression(depth - 2);
    }
    
    return node;
}

/* Calculate hash of AST */
static uint32_t hash_ast(ASTNode *node) {
    if (!node) return 0;
    
    uint32_t hash = 5381;
    
    /* Hash data using memory operations */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    /* Recursive hash */
    hash += hash_ast(node->left);
    hash += hash_ast(node->right);
    
    return hash;
}

/* Free AST with memory clearing */
static void free_ast(ASTNode *node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear sensitive data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[128];
        char src_buf[128];
        
        /* Initialize with memset */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        __builtin_memset(src_buf, 'X', sizeof(src_buf));
        
        /* Copy with memcpy */
        size_t copy_len = (volatile_len + thread_id) % sizeof(local_buf);
        __builtin_memcpy(local_buf, src_buf, copy_len);
        
        /* Move with memmove (potential overlap) */
        if (copy_len > 16) {
            __builtin_memmove(local_buf + 16, local_buf, copy_len - 16);
        }
        
        /* Update global tokens atomically */
        #pragma omp critical
        {
            size_t offset = token_index % sizeof(global_tokens);
            size_t remaining = sizeof(global_tokens) - offset;
            size_t to_copy = copy_len < remaining ? copy_len : remaining;
            
            __builtin_memcpy(global_tokens + offset, local_buf, to_copy);
            token_index = (token_index + to_copy) % sizeof(global_tokens);
        }
    }
}

/* Complex control flow with goto */
static void complex_control_flow(void) {
    char buffer_a[256];
    char buffer_b[256];
    
    /* Initialize buffers */
    __builtin_memset(buffer_a, 'A', sizeof(buffer_a));
    __builtin_memset(buffer_b, 'B', sizeof(buffer_b));
    
    int mode = volatile_flag % 3;
    
    switch (mode) {
        case 0:
            /* Direct copy */
            __builtin_memcpy(buffer_a, buffer_b, volatile_len % sizeof(buffer_a));
            break;
            
        case 1: {
            /* Copy with goto into block */
            goto memmove_block;
            
        memmove_block:
            /* This tests flow-sensitivity */
            __builtin_memmove(buffer_a, buffer_b, 
                            (volatile_len * 2) % sizeof(buffer_a));
            goto after_block;
        }
            
        case 2:
            /* Nested goto */
            if (volatile_flag) {
                goto setup_memset;
            }
            
        setup_memset:
            __builtin_memset(buffer_a, 0xFF, sizeof(buffer_a));
            break;
            
        after_block:
            /* Final operation */
            __builtin_memcpy(buffer_b, buffer_a, 64);
            break;
    }
}

int main(void) {
    printf("Starting ASAN memory operation tests...\n");
    
    /* Phase 1: Recursive AST parsing */
    ASTNode *root = parse_expression(5);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Hash calculation */
    uint32_t ast_hash = hash_ast(root);
    printf("AST Hash: %u\n", ast_hash);
    
    /* Phase 3: Parallel memory operations */
    parallel_memory_ops();
    
    /* Phase 4: Complex control flow */
    complex_control_flow();
    
    /* Phase 5: Additional memory operations between AST nodes */
    if (root->left && root->right) {
        /* Copy data between nodes */
        __builtin_memcpy(root->left->data, root->right->data, 
                        sizeof(root->left->data));
        
        /* Move within same node with overlap */
        __builtin_memmove(root->data + 8, root->data, 16);
    }
    
    /* Calculate final verification hash */
    uint32_t final_hash = ast_hash;
    for (size_t i = 0; i < sizeof(global_tokens); i++) {
        final_hash = ((final_hash << 5) + final_hash) + global_tokens[i];
    }
    
    printf("Final Verification Hash: %u\n", final_hash);
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
