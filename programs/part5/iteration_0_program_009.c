/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_token_array[1024];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token array with pattern */
    for (int i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)((i * 13) & 0xFF);
    }
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    /* Clear sensitive data */
    __builtin_memset(g_token_array, 0, sizeof(g_token_array));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    node->left = node->right = NULL;
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy pattern from global array */
    size_t copy_len = (size_t)(g_mem_size % 64);
    if (copy_len > 0) {
        __builtin_memcpy(node->data, 
                        &g_token_array[node->id % 256], 
                        copy_len);
    }
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_goto = (depth % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, counter);
        node->right = create_ast(depth - 1, counter);
        
        if (use_goto) {
            create_children:
            /* Jump target with __builtin_memmove */
            ASTNode temp;
            if (node->left) {
                __builtin_memmove(&temp, node->left, sizeof(ASTNode));
                __builtin_memcpy(node->left, &temp, sizeof(ASTNode));
            }
        }
    }
    
    return node;
}

/* Process AST with memory operations */
static long process_ast(ASTNode* node, int depth) {
    if (!node) return 0;
    
    long sum = 0;
    
    /* Compute hash of node data */
    for (int i = 0; i < 64; i++) {
        sum += node->data[i];
    }
    
    /* Conditional memmove between nodes */
    if (node->left && node->right) {
        char buffer[128];
        
        /* Move data from left to buffer */
        __builtin_memmove(buffer, node->left->data, 64);
        
        /* Copy from buffer to right */
        __builtin_memcpy(node->right->data, buffer, 64);
        
        /* Clear buffer */
        __builtin_memset(buffer, 0, sizeof(buffer));
    }
    
    /* Recursive processing */
    sum += process_ast(node->left, depth + 1);
    sum += process_ast(node->right, depth + 1);
    
    return sum;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear node data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf1[256];
        char local_buf2[256];
        
        /* Initialize with pattern */
        for (int i = 0; i < 256; i++) {
            local_buf1[i] = (char)((i + thread_id * 17) & 0xFF);
        }
        
        /* Use all three builtins in parallel region */
        __builtin_memset(local_buf2, thread_id, 256);
        
        /* Conditional memcpy based on thread ID */
        if (thread_id % 2 == 0) {
            __builtin_memcpy(local_buf2, local_buf1, 128);
        } else {
            __builtin_memmove(local_buf2, local_buf1, 192);
        }
        
        /* Additional memset with volatile size */
        volatile size_t fill_size = g_mem_size % 256;
        __builtin_memset(&local_buf2[128], 0, fill_size);
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Final memcpy to global array (with critical section) */
        #pragma omp critical
        {
            size_t offset = (thread_id * 64) % 768;
            __builtin_memcpy(&g_token_array[offset], 
                           local_buf2, 
                           64);
        }
    }
}

/* Main test driver */
int main(void) {
    /* Wait for constructor */
    while (!g_init_flag) {}
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Create and process AST */
    int counter = 0;
    ASTNode* root = create_ast(5, &counter);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    long ast_sum = process_ast(root, 0);
    printf("AST processing complete. Sum: %ld\n", ast_sum);
    
    /* Phase 2: Parallel memory operations */
    printf("Starting parallel memory operations...\n");
    parallel_memory_ops();
    
    /* Phase 3: Direct built-in usage with volatile control */
    char final_buffer[512];
    volatile int use_memcpy = 1;
    
    for (int i = 0; i < 3; i++) {
        if (use_memcpy) {
            __builtin_memcpy(final_buffer, 
                           g_token_array, 
                           (size_t)(g_mem_size % 512));
        } else {
            __builtin_memset(final_buffer, 
                           i, 
                           (size_t)(g_mem_size % 512));
        }
        
        /* Toggle operation */
        use_memcpy = !use_memcpy;
        
        /* Use memmove in loop */
        char temp[512];
        __builtin_memmove(temp, final_buffer, 256);
        __builtin_memmove(&final_buffer[256], temp, 256);
    }
    
    /* Compute final verification hash */
    unsigned long final_hash = 0;
    for (int i = 0; i < 512; i++) {
        final_hash = final_hash * 31 + (unsigned char)final_buffer[i];
    }
    
    printf("Final verification hash: 0x%08lx\n", final_hash);
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully.\n");
    return 0;
}
