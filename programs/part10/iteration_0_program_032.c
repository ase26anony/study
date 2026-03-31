#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Complex AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile int volatile_marker;  /* Prevent optimization */
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
} ASTNode;

/* Global token array */
volatile int token_array[256];
volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force initialization of memory builtins early */
    volatile char buffer[32];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(&token_array[0], buffer, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    volatile char cleanup_buf[64];
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
}

/* Recursive AST creation with memory operations */
ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    node->value = (*counter)++;
    node->volatile_marker = depth * 1000;
    
    /* Fill data with pattern */
    volatile int pattern = 0xDEADBEEF;
    __builtin_memset(node->data, (char)(pattern >> (depth * 8)), 32);
    
    /* Recursive creation */
    node->left = create_ast(depth - 1, counter);
    node->right = create_ast(depth - 1, counter);
    
    return node;
}

/* Complex memory operation with goto flow control */
void process_ast_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int use_memmove = 0;
    
    /* Jump into memory operation block */
    goto entry_point;
    
memory_operation:
    {
        volatile size_t copy_size = sizeof(src->data);
        
        if (use_memmove) {
            /* This tests the BUILT_IN_MEMMOVE case */
            __builtin_memmove(dst->data, src->data, copy_size);
        } else {
            /* This tests the BUILT_IN_MEMCPY case */
            __builtin_memcpy(dst->data, src->data, copy_size);
        }
        
        /* Test BUILT_IN_MEMSET */
        volatile char fill_char = 0x55;
        __builtin_memset(&dst->value, fill_char, sizeof(dst->value));
    }
    goto exit_point;
    
entry_point:
    /* Conditional goto to force flow sensitivity */
    if (src->type > 2) {
        use_memmove = 1;
        goto memory_operation;
    } else {
        goto memory_operation;
    }
    
exit_point:
    /* Additional operation after jump */
    volatile int temp[4];
    __builtin_memset(temp, 0xFF, sizeof(temp));
}

/* Parallel memory dispatch using OpenMP */
void parallel_memory_operations(ASTNode** nodes, int count) {
    volatile int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i] && nodes[(i + 1) % count]) {
            volatile size_t op_size = 16 + (i * 8) % 32;
            
            /* Mix different builtins in parallel regions */
            switch (i % 3) {
                case 0:
                    __builtin_memcpy(nodes[i]->data, 
                                   nodes[(i + 1) % count]->data, 
                                   op_size);
                    break;
                case 1:
                    __builtin_memset(nodes[i]->data, i, op_size);
                    break;
                case 2:
                    __builtin_memmove(nodes[i]->data, 
                                    nodes[(i + 1) % count]->data, 
                                    op_size);
                    break;
            }
        }
    }
}

/* Recursive parser with memory operations */
int recursive_parser(ASTNode* node, int depth) {
    if (!node || depth <= 0) return 0;
    
    volatile int result = node->value;
    
    /* Process children with goto jumps */
    ASTNode temp_node;
    
    /* Jump label for flow control */
    process_with_goto:
    process_ast_with_goto(node, &temp_node);
    
    /* Recursive calls */
    result += recursive_parser(node->left, depth - 1);
    
    /* Conditional goto */
    if (node->type % 3 == 0) {
        goto skip_right;
    }
    
    result += recursive_parser(node->right, depth - 1);
    
skip_right:
    /* Final memory operation */
    volatile char final_buf[128];
    __builtin_memset(final_buf, result & 0xFF, sizeof(final_buf));
    
    return result;
}

/* Free AST recursively */
void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear memory before free */
    volatile char clear_pattern = 0;
    __builtin_memset(node, clear_pattern, sizeof(ASTNode));
    free(node);
}

int main(void) {
    int counter = 1;
    volatile int hash_result = 0;
    
    /* Initialize token array with volatile operations */
    for (volatile int i = 0; i < 256; i++) {
        token_array[i] = i * 3;
    }
    
    /* Create complex AST */
    ASTNode* root = create_ast(4, &counter);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    ASTNode* node_array[8];
    node_array[0] = root;
    
    /* Build additional nodes */
    for (int i = 1; i < 8; i++) {
        node_array[i] = create_ast(3, &counter);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_operations(node_array, 8);
    
    /* Process with recursive parser and goto flow */
    hash_result = recursive_parser(root, 4);
    
    /* Additional complex memory pattern */
    volatile char pattern_buffer[1024];
    volatile char* dest = pattern_buffer;
    volatile char* src = pattern_buffer + 512;
    
    /* Force multiple builtin calls */
    for (volatile int i = 0; i < 16; i++) {
        volatile size_t len = 32 + (i * 7) % 64;
        
        switch (i % 3) {
            case 0:
                __builtin_memcpy(dest + i * 32, src, len);
                break;
            case 1:
                __builtin_memset(dest + i * 32, i, len);
                break;
            case 2:
                __builtin_memmove(dest + i * 32, src, len);
                break;
        }
        
        /* Update hash with volatile memory content */
        hash_result ^= *(volatile int*)(dest + i * 32);
    }
    
    /* Print verification result */
    printf("Result hash: %d\n", hash_result);
    printf("Token array[127]: %d\n", token_array[127]);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        if (node_array[i]) {
            free_ast(node_array[i]);
        }
    }
    
    return 0;
}
