/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 1024;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char* data;
    size_t data_len;
    int node_id;
} ASTNode;

/* Global token array */
static char g_token_array[4096];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token array with pattern */
    for (int i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)((i * 31) & 0xFF);
    }
    
    /* Use __builtin_memset in constructor */
    volatile char local_buf[256];
    __builtin_memset(local_buf, 0xAA, sizeof(local_buf));
    
    /* Force memcpy in constructor context */
    __builtin_memcpy(&g_token_array[512], local_buf, 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use __builtin_memset in destructor */
    volatile char cleanup_buf[128];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
    
    /* Verify cleanup */
    __builtin_memcpy(&g_token_array[1024], cleanup_buf, 64);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with __builtin_memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->node_id = depth;
    node->data_len = (size_t)(depth * 64 + 32);
    node->data = malloc(node->data_len);
    
    if (node->data) {
        /* Pattern fill using __builtin_memset */
        __builtin_memset(node->data, (char)(depth & 0xFF), node->data_len);
        
        /* Copy from global token array */
        size_t copy_len = node->data_len;
        if (copy_len > sizeof(g_token_array) - g_token_index) {
            copy_len = sizeof(g_token_array) - g_token_index;
        }
        
        __builtin_memcpy(node->data, &g_token_array[g_token_index], copy_len);
        g_token_index = (g_token_index + (int)copy_len) % sizeof(g_token_array);
    }
    
    /* Recursive creation with goto for control flow */
    if (depth < max_depth - 1) {
        goto create_children;
    } else {
        goto skip_children;
    }
    
create_children:
    node->left = create_ast(depth + 1, max_depth);
    
    /* Use __builtin_memmove with goto */
    if (node->left && node->data) {
        volatile char temp_buf[128];
        size_t move_size = (node->data_len < 128) ? node->data_len : 128;
        
        __builtin_memcpy(temp_buf, node->data, move_size);
        
        /* Jump label for memmove testing */
        memmove_test:
        if (g_use_memmove) {
            __builtin_memmove(node->data, temp_buf, move_size);
        }
    }
    
skip_children:
    node->right = create_ast(depth + 1, max_depth);
    
    return node;
}

/* Function with complex control flow and memory operations */
static void process_ast(ASTNode* node, int* result) {
    if (!node) return;
    
    volatile int local_sum = 0;
    
    /* Label for goto into memory operation block */
    if (node->left && node->right) {
        goto process_with_memmove;
    }
    
    /* Normal processing */
    if (node->data) {
        for (size_t i = 0; i < node->data_len && i < 64; i++) {
            local_sum += node->data[i];
        }
    }
    
    goto continue_processing;
    
process_with_memmove:
    {
        /* Block with __builtin_memmove */
        volatile char overlap_buf[256];
        size_t op_size = (g_mem_size < 256) ? (size_t)g_mem_size : 256;
        
        if (node->data && op_size <= node->data_len) {
            /* Copy to temp buffer */
            __builtin_memcpy(overlap_buf, node->data, op_size);
            
            /* Move with potential overlap */
            __builtin_memmove(node->data, overlap_buf, op_size);
            
            /* Process moved data */
            for (size_t i = 0; i < op_size; i++) {
                local_sum += node->data[i];
            }
        }
    }
    
continue_processing:
    *result += local_sum + node->node_id;
    
    process_ast(node->left, result);
    process_ast(node->right, result);
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    if (node->data) {
        /* Clear sensitive data before free */
        __builtin_memset(node->data, 0, node->data_len);
        free(node->data);
    }
    
    /* Clear node structure */
    __builtin_memset(node, 0, sizeof(ASTNode));
    free(node);
}

/* Main test function */
int main(void) {
    int final_result = 0;
    volatile size_t dynamic_size = 512;
    
    printf("Starting ASAN/HWASAN built-in redirection test...\n");
    
    /* Create AST structure */
    ASTNode* root = create_ast(0, 4);
    
    /* OpenMP parallel region with memory operations */
    #pragma omp parallel reduction(+:final_result)
    {
        int thread_result = 0;
        
        #pragma omp for
        for (int i = 0; i < 16; i++) {
            volatile char thread_buf[256];
            size_t op_size = dynamic_size % 256;
            
            /* Use all three built-ins in parallel region */
            __builtin_memset(thread_buf, (char)i, sizeof(thread_buf));
            
            volatile char src_buf[256];
            __builtin_memset(src_buf, (char)(i * 2), sizeof(src_buf));
            
            __builtin_memcpy(thread_buf, src_buf, op_size);
            
            /* Conditional memmove */
            if (i % 3 == 0) {
                __builtin_memmove(&thread_buf[64], &thread_buf[32], 128);
            }
            
            /* Accumulate results */
            for (size_t j = 0; j < op_size; j++) {
                thread_result += thread_buf[j];
            }
        }
        
        final_result += thread_result;
    }
    
    /* Process AST (serial section) */
    process_ast(root, &final_result);
    
    /* Additional memory operations in main */
    volatile char main_buf[1024];
    __builtin_memset(main_buf, 0xCC, sizeof(main_buf));
    
    /* Overlapping copy with memmove */
    __builtin_memmove(&main_buf[256], &main_buf[128], 512);
    
    /* Final verification copy */
    volatile char verify_buf[128];
    __builtin_memcpy(verify_buf, &main_buf[384], 128);
    
    for (int i = 0; i < 128; i++) {
        final_result += verify_buf[i];
    }
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed. Result hash: %d\n", final_result & 0xFFFF);
    
    /* Force exit with non-zero if result is 0 (unlikely) */
    return (final_result == 0) ? 1 : 0;
}
