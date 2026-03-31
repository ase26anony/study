#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile int flags;  /* volatile to prevent optimization */
    struct ASTNode *left;
    struct ASTNode *right;
    char padding[32];    /* Ensure size for memcpy operations */
} ASTNode;

/* Global volatile variables to prevent constant folding */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 96;

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_constructor(void) {
    printf("Constructor: Initializing ASAN environment\n");
    
    /* Force early builtin usage in constructor */
    char buffer1[256];
    char buffer2[256];
    
    /* Use all three builtins in constructor */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1, buffer2, sizeof(buffer1));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    printf("Destructor: Cleaning up ASAN resources\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int value) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = value;
    node->flags = depth * 100 + value;
    
    /* Create children with goto-based control flow */
    if (depth > 1) {
        int use_goto = (value % 3 == 0);
        
        if (use_goto) {
            goto create_left;
        }
        
        node->left = create_ast(depth - 1, value * 2);
        
    create_left:
        if (use_goto) {
            node->left = create_ast(depth - 1, value * 2 + 1);
        }
        
        node->right = create_ast(depth - 1, value * 3);
        
        /* Copy node data using builtin memcpy with goto */
        if (node->left && node->right) {
            int should_copy = 1;
            
            if (value % 5 == 0) {
                goto skip_copy;
            }
            
            if (should_copy) {
                /* Use volatile length to prevent optimization */
                size_t copy_len = g_memcpy_len % sizeof(ASTNode);
                __builtin_memcpy(node->right, node->left, copy_len);
            }
            
        skip_copy:
            /* memmove with goto jumping into block */
            if (value % 7 == 0) {
                goto do_memmove;
            }
            
            /* This goto jumps into the memmove block */
            if (value % 2 == 0) {
                goto memmove_entry;
            }
            
            node->right->value = node->left->value + 1;
            
        memmove_entry:
            /* Label inside memmove block */
            size_t move_len = g_memmove_len % sizeof(ASTNode);
        do_memmove:
            __builtin_memmove(&node->right->padding, 
                            &node->left->padding, 
                            move_len);
        }
    } else {
        node->left = NULL;
        node->right = NULL;
    }
    
    return node;
}

/* Function with complex memory operations */
static void process_ast(ASTNode* root, int* result) {
    if (!root) return;
    
    /* Use all three builtins in different contexts */
    char local_buf[512];
    char temp_buf[512];
    
    /* memset with volatile length */
    size_t set_len = g_memset_len % sizeof(local_buf);
    __builtin_memset(local_buf, root->value, set_len);
    
    /* memcpy between buffers */
    size_t copy_len = g_memcpy_len % sizeof(local_buf);
    __builtin_memcpy(temp_buf, local_buf, copy_len);
    
    /* memmove overlapping regions */
    size_t move_len = g_memmove_len % (sizeof(local_buf) / 2);
    __builtin_memmove(local_buf + 100, local_buf, move_len);
    
    /* Process recursively */
    *result += root->value;
    process_ast(root->left, result);
    process_ast(root->right, result);
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread uses builtins independently */
        char thread_buf[1024];
        char shared_pattern[256];
        
        /* Initialize pattern */
        __builtin_memset(shared_pattern, thread_id, sizeof(shared_pattern));
        
        /* Copy pattern to thread buffer */
        __builtin_memcpy(thread_buf, shared_pattern, sizeof(shared_pattern));
        
        /* Move data within buffer */
        __builtin_memmove(thread_buf + 128, thread_buf, 128);
        
        /* Additional memset */
        __builtin_memset(thread_buf + 256, 0xFF, 64);
        
        #pragma omp barrier
        
        /* Verify operations */
        int sum = 0;
        for (size_t i = 0; i < sizeof(thread_buf); i++) {
            sum += thread_buf[i];
        }
        
        #pragma omp critical
        {
            printf("Thread %d: buffer sum = %d\n", thread_id, sum);
        }
    }
}

/* Multi-stage initialization */
static void initialize_token_array(char tokens[][64], int count) {
    for (int i = 0; i < count; i++) {
        /* Use different builtins based on index */
        if (i % 3 == 0) {
            __builtin_memset(tokens[i], 'A' + (i % 26), 64);
        } else if (i % 3 == 1) {
            if (i > 0) {
                __builtin_memcpy(tokens[i], tokens[i-1], 64);
            }
        } else {
            if (i > 1) {
                __builtin_memmove(tokens[i], tokens[i-2], 64);
            }
        }
    }
}

int main(void) {
    printf("Starting ASAN builtin redirection test\n");
    
    /* Stage 1: Initialize complex token array */
    char tokens[10][64];
    initialize_token_array(tokens, 10);
    
    /* Stage 2: Create recursive AST */
    ASTNode* root = create_ast(4, 1);
    
    /* Stage 3: Process AST with memory operations */
    int ast_result = 0;
    process_ast(root, &ast_result);
    printf("AST traversal result: %d\n", ast_result);
    
    /* Stage 4: Execute parallel memory operations */
    #ifdef _OPENMP
    printf("Running OpenMP parallel section\n");
    parallel_memory_operations();
    #endif
    
    /* Stage 5: Final verification with all builtins */
    char final_buf[2048];
    char verify_buf[2048];
    
    /* Chain all three builtins */
    __builtin_memset(final_buf, 0x55, sizeof(final_buf));
    __builtin_memcpy(verify_buf, final_buf, sizeof(final_buf));
    __builtin_memmove(final_buf + 512, final_buf, 1024);
    
    /* Calculate verification hash */
    uint32_t hash = 0;
    for (size_t i = 0; i < sizeof(final_buf); i++) {
        hash = (hash * 31) + final_buf[i];
    }
    
    printf("Final verification hash: 0x%08X\n", hash);
    
    /* Cleanup */
    /* Note: In real ASAN, memory would be freed here */
    
    return 0;
}
