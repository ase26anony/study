/* ISO C99-compliant test program for ASAN built-in redirection coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    int value;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor))
static void init_asan_early(void) {
    volatile char buffer[128];
    /* Force memcpy built-in redirection early */
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, "constructor_init", 16);
}

/* Destructor for cleanup coordination */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char final_check[32];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile size */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data with memcpy builtin */
    size_t copy_len = (g_mem_size % 64) + 1;
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Create children recursively */
    node->left = create_ast(depth - 1, base_data);
    node->right = create_ast(depth - 1, base_data);
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    volatile int condition = 1;
    
    if (condition) {
        goto copy_block;
    }
    
    skip_copy:
    return;
    
    copy_block:
    {
        /* This block should trigger memmove redirection */
        char temp[64];
        __builtin_memcpy(temp, src->data, sizeof(temp));
        
        if (dst) {
            /* Jump back and forth */
            goto perform_move;
        }
        
        goto skip_copy;
    }
    
    perform_move:
    __builtin_memmove(dst->data, src->data, sizeof(dst->data));
    goto skip_copy;
}

/* Parallel memory dispatch */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        volatile char local_buf[256];
        volatile int thread_id = 0;
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            char temp[128];
            /* Mix memcpy and memset in parallel */
            __builtin_memset(temp, i, sizeof(temp));
            __builtin_memcpy(local_buf + (i % 128), temp, 64);
        }
        
        /* Barrier with memmove */
        #pragma omp barrier
        if (thread_id == 0) {
            __builtin_memmove(local_buf + 128, local_buf, 128);
        }
    }
}

/* Multi-stage interaction function */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    volatile char* data = node->data;
    
    /* Process data with builtin-assisted loop */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        char temp[16];
        __builtin_memset(temp, data[i], sizeof(temp));
        hash = ((hash << 5) + hash) + data[i];
    }
    
    /* Recursive hash combination */
    unsigned long left_hash = compute_ast_hash(node->left);
    unsigned long right_hash = compute_ast_hash(node->right);
    
    /* Combine with memcpy for mixing */
    unsigned long combined[2];
    __builtin_memcpy(combined, &left_hash, sizeof(left_hash));
    __builtin_memcpy(combined + 1, &right_hash, sizeof(right_hash));
    
    return hash ^ combined[0] ^ combined[1];
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Initialize AST structures */
    ASTNode* ast1 = create_ast(3, "AST_Node_Data_1");
    ASTNode* ast2 = create_ast(3, "AST_Node_Data_2");
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST structures\n");
        return 1;
    }
    
    /* Phase 2: Test goto edge cases */
    process_with_goto(ast1, ast2);
    
    /* Phase 3: Parallel memory operations */
    parallel_memory_ops();
    
    /* Phase 4: Cross-copy between structures */
    volatile size_t copy_size = g_mem_size % 128;
    __builtin_memcpy(ast2->data, ast1->data, copy_size);
    __builtin_memmove(ast1->data + 32, ast2->data + 32, 32);
    
    /* Phase 5: Final memset */
    char final_buffer[512];
    __builtin_memset(final_buffer, 0xAA, sizeof(final_buffer));
    __builtin_memcpy(final_buffer + 256, ast1->data, sizeof(ast1->data));
    
    /* Compute and verify result */
    unsigned long hash1 = compute_ast_hash(ast1);
    unsigned long hash2 = compute_ast_hash(ast2);
    unsigned long final_hash = hash1 ^ hash2;
    
    printf("AST Hash 1: %lu\n", hash1);
    printf("AST Hash 2: %lu\n", hash2);
    printf("Final Hash: %lu\n", final_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(ast1);
    free(ast2);
    
    return 0;
}
