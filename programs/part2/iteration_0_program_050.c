/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer1[128];
    volatile char buffer2[128];
    
    /* Force __builtin_memset initialization */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    
    /* Jump into memory operation block */
    goto mem_block;
    
mem_block:
    /* Force __builtin_memcpy initialization */
    __builtin_memcpy(buffer2, buffer1, volatile_len);
    
    printf("[constructor] Initialized memory buffers\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    printf("[destructor] Program completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with __builtin_memset */
    __builtin_memset(node->data, id % 256, sizeof(node->data));
    node->id = id;
    
    /* Create left child with data copy */
    node->left = create_ast(depth - 1, id * 2);
    if (node->left) {
        /* Copy data between nodes using __builtin_memcpy */
        __builtin_memcpy(node->data + 128, node->left->data, volatile_len % 128);
    }
    
    /* Create right child */
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    /* Conditional memmove between children */
    if (node->left && node->right && use_memmove) {
        __builtin_memmove(node->left->data, node->right->data, volatile_len % 64);
    }
    
    return node;
}

/* Function with goto flow control */
static void process_with_goto(ASTNode* node) {
    if (!node) return;
    
    volatile int do_copy = 1;
    
copy_block:
    if (do_copy && node->left) {
        /* Force memcpy redirection */
        __builtin_memcpy(node->data, node->left->data, volatile_len % 256);
        do_copy = 0;
        goto move_block;
    }
    
move_block:
    if (node->right) {
        /* Force memmove redirection with goto */
        __builtin_memmove(node->right->data, node->data, volatile_len % 128);
        goto end_block;
    }
    
end_block:
    /* Additional memset */
    __builtin_memset(node->data + 192, 0xFF, 32);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Each thread uses builtins */
            __builtin_memset(nodes[i]->data, i, 64);
            
            if (i > 0 && nodes[i-1]) {
                __builtin_memcpy(nodes[i]->data + 64, nodes[i-1]->data, 64);
            }
            
            /* Conditional memmove */
            if (i % 3 == 0 && nodes[i]->left) {
                __builtin_memmove(nodes[i]->left->data, nodes[i]->data, 32);
            }
        }
    }
}

/* Compute hash from AST */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* p = node->data;
    
    for (size_t i = 0; i < sizeof(node->data); i++) {
        hash = ((hash << 5) + hash) + p[i];
    }
    
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create complex AST structure */
    ASTNode* root = create_ast(4, 1);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process with goto flow */
    process_with_goto(root);
    
    /* Create array for parallel processing */
    ASTNode* node_array[8];
    node_array[0] = root;
    
    for (int i = 1; i < 8; i++) {
        node_array[i] = create_ast(3, i + 100);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(node_array, 8);
    
    /* Additional direct builtin calls */
    volatile char final_buffer[512];
    volatile char src_buffer[512];
    
    __builtin_memset(src_buffer, 0xCC, sizeof(src_buffer));
    __builtin_memcpy(final_buffer, src_buffer, volatile_len);
    
    if (use_memmove) {
        __builtin_memmove(final_buffer + 256, final_buffer, 256);
    }
    
    /* Compute and print verification result */
    unsigned long total_hash = compute_ast_hash(root);
    
    for (int i = 1; i < 8; i++) {
        total_hash ^= compute_ast_hash(node_array[i]);
    }
    
    printf("Verification hash: %lu\n", total_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        free(node_array[i]);
    }
    
    return 0;
}
