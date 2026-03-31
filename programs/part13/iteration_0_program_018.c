#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    char data[64];
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Global volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_flag = 1;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    volatile char buffer[128];
    /* Force __builtin_memset in constructor context */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    volatile char buffer[64];
    /* Force __builtin_memcpy in destructor context */
    char src[64];
    for (int i = 0; i < 64; i++) src[i] = i;
    __builtin_memcpy(buffer, src, 64);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = (*counter)++;
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[64];
    for (int i = 0; i < 64; i++) pattern[i] = (char)(i + depth);
    __builtin_memcpy(node->data, pattern, volatile_len & 63);
    
    /* Recursive creation with goto for control flow */
    int use_left = volatile_flag & 1;
    
    if (use_left) {
        goto create_left;
    } else {
        goto create_right;
    }
    
create_left:
    node->left = create_ast(depth - 1, counter);
    goto after_left;
    
create_right:
    node->right = create_ast(depth - 1, counter);
    goto after_left;
    
after_left:
    /* Create the other child */
    if (use_left) {
        node->right = create_ast(depth - 2, counter);
    } else {
        node->left = create_ast(depth - 2, counter);
    }
    
    return node;
}

/* Function with complex memory operations and goto */
static void process_ast_with_goto(ASTNode* node1, ASTNode* node2) {
    if (!node1 || !node2) return;
    
    volatile int direction = volatile_flag;
    
    if (direction & 1) {
        goto copy_left_to_right;
    } else {
        goto copy_right_to_left;
    }
    
copy_left_to_right:
    /* Use __builtin_memmove with goto context */
    if (node1->left && node2->right) {
        __builtin_memmove(node2->right->data, 
                         node1->left->data, 
                         volatile_len & 31);
    }
    goto after_copy;
    
copy_right_to_left:
    /* Another __builtin_memmove in different goto path */
    if (node1->right && node2->left) {
        __builtin_memmove(node2->left->data, 
                         node1->right->data, 
                         volatile_len & 31);
    }
    goto after_copy;
    
after_copy:
    /* Use __builtin_memcpy for remaining data */
    size_t copy_len = (volatile_len & 15) + 1;
    __builtin_memcpy(node2->data + 32, 
                    node1->data + 16, 
                    copy_len);
}

/* OpenMP parallel section with memory operations */
static uint64_t parallel_memory_operations(ASTNode** nodes, int count) {
    uint64_t total_hash = 0;
    
    #pragma omp parallel reduction(+:total_hash)
    {
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Use all three builtins in parallel context */
                char temp[64];
                
                /* __builtin_memcpy */
                __builtin_memcpy(temp, nodes[i]->data, 32);
                
                /* __builtin_memset on part of data */
                __builtin_memset(nodes[i]->data + 32, i, 32);
                
                /* __builtin_memmove for overlapping regions */
                __builtin_memmove(nodes[i]->data + 16, 
                                 nodes[i]->data + 8, 
                                 24);
                
                /* Compute simple hash */
                uint64_t hash = 0;
                for (int j = 0; j < 32; j++) {
                    hash = hash * 31 + (uint8_t)temp[j];
                }
                total_hash += hash;
            }
        }
    }
    
    return total_hash;
}

/* Main test driver */
int main(void) {
    int counter = 0;
    
    /* Create complex AST structure */
    ASTNode* root1 = create_ast(5, &counter);
    ASTNode* root2 = create_ast(4, &counter);
    
    if (!root1 || !root2) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Process with goto jumps */
    process_ast_with_goto(root1, root2);
    
    /* Create array of nodes for parallel processing */
    ASTNode* nodes[8];
    nodes[0] = root1;
    nodes[1] = root2;
    
    /* Create additional nodes with different patterns */
    for (int i = 2; i < 8; i++) {
        nodes[i] = create_ast(3 + (i % 3), &counter);
        
        /* Initialize with __builtin_memset pattern */
        if (nodes[i]) {
            __builtin_memset(nodes[i]->data, 0xFF - i, 64);
        }
    }
    
    /* Execute parallel memory operations */
    uint64_t result = parallel_memory_operations(nodes, 8);
    
    /* Final memory rearrangement using __builtin_memmove */
    if (root1 && root2) {
        volatile size_t move_len = volatile_len & 48;
        __builtin_memmove(root1->data, root2->data, move_len);
    }
    
    /* Print verification result */
    printf("Memory operations completed. Hash: %llu\n", 
           (unsigned long long)result);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        if (nodes[i]) free(nodes[i]);
    }
    
    return 0;
}
