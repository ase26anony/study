#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile size_t size;  /* volatile to prevent optimization */
    struct ASTNode *left;
    struct ASTNode *right;
    char buffer[64];
} ASTNode;

/* Global token array */
static volatile char global_tokens[256];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Force initialization with builtins */
    volatile char init_buf[32];
    __builtin_memset(init_buf, 0xA5, sizeof(init_buf));
    __builtin_memcpy((void*)global_tokens, init_buf, 32);
    
    /* Initialize token array with pattern */
    for (int i = 0; i < 256; i++) {
        global_tokens[i] = (char)(i ^ 0x55);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    volatile char cleanup_buf[16];
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
    __builtin_memcpy((void*)&global_tokens[240], cleanup_buf, 16);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, volatile int* counter) {
    if (depth <= 0 || *counter >= 100) {
        return NULL;
    }
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile variables for sizes */
    volatile size_t copy_size = (depth * 8) % 64;
    volatile size_t set_size = (depth * 7) % 64;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    node->value = (*counter)++;
    node->size = copy_size;
    
    /* Fill buffer with pattern using builtin memcpy */
    char pattern[64];
    __builtin_memset(pattern, node->value, sizeof(pattern));
    __builtin_memcpy(node->buffer, pattern, copy_size);
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 3) {
        goto skip_left;
    }
    
    create_left_label:
    node->left = create_ast_node(depth - 1, counter);
    
    skip_left:
    if (!create_left) {
        goto create_right;
    }
    
    /* Jump back into block with memmove */
    if (depth % 2 == 0) {
        create_left = 0;
        char temp[64];
        __builtin_memcpy(temp, node->buffer, copy_size);
        __builtin_memmove(node->buffer + 8, node->buffer, copy_size - 8);
        __builtin_memcpy(node->buffer, temp, 8);
        goto create_left_label;
    }
    
    create_right:
    node->right = create_ast_node(depth - 2, counter);
    
    return node;
}

/* Process AST with memory operations */
static uint32_t process_ast(ASTNode* node, volatile int mode) {
    if (!node) return 0;
    
    uint32_t hash = 0;
    volatile char temp_buf[128];
    
    /* Different memory operations based on mode */
    switch (mode % 3) {
        case 0:
            __builtin_memcpy(temp_buf, node->buffer, node->size);
            __builtin_memset(node->buffer + node->size/2, 0xCC, node->size/2);
            break;
        case 1:
            __builtin_memmove(temp_buf + 32, temp_buf, node->size);
            __builtin_memset(temp_buf, node->value, 32);
            break;
        case 2:
            __builtin_memcpy(temp_buf, global_tokens + token_index, 64);
            __builtin_memmove(global_tokens + token_index, 
                            global_tokens + token_index + 32, 32);
            token_index = (token_index + 1) % 192;
            break;
    }
    
    /* Compute hash from buffer */
    for (size_t i = 0; i < node->size && i < 64; i++) {
        hash = (hash << 5) - hash + (uint8_t)node->buffer[i];
    }
    
    /* Recursive processing */
    hash ^= process_ast(node->left, mode + 1);
    hash ^= process_ast(node->right, mode + 2);
    
    return hash;
}

/* Parallel memory dispatch */
static uint32_t parallel_memory_ops(void) {
    uint32_t total_hash = 0;
    volatile int counters[4] = {0, 0, 0, 0};
    ASTNode* trees[4] = {NULL};
    
    #pragma omp parallel for reduction(^:total_hash)
    for (int i = 0; i < 4; i++) {
        volatile int counter = 0;
        ASTNode* tree = create_ast_node(5 + i, &counter);
        trees[i] = tree;
        
        if (tree) {
            uint32_t hash = process_ast(tree, i);
            
            /* Additional memory operations in parallel region */
            volatile char local_buf[256];
            __builtin_memcpy(local_buf, global_tokens, 256);
            
            /* Use all three builtins */
            __builtin_memset(local_buf + 128, i, 64);
            __builtin_memmove(local_buf + 64, local_buf, 128);
            __builtin_memcpy((void*)global_tokens, local_buf + 64, 128);
            
            total_hash ^= hash;
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        if (trees[i]) {
            free(trees[i]);
        }
    }
    
    return total_hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN coverage test...\n");
    
    /* Initialize complex state */
    volatile int init_counter = 0;
    ASTNode* root = create_ast_node(6, &init_counter);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process with different modes */
    uint32_t hash1 = process_ast(root, 0);
    uint32_t hash2 = process_ast(root, 1);
    
    /* Execute parallel operations */
    uint32_t parallel_hash = parallel_memory_ops();
    
    /* Final memory operations with goto flow control */
    volatile char final_buf[512];
    int use_memmove = 1;
    
    goto start_ops;
    
    memcpy_block:
    __builtin_memcpy(final_buf, root->buffer, root->size);
    goto next_op;
    
    start_ops:
    if (use_memmove) {
        __builtin_memmove(final_buf + 256, final_buf, 256);
        use_memmove = 0;
        goto memcpy_block;
    }
    
    next_op:
    __builtin_memset(final_buf + 384, 0xFF, 128);
    
    /* Compute final result */
    uint32_t final_hash = hash1 ^ hash2 ^ parallel_hash;
    for (int i = 0; i < 256; i++) {
        final_hash = (final_hash << 3) - final_hash + (uint8_t)global_tokens[i];
    }
    
    printf("Result hash: 0x%08X\n", final_hash);
    
    /* Cleanup */
    free(root);
    
    /* Verify with one last builtin */
    volatile char verify[16];
    __builtin_memset(verify, 0xAA, sizeof(verify));
    
    return (final_hash != 0) ? 0 : 1;
}
