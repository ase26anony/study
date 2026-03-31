/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Force ASAN initialization early */
    volatile char init_buf[128];
    __builtin_memset(init_buf, 0xA5, sizeof(init_buf));
    
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)(i % 256);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Final memory operation in destructor */
    volatile char cleanup_buf[64];
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for data initialization */
    size_t copy_len = (size_t)(volatile_len % 128) + 64;
    if (copy_len > 255) copy_len = 255;
    
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, base_data, copy_len);
    
    node->id = depth;
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        char child_data[256];
        __builtin_memcpy(child_data, node->data, sizeof(child_data));
        
        /* Complex control flow with goto */
        if (volatile_flag) {
            goto create_left;
        }
        
        node->right = create_ast(depth - 1, child_data);
        goto skip_left;
        
    create_left:
        node->left = create_ast(depth - 1, child_data);
        
        /* Use __builtin_memmove for overlapping regions */
        if (depth > 2) {
            __builtin_memmove(node->data + 32, node->data, 128);
        }
        
    skip_left:
        node->right = create_ast(depth - 1, child_data);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void goto_memory_operations(void) {
    volatile char buffer_a[256];
    volatile char buffer_b[256];
    
    /* Initialize buffers */
    __builtin_memset(buffer_a, 0xAA, sizeof(buffer_a));
    __builtin_memset(buffer_b, 0xBB, sizeof(buffer_b));
    
    /* Jump into memory operation */
    if (volatile_flag) {
        goto perform_memmove;
    }
    
    /* Normal path */
    __builtin_memcpy(buffer_a, buffer_b, 128);
    goto end_operations;
    
perform_memmove:
    /* This tests flow-sensitivity of ASAN logic */
    __builtin_memmove(buffer_a, buffer_b, 192);
    
    /* Jump out */
    if (volatile_flag > 0) {
        goto end_operations;
    }
    
    /* More operations */
    __builtin_memset(buffer_a + 64, 0xCC, 64);
    
end_operations:
    /* Verify with small operation */
    __builtin_memcpy(buffer_b, buffer_a, 32);
}

/* Parallel memory dispatch with OpenMP */
static void parallel_memory_dispatch(void) {
    const int num_workers = 4;
    char results[num_workers][256];
    
    #pragma omp parallel num_threads(num_workers)
    {
        int tid = omp_get_thread_num();
        volatile char local_buf[512];
        
        /* Each thread performs different memory operations */
        switch (tid % 3) {
            case 0:
                __builtin_memset(local_buf, tid, sizeof(local_buf));
                __builtin_memcpy(results[tid], local_buf + 128, 128);
                break;
            case 1:
                __builtin_memcpy(local_buf, token_pool + tid * 256, 256);
                __builtin_memmove(local_buf + 64, local_buf, 192);
                __builtin_memcpy(results[tid], local_buf, 128);
                break;
            case 2:
                __builtin_memset(local_buf, 0, sizeof(local_buf));
                for (int i = 0; i < 8; i++) {
                    __builtin_memcpy(local_buf + i * 64, 
                                   token_pool + (tid + i) * 32, 32);
                }
                __builtin_memcpy(results[tid], local_buf, 128);
                break;
        }
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Final operation after barrier */
        if (tid == 0) {
            __builtin_memset(results[0] + 128, 0xFF, 64);
        }
    }
    
    /* Verify results with memory operations */
    char final_result[256] = {0};
    for (int i = 0; i < num_workers; i++) {
        __builtin_memcpy(final_result + i * 32, results[i], 32);
    }
}

/* Complex recursive parser with memory operations */
static int recursive_parser(const char* input, int depth, char* output) {
    if (depth <= 0 || !input || !output) return 0;
    
    volatile int local_sum = 0;
    char temp_buf[512];
    
    /* Copy input with volatile length */
    size_t len = (size_t)(volatile_len % 256);
    __builtin_memcpy(temp_buf, input, len);
    
    /* Process recursively */
    for (int i = 0; i < depth; i++) {
        char child_buf[256];
        
        /* Alternate between memcpy and memmove */
        if (i % 2 == 0) {
            __builtin_memcpy(child_buf, temp_buf + i * 16, 128);
        } else {
            __builtin_memmove(child_buf, temp_buf + i * 8, 192);
        }
        
        /* Recursive call */
        local_sum += recursive_parser(child_buf, depth - 1, output + i * 32);
        
        /* Clear buffer periodically */
        if (i % 3 == 0) {
            __builtin_memset(child_buf, 0, sizeof(child_buf));
        }
    }
    
    /* Final memory operation */
    __builtin_memcpy(output, temp_buf, 64);
    return local_sum + depth;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize and test basic operations */
    volatile char phase1_buf[1024];
    __builtin_memset(phase1_buf, 0x5A, sizeof(phase1_buf));
    
    /* Force all three built-ins early */
    __builtin_memcpy(phase1_buf + 256, token_pool, 256);
    __builtin_memmove(phase1_buf + 512, phase1_buf + 256, 256);
    __builtin_memset(phase1_buf + 768, 0x33, 128);
    
    /* Phase 2: Test control flow with goto */
    goto_memory_operations();
    
    /* Phase 3: Create and manipulate AST structure */
    ASTNode* root = create_ast(4, "AST Base Data");
    if (root) {
        /* Copy between AST nodes */
        char node_copy[256];
        __builtin_memcpy(node_copy, root->data, sizeof(node_copy));
        
        if (root->left) {
            __builtin_memcpy(root->left->data, node_copy, 128);
            __builtin_memmove(root->left->data + 64, root->data, 64);
        }
        
        /* Free AST (simplified - real code would free recursively) */
        free(root);
    }
    
    /* Phase 4: Parallel execution */
    parallel_memory_dispatch();
    
    /* Phase 5: Recursive parser */
    char parser_output[1024];
    int parser_result = recursive_parser(token_pool, 3, parser_output);
    
    /* Phase 6: Final verification with all built-ins */
    volatile char final_buf[2048];
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf, parser_output, 512);
    __builtin_memmove(final_buf + 1024, final_buf, 512);
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < 1024; i++) {
        hash = (hash * 31) + (unsigned char)final_buf[i];
    }
    
    printf("Test completed. Hash: 0x%08lx\n", hash);
    printf("Parser result: %d\n", parser_result);
    
    return 0;
}
