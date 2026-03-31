/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Recursive AST-like structure */
struct ASTNode {
    int type;
    int value;
    volatile int volatile_field; /* Prevent optimization */
    struct ASTNode *left;
    struct ASTNode *right;
    char data[64];
};

/* Global token array for parser simulation */
volatile int token_array[256];
volatile int token_index = 0;

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force initialization of ASAN runtime before main */
    volatile char buffer[16];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor to test cleanup paths */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char buffer[8];
    __builtin_memset(buffer, 0xFF, sizeof(buffer));
}

/* Recursive parser with memory operations */
static struct ASTNode* parse_expression(int depth) {
    if (depth <= 0 || token_index >= 256) {
        return NULL;
    }
    
    struct ASTNode *node = malloc(sizeof(struct ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(struct ASTNode));
    
    node->type = token_array[token_index++];
    node->value = depth;
    node->volatile_field = depth * 2;
    
    /* Copy data using builtin memcpy */
    char temp[64];
    __builtin_snprintf(temp, sizeof(temp), "Node_%d_%d", depth, node->type);
    __builtin_memcpy(node->data, temp, strlen(temp) + 1);
    
    /* Recursive parsing */
    node->left = parse_expression(depth - 1);
    node->right = parse_expression(depth - 2);
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(struct ASTNode *dest, struct ASTNode *src) {
    if (!dest || !src) return;
    
    volatile int use_memmove = 1;
    
    if (use_memmove) {
        goto use_memmove_block;
    } else {
        goto skip_memmove;
    }
    
use_memmove_block:
    /* This block should trigger memmove redirection */
    __builtin_memmove(dest->data, src->data, sizeof(dest->data));
    goto after_memmove;
    
skip_memmove:
    /* Alternative path */
    __builtin_memset(dest->data, 0, sizeof(dest->data));
    
after_memmove:
    /* Continue processing */
    dest->value = src->value;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(struct ASTNode **nodes, int count) {
    volatile int chunk_size = 32; /* Volatile to prevent folding */
    
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Mix different builtins in parallel regions */
                if (tid % 3 == 0) {
                    __builtin_memset(nodes[i]->data, tid, chunk_size);
                } else if (tid % 3 == 1) {
                    char pattern[64];
                    __builtin_memset(pattern, 'A' + tid, sizeof(pattern));
                    __builtin_memcpy(nodes[i]->data, pattern, chunk_size);
                } else {
                    /* Create overlapping copy with memmove */
                    char temp[128];
                    __builtin_memcpy(temp, nodes[i]->data, sizeof(nodes[i]->data));
                    __builtin_memmove(nodes[i]->data, temp + 16, chunk_size);
                }
            }
        }
    }
}

/* Calculate hash of AST structure */
static int calculate_ast_hash(struct ASTNode *node) {
    if (!node) return 0;
    
    int hash = node->type ^ node->value ^ node->volatile_field;
    
    /* Process data with builtin memory access pattern */
    volatile int len = strlen(node->data);
    for (volatile int i = 0; i < len && i < 64; i++) {
        hash = (hash * 31) + node->data[i];
    }
    
    hash ^= calculate_ast_hash(node->left);
    hash ^= calculate_ast_hash(node->right);
    
    return hash;
}

int main(void) {
    /* Initialize token array with volatile writes */
    for (volatile int i = 0; i < 256; i++) {
        token_array[i] = (i * 13) % 97;
    }
    
    /* Create AST structure */
    struct ASTNode *root = parse_expression(5);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    struct ASTNode *nodes[8];
    nodes[0] = root;
    for (int i = 1; i < 8; i++) {
        nodes[i] = parse_expression(3);
    }
    
    /* Test goto with memmove */
    process_with_goto(nodes[1], nodes[0]);
    
    /* Parallel memory operations */
    parallel_memory_ops(nodes, 8);
    
    /* Additional builtin calls in main control flow */
    volatile char buffer1[128], buffer2[128];
    volatile int copy_size = 64;
    
    __builtin_memset(buffer1, 'X', sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, copy_size);
    __builtin_memmove(buffer1 + 32, buffer1, copy_size - 32);
    
    /* Calculate and print verification result */
    int total_hash = 0;
    for (int i = 0; i < 8; i++) {
        if (nodes[i]) {
            total_hash ^= calculate_ast_hash(nodes[i]);
        }
    }
    
    printf("Verification hash: 0x%08X\n", total_hash);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        free(nodes[i]);
    }
    
    return 0;
}
