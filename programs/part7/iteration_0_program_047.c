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
    /* Force initialization of ASAN runtime */
    volatile char init_buf[32];
    __builtin_memset(init_buf, 0, sizeof(init_buf));
    
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)(i % 256);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Cleanup memory operations */
    volatile char cleanup_buf[16];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = depth;
    
    /* Use __builtin_memcpy with volatile length */
    int copy_len = volatile_len % 128;
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Fill remainder with __builtin_memset */
    __builtin_memset(node->data + copy_len, depth, sizeof(node->data) - copy_len);
    
    /* Create children recursively */
    node->left = create_ast(depth - 1, base_data);
    node->right = create_ast(depth - 1, base_data + 1);
    
    return node;
}

/* Function with goto and memory operations */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int state = 0;
    
    if (volatile_flag) {
        goto copy_block;
    }
    
normal_path:
    /* Normal memory move */
    __builtin_memmove(dst->data, src->data, sizeof(dst->data));
    return;
    
copy_block:
    /* Jump into block with memcpy */
    __builtin_memcpy(dst->data, src->data, sizeof(dst->data) / 2);
    
    if (state++ < 3) {
        goto copy_block;  /* Loop back */
    }
    
    goto normal_path;  /* Jump out */
}

/* Parallel memory dispatch function */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        ASTNode* current = nodes[i];
        if (current && current->left && current->right) {
            /* Complex memory operation sequence */
            char temp[256];
            
            /* Test all three builtins */
            __builtin_memcpy(temp, current->data, sizeof(temp));
            __builtin_memset(current->data, i, sizeof(current->data));
            __builtin_memmove(current->right->data, temp, sizeof(temp));
            
            /* Conditional memcpy with volatile */
            if (volatile_flag) {
                __builtin_memcpy(current->left->data, 
                               current->right->data, 
                               volatile_len % 128);
            }
        }
    }
}

/* Token parser with memory operations */
static int parse_tokens(void) {
    char local_buf[512];
    int hash = 0;
    
    /* Copy from global pool with memcpy */
    __builtin_memcpy(local_buf, token_pool + token_index, sizeof(local_buf));
    
    /* Process tokens */
    for (int i = 0; i < sizeof(local_buf); i++) {
        if (local_buf[i] == 0) {
            /* Use memset when null byte found */
            __builtin_memset(local_buf + i, 0xFF, 16);
            i += 15;
        }
        
        /* Move data around */
        if (i > 256) {
            __builtin_memmove(local_buf, local_buf + 128, 128);
        }
        
        hash += local_buf[i];
    }
    
    token_index = (token_index + 256) % sizeof(token_pool);
    return hash;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create AST structure */
    ASTNode* root = create_ast(4, "BaseDataForAST");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    ASTNode* node_array[8];
    node_array[0] = root;
    
    /* Build tree array */
    for (int i = 1; i < 8; i++) {
        node_array[i] = create_ast(3, "NodeData");
    }
    
    /* Test goto with memory operations */
    if (root->left && root->right) {
        process_with_goto(root->left, root->right);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(node_array, 8);
    
    /* Parse tokens multiple times */
    int total_hash = 0;
    for (int i = 0; i < 10; i++) {
        total_hash += parse_tokens();
    }
    
    /* Final verification with all three builtins */
    char verify_buf[1024];
    char source_buf[1024];
    
    __builtin_memset(source_buf, 0xAA, sizeof(source_buf));
    __builtin_memcpy(verify_buf, source_buf, sizeof(verify_buf));
    __builtin_memmove(verify_buf + 512, verify_buf, 512);
    
    /* Calculate final checksum */
    int final_checksum = total_hash;
    for (int i = 0; i < sizeof(verify_buf); i++) {
        final_checksum += verify_buf[i];
    }
    
    printf("Test completed. Final checksum: %d\n", final_checksum);
    
    /* Cleanup */
    /* Recursive free omitted for brevity - would need proper implementation */
    
    return 0;
}
