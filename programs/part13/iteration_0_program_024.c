/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#endif

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

/* Global token array */
static char g_token_pool[4096];
volatile int g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Initialize token pool with non-zero pattern */
    __builtin_memset(g_token_pool, 0xAA, sizeof(g_token_pool));
    
    /* Force early initialization of memcpy redirection */
    char temp[32];
    __builtin_memcpy(temp, "constructor_init", 16);
    
    printf("[Constructor] ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    /* Verify memory was properly sanitized */
    __builtin_memset(g_token_pool, 0, 32);
    printf("[Destructor] ASAN cleanup completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* node_id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*node_id)++;
    
    /* Use builtins with volatile-controlled size */
    size_t copy_size = g_mem_size % 64;
    __builtin_memset(node->data, node->id, copy_size);
    
    /* Copy pattern from global pool */
    volatile int offset = g_token_idx;
    __builtin_memcpy(&node->data[32], &g_token_pool[offset], 16);
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, node_id);
        node->right = NULL;
        
        create_children:
        /* Jump target with memmove operation */
        if (depth > 2) {
            char buffer[128];
            volatile size_t move_size = 48;
            
            __builtin_memcpy(buffer, node->data, 64);
            __builtin_memmove(&buffer[32], buffer, move_size);
            __builtin_memcpy(node->data, &buffer[16], 48);
        }
        
        node->right = create_ast(depth - 2, node_id);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with complex memory operations and OpenMP */
static void parallel_memory_operations(ASTNode* root, int iterations) {
    if (!root) return;
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        #pragma omp for
        for (int i = 0; i < iterations; i++) {
            /* Each thread works on different memory regions */
            char local_buf[256];
            volatile size_t op_size = (g_mem_size + i + thread_id) % 128 + 16;
            
            /* Mix of all three builtins */
            __builtin_memset(local_buf, thread_id, op_size);
            
            if (root->left) {
                __builtin_memcpy(&local_buf[64], root->left->data, 32);
            }
            
            /* Overlapping memory move */
            __builtin_memmove(&local_buf[32], local_buf, 64);
            
            /* Update global token pool */
            #pragma omp critical
            {
                int idx = g_token_idx;
                __builtin_memcpy(&g_token_pool[idx], local_buf, 48);
                g_token_idx = (idx + 48) % 4096;
            }
        }
    }
}

/* Function with goto jumping into memory operation block */
static void test_flow_sensitivity(void) {
    char buffer_a[256];
    char buffer_b[256];
    volatile int condition = 1;
    
    /* Initialize buffers */
    __builtin_memset(buffer_a, 0x11, sizeof(buffer_a));
    __builtin_memset(buffer_b, 0x22, sizeof(buffer_b));
    
    if (condition) {
        goto jump_into_memop;
    }
    
    /* Normal path */
    __builtin_memcpy(buffer_a, buffer_b, 128);
    
jump_into_memop:
    /* Target label with memmove */
    volatile size_t move_len = 96;
    __builtin_memmove(&buffer_a[64], buffer_a, move_len);
    
    /* Jump out to different context */
    if (buffer_a[0] != 0x11) {
        goto finalize;
    }
    
    /* More operations */
    __builtin_memset(&buffer_b[128], 0x33, 64);
    
finalize:
    /* Verify operations */
    __builtin_memcpy(&g_token_pool[512], buffer_a, 32);
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Initialize AST */
    int node_id = 1;
    ASTNode* ast_root = create_ast(5, &node_id);
    
    if (!ast_root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Test flow sensitivity */
    test_flow_sensitivity();
    
    /* Phase 3: Parallel memory operations */
    parallel_memory_operations(ast_root, 100);
    
    /* Phase 4: Complex memory pattern */
    char final_buffer[1024];
    volatile size_t pattern_size = g_mem_size % 512 + 256;
    
    /* Chain of builtin calls */
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, ast_root->data, 64);
    
    if (ast_root->left && ast_root->right) {
        /* Overlapping copy between children */
        __builtin_memmove(ast_root->left->data, 
                         ast_root->right->data, 32);
        __builtin_memcpy(final_buffer + 128, 
                        ast_root->left->data, 64);
    }
    
    /* Final memset with volatile size */
    __builtin_memset(final_buffer + 256, 0xFF, pattern_size);
    
    /* Compute verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        hash = hash * 31 + final_buffer[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("AST nodes created: %d\n", node_id - 1);
    printf("Token pool usage: %d bytes\n", g_token_idx);
    
    /* Cleanup */
    /* Note: In real code, you'd want to properly free the AST */
    
    return 0;
}
