/* asan_coverage.c - Comprehensive test for ASAN memory built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ast_node {
    struct ast_node* left;
    struct ast_node* right;
    char data[64];
    int value;
} ast_node_t;

/* Global token array */
static char g_token_array[4096];
static int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Force initialization of ASAN runtime */
    volatile char buffer[128];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[64];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive parser with memory operations */
static ast_node_t* parse_expression(int depth) {
    if (depth <= 0) {
        ast_node_t* leaf = malloc(sizeof(ast_node_t));
        if (!leaf) return NULL;
        
        __builtin_memset(leaf, 0, sizeof(*leaf));
        __builtin_memcpy(leaf->data, "LEAF", 5);
        leaf->value = depth;
        return leaf;
    }
    
    ast_node_t* node = malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(*node));
    
    /* Create left subtree */
    node->left = parse_expression(depth - 1);
    
    /* Use goto for control flow edge case */
    int use_memmove = 0;
    goto check_memmove;
    
check_memmove:
    if (node->left) {
        /* Copy data between nodes using memcpy */
        __builtin_memcpy(node->data, node->left->data, 
                        sizeof(node->data) < sizeof(node->left->data) ? 
                        sizeof(node->data) : sizeof(node->left->data));
        
        /* Conditional memmove with goto */
        if (depth % 2 == 0) {
            use_memmove = 1;
            goto perform_memmove;
        }
    }
    
    /* Create right subtree */
    node->right = parse_expression(depth - 2);
    
    if (node->right) {
        /* Another memcpy operation */
        size_t copy_len = g_mem_size % 32;
        if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
        
        __builtin_memcpy(&node->data[32], node->right->data, copy_len);
    }
    
    node->value = depth;
    return node;

perform_memmove:
    /* This block tests memmove redirection */
    char temp_buf[128];
    __builtin_memset(temp_buf, 0xCC, sizeof(temp_buf));
    
    /* Overlapping memmove */
    __builtin_memmove(&temp_buf[32], &temp_buf[16], 64);
    
    /* Copy back to node */
    __builtin_memcpy(node->data, temp_buf, 
                    sizeof(node->data) < sizeof(temp_buf) ? 
                    sizeof(node->data) : sizeof(temp_buf));
    
    goto check_memmove; /* Jump back */
}

/* Calculate hash of AST */
static unsigned long long hash_ast(const ast_node_t* node) {
    if (!node) return 0;
    
    unsigned long long hash = 5381;
    const char* p = node->data;
    
    /* Simple hash calculation */
    while (*p) {
        hash = ((hash << 5) + hash) + *p++;
    }
    
    hash += hash_ast(node->left);
    hash += hash_ast(node->right);
    hash += node->value;
    
    return hash;
}

/* Free AST recursively */
static void free_ast(ast_node_t* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear memory before free */
    volatile char* check = (volatile char*)node;
    __builtin_memset(node, 0xDD, sizeof(*node));
    free(node);
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf1[256];
        char local_buf2[256];
        
        /* Initialize with memset */
        __builtin_memset(local_buf1, thread_id, sizeof(local_buf1));
        __builtin_memset(local_buf2, 0xFF - thread_id, sizeof(local_buf2));
        
        /* Copy between buffers */
        size_t copy_size = (g_mem_size + thread_id) % 128;
        __builtin_memcpy(local_buf2, local_buf1, copy_size);
        
        /* Overlapping memmove */
        __builtin_memmove(&local_buf1[64], &local_buf1[32], 128);
        
        /* Store result in global array */
        #pragma omp critical
        {
            size_t offset = (thread_id * 64) % sizeof(g_token_array);
            __builtin_memcpy(&g_token_array[offset], local_buf1, 
                           sizeof(local_buf1) < (sizeof(g_token_array) - offset) ?
                           sizeof(local_buf1) : (sizeof(g_token_array) - offset));
            g_token_index++;
        }
    }
}

/* Main test driver */
int main(void) {
    unsigned long long total_hash = 0;
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize token array */
    volatile size_t init_size = g_mem_size % sizeof(g_token_array);
    __builtin_memset(g_token_array, 0xAB, init_size);
    
    /* Phase 2: Create and process AST */
    ast_node_t* root = parse_expression(4);
    if (root) {
        total_hash = hash_ast(root);
        printf("AST hash: %llu\n", total_hash);
    }
    
    /* Phase 3: Parallel memory operations */
    parallel_memory_operations();
    
    /* Phase 4: Additional memory operations with gotos */
    {
        char final_buf[512];
        int operation = 0;
        
    start_operations:
        switch (operation) {
            case 0:
                __builtin_memset(final_buf, 0x11, sizeof(final_buf));
                operation = 1;
                goto start_operations;
                
            case 1:
                __builtin_memcpy(&final_buf[128], g_token_array, 
                               sizeof(g_token_array) < 256 ? 
                               sizeof(g_token_array) : 256);
                operation = 2;
                goto start_operations;
                
            case 2:
                __builtin_memmove(&final_buf[64], &final_buf[192], 128);
                operation = 3;
                goto start_operations;
                
            case 3:
                /* Calculate checksum */
                for (size_t i = 0; i < sizeof(final_buf); i++) {
                    total_hash += final_buf[i];
                }
                break;
        }
    }
    
    /* Phase 5: Cleanup */
    if (root) {
        free_ast(root);
    }
    
    /* Final verification */
    printf("Total operations: %d\n", g_token_index);
    printf("Final hash: %llu\n", total_hash);
    printf("Test completed successfully.\n");
    
    return 0;
}
