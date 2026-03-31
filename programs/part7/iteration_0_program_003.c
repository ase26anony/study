/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* AST-like recursive structure */
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
static void init_asan_test(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)(i % 256);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive AST builder with memory operations */
static ASTNode* build_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = depth;
    node->left = NULL;
    node->right = NULL;
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Fill with pattern using volatile length */
    int len = volatile_len % 128;
    for (int i = 0; i < len; i++) {
        node->data[i] = (char)((depth * 31 + i * 7) % 256);
    }
    
    if (depth < max_depth - 1) {
        /* Recursive calls with goto for control flow */
        if (depth % 2 == 0) {
            goto build_left;
        } else {
            goto build_right;
        }
        
    build_left:
        node->left = build_ast(depth + 1, max_depth);
        goto continue_build;
        
    build_right:
        node->right = build_ast(depth + 1, max_depth);
        goto continue_build;
        
    continue_build:
        /* Copy data between nodes if both children exist */
        if (node->left && node->right) {
            /* Use __builtin_memcpy with volatile control */
            if (volatile_flag) {
                int copy_len = (volatile_len % 64) + 32;
                __builtin_memcpy(node->right->data, node->left->data, copy_len);
            }
        }
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void memory_operations_with_goto(void* dest, void* src, size_t size) {
    char* d = (char*)dest;
    char* s = (char*)src;
    
    if (size == 0) return;
    
    /* Jump into memory operation block */
    goto start_operation;
    
    /* This label is jumped into */
    operation_block:
        /* Use __builtin_memmove with overlapping regions */
        __builtin_memmove(d + size/2, s, size/2);
        goto finish;
    
    start_operation:
        /* Use __builtin_memcpy for first half */
        __builtin_memcpy(d, s, size/2);
        
        /* Conditional jump into operation block */
        if (volatile_flag) {
            goto operation_block;
        }
        
    finish:
        /* Final touch with __builtin_memset */
        __builtin_memset(d + size - 16, 0xFF, 16);
}

/* Parallel memory dispatch using OpenMP */
static void parallel_memory_dispatch(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Each thread performs memory operations */
            char buffer[512];
            
            /* Copy node data to buffer */
            __builtin_memcpy(buffer, nodes[i]->data, 256);
            
            /* Move data around */
            __builtin_memmove(buffer + 128, buffer, 128);
            
            /* Clear part of buffer */
            __builtin_memset(buffer + 384, 0, 128);
            
            /* Copy back to node */
            __builtin_memcpy(nodes[i]->data + 128, buffer + 128, 128);
        }
    }
}

/* Calculate hash of AST structure */
static unsigned long long hash_ast(ASTNode* node, int depth) {
    if (!node || depth <= 0) return 0;
    
    unsigned long long hash = 0;
    
    /* Hash node data using memory operations */
    for (int i = 0; i < 256; i += 16) {
        unsigned long long chunk = 0;
        __builtin_memcpy(&chunk, node->data + i, sizeof(chunk));
        hash ^= chunk;
    }
    
    /* Recursive hash of children */
    hash ^= hash_ast(node->left, depth - 1);
    hash ^= hash_ast(node->right, depth - 1);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Build AST structure */
    ASTNode* root = build_ast(0, 4);
    if (!root) {
        fprintf(stderr, "Failed to build AST\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    ASTNode* nodes[8];
    nodes[0] = root;
    
    /* Build subtree nodes */
    for (int i = 1; i < 8; i++) {
        nodes[i] = build_ast(1, 3);
    }
    
    /* Perform complex memory operations with goto */
    char src_buffer[1024];
    char dst_buffer[1024];
    
    /* Initialize source buffer */
    for (int i = 0; i < sizeof(src_buffer); i++) {
        src_buffer[i] = (char)(i * 3 % 256);
    }
    
    /* Test memory operations with goto flow */
    memory_operations_with_goto(dst_buffer, src_buffer, sizeof(src_buffer));
    
    /* Copy between AST nodes using builtins */
    if (root->left && root->right) {
        int copy_size = (volatile_len % 128) + 64;
        __builtin_memcpy(root->right->data, root->left->data, copy_size);
        __builtin_memmove(root->left->data + 64, root->right->data, 64);
    }
    
    /* Parallel memory operations */
    printf("Starting parallel memory dispatch\n");
    parallel_memory_dispatch(nodes, 8);
    
    /* Calculate and print verification hash */
    unsigned long long total_hash = 0;
    for (int i = 0; i < 8; i++) {
        if (nodes[i]) {
            total_hash ^= hash_ast(nodes[i], 3);
        }
    }
    
    /* Additional memory operations in main */
    char final_buffer[2048];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, dst_buffer, 512);
    __builtin_memmove(final_buffer + 1024, final_buffer, 512);
    
    /* Mix in token pool */
    __builtin_memcpy(final_buffer + 1536, token_pool + token_index, 512);
    
    /* Final hash calculation */
    for (int i = 0; i < sizeof(final_buffer); i += 64) {
        unsigned long long chunk = 0;
        __builtin_memcpy(&chunk, final_buffer + i, 8);
        total_hash += chunk;
    }
    
    printf("Verification hash: %llu\n", total_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real usage, free all allocated nodes */
    
    return 0;
}
