/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[128];
static volatile char volatile_src[128];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "test"
};
static const int token_count = sizeof(tokens) / sizeof(tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_environment(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < sizeof(volatile_src); i++) {
        volatile_src[i] = (char)(i % 256);
    }
    
    /* Force early built-in usage in constructor */
    __builtin_memset(volatile_dest, 0, sizeof(volatile_dest));
    __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final built-in usage in destructor */
    __builtin_memset(volatile_dest, 0xFF, sizeof(volatile_dest));
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with built-in */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy token into node data with control flow */
    int token_idx = id % token_count;
    size_t copy_len = strlen(tokens[token_idx]);
    if (copy_len > sizeof(node->data) - 1)
        copy_len = sizeof(node->data) - 1;
    
    /* Use goto to create complex control flow around memcpy */
    if (copy_len > 0) {
        goto copy_block;
    copy_block:
        __builtin_memcpy(node->data, tokens[token_idx], copy_len);
        goto after_copy;
    after_copy:
        node->data[copy_len] = '\0';
    }
    
    node->id = id;
    
    /* Recursive creation with alternating patterns */
    if (depth % 2 == 0) {
        node->left = create_ast_node(depth - 1, id * 2);
        node->right = create_ast_node(depth - 1, id * 2 + 1);
    } else {
        node->right = create_ast_node(depth - 1, id * 2 + 1);
        node->left = create_ast_node(depth - 1, id * 2);
    }
    
    return node;
}

/* AST traversal with memory operations between nodes */
static int traverse_and_process(ASTNode* node, char* buffer, int buffer_size) {
    if (!node || buffer_size < 32) return 0;
    
    int sum = 0;
    
    /* Process left subtree */
    if (node->left) {
        /* Copy data between nodes using built-in */
        __builtin_memcpy(buffer, node->data, 16);
        __builtin_memcpy(node->left->data, buffer, 16);
        
        sum += traverse_and_process(node->left, buffer, buffer_size);
    }
    
    /* Process current node */
    for (int i = 0; i < 16 && i < sizeof(node->data); i++) {
        sum += (int)node->data[i];
    }
    
    /* Process right subtree with goto control flow */
    if (node->right) {
        char temp[32];
        
        /* Jump into memory operation block */
        if (node->id % 3 == 0) {
            goto memmove_block;
        memmove_block:
            __builtin_memmove(temp, node->data, 16);
            __builtin_memcpy(node->right->data, temp, 16);
            goto after_memmove;
        after_memmove:
            ; /* Empty statement for label */
        }
        
        sum += traverse_and_process(node->right, buffer, buffer_size);
    }
    
    return sum;
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_operations(void) {
    const int num_blocks = 8;
    char* blocks[num_blocks];
    
    #pragma omp parallel for
    for (int i = 0; i < num_blocks; i++) {
        blocks[i] = (char*)malloc(volatile_len);
        if (blocks[i]) {
            /* Use all three built-ins in parallel region */
            __builtin_memset(blocks[i], i, volatile_len);
            
            if (i > 0) {
                __builtin_memcpy(blocks[i], blocks[i-1], volatile_len / 2);
            }
            
            /* Conditional memmove with volatile length */
            if (volatile_len > 32) {
                char temp[64];
                __builtin_memmove(temp, blocks[i], 32);
                __builtin_memcpy(blocks[i] + 16, temp, 32);
            }
        }
    }
    
    /* Cleanup */
    #pragma omp parallel for
    for (int i = 0; i < num_blocks; i++) {
        if (blocks[i]) {
            free(blocks[i]);
        }
    }
}

/* Main test driver */
int main(void) {
    int result = 0;
    char work_buffer[256];
    
    /* Phase 1: Initialize and test AST operations */
    printf("Phase 1: AST memory operations\n");
    ASTNode* root = create_ast_node(4, 1);
    
    if (root) {
        /* Clear buffer with built-in */
        __builtin_memset(work_buffer, 0, sizeof(work_buffer));
        
        /* Traverse AST with memory operations */
        result = traverse_and_process(root, work_buffer, sizeof(work_buffer));
        printf("AST traversal sum: %d\n", result);
    }
    
    /* Phase 2: Parallel memory operations */
    printf("\nPhase 2: Parallel memory operations\n");
    parallel_memory_operations();
    
    /* Phase 3: Direct built-in stress testing */
    printf("\nPhase 3: Built-in stress testing\n");
    
    char dest[128];
    char src[128];
    
    /* Initialize source with pattern */
    for (int i = 0; i < sizeof(src); i++) {
        src[i] = (char)((i * 7) % 256);
    }
    
    /* Test all three built-ins in sequence */
    __builtin_memset(dest, 0xAA, sizeof(dest));
    __builtin_memcpy(dest, src, volatile_len);
    
    /* Use goto to jump around memmove */
    int use_memmove = 1;
    if (use_memmove) {
        goto do_memmove;
    do_memmove:
        __builtin_memmove(dest + 16, dest, 48);
        goto after_ops;
    }
    
after_ops:
    /* Verify results */
    int verify_sum = 0;
    for (int i = 0; i < volatile_len && i < sizeof(dest); i++) {
        verify_sum += (int)dest[i];
    }
    printf("Verification sum: %d\n", verify_sum);
    
    /* Phase 4: Complex control flow with memory ops */
    printf("\nPhase 4: Complex control flow\n");
    
    char buffer1[64], buffer2[64];
    int counter = 0;
    
    /* Nested loops with built-ins */
    for (int i = 0; i < 3; i++) {
        __builtin_memset(buffer1, i, sizeof(buffer1));
        
        for (int j = 0; j < 2; j++) {
            __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
            
            /* Conditional goto into memory operation */
            if (i == 1 && j == 1) {
                goto nested_memmove;
            nested_memmove:
                __builtin_memmove(buffer1, buffer2, 32);
                counter++;
            }
        }
    }
    
    printf("Complex flow counter: %d\n", counter);
    
    /* Cleanup */
    /* Note: Proper AST cleanup would require recursive free function */
    
    printf("\nTest completed successfully\n");
    return 0;
}
