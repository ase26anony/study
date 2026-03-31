/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    char *data;
    size_t data_len;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Global token array for parser simulation */
static const char *tokens[] = {"memcpy", "memset", "memmove", "data", "node"};
static const int token_count = 5;

/* Constructor/destructor for initialization coordination */
__attribute__((constructor)) static void init_asan_hooks(void) {
    volatile char init_buf[64];
    /* Force builtin usage in constructor */
    __builtin_memset(init_buf, 0xA5, sizeof(init_buf));
    __builtin_memcpy(init_buf + 32, init_buf, 16);
}

__attribute__((destructor)) static void cleanup_asan_hooks(void) {
    volatile char cleanup_buf[32];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive parser with goto flow control */
static ASTNode* parse_expression(int depth, int *index) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    __builtin_memset(node, 0, sizeof(*node));
    
    if (depth <= 0) {
        node->type = 1;
        node->value = *index % 256;
        node->data_len = g_mem_size % 128 + 1;
        node->data = malloc(node->data_len);
        if (node->data) {
            __builtin_memset(node->data, node->value, node->data_len);
        }
        return node;
    }
    
    /* Use goto to create complex control flow around memmove */
    int use_memmove = g_use_memmove;
    
    if (use_memmove) {
        goto use_memmove_block;
    } else {
        node->left = parse_expression(depth - 1, index);
        (*index)++;
        goto skip_memmove;
    }
    
use_memmove_block:
    {
        volatile char temp_buf[128];
        char src_buf[64];
        __builtin_memset(src_buf, 0xCC, sizeof(src_buf));
        
        /* This memmove should trigger redirection */
        __builtin_memmove(temp_buf, src_buf, sizeof(src_buf));
        
        node->left = parse_expression(depth - 1, index);
        (*index)++;
        
        /* Jump out of memmove block */
        goto after_memmove;
    }
    
skip_memmove:
    /* Alternative path without memmove */
    node->right = parse_expression(depth - 2, index);
    (*index)++;
    goto node_done;
    
after_memmove:
    node->right = parse_expression(depth - 1, index);
    (*index)++;

node_done:
    node->type = 2;
    return node;
}

/* Copy AST node data with builtins */
static void copy_node_data(ASTNode *dest, const ASTNode *src) {
    if (!dest || !src || !src->data) return;
    
    size_t copy_len = src->data_len;
    if (copy_len > dest->data_len) {
        copy_len = dest->data_len;
    }
    
    /* Force memcpy redirection */
    __builtin_memcpy(dest->data, src->data, copy_len);
    
    /* Conditional memmove with volatile length */
    volatile size_t overlap_check = copy_len / 2;
    if (overlap_check > 16) {
        char temp_buf[256];
        __builtin_memcpy(temp_buf, dest->data, overlap_check);
        __builtin_memmove(dest->data + 10, temp_buf, overlap_check - 10);
    }
}

/* Parallel memory operations with OpenMP */
static void parallel_mem_operations(ASTNode **nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i] && nodes[i]->data) {
            volatile char local_buf[512];
            size_t op_size = (g_mem_size + i) % 256 + 1;
            
            /* Mix of builtins in parallel region */
            __builtin_memset(local_buf, i, op_size);
            __builtin_memcpy(local_buf + 128, nodes[i]->data, 
                           nodes[i]->data_len < 128 ? nodes[i]->data_len : 128);
            
            /* Conditional memmove */
            if (i % 3 == 0) {
                __builtin_memmove(local_buf + 64, local_buf, 32);
            }
            
            /* Copy back with overlap */
            __builtin_memcpy(nodes[i]->data, local_buf + 64, 
                           nodes[i]->data_len < 64 ? nodes[i]->data_len : 64);
        }
    }
}

/* Calculate tree hash */
static int calculate_tree_hash(ASTNode *node) {
    if (!node) return 0;
    
    int hash = node->type * 31 + node->value;
    
    if (node->data) {
        volatile int data_hash = 0;
        for (size_t i = 0; i < node->data_len && i < 64; i++) {
            data_hash ^= node->data[i] << (i % 8);
        }
        hash ^= data_hash;
    }
    
    hash ^= calculate_tree_hash(node->left);
    hash ^= calculate_tree_hash(node->right);
    
    return hash & 0x7FFFFFFF;
}

/* Free tree */
static void free_tree(ASTNode *node) {
    if (!node) return;
    
    free_tree(node->left);
    free_tree(node->right);
    
    if (node->data) {
        /* Clear sensitive data before free */
        __builtin_memset(node->data, 0, node->data_len);
        free(node->data);
    }
    
    __builtin_memset(node, 0, sizeof(*node));
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Create AST tree */
    int index = 0;
    ASTNode *root = parse_expression(4, &index);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    const int node_count = 8;
    ASTNode *nodes[node_count];
    
    nodes[0] = root;
    for (int i = 1; i < node_count; i++) {
        nodes[i] = parse_expression(2, &index);
    }
    
    /* Perform data copying between nodes */
    for (int i = 1; i < node_count; i++) {
        if (nodes[i] && root) {
            copy_node_data(nodes[i], root);
        }
    }
    
    /* Execute parallel memory operations */
    parallel_mem_operations(nodes, node_count);
    
    /* Additional builtin stress in main */
    volatile char main_buf[1024];
    volatile size_t buf_size = g_mem_size % 512 + 256;
    
    __builtin_memset(main_buf, 0xAA, buf_size);
    __builtin_memcpy(main_buf + 256, main_buf, 128);
    
    /* Force memmove with overlapping regions */
    for (int i = 0; i < 3; i++) {
        __builtin_memmove(main_buf + 128 + i * 64, main_buf + i * 64, 32);
    }
    
    /* Calculate and print verification result */
    int total_hash = 0;
    for (int i = 0; i < node_count; i++) {
        if (nodes[i]) {
            total_hash ^= calculate_tree_hash(nodes[i]);
        }
    }
    
    /* Incorporate buffer hash */
    volatile int buf_hash = 0;
    for (size_t i = 0; i < buf_size && i < 256; i++) {
        buf_hash += main_buf[i] * (i + 1);
    }
    total_hash ^= buf_hash;
    
    printf("Verification hash: 0x%08X\n", total_hash);
    
    /* Cleanup */
    for (int i = 1; i < node_count; i++) {
        free_tree(nodes[i]);
    }
    free_tree(root);
    
    /* Final builtin calls */
    volatile char final_buf[64];
    __builtin_memset(final_buf, total_hash & 0xFF, sizeof(final_buf));
    
    printf("Test completed successfully\n");
    return 0;
}
