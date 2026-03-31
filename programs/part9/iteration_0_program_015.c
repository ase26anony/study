/* ISO C99-compliant test program for ASAN built-in redirection */
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
static void init_asan_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(global_tokens, 'A', sizeof(global_tokens));
    
    /* Force ASAN to see this early call */
    volatile char* ptr = global_tokens + 512;
    __builtin_memset(ptr, 'B', 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Final memory operation in destructor */
    char buffer[64];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy for initialization */
    __builtin_memcpy(node->data, base_data, 
                     (size_t)(volatile_len % 256));
    
    node->id = depth;
    
    /* Create children with goto-controlled flow */
    if (depth > 1) {
        char child_data[256];
        
        /* Jump into memory operation block */
        goto create_left;
        
    create_left:
        __builtin_memcpy(child_data, node->data, 
                         (size_t)(volatile_len % 128));
        node->left = create_ast(depth - 1, child_data);
        
        /* Jump out and back in */
        if (volatile_flag) {
            goto create_right;
        }
        
    create_right:
        /* Reverse the data for right child */
        for (int i = 0; i < 128; i++) {
            child_data[i] = node->data[127 - i];
        }
        __builtin_memmove(child_data + 64, child_data, 64);
        node->right = create_ast(depth - 1, child_data);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Parallel memory dispatch logic */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        char local_buffer[256];
        char src_buffer[256];
        
        /* Initialize source with builtin memset */
        __builtin_memset(src_buffer, 'X', sizeof(src_buffer));
        
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            /* Vary operations based on thread ID */
            int op_type = i % 3;
            size_t len = (size_t)((i * 17 + 23) % 128);
            
            switch (op_type) {
                case 0:
                    /* memcpy with volatile length */
                    __builtin_memcpy(local_buffer + i, src_buffer, 
                                    (size_t)(volatile_len % len));
                    break;
                case 1:
                    /* memset with computed pattern */
                    __builtin_memset(local_buffer + i, 
                                    (char)(i & 0xFF), len);
                    break;
                case 2:
                    /* memmove with overlapping regions */
                    if (len > 32) {
                        __builtin_memmove(local_buffer + 16, 
                                         local_buffer, len - 32);
                    }
                    break;
            }
        }
        
        /* Barrier with memory operation */
        #pragma omp barrier
        
        /* Final collective operation */
        #pragma omp single
        {
            char collective[512];
            __builtin_memset(collective, 0, sizeof(collective));
            __builtin_memcpy(collective + 128, src_buffer, 128);
            __builtin_memmove(collective, collective + 128, 128);
        }
    }
}

/* Calculate hash of AST */
static unsigned long long hash_ast(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long long hash = 0;
    
    /* Process data with builtin memcpy to temp buffer */
    char temp[256];
    size_t len = (size_t)(volatile_len % 256);
    __builtin_memcpy(temp, node->data, len);
    
    for (size_t i = 0; i < len; i++) {
        hash = hash * 31 + (unsigned char)temp[i];
    }
    
    /* Recursive hash combination */
    hash ^= hash_ast(node->left);
    hash ^= (hash_ast(node->right) << 1);
    
    return hash;
}

/* Free AST with memory sanitization */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear sensitive data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize complex token array */
    for (int i = 0; i < sizeof(global_tokens); i++) {
        global_tokens[i] = (char)((i * 7) & 0xFF);
    }
    
    /* Create recursive AST structure */
    ASTNode* root = create_ast(4, global_tokens);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Execute parallelized memory operations */
    parallel_memory_operations();
    
    /* Additional builtin calls in main flow */
    char main_buffer[1024];
    
    /* Chain of memory operations */
    __builtin_memset(main_buffer, 0, sizeof(main_buffer));
    __builtin_memcpy(main_buffer, global_tokens, 512);
    __builtin_memmove(main_buffer + 256, main_buffer, 256);
    
    /* Goto jumping around memory operations */
    if (volatile_flag) {
        goto skip_ops;
    }
    
    __builtin_memset(main_buffer + 768, 0xFF, 128);
    
skip_ops:
    /* Always execute this */
    __builtin_memcpy(main_buffer + 896, main_buffer, 128);
    
    /* Calculate and print verification result */
    unsigned long long ast_hash = hash_ast(root);
    unsigned long long buffer_hash = 0;
    
    for (int i = 0; i < sizeof(main_buffer); i++) {
        buffer_hash = buffer_hash * 31 + (unsigned char)main_buffer[i];
    }
    
    printf("AST Hash: %llu\n", ast_hash);
    printf("Buffer Hash: %llu\n", buffer_hash);
    printf("Final Result: %llu\n", ast_hash ^ buffer_hash);
    
    /* Cleanup */
    free_ast(root);
    
    /* Final memory operation */
    __builtin_memset(main_buffer, 0, sizeof(main_buffer));
    
    printf("Test completed successfully.\n");
    return 0;
}
