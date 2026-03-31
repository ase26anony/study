/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
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

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force memcpy built-in in constructor */
    __builtin_memcpy(buffer, "constructor_init", 16);
    printf("Constructor: ASAN early init triggered\n");
}

/* Destructor to test cleanup paths */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char cleanup_buf[16];
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
    printf("Destructor: Cleanup completed\n");
}

/* Function with goto jumps around memmove */
static void test_goto_memmove(char* dest, const char* src, size_t n) {
    int use_memmove = 1;
    
    if (n == 0) goto skip_copy;
    
    /* Jump into memory operation block */
    goto do_copy;
    
copy_block:
    /* This label is jumped into from outside */
    __builtin_memmove(dest, src, n);
    goto after_copy;
    
do_copy:
    if (use_memmove) {
        goto copy_block;
    } else {
        __builtin_memcpy(dest, src, n);
    }
    
after_copy:
    /* Verify copy */
    for (size_t i = 0; i < n; i++) {
        if (dest[i] != src[i]) {
            printf("Copy mismatch at index %zu\n", i);
            break;
        }
    }
    return;
    
skip_copy:
    __builtin_memset(dest, 0, n);
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast_node(const char* data, size_t depth) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use built-in memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using built-in memcpy */
    size_t copy_len = strlen(data) < 63 ? strlen(data) : 63;
    __builtin_memcpy(node->data, data, copy_len);
    node->data[copy_len] = '\0';
    node->size = copy_len;
    
    if (depth > 0) {
        char left_data[64], right_data[64];
        snprintf(left_data, sizeof(left_data), "%s_L%d", data, (int)depth);
        snprintf(right_data, sizeof(right_data), "%s_R%d", data, (int)depth);
        
        node->left = create_ast_node(left_data, depth - 1);
        node->right = create_ast_node(right_data, depth - 1);
        
        /* Copy between nodes if both exist */
        if (node->left && node->right) {
            size_t min_size = node->left->size < node->right->size ? 
                            node->left->size : node->right->size;
            __builtin_memcpy(node->left->data, node->right->data, min_size);
        }
    }
    
    return node;
}

/* Complex memory operation with volatile control */
static void complex_memory_ops(void) {
    volatile size_t local_size = g_mem_size;
    char* buffer1 = (char*)malloc(local_size);
    char* buffer2 = (char*)malloc(local_size);
    
    if (!buffer1 || !buffer2) {
        free(buffer1);
        free(buffer2);
        return;
    }
    
    /* Pattern 1: Simple built-in usage */
    __builtin_memset(buffer1, 0xAA, local_size);
    __builtin_memset(buffer2, 0xBB, local_size);
    
    /* Pattern 2: Overlapping copy with memmove */
    size_t overlap = local_size / 2;
    __builtin_memmove(buffer1 + overlap, buffer1, overlap);
    
    /* Pattern 3: Conditional memcpy */
    volatile int do_copy = 1;
    if (do_copy) {
        __builtin_memcpy(buffer2, buffer1, local_size / 4);
    }
    
    /* Pattern 4: Nested memory operations */
    char temp[128];
    __builtin_memset(temp, 0xCC, sizeof(temp));
    __builtin_memcpy(buffer1 + 64, temp, 64);
    __builtin_memmove(buffer2 + 32, buffer1 + 64, 32);
    
    free(buffer1);
    free(buffer2);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    const int num_threads = 4;
    char* shared_buffers[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        size_t size = 1024 + tid * 256;
        
        shared_buffers[tid] = (char*)malloc(size);
        
        if (shared_buffers[tid]) {
            /* Each thread uses different built-ins */
            switch (tid % 3) {
                case 0:
                    __builtin_memset(shared_buffers[tid], tid, size);
                    break;
                case 1:
                    if (tid > 0) {
                        __builtin_memcpy(shared_buffers[tid], 
                                       shared_buffers[tid-1], 
                                       size < 1024 ? size : 1024);
                    }
                    break;
                case 2:
                    __builtin_memmove(shared_buffers[tid] + size/2,
                                    shared_buffers[tid],
                                    size/2);
                    break;
            }
            
            /* Barrier to ensure all allocations done */
            #pragma omp barrier
            
            /* Cross-thread memory operation */
            if (tid == 0) {
                for (int i = 1; i < num_threads; i++) {
                    __builtin_memcpy(shared_buffers[0] + i*256,
                                   shared_buffers[i],
                                   256);
                }
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_threads; i++) {
        free(shared_buffers[i]);
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Basic built-in calls */
    char test_buf1[256], test_buf2[256];
    __builtin_memset(test_buf1, 'A', sizeof(test_buf1));
    __builtin_memcpy(test_buf2, test_buf1, sizeof(test_buf1));
    __builtin_memmove(test_buf1 + 128, test_buf1, 128);
    
    /* Phase 2: Goto-controlled memmove */
    test_goto_memmove(test_buf1, test_buf2, 64);
    
    /* Phase 3: Recursive AST operations */
    ASTNode* root = create_ast_node("ROOT", 3);
    
    /* Phase 4: Complex memory patterns */
    complex_memory_ops();
    
    /* Phase 5: OpenMP parallel section */
    #ifdef _OPENMP
    parallel_memory_ops();
    #endif
    
    /* Phase 6: Verify results */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(test_buf1); i++) {
        hash = hash * 31 + test_buf1[i];
    }
    
    /* Recursive tree hash */
    ASTNode* stack[32];
    int top = 0;
    if (root) stack[top++] = root;
    
    while (top > 0) {
        ASTNode* node = stack[--top];
        for (size_t i = 0; i < node->size; i++) {
            hash = hash * 31 + node->data[i];
        }
        if (node->right) stack[top++] = node->right;
        if (node->left) stack[top++] = node->left;
        free(node);
    }
    
    printf("Test completed. Final hash: %lu\n", hash);
    printf("ASAN built-in redirection should be fully exercised\n");
    
    return 0;
}
