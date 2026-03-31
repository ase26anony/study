/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    printf("Initializing ASAN test environment...\n");
    /* Force early initialization of memory builtins */
    char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Cleaning up ASAN test environment...\n");
}

/* Complex token array initialization */
static void init_token_array(char tokens[][64], size_t count) {
    for (size_t i = 0; i < count; ++i) {
        volatile size_t len = (i * 7) % 64 + 1; /* Non-foldable */
        __builtin_memset(tokens[i], 'A' + (i % 26), len);
        tokens[i][len] = '\0';
    }
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_recursive(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy with volatile size */
    volatile size_t copy_size = (depth * 16) % 256;
    __builtin_memcpy(node->data, base_data, copy_size);
    node->data[copy_size] = '\0';
    node->size = copy_size;
    
    /* Create children with goto-based control flow */
    node->left = NULL;
    node->right = NULL;
    
    if (depth > 1) {
        /* Jump into memory operation block */
        goto create_left;
        
        create_left:
        node->left = create_ast_recursive(depth - 1, node->data);
        
        /* Jump out and back in */
        if (depth % 2 == 0) {
            goto skip_right;
        }
        
        node->right = create_ast_recursive(depth - 1, node->data);
        goto done;
        
        skip_right:
        /* Use __builtin_memmove with goto */
        char temp[256];
        __builtin_memmove(temp, node->data, node->size);
        __builtin_memmove(node->data, temp, node->size);
        goto done;
    }
    
    done:
    return node;
}

/* Parallel memory dispatch logic */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        /* Each thread gets its own buffers */
        char src_buf[128];
        char dst_buf[128];
        char fill_buf[128];
        
        /* Initialize with __builtin_memset */
        volatile size_t init_size = g_mem_size;
        __builtin_memset(src_buf, 'S', init_size);
        __builtin_memset(dst_buf, 'D', init_size);
        
        /* Copy between buffers */
        __builtin_memcpy(dst_buf, src_buf, init_size);
        
        /* Move data around */
        __builtin_memmove(fill_buf, dst_buf, init_size);
        
        /* Clear with memset */
        __builtin_memset(fill_buf, 0, init_size);
        
        #pragma omp barrier
        
        /* Test all three builtins in sequence */
        volatile int thread_id = omp_get_thread_num();
        char thread_buf[256];
        
        __builtin_memset(thread_buf, thread_id, 128);
        __builtin_memcpy(thread_buf + 128, thread_buf, 64);
        __builtin_memmove(thread_buf, thread_buf + 64, 64);
    }
}

/* Calculate hash from AST */
static size_t calculate_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    size_t hash = 0;
    for (size_t i = 0; i < node->size; ++i) {
        hash = hash * 31 + node->data[i];
    }
    
    /* Process children */
    size_t left_hash = calculate_ast_hash(node->left);
    size_t right_hash = calculate_ast_hash(node->right);
    
    return hash ^ (left_hash << 1) ^ (right_hash << 2);
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize complex token array */
    char tokens[8][64];
    init_token_array(tokens, 8);
    
    /* Phase 2: Create recursive AST */
    ASTNode* root = create_ast_recursive(4, "BaseDataForAST");
    
    /* Phase 3: Execute parallel memory operations */
    #ifdef _OPENMP
    printf("Running parallel memory operations...\n");
    parallel_memory_operations();
    #endif
    
    /* Phase 4: Calculate and verify result */
    size_t final_hash = calculate_ast_hash(root);
    
    /* Additional builtin calls to ensure coverage */
    char verify_buf[256];
    volatile size_t verify_size = g_mem_size * 2;
    
    __builtin_memset(verify_buf, 0xAA, verify_size);
    __builtin_memcpy(verify_buf + 128, verify_buf, 64);
    __builtin_memmove(verify_buf, verify_buf + 64, 128);
    
    /* Mix in token data */
    for (int i = 0; i < 8; ++i) {
        volatile size_t len = strlen(tokens[i]);
        __builtin_memcpy(verify_buf + (i * 8), tokens[i], len);
    }
    
    /* Final hash calculation */
    for (size_t i = 0; i < sizeof(verify_buf); ++i) {
        final_hash = final_hash * 37 + verify_buf[i];
    }
    
    printf("Final hash: %zu\n", final_hash);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free_ast(root);
    
    return 0;
}
