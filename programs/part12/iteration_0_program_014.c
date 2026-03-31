/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int depth;
} ASTNode;

/* Global token array */
static char g_tokens[][32] = {
    "memcpy_test", "memset_test", "memmove_test",
    "recursive", "parallel", "asan", "hwasan"
};

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_env(void) {
    printf("Initializing ASAN environment...\n");
    /* Force initialization of ASAN runtime */
    volatile char* dummy = malloc(16);
    if (dummy) {
        __builtin_memset(dummy, 0, 16);
        free((void*)dummy);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_env(void) {
    printf("Cleaning up ASAN environment...\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->depth = depth;
    
    /* Copy data using __builtin_memcpy with goto for flow control */
    size_t copy_len = strlen(base_data);
    if (copy_len > sizeof(node->data) - 1)
        copy_len = sizeof(node->data) - 1;
    
    /* Goto label for flow control testing */
    copy_start:
    __builtin_memcpy(node->data, base_data, copy_len);
    node->data[copy_len] = '\0';
    
    /* Create children with goto jumping out */
    if (depth > 1) {
        char child_data[64];
        __builtin_snprintf(child_data, sizeof(child_data), 
                          "%s_L%d", base_data, depth);
        
        /* Jump to avoid simple optimization */
        if (depth % 2 == 0) goto create_left;
        else goto create_right;
        
        create_left:
        node->left = create_ast(depth - 1, child_data);
        goto after_left;
        
        create_right:
        node->right = create_ast(depth - 1, child_data);
        goto after_right;
        
        after_left:
        after_right:;
    }
    
    return node;
}

/* Function with complex memory operations and goto */
static void process_ast(ASTNode* node, char* buffer, size_t buf_size) {
    if (!node || !buffer) return;
    
    volatile size_t local_size = buf_size;
    volatile char* temp_buf = malloc(local_size);
    
    if (!temp_buf) return;
    
    /* Multiple memory operations with goto */
    operation_start:
    
    /* Test __builtin_memcpy */
    __builtin_memcpy(temp_buf, node->data, 
                    strlen(node->data) < local_size ? 
                    strlen(node->data) : local_size);
    
    /* Jump based on condition */
    if (node->left) goto process_left;
    if (node->right) goto process_right;
    goto finish;
    
    process_left:
    /* Test __builtin_memmove with overlap */
    size_t offset = local_size / 2;
    __builtin_memmove(temp_buf + offset, temp_buf, offset);
    process_ast(node->left, buffer, buf_size);
    goto check_right;
    
    process_right:
    /* Test __builtin_memset */
    __builtin_memset(temp_buf + local_size/4, 0xAA, local_size/4);
    process_ast(node->right, buffer, buf_size);
    
    check_right:
    finish:
    
    /* Copy result back */
    __builtin_memcpy(buffer, temp_buf, 
                    local_size < buf_size ? local_size : buf_size);
    
    free((void*)temp_buf);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        volatile size_t thread_buf_size = 256 + thread_id * 64;
        char* thread_buf = malloc(thread_buf_size);
        
        if (thread_buf) {
            /* Each thread uses different builtins */
            switch (thread_id % 3) {
                case 0:
                    __builtin_memset(thread_buf, thread_id, thread_buf_size);
                    break;
                case 1:
                    __builtin_memcpy(thread_buf, g_tokens[thread_id % 7], 
                                    strlen(g_tokens[thread_id % 7]));
                    break;
                case 2:
                    __builtin_memmove(thread_buf, thread_buf + 32, 
                                     thread_buf_size > 32 ? thread_buf_size - 32 : 0);
                    break;
            }
            
            /* Barrier to ensure all threads complete */
            #pragma omp barrier
            
            free(thread_buf);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize complex data structures */
    ASTNode* root = create_ast(4, "root");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process buffer for results */
    char result_buffer[4096];
    volatile size_t result_size = sizeof(result_buffer);
    
    /* Clear buffer using __builtin_memset */
    __builtin_memset(result_buffer, 0, result_size);
    
    /* Process AST with memory operations */
    process_ast(root, result_buffer, result_size);
    
    /* Execute parallel memory operations */
    #ifdef _OPENMP
    parallel_memory_ops();
    #endif
    
    /* Additional direct builtin calls to ensure coverage */
    volatile char* test_buf1 = malloc(512);
    volatile char* test_buf2 = malloc(512);
    
    if (test_buf1 && test_buf2) {
        /* Chain of memory operations */
        __builtin_memset(test_buf1, 0xCC, 512);
        __builtin_memcpy(test_buf2, test_buf1, 512);
        __builtin_memmove(test_buf1, test_buf1 + 128, 384);
        
        /* Overlapping operations */
        __builtin_memcpy(test_buf1 + 256, test_buf2, 256);
        __builtin_memset(test_buf2 + 128, 0xDD, 128);
    }
    
    /* Calculate verification hash */
    uint32_t hash = 0;
    for (size_t i = 0; i < result_size && i < 1024; i++) {
        hash = (hash * 31) + (uint8_t)result_buffer[i];
    }
    
    printf("Test completed. Result hash: 0x%08X\n", hash);
    
    /* Cleanup */
    free((void*)test_buf1);
    free((void*)test_buf2);
    
    /* Note: In real code, you'd need to properly free the AST */
    
    return 0;
}
