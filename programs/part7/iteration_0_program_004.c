/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char g_token_array[1024];
static volatile size_t g_token_index = 0;

/* Constructor function - forces early initialization */
__attribute__((constructor))
static void init_asan_globals(void) {
    /* Initialize token array with pattern */
    for (size_t i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)((i * 13) & 0xFF);
    }
    
    /* Force builtin usage in constructor */
    volatile char local_buf[32];
    __builtin_memset(local_buf, 0xAA, sizeof(local_buf));
    __builtin_memcpy(&g_token_array[0], local_buf, 16);
}

/* Destructor function */
__attribute__((destructor))
static void cleanup_asan_globals(void) {
    /* Clear sensitive data */
    volatile char* ptr = g_token_array;
    __builtin_memset(ptr, 0, sizeof(g_token_array));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* src, size_t depth) {
    if (depth > 3) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile size */
    volatile size_t copy_size = 32;
    if (copy_size > sizeof(node->data)) copy_size = sizeof(node->data);
    
    /* Control flow with goto */
    if (src) {
        __builtin_memcpy(node->data, src, copy_size);
        goto label_after_copy;
    } else {
        __builtin_memset(node->data, 0xCC, copy_size);
    }
    
label_after_copy:
    node->size = copy_size;
    node->left = create_ast_node(node->data, depth + 1);
    node->right = create_ast_node(node->data + 16, depth + 1);
    
    return node;
}

/* Complex memory operation with goto jumping */
static void process_with_goto(ASTNode* node) {
    if (!node) return;
    
    volatile char temp_buf[128];
    
    /* Jump into memory operation block */
    goto jump_into_memmove;
    
    /* This label is inside the memmove block */
    inside_memmove:
    __builtin_memmove(node->data, temp_buf, node->size);
    goto after_operations;
    
jump_into_memmove:
    /* Prepare data with memset */
    __builtin_memset(temp_buf, 0x55, sizeof(temp_buf));
    
    /* Copy to temp buffer */
    __builtin_memcpy(temp_buf, node->data, node->size);
    
    /* Jump to memmove */
    goto inside_memmove;
    
after_operations:
    /* Process children */
    if (node->left) process_with_goto(node->left);
    if (node->right) process_with_goto(node->right);
}

/* Parallel memory dispatch */
static void parallel_memory_operations(void) {
    volatile size_t block_size = g_mem_size;
    char* buffers[4];
    
    /* Allocate buffers */
    for (int i = 0; i < 4; i++) {
        buffers[i] = malloc(block_size);
        if (!buffers[i]) return;
    }
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread uses different builtins */
        #pragma omp for
        for (int i = 0; i < 4; i++) {
            switch (thread_id % 3) {
                case 0:
                    __builtin_memset(buffers[i], thread_id, block_size);
                    break;
                case 1:
                    __builtin_memcpy(buffers[(i + 1) % 4], buffers[i], block_size / 2);
                    break;
                case 2:
                    __builtin_memmove(buffers[i], buffers[(i + 2) % 4], block_size / 4);
                    break;
            }
        }
        
        /* Barrier to ensure all operations complete */
        #pragma omp barrier
        
        /* Additional memcpy in critical section */
        #pragma omp critical
        {
            volatile size_t crit_size = block_size / 8;
            if (crit_size > 0) {
                __builtin_memcpy(buffers[0], buffers[3], crit_size);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        free(buffers[i]);
    }
}

/* Calculate hash from AST */
static size_t calculate_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    size_t hash = 0;
    volatile char* ptr = node->data;
    
    /* Use builtin to copy to local buffer for hashing */
    char local_copy[64];
    volatile size_t copy_len = node->size;
    if (copy_len > sizeof(local_copy)) copy_len = sizeof(local_copy);
    
    __builtin_memcpy(local_copy, ptr, copy_len);
    
    /* Simple hash calculation */
    for (size_t i = 0; i < copy_len; i++) {
        hash = (hash * 31) + (size_t)local_copy[i];
    }
    
    /* Recursively hash children */
    hash += calculate_ast_hash(node->left);
    hash += calculate_ast_hash(node->right);
    
    return hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    /* Clear node data before freeing */
    volatile char* data_ptr = node->data;
    __builtin_memset(data_ptr, 0, sizeof(node->data));
    
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Create and process AST */
    ASTNode* root = create_ast_node(g_token_array, 0);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Process with goto jumps */
    process_with_goto(root);
    
    /* Phase 3: Parallel operations */
    parallel_memory_operations();
    
    /* Phase 4: Calculate and verify result */
    size_t final_hash = calculate_ast_hash(root);
    printf("AST hash result: %zu\n", final_hash);
    
    /* Phase 5: Additional builtin stress test */
    {
        volatile char final_buffer[512];
        volatile size_t op_size = g_mem_size % 256;
        
        /* Chain of memory operations */
        __builtin_memset(final_buffer, 0xDE, sizeof(final_buffer));
        __builtin_memcpy(final_buffer + 128, g_token_array, op_size);
        __builtin_memmove(final_buffer, final_buffer + 64, op_size);
        __builtin_memset(final_buffer + 256, 0xAD, op_size);
        __builtin_memcpy(final_buffer + 384, final_buffer, op_size);
    }
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully.\n");
    return 0;
}
