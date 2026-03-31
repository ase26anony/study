/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
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
    size_t size;
} ASTNode;

/* Global token array */
static char g_tokens[][32] = {
    "memcpy_test_token_1",
    "memset_test_token_2", 
    "memmove_test_token_3",
    "asan_verification_4"
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    volatile char init_buf[128];
    /* Force builtin usage in constructor */
    __builtin_memset(init_buf, 0xAA, sizeof(init_buf));
    __builtin_memcpy(init_buf + 64, g_tokens[0], 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[64];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_recursive(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile control */
    size_t copy_size = g_mem_size % 64;
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, base_data, copy_size);
    
    node->size = copy_size;
    
    /* Create children with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 2 == 0);
        
        if (use_goto) {
            goto create_left;
        }
        
        node->left = create_ast_recursive(depth - 1, g_tokens[1]);
        
        create_left:
        node->left = create_ast_recursive(depth - 1, g_tokens[2]);
        
        /* Jump around memmove operation */
        if (depth > 2) {
            goto skip_memmove;
        }
        
        volatile char temp_buf[128];
        __builtin_memmove(temp_buf, node->data, node->size);
        __builtin_memcpy(node->data, temp_buf, node->size);
        
        skip_memmove:
        node->right = create_ast_recursive(depth - 2, g_tokens[3]);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Parallel memory dispatch logic */
static void parallel_memory_operations(void) {
    volatile char buffers[4][256];
    volatile int results[4] = {0};
    
    #pragma omp parallel for
    for (int i = 0; i < 4; i++) {
        size_t op_size = (g_mem_size + i * 32) % 128;
        
        switch (i % 3) {
            case 0:
                __builtin_memset(buffers[i], i + 0x30, op_size);
                break;
            case 1:
                if (i > 0) {
                    __builtin_memcpy(buffers[i], buffers[i-1], op_size);
                }
                break;
            case 2:
                __builtin_memmove(buffers[i], buffers[(i+1)%4], op_size);
                break;
        }
        
        /* Compute simple hash */
        for (size_t j = 0; j < op_size; j++) {
            results[i] += buffers[i][j];
        }
    }
    
    /* Verify parallel execution */
    volatile int total = 0;
    for (int i = 0; i < 4; i++) {
        total += results[i];
    }
}

/* Complex memory operation with goto edge cases */
static void complex_flow_with_builtins(void) {
    volatile char src[512], dst[512], temp[512];
    int state = 0;
    
    __builtin_memset(src, 0xCC, sizeof(src));
    
    start_loop:
    if (state >= 3) goto finish;
    
    switch (state) {
        case 0:
            __builtin_memcpy(dst, src, g_mem_size % 256);
            state++;
            goto start_loop;
            
        case 1:
            /* Jump into memmove block */
            goto memmove_block;
            
        memmove_block:
            __builtin_memmove(temp, dst, g_mem_size % 128);
            __builtin_memcpy(dst, temp, g_mem_size % 128);
            state++;
            goto start_loop;
            
        case 2:
            __builtin_memset(dst + 128, 0xAA, g_mem_size % 64);
            state++;
            goto start_loop;
    }
    
    finish:
    /* Final builtin usage after goto */
    __builtin_memcpy(src, dst, 64);
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize AST */
    ASTNode* root = create_ast_recursive(4, g_tokens[0]);
    
    /* Perform AST memory operations */
    if (root) {
        volatile char ast_buffer[256];
        
        /* Copy between AST nodes */
        if (root->left) {
            __builtin_memcpy(ast_buffer, root->data, root->size);
            __builtin_memcpy(root->left->data, ast_buffer, root->size);
        }
        
        if (root->right && root->right->left) {
            __builtin_memmove(root->right->data, root->left->data, 
                            root->size < root->left->size ? root->size : root->left->size);
        }
    }
    
    /* Execute parallel operations */
    parallel_memory_operations();
    
    /* Test complex control flow */
    complex_flow_with_builtins();
    
    /* Compute verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 32; j++) {
            hash += g_tokens[i][j];
        }
    }
    
    if (root) {
        hash += root->size;
        /* Cleanup */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    printf("Test completed. Verification hash: %lu\n", hash);
    printf("Compile with: -O2 -fsanitize=address -fopenmp\n");
    printf("Or for HWASAN: -O2 -fsanitize=kernel-hwaddress -fopenmp\n");
    
    return (int)(hash % 256);
}
