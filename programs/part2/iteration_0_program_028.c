/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 256 + (rand() % 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Function with goto statements for flow control */
static void test_memmove_with_goto(char* dest, const char* src, size_t n) {
    int use_builtin = 1;
    
    if (n == 0) goto skip_memmove;
    
    /* Jump into memory operation block */
    goto do_memmove;
    
do_memmove:
    if (use_builtin) {
        /* Force builtin memmove call */
        __builtin_memmove(dest, src, n);
    } else {
        memmove(dest, src, n);
    }
    
    /* Jump out of block */
    goto after_memmove;
    
skip_memmove:
    dest[0] = '\0';
    
after_memmove:
    return;
}

/* Recursive function using memory builtins */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    node->left = NULL;
    node->right = NULL;
    
    /* Initialize data with memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Fill with pattern using memcpy */
    char pattern[32];
    snprintf(pattern, sizeof(pattern), "AST%d_Depth%d", node->id, depth);
    __builtin_memcpy(node->data, pattern, strlen(pattern) + 1);
    
    if (depth > 1) {
        node->left = create_ast(depth - 1, counter);
        node->right = create_ast(depth - 1, counter);
        
        /* Copy data between nodes */
        if (node->left && node->right) {
            __builtin_memcpy(node->right->data, node->left->data, 
                           sizeof(node->left->data));
        }
    }
    
    return node;
}

/* Function with complex memory operations */
static size_t process_ast(ASTNode* root, char* buffer) {
    if (!root) return 0;
    
    size_t total = 0;
    volatile size_t copy_size = sizeof(root->data);
    
    /* Copy node data to buffer */
    __builtin_memcpy(buffer + total, root->data, copy_size);
    total += copy_size;
    
    /* Process children */
    total += process_ast(root->left, buffer + total);
    total += process_ast(root->right, buffer + total);
    
    return total;
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_operations(void) {
    const int num_threads = 4;
    char* buffers[num_threads];
    size_t sizes[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        volatile size_t local_size = g_mem_size / num_threads;
        
        buffers[tid] = (char*)malloc(local_size);
        if (buffers[tid]) {
            /* Initialize with memset */
            __builtin_memset(buffers[tid], tid, local_size);
            
            /* Create pattern with memcpy */
            char pattern[16];
            snprintf(pattern, sizeof(pattern), "Thread%d", tid);
            size_t pattern_len = strlen(pattern) + 1;
            
            if (pattern_len < local_size) {
                __builtin_memcpy(buffers[tid], pattern, pattern_len);
            }
            
            /* Move data around */
            if (tid > 0) {
                __builtin_memmove(buffers[tid] + local_size/2, 
                                buffers[tid], 
                                local_size/4);
            }
            
            sizes[tid] = local_size;
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_threads; i++) {
        if (buffers[i]) free(buffers[i]);
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Basic built-in calls */
    char buffer1[512];
    char buffer2[512];
    volatile size_t size1 = g_mem_size;
    
    __builtin_memset(buffer1, 0xAA, size1);
    __builtin_memcpy(buffer2, buffer1, size1);
    __builtin_memmove(buffer1 + 128, buffer1, 64);
    
    /* Phase 2: Goto flow control */
    test_memmove_with_goto(buffer1 + 256, buffer2, 32);
    
    /* Phase 3: Recursive AST operations */
    int counter = 1;
    ASTNode* ast_root = create_ast(4, &counter);
    
    if (ast_root) {
        char ast_buffer[4096];
        size_t ast_data_size = process_ast(ast_root, ast_buffer);
        
        /* Verify data with memcmp */
        int cmp_result = __builtin_memcmp(ast_buffer, 
                                         ast_root->data, 
                                         sizeof(ast_root->data));
        printf("AST data comparison: %s\n", 
               cmp_result == 0 ? "Match" : "Mismatch");
        
        /* Cleanup AST */
        /* Note: In real ASAN, this would detect leaks */
    }
    
    /* Phase 4: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Phase 5: Mixed operations with volatile control */
    volatile int iterations = 3;
    for (volatile int i = 0; i < iterations; i++) {
        char temp[128];
        size_t op_size = 64 + (i * 16);
        
        switch (i % 3) {
            case 0:
                __builtin_memset(temp, i, op_size);
                break;
            case 1:
                __builtin_memcpy(temp, buffer1, op_size);
                break;
            case 2:
                __builtin_memmove(temp + 16, temp, op_size - 16);
                break;
        }
    }
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        hash = (hash * 31) + buffer1[i];
    }
    
    printf("Test completed. Verification hash: %lu\n", hash);
    printf("Return code: %d\n", (hash % 1000) == 123 ? 0 : 1);
    
    return (hash % 1000) == 123 ? 0 : 1;
}
