/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    int value;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    g_init_flag = 1;
    printf("Constructor: Initializing ASAN globals\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_globals(void) {
    printf("Destructor: Cleaning up ASAN resources\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* prefix) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Build node data using __builtin_memcpy */
    char temp[64];
    __builtin_memset(temp, 0, sizeof(temp));
    __builtin_memcpy(temp, prefix, strlen(prefix));
    __builtin_memcpy(node->data, temp, sizeof(node->data) - 1);
    
    node->value = depth;
    node->left = create_ast(depth - 1, "left_");
    node->right = create_ast(depth - 1, "right_");
    
    return node;
}

/* Function with goto and memory operations */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    volatile int use_goto = 1;
    
    if (use_goto) {
        goto copy_block;
    }
    
    normal_path:
    /* Use __builtin_memmove for overlapping regions */
    if (src && dst) {
        __builtin_memmove(dst->data, src->data, 32);
    }
    return;
    
    copy_block:
    /* Jump into block with __builtin_memcpy */
    __builtin_memcpy(dst->data + 16, src->data, 16);
    goto normal_path;
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        char buffer1[256];
        char buffer2[256];
        volatile size_t local_size = g_mem_size;
        
        /* Force ASAN to intercept these builtins */
        #pragma omp for
        for (int i = 0; i < 10; i++) {
            /* Use all three builtins in parallel region */
            __builtin_memset(buffer1, i, local_size);
            __builtin_memcpy(buffer2, buffer1, local_size);
            
            /* Create overlapping scenario for memmove */
            if (i % 2 == 0) {
                __builtin_memmove(buffer1 + 32, buffer1, 64);
            }
        }
    }
}

/* Multi-stage processing with different memory patterns */
static uint32_t process_ast(ASTNode* root) {
    uint32_t hash = 0;
    ASTNode* stack[32];
    int top = 0;
    
    if (!root) return 0;
    
    stack[top++] = root;
    
    while (top > 0) {
        ASTNode* current = stack[--top];
        
        /* Process node data with builtins */
        char temp[64];
        __builtin_memset(temp, 0, sizeof(temp));
        __builtin_memcpy(temp, current->data, 32);
        
        /* Hash computation */
        for (int i = 0; i < 32; i++) {
            hash = (hash * 31) + temp[i];
        }
        
        /* Push children */
        if (current->right) {
            /* Use memmove for stack adjustment */
            __builtin_memmove(&stack[top], &stack[top], 0); /* Zero-size, but still calls builtin */
            stack[top++] = current->right;
        }
        if (current->left) {
            stack[top++] = current->left;
        }
    }
    
    return hash;
}

/* Function with variable-length memory operations */
static void variable_length_ops(void) {
    volatile int sizes[] = {16, 32, 64, 128};
    char src[256];
    char dst[256];
    
    __builtin_memset(src, 0xAA, sizeof(src));
    
    for (int i = 0; i < 4; i++) {
        volatile size_t len = sizes[i];
        
        /* Force different builtin calls */
        switch (i % 3) {
            case 0:
                __builtin_memcpy(dst, src, len);
                break;
            case 1:
                __builtin_memset(dst + len/2, 0xBB, len);
                break;
            case 2:
                __builtin_memmove(dst, dst + 16, len);
                break;
        }
    }
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Create complex AST structure */
    ASTNode* ast1 = create_ast(4, "root_");
    ASTNode* ast2 = create_ast(3, "copy_");
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST structures\n");
        return 1;
    }
    
    /* Test goto with memory operations */
    process_with_goto(ast1, ast2);
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Process AST with memory builtins */
    uint32_t hash1 = process_ast(ast1);
    uint32_t hash2 = process_ast(ast2);
    
    /* Variable length operations */
    variable_length_ops();
    
    /* Additional builtin calls in main */
    char final_buffer[128];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, ast1->data, 32);
    __builtin_memmove(final_buffer + 16, final_buffer, 16);
    
    /* Print verification result */
    printf("Hash1: %u, Hash2: %u\n", hash1, hash2);
    printf("Final buffer[0]: 0x%02x\n", (unsigned char)final_buffer[0]);
    
    /* Cleanup */
    /* Note: In real ASAN, memory would be automatically checked */
    
    printf("Test completed successfully\n");
    return 0;
}
