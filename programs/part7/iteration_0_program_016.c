/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
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

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_asan_env(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    /* Force initialization of memory function caches */
    char buf1[32], buf2[32];
    __builtin_memset(buf1, 0, sizeof(buf1));
    __builtin_memcpy(buf2, buf1, sizeof(buf1));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_asan_env(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile size */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data with builtin */
    size_t copy_len = (g_mem_size < 64) ? g_mem_size : 64;
    __builtin_memcpy(node->data, base_data, copy_len);
    node->size = copy_len;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_left = 1;
        
        /* Goto into memory operation block */
        if (depth % 2 == 0) {
            goto create_right;
        }
        
        create_left:
        node->left = create_ast(depth - 1, "left_branch");
        
        create_right:
        /* Jump target with memmove */
        char temp[64];
        __builtin_memcpy(temp, node->data, node->size);
        __builtin_memmove(node->data, temp, node->size);
        node->right = create_ast(depth - 1, "right_branch");
        
        /* Jump back */
        if (use_left) {
            use_left = 0;
            goto create_left;
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Process AST with memory operations */
static size_t process_ast(ASTNode* node, int depth) {
    if (!node) return 0;
    
    size_t hash = 0;
    
    /* Process data with builtins */
    char buffer[128];
    volatile size_t op_size = node->size * 2;
    if (op_size > sizeof(buffer)) op_size = sizeof(buffer);
    
    /* Force multiple builtin calls */
    __builtin_memset(buffer, 0xAA, op_size);
    __builtin_memcpy(buffer + 32, node->data, node->size);
    __builtin_memmove(node->data, buffer, node->size);
    
    /* Calculate simple hash */
    for (size_t i = 0; i < node->size; i++) {
        hash = (hash * 31) + node->data[i];
    }
    
    /* Recursive processing */
    hash += process_ast(node->left, depth + 1);
    hash += process_ast(node->right, depth + 1);
    
    return hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, node->size);
    free(node);
}

/* Parallel memory dispatch logic */
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
        
        /* Initialize with builtins */
        __builtin_memset(local_buf1, thread_id, sizeof(local_buf1));
        
        /* Copy between buffers */
        volatile size_t copy_size = g_mem_size;
        if (copy_size > sizeof(local_buf1)) copy_size = sizeof(local_buf1);
        
        __builtin_memcpy(local_buf2, local_buf1, copy_size);
        
        /* Move data around */
        for (int i = 0; i < 3; i++) {
            size_t offset = (i * 64) % 192;
            __builtin_memmove(local_buf1 + offset, 
                            local_buf2 + (offset + 32) % 192, 
                            64);
        }
        
        /* Verify with builtin comparison */
        int cmp = __builtin_memcmp(local_buf1, local_buf2, 64);
        
        #pragma omp critical
        {
            printf("Thread %d: memcmp result = %d\n", thread_id, cmp);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("=== ASAN/HWASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Initialize and create AST */
    ASTNode* root = create_ast(4, "root_node_data");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Process AST recursively */
    size_t ast_hash = process_ast(root, 0);
    printf("AST processing complete. Hash: %zu\n", ast_hash);
    
    /* Phase 3: Parallel memory operations */
    printf("\nStarting parallel memory operations...\n");
    parallel_memory_ops();
    
    /* Phase 4: Additional builtin stress tests */
    printf("\nBuilt-in function stress test...\n");
    
    /* Array of different sizes */
    size_t sizes[] = {16, 32, 64, 128, 256};
    char* buffers[5];
    
    for (int i = 0; i < 5; i++) {
        buffers[i] = (char*)malloc(sizes[i]);
        if (buffers[i]) {
            /* Use all three builtins */
            __builtin_memset(buffers[i], i, sizes[i]);
            
            if (i > 0) {
                __builtin_memcpy(buffers[i], buffers[i-1], 
                               sizes[i] < sizes[i-1] ? sizes[i] : sizes[i-1]);
            }
            
            /* Circular shift with memmove */
            if (sizes[i] > 32) {
                __builtin_memmove(buffers[i] + 16, buffers[i], sizes[i] - 16);
            }
        }
    }
    
    /* Phase 5: Complex goto flow with memory ops */
    printf("\nTesting goto flow control with memory operations...\n");
    
    char flow_buf[512];
    int state = 0;
    
    start_flow:
    __builtin_memset(flow_buf, state, 128);
    state++;
    
    if (state == 1) goto middle_flow;
    if (state == 2) goto end_flow;
    
    middle_flow:
    __builtin_memcpy(flow_buf + 128, flow_buf, 128);
    goto start_flow;
    
    end_flow:
    __builtin_memmove(flow_buf, flow_buf + 256, 256);
    
    /* Cleanup */
    for (int i = 0; i < 5; i++) {
        if (buffers[i]) {
            free(buffers[i]);
        }
    }
    
    free_ast(root);
    
    printf("\n=== Test completed successfully ===\n");
    return 0;
}
