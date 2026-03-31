/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    volatile char buffer[128];
    /* Force __builtin_memset in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    g_mem_size = 128;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_globals(void) {
    volatile char final_buffer[64];
    /* Force __builtin_memmove in destructor */
    __builtin_memmove(final_buffer, final_buffer + 16, 32);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy with volatile size */
    size_t copy_size = g_mem_size > 256 ? 256 : g_mem_size;
    __builtin_memcpy(node->data, base_data, copy_size);
    
    node->size = copy_size;
    
    /* Create children with goto-based control flow */
    int create_left = 1;
    if (depth > 3) {
        create_left = 0;
        goto skip_left;
    }
    
    node->left = create_ast(depth - 1, base_data + 1);
    
skip_left:
    /* Jump back into normal flow */
    if (create_left) {
        /* Force __builtin_memset on left child data */
        if (node->left) {
            __builtin_memset(node->left->data, depth, node->left->size);
        }
    }
    
    /* Right child with different pattern */
    node->right = create_ast(depth - 2, base_data + 2);
    if (node->right) {
        /* Use __builtin_memmove within structure */
        __builtin_memmove(node->right->data + 10, node->right->data, 50);
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ASTNode* node) {
    if (!node) return;
    
    volatile int use_memmove = 1;
    
    if (node->size > 100) {
        use_memmove = 0;
        goto direct_copy;
    }
    
memmove_block:
    /* This block is entered via goto */
    {
        char temp[256];
        __builtin_memmove(temp, node->data, node->size);
        __builtin_memcpy(node->data, temp, node->size);
    }
    goto after_ops;
    
direct_copy:
    /* Direct path without memmove */
    {
        char temp[256];
        __builtin_memcpy(temp, node->data, node->size);
        __builtin_memset(node->data, 0, node->size);
        __builtin_memcpy(node->data, temp, node->size);
    }
    
    if (use_memmove) {
        /* Jump into memmove block */
        goto memmove_block;
    }
    
after_ops:
    /* Continue normal processing */
    if (node->left) process_with_goto(node->left);
    if (node->right) process_with_goto(node->right);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        volatile char local_buf[512];
        volatile int thread_id = omp_get_thread_num();
        
        /* Each thread uses all three builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        char temp_buf[512];
        __builtin_memcpy(temp_buf, local_buf, sizeof(local_buf));
        
        #pragma omp barrier
        
        __builtin_memmove(local_buf, local_buf + 128, 256);
        
        /* Cross-thread memory operation simulation */
        #pragma omp master
        {
            volatile char master_buf[1024];
            __builtin_memset(master_buf, 0xFF, sizeof(master_buf));
        }
    }
}

/* Multi-stage initialization with memory builtins */
static void initialize_token_array(char tokens[][256], int count) {
    volatile int i = 0;
    
    for (i = 0; i < count; i++) {
        /* Pattern: memset -> memcpy -> memmove */
        __builtin_memset(tokens[i], i, 256);
        
        if (i > 0) {
            __builtin_memcpy(tokens[i] + 128, tokens[i-1], 128);
        }
        
        if (i % 3 == 0) {
            __builtin_memmove(tokens[i], tokens[i] + 64, 192);
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Stage 1: Initialize complex token array */
    char tokens[8][256];
    initialize_token_array(tokens, 8);
    
    /* Stage 2: Create recursive AST */
    ASTNode* root = create_ast(5, "BaseDataForASTConstructionWithMemoryOperations");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Stage 3: Process with goto control flow */
    process_with_goto(root);
    
    /* Stage 4: Execute parallel memory operations */
    #ifdef _OPENMP
    parallel_memory_ops();
    #endif
    
    /* Stage 5: Verify operations with checksum */
    unsigned long checksum = 0;
    ASTNode* stack[32];
    int stack_top = 0;
    stack[stack_top++] = root;
    
    while (stack_top > 0) {
        ASTNode* current = stack[--stack_top];
        
        /* Compute simple checksum */
        for (size_t i = 0; i < current->size; i++) {
            checksum += (unsigned char)current->data[i];
        }
        
        if (current->right) stack[stack_top++] = current->right;
        if (current->left) stack[stack_top++] = current->left;
        
        /* Final memory operation on node */
        if (current != root) {
            __builtin_memmove(current->data, current->data + 32, 64);
        }
    }
    
    /* Cleanup */
    /* Recursive free would be here in real implementation */
    
    printf("Test completed. Checksum: %lu\n", checksum);
    printf("ASAN built-in redirection should be fully exercised.\n");
    
    return 0;
}
