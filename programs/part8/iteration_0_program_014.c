/* ISO C99-compliant test program for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations for attribute functions */
void __attribute__((constructor)) init_asan_test(void);
void __attribute__((destructor)) cleanup_asan_test(void);

/* Recursive AST-like structure */
typedef struct ast_node {
    int type;
    int value;
    volatile size_t size;  /* Volatile to prevent optimization */
    struct ast_node* left;
    struct ast_node* right;
    unsigned char data[256];
} ast_node_t;

/* Global token array with volatile elements */
static volatile unsigned char global_tokens[4096];
static volatile int token_index = 0;

/* Constructor - forces early initialization */
void __attribute__((constructor)) init_asan_test(void) {
    /* Initialize global tokens with pattern */
    for (int i = 0; i < sizeof(global_tokens); i++) {
        global_tokens[i] = (unsigned char)(i % 256);
    }
    token_index = 0;
}

/* Destructor - forces cleanup logic */
void __attribute__((destructor)) cleanup_asan_test(void) {
    /* Clear sensitive data */
    __builtin_memset(global_tokens, 0, sizeof(global_tokens));
}

/* Recursive parser with memory operations */
static ast_node_t* parse_expression(int depth, volatile int* counter) {
    if (depth <= 0 || *counter >= 100) {
        ast_node_t* leaf = malloc(sizeof(ast_node_t));
        if (!leaf) return NULL;
        
        /* Use builtin memset for initialization */
        __builtin_memset(leaf, 0, sizeof(*leaf));
        leaf->type = 1;
        leaf->value = (*counter)++;
        leaf->size = sizeof(*leaf);
        
        /* Fill data with pattern */
        for (int i = 0; i < sizeof(leaf->data); i++) {
            leaf->data[i] = (unsigned char)((leaf->value + i) % 256);
        }
        return leaf;
    }
    
    ast_node_t* node = malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(*node));
    node->type = 2;
    node->size = sizeof(*node);
    
    /* Recursive construction */
    node->left = parse_expression(depth - 1, counter);
    node->right = parse_expression(depth - 1, counter);
    
    /* Copy data between nodes using builtin memcpy */
    if (node->left && node->right) {
        size_t copy_size = sizeof(node->data);
        if (node->left->size < copy_size) copy_size = node->left->size;
        if (node->right->size < copy_size) copy_size = node->right->size;
        
        /* Force memcpy redirection */
        __builtin_memcpy(node->data, node->left->data, copy_size);
        
        /* Use goto for control flow edge case */
        volatile int use_memmove = 1;
        
        if (use_memmove) {
            goto use_memmove_block;
        } else {
            /* Regular path */
            __builtin_memcpy(node->right->data, node->data, copy_size);
        }
        
        use_memmove_block:
        /* Jump target with memmove */
        __builtin_memmove(node->right->data + 128, node->data, 
                         copy_size > 128 ? 128 : copy_size);
    }
    
    return node;
}

/* Free AST recursively */
static void free_ast(ast_node_t* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear node data before free */
    volatile size_t clear_size = node->size;
    if (clear_size > sizeof(*node)) clear_size = sizeof(*node);
    __builtin_memset(node, 0, clear_size);
    free(node);
}

/* Parallel memory operations with OpenMP */
static unsigned long process_ast_parallel(ast_node_t* root) {
    unsigned long total_hash = 0;
    
    #pragma omp parallel reduction(+:total_hash)
    {
        /* Each thread processes different memory regions */
        #pragma omp for
        for (int i = 0; i < 16; i++) {
            unsigned char buffer[1024];
            volatile size_t op_size = 256 + (i * 32);
            
            /* Initialize buffer */
            __builtin_memset(buffer, i, sizeof(buffer));
            
            /* Copy from global tokens with varying sizes */
            size_t copy_len = op_size % sizeof(buffer);
            __builtin_memcpy(buffer + 128, 
                           (void*)&global_tokens[i * 256], 
                           copy_len);
            
            /* Move data around within buffer */
            __builtin_memmove(buffer, buffer + 64, 192);
            
            /* Compute simple hash */
            unsigned long local_hash = 0;
            for (int j = 0; j < sizeof(buffer); j++) {
                local_hash = (local_hash * 31) + buffer[j];
            }
            total_hash += local_hash;
        }
    }
    
    return total_hash;
}

/* Main test driver */
int main(void) {
    volatile int parse_counter = 0;
    unsigned long final_hash = 0;
    
    /* Phase 1: Build and process recursive AST */
    ast_node_t* ast = parse_expression(4, &parse_counter);
    
    if (ast) {
        /* Phase 2: Parallel processing */
        final_hash = process_ast_parallel(ast);
        
        /* Phase 3: Additional memory operations with goto */
        volatile int stage = 0;
        
        stage_switch:
        switch (stage) {
            case 0: {
                unsigned char temp[512];
                __builtin_memset(temp, 0xAA, sizeof(temp));
                __builtin_memcpy(temp + 256, ast->data, 
                               ast->size > 256 ? 256 : ast->size);
                stage = 1;
                goto stage_switch;  /* Jump back to switch */
            }
            case 1: {
                /* Overlapping memory move */
                unsigned char overlap[768];
                __builtin_memset(overlap, 0x55, sizeof(overlap));
                __builtin_memmove(overlap + 256, overlap, 512);
                stage = 2;
                /* Fall through */
            }
            case 2:
                /* Final memory operation */
                if (ast->left) {
                    size_t move_size = ast->left->size;
                    if (move_size > 128) move_size = 128;
                    __builtin_memmove(ast->right->data, 
                                     ast->left->data, 
                                     move_size);
                }
                break;
        }
        
        /* Cleanup */
        free_ast(ast);
    }
    
    /* Print verification result */
    printf("ASAN built-in test completed. Hash: %lu\n", final_hash);
    
    /* Final memory operations to ensure all builtins are used */
    volatile unsigned char final_buf[1024];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
    __builtin_memcpy(final_buf + 512, final_buf, 256);
    __builtin_memmove(final_buf, final_buf + 256, 768);
    
    return 0;
}
