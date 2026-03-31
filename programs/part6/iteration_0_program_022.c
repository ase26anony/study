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
static char g_token_pool[4096];
static volatile size_t g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token pool with pattern */
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i % 26) + 'A');
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(size_t depth, const char* base_data) {
    if (depth == 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with goto for flow control */
    size_t copy_len = (depth < 64) ? depth : 64;
    
    /* Goto-based control flow around memcpy */
    if (depth % 2 == 0) {
        goto even_depth;
    }
    
    __builtin_memcpy(node->data, base_data, copy_len);
    goto after_copy;
    
even_depth:
    /* Alternative path with different memcpy */
    __builtin_memcpy(node->data, base_data + 1, copy_len - 1);
    
after_copy:
    node->size = copy_len;
    
    /* Recursive creation with goto jumps */
    if (depth > 1) {
        node->left = create_ast(depth - 1, node->data);
        
        /* Jump over right creation in some cases */
        if (depth % 3 == 0) {
            goto skip_right;
        }
        
        node->right = create_ast(depth - 2, node->data);
        goto after_children;
        
    skip_right:
        node->right = NULL;
        
    after_children:
        /* Use __builtin_memmove to reorganize data */
        if (node->left && node->right) {
            char temp[64];
            __builtin_memcpy(temp, node->left->data, 32);
            __builtin_memmove(node->left->data, node->right->data, 32);
            __builtin_memmove(node->right->data, temp, 32);
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Complex memory operation with OpenMP */
static void parallel_memory_operations(void) {
    size_t local_size = g_mem_size;
    char* buffer1 = (char*)malloc(local_size * 2);
    char* buffer2 = (char*)malloc(local_size * 2);
    
    if (!buffer1 || !buffer2) {
        free(buffer1);
        free(buffer2);
        return;
    }
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        size_t chunk = local_size / omp_get_num_threads();
        size_t start = thread_id * chunk;
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(buffer1 + start, thread_id, chunk);
                __builtin_memcpy(buffer2 + start, buffer1 + start, chunk);
                break;
            case 1:
                __builtin_memcpy(buffer2 + start, g_token_pool + start, 
                               (chunk < sizeof(g_token_pool) - start) ? 
                                chunk : sizeof(g_token_pool) - start);
                break;
            case 2:
                /* Use memmove with overlapping regions */
                __builtin_memmove(buffer1 + start + chunk/2, 
                                buffer1 + start, chunk/2);
                break;
        }
        
        /* Barrier to ensure all memory ops complete */
        #pragma omp barrier
        
        /* Verify operations with another memcpy */
        char verify[256];
        __builtin_memcpy(verify, buffer1 + start, 
                        (chunk < 256) ? chunk : 256);
    }
    
    /* Final overlapping memmove */
    __builtin_memmove(buffer1, buffer1 + local_size/2, local_size/2);
    
    free(buffer1);
    free(buffer2);
}

/* Function with varied built-in usage patterns */
static size_t process_tokens(ASTNode* node) {
    if (!node) return 0;
    
    volatile size_t hash = 0;
    char temp_buf[128];
    
    /* Multiple memory operations in sequence */
    __builtin_memset(temp_buf, 0, sizeof(temp_buf));
    __builtin_memcpy(temp_buf, node->data, node->size);
    
    /* Conditional memmove with goto */
    if (node->size > 32) {
        goto large_node;
    }
    
    __builtin_memmove(temp_buf + 16, temp_buf, 16);
    goto after_move;
    
large_node:
    __builtin_memmove(temp_buf + 32, temp_buf, 32);
    
after_move:
    /* Compute simple hash */
    for (size_t i = 0; i < node->size && i < sizeof(temp_buf); i++) {
        hash = (hash * 31) + temp_buf[i];
    }
    
    /* Recursive processing */
    hash += process_tokens(node->left);
    hash += process_tokens(node->right);
    
    return hash;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST creation with memory ops */
    ASTNode* root = create_ast(5, "BaseDataForASTConstruction");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Process tokens with built-in functions */
    size_t token_hash = process_tokens(root);
    printf("Token hash: %zu\n", token_hash);
    
    /* Phase 3: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Phase 4: Direct built-in calls with volatile control */
    volatile char direct_buf[512];
    volatile size_t op_size = g_mem_size % 512;
    
    __builtin_memset((void*)direct_buf, 0xAA, op_size);
    __builtin_memcpy((void*)(direct_buf + 128), (void*)direct_buf, 128);
    __builtin_memmove((void*)(direct_buf + 256), (void*)(direct_buf + 128), 128);
    
    /* Phase 5: Nested memory operations */
    char* nested_buf = (char*)malloc(1024);
    if (nested_buf) {
        for (int i = 0; i < 10; i++) {
            size_t offset = (i * 97) % 1024;
            size_t len = (i * 23) % 256;
            
            __builtin_memset(nested_buf + offset, i, len);
            
            if (i % 2 == 0) {
                __builtin_memcpy(g_token_pool + g_token_idx, 
                               nested_buf + offset, 
                               (len < sizeof(g_token_pool) - g_token_idx) ? 
                                len : sizeof(g_token_pool) - g_token_idx);
                g_token_idx = (g_token_idx + len) % sizeof(g_token_pool);
            }
        }
        
        /* Final overlapping operation */
        __builtin_memmove(nested_buf + 512, nested_buf, 512);
        free(nested_buf);
    }
    
    /* Cleanup */
    /* Note: In real code, would need proper AST freeing function */
    
    printf("Test completed successfully\n");
    return 0;
}
