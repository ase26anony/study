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

/* Global token array */
static char g_token_pool[4096];
static volatile size_t g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_token_pool(void) {
    /* Initialize with pattern to detect corruption */
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)(i % 256);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void verify_token_pool(void) {
    size_t errors = 0;
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        if (g_token_pool[i] != (char)(i % 256)) {
            errors++;
        }
    }
    if (errors > 0) {
        fprintf(stderr, "Token pool corruption detected: %zu errors\n", errors);
    }
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* src, size_t depth) {
    if (depth > 3) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with volatile size */
    volatile size_t copy_size = (g_mem_size < 256) ? g_mem_size : 256;
    __builtin_memcpy(node->data, src, copy_size);
    node->size = copy_size;
    
    /* Recursive creation with goto for flow control */
    if (depth < 3) {
        char left_data[256];
        char right_data[256];
        
        /* Prepare data with memmove */
        __builtin_memmove(left_data, src, copy_size);
        __builtin_memset(left_data + copy_size/2, 'L', copy_size/2);
        
        __builtin_memmove(right_data, src, copy_size);
        __builtin_memset(right_data + copy_size/2, 'R', copy_size/2);
        
        /* Use goto to create unusual control flow */
        create_left:
        node->left = create_ast_node(left_data, depth + 1);
        if (!node->left) goto create_right;
        
        create_right:
        node->right = create_ast_node(right_data, depth + 1);
    }
    
    return node;
}

/* Complex memory operation with OpenMP */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        char buffer1[512];
        char buffer2[512];
        volatile size_t local_size = g_mem_size;
        
        /* Initialize buffers */
        __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
        __builtin_memset(buffer2, 0x55, sizeof(buffer2));
        
        /* Perform memory operations in parallel */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            /* Mix of memory operations */
            if (i % 3 == 0) {
                __builtin_memcpy(buffer1 + i*4, buffer2, local_size);
            } else if (i % 3 == 1) {
                __builtin_memmove(buffer2 + i*2, buffer1, local_size);
            } else {
                __builtin_memset(buffer1 + i, i, local_size);
            }
        }
        
        /* Barrier to ensure all operations complete */
        #pragma omp barrier
        
        /* Final consolidation */
        #pragma omp single
        {
            __builtin_memcpy(g_token_pool + g_token_index, buffer1, 256);
            g_token_index += 256;
            __builtin_memcpy(g_token_pool + g_token_index, buffer2, 256);
            g_token_index += 256;
        }
    }
}

/* Function with goto jumping into memory operation block */
static void goto_memory_operation(void) {
    char src[128];
    char dst[128];
    
    /* Initialize */
    __builtin_memset(src, 'X', sizeof(src));
    __builtin_memset(dst, 'Y', sizeof(dst));
    
    /* Jump into the middle of operations */
    goto jump_point;
    
    normal_path:
    __builtin_memcpy(dst, src, g_mem_size);
    return;
    
    jump_point:
    {
        /* This block should trigger ASAN instrumentation */
        volatile int use_memmove = 1;
        if (use_memmove) {
            __builtin_memmove(dst, src, g_mem_size);
        }
        goto normal_path;
    }
}

/* Calculate hash of AST tree */
static size_t hash_ast_tree(ASTNode* node) {
    if (!node) return 0;
    
    size_t hash = 5381;
    for (size_t i = 0; i < node->size; i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    /* Recursive hash combination */
    size_t left_hash = hash_ast_tree(node->left);
    size_t right_hash = hash_ast_tree(node->right);
    
    /* Combine using memory operations */
    char hash_buffer[32];
    __builtin_memset(hash_buffer, 0, sizeof(hash_buffer));
    __builtin_memcpy(hash_buffer, &hash, sizeof(hash));
    __builtin_memcpy(hash_buffer + 8, &left_hash, sizeof(left_hash));
    __builtin_memcpy(hash_buffer + 16, &right_hash, sizeof(right_hash));
    
    /* Final hash calculation */
    size_t final_hash = 0;
    for (size_t i = 0; i < sizeof(hash_buffer); i++) {
        final_hash = final_hash * 31 + hash_buffer[i];
    }
    
    return final_hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Create and populate AST */
    ASTNode* root = create_ast_node("Initial AST Data", 0);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Perform parallel memory operations */
    parallel_memory_operations();
    
    /* Phase 3: Execute goto-based memory operations */
    goto_memory_operation();
    
    /* Phase 4: Additional memory stress */
    {
        char* dynamic_buffer = (char*)malloc(1024);
        if (dynamic_buffer) {
            /* Use all three built-ins on dynamic memory */
            __builtin_memset(dynamic_buffer, 0xCC, 1024);
            __builtin_memcpy(dynamic_buffer + 512, g_token_pool, 256);
            __builtin_memmove(dynamic_buffer, dynamic_buffer + 256, 256);
            
            /* Copy back to token pool */
            __builtin_memcpy(g_token_pool + g_token_index, dynamic_buffer, 512);
            g_token_index += 512;
            
            free(dynamic_buffer);
        }
    }
    
    /* Phase 5: Calculate and verify results */
    size_t ast_hash = hash_ast_tree(root);
    size_t pool_hash = 0;
    
    for (size_t i = 0; i < g_token_index; i++) {
        pool_hash = pool_hash * 31 + g_token_pool[i];
    }
    
    printf("AST Hash: %zu\n", ast_hash);
    printf("Token Pool Hash: %zu\n", pool_hash);
    printf("Total operations performed: %zu bytes\n", g_token_index);
    
    /* Cleanup */
    /* Note: In real code, you'd want to properly free the AST tree */
    
    return 0;
}
