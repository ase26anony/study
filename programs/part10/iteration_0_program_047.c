/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_array[1024];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_constructor(void) {
    /* Initialize token array with pattern */
    for (int i = 0; i < sizeof(token_array); i++) {
        token_array[i] = (char)((i * 7) % 256);
    }
    
    /* Use builtins in constructor */
    __builtin_memset(token_array, 0xAA, 128);
    __builtin_memcpy(token_array + 128, token_array, 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_destructor(void) {
    /* Final memory operations */
    volatile char cleanup_buf[64];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    snprintf(node->data, sizeof(node->data), "Node%d", id);
    node->id = id;
    
    /* Create children with goto control flow */
    int use_goto = (id % 3 == 0);
    
    if (use_goto) {
        goto create_left;
    }
    
    node->left = create_ast(depth - 1, id * 2);
    
create_left:
    if (!use_goto) {
        goto create_right;
    }
    
    /* Memory operation inside goto block */
    char temp_buf[32];
    __builtin_memcpy(temp_buf, node->data, 16);
    __builtin_memmove(node->data + 8, node->data, 16);
    node->left = create_ast(depth - 1, id * 2 + 1);
    
create_right:
    node->right = create_ast(depth - 1, id * 2 + 2);
    
    /* Copy between nodes */
    if (node->left && node->right) {
        __builtin_memcpy(node->right->data, node->left->data, 16);
    }
    
    return node;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

/* Process AST with memory operations */
static int process_ast(ASTNode* node, int* sum) {
    if (!node) return 0;
    
    int local_sum = 0;
    
    /* Process data with volatile length */
    int len = volatile_len % 32;
    for (int i = 0; i < len; i++) {
        local_sum += node->data[i];
    }
    
    /* Memory operations with volatile destinations */
    if (node->left) {
        __builtin_memcpy((void*)volatile_dest, node->left->data, 16);
        __builtin_memmove(node->data, volatile_dest, 16);
    }
    
    /* Recursive processing */
    local_sum += process_ast(node->left, sum);
    local_sum += process_ast(node->right, sum);
    
    *sum += local_sum;
    return local_sum;
}

/* OpenMP parallel memory operations */
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
        
        /* Initialize with builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        __builtin_memset(src_buf, 0xCC, sizeof(src_buf));
        
        /* Memory operations in parallel region */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            int offset = (i * thread_id) % 64;
            
            /* Use all three builtins */
            __builtin_memcpy(local_buf + offset, src_buf, 32);
            __builtin_memset(local_buf + offset + 32, i, 16);
            __builtin_memmove(local_buf + offset + 48, local_buf + offset, 16);
            
            /* Update token array */
            #pragma omp critical
            {
                __builtin_memcpy(token_array + token_index, local_buf + offset, 8);
                token_index = (token_index + 8) % sizeof(token_array);
            }
        }
        
        /* Final memory operation with goto */
        int do_memmove = (thread_id % 2 == 0);
        
        if (do_memmove) {
            goto perform_memmove;
        }
        
        __builtin_memcpy(local_buf + 96, src_buf, 16);
        goto skip_memmove;
        
    perform_memmove:
        __builtin_memmove(local_buf + 96, src_buf, 16);
        
    skip_memmove:
        /* Use the result */
        volatile_dest[thread_id % 64] = local_buf[0];
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize volatile source */
    for (int i = 0; i < sizeof(volatile_src); i++) {
        volatile_src[i] = (char)(i % 128);
    }
    
    /* Create recursive AST */
    ASTNode* root = create_ast(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process AST */
    int ast_sum = 0;
    process_ast(root, &ast_sum);
    printf("AST checksum: %d\n", ast_sum);
    
    /* Perform parallel memory operations */
    parallel_memory_ops();
    
    /* Additional builtin usage with complex flow */
    char buffer1[256], buffer2[256];
    
    /* Chain of memory operations */
    __builtin_memset(buffer1, 0x11, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    
    /* Conditional memmove with goto */
    int condition = ast_sum % 2;
    
    if (condition) {
        goto use_memmove;
    }
    
    __builtin_memcpy(buffer1 + 128, buffer2, 64);
    goto after_memmove;
    
use_memmove:
    __builtin_memmove(buffer1 + 128, buffer2, 64);
    
after_memmove:
    /* Mix with volatile operations */
    int len = volatile_len % 128;
    __builtin_memcpy((void*)volatile_dest, buffer1, len);
    __builtin_memset(buffer2, volatile_dest[0], len);
    
    /* Calculate final hash */
    unsigned long hash = 0;
    for (int i = 0; i < sizeof(token_array); i++) {
        hash = hash * 31 + token_array[i];
    }
    
    for (int i = 0; i < 64; i++) {
        hash = hash * 17 + volatile_dest[i];
    }
    
    printf("Final hash: %lu\n", hash);
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
