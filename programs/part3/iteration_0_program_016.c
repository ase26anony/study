/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    uint32_t checksum;
} ASTNode;

/* Global token array */
static volatile char g_token_pool[1024];

/* Constructor function (runs before main) */
static void __attribute__((constructor)) init_tokens(void) {
    /* Initialize token pool with pattern */
    for (size_t i = 0; i < sizeof(g_token_pool); ++i) {
        g_token_pool[i] = (char)((i * 31) & 0xFF);
    }
}

/* Destructor function (runs after main) */
static void __attribute__((destructor)) cleanup(void) {
    /* Clear sensitive data */
    __builtin_memset(g_token_pool, 0, sizeof(g_token_pool));
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth, volatile char* src) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Copy data using __builtin_memcpy */
    __builtin_memcpy(node->data, src, g_mem_size);
    
    /* Calculate checksum */
    uint32_t sum = 0;
    for (int i = 0; i < 64; ++i) {
        sum += (uint8_t)node->data[i];
    }
    node->checksum = sum;
    
    /* Recursive calls with goto for flow control */
    if (depth > 1) {
        int use_left = (sum & 1);
        
        if (use_left) {
            node->left = parse_expression(depth - 1, src + 32);
            goto skip_right;
        }
        
        node->right = parse_expression(depth - 1, src + 64);
        
        skip_right:
        /* Use __builtin_memset on alternate path */
        if (!use_left) {
            __builtin_memset(node->data + 32, 0xAA, 32);
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Parallel memory dispatch function */
static void parallel_memory_ops(void) {
    char buffer1[256];
    char buffer2[256];
    volatile int mode = 0;
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 0xCC, sizeof(buffer1));
    __builtin_memcpy(buffer2, g_token_pool, 256);
    
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < 4; ++i) {
            char local_buf[128];
            
            /* Conditional memmove with goto */
            if (g_use_memmove && i % 2 == 0) {
                __builtin_memcpy(local_buf, buffer1 + i * 32, 64);
                goto do_operation;
            }
            
            __builtin_memcpy(local_buf, buffer2 + i * 32, 64);
            
            do_operation:
            /* Use all three builtins */
            if (mode) {
                __builtin_memset(local_buf + 16, i, 32);
            } else {
                __builtin_memmove(local_buf + 8, local_buf, 56);
            }
            
            /* Copy back */
            __builtin_memcpy(buffer1 + i * 32, local_buf, 64);
        }
    }
    
    /* Final memmove between buffers */
    if (g_use_memmove) {
        __builtin_memmove(buffer2, buffer1, 128);
    }
}

/* Tree traversal with memory operations */
static uint32_t traverse_and_hash(ASTNode* root) {
    if (!root) return 0;
    
    uint32_t hash = root->checksum;
    char temp[64];
    
    /* Copy node data to temp */
    __builtin_memcpy(temp, root->data, 64);
    
    /* Conditional memset */
    if (hash % 3 == 0) {
        __builtin_memset(temp + 32, hash & 0xFF, 16);
    }
    
    /* Process children */
    uint32_t left_hash = traverse_and_hash(root->left);
    uint32_t right_hash = traverse_and_hash(root->right);
    
    /* Memmove between temp locations */
    if (left_hash > right_hash) {
        __builtin_memmove(temp + 16, temp, 48);
    }
    
    return hash + left_hash * 31 + right_hash * 17;
}

/* Free tree memory */
static void free_tree(ASTNode* node) {
    if (!node) return;
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in test\n");
    
    /* Phase 1: Recursive parsing with memory ops */
    ASTNode* ast = parse_expression(4, (char*)g_token_pool);
    
    /* Phase 2: Parallel operations */
    parallel_memory_ops();
    
    /* Phase 3: Tree traversal and verification */
    uint32_t final_hash = traverse_and_hash(ast);
    printf("Final hash: %u\n", final_hash);
    
    /* Phase 4: Cleanup with memory operations */
    if (ast) {
        /* One more memmove before cleanup */
        char verify_buf[64];
        __builtin_memcpy(verify_buf, ast->data, 64);
        __builtin_memmove(verify_buf + 16, verify_buf, 32);
        
        free_tree(ast);
    }
    
    /* Final builtin calls */
    char final_buf[128];
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf, g_token_pool + 512, 64);
    __builtin_memmove(final_buf + 64, final_buf, 64);
    
    printf("Test completed successfully\n");
    return 0;
}
