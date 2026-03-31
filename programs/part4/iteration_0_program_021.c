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
    unsigned char data[64];
} ASTNode;

/* Global volatile variables to prevent constant folding */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

/* Constructor function to force early initialization */
__attribute__((constructor))
static void init_asan_early(void) {
    /* Force initialization of memory function redirection */
    volatile char buffer[128];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor to test cleanup paths */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char final_check[32];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int value) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile size to prevent optimization */
    volatile size_t copy_size = g_mem_size / (depth + 1);
    if (copy_size > sizeof(node->data)) copy_size = sizeof(node->data);
    
    /* Initialize node data with builtins */
    __builtin_memset(node->data, value, copy_size);
    
    node->type = depth;
    node->value = value;
    node->size = copy_size;
    
    /* Create children with different memory patterns */
    node->left = create_ast(depth - 1, value * 2);
    node->right = create_ast(depth - 1, value * 3);
    
    /* Copy data between nodes if children exist */
    if (node->left && node->right) {
        size_t min_size = node->left->size < node->right->size ? 
                         node->left->size : node->right->size;
        
        /* Use goto to create complex control flow around memmove */
        if (min_size > 16) {
            goto do_memmove;
        } else {
            __builtin_memcpy(node->right->data, node->left->data, min_size);
            goto skip_memmove;
        }
        
    do_memmove:
        /* This tests the memmove redirection with goto */
        __builtin_memmove(node->right->data, node->left->data, min_size);
        
    skip_memmove:
        /* Additional memcpy after the label */
        if (node->value % 2) {
            __builtin_memcpy(node->data, node->left->data, 
                           min_size < sizeof(node->data) ? min_size : sizeof(node->data));
        }
    }
    
    return node;
}

/* Function with OpenMP parallel section */
static void parallel_memory_operations(ASTNode* nodes[], int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i] && nodes[i]->left) {
            volatile size_t op_size = nodes[i]->size;
            
            /* Mix different memory operations in parallel */
            switch (i % 3) {
                case 0:
                    __builtin_memcpy(nodes[i]->data, nodes[i]->left->data, 
                                   op_size < sizeof(nodes[i]->data) ? op_size : sizeof(nodes[i]->data));
                    break;
                case 1:
                    __builtin_memset(nodes[i]->right->data, i, 
                                   op_size < sizeof(nodes[i]->right->data) ? op_size : sizeof(nodes[i]->right->data));
                    break;
                case 2:
                    if (nodes[i]->right) {
                        __builtin_memmove(nodes[i]->data, nodes[i]->right->data,
                                        op_size < sizeof(nodes[i]->data) ? op_size : sizeof(nodes[i]->data));
                    }
                    break;
            }
        }
    }
}

/* Complex control flow with goto around memory operations */
static void test_goto_memmove(void) {
    volatile char src[256];
    volatile char dst[256];
    volatile int condition = 1;
    
    /* Initialize source with pattern */
    for (int i = 0; i < sizeof(src); i++) {
        src[i] = i % 256;
    }
    
    /* Use goto to jump into block with memmove */
    if (condition) {
        goto jump_into_memmove;
    }
    
    /* This should be skipped */
    __builtin_memset(dst, 0, sizeof(dst));
    
jump_into_memmove:
    /* This memmove should be redirected by ASAN */
    __builtin_memmove((void*)dst, (void*)src, sizeof(src));
    
    /* Jump out and do another operation */
    goto after_memmove;
    
    /* Unreachable code with memcpy */
    __builtin_memcpy(dst, src, sizeof(src));
    
after_memmove:
    /* Final memset to ensure all builtins are used */
    __builtin_memset((void*)src, 0xFF, sizeof(src) / 2);
}

/* Calculate hash of AST tree */
static uint32_t hash_ast(ASTNode* node) {
    if (!node) return 0;
    
    uint32_t hash = node->value;
    for (size_t i = 0; i < sizeof(node->data) && i < node->size; i++) {
        hash = (hash * 31) + node->data[i];
    }
    
    hash ^= hash_ast(node->left);
    hash ^= hash_ast(node->right);
    
    return hash;
}

int main(void) {
    ASTNode* nodes[8] = {0};
    uint32_t final_hash = 0;
    
    /* Initialize token array */
    volatile int tokens[128];
    for (int i = 0; i < 128; i++) {
        tokens[i] = i * 3;
    }
    
    /* Create recursive AST structures */
    for (int i = 0; i < 4; i++) {
        nodes[i] = create_ast(3 + i, 100 + i * 50);
    }
    
    /* Test goto-based control flow */
    test_goto_memmove();
    
    /* Perform parallel memory operations */
    parallel_memory_operations(nodes, 4);
    
    /* Calculate verification hash */
    for (int i = 0; i < 4; i++) {
        if (nodes[i]) {
            final_hash ^= hash_ast(nodes[i]);
            
            /* Additional memory operation in main */
            if (nodes[i]->left) {
                volatile size_t final_size = g_mem_size / 4;
                __builtin_memcpy(nodes[i]->right->data, 
                               nodes[i]->left->data,
                               final_size < sizeof(nodes[i]->right->data) ? 
                               final_size : sizeof(nodes[i]->right->data));
            }
        }
    }
    
    /* Print result for verification */
    printf("Final hash: 0x%08X\n", final_hash);
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        if (nodes[i]) {
            /* Use memset before free */
            __builtin_memset(nodes[i]->data, 0, sizeof(nodes[i]->data));
            free(nodes[i]);
        }
    }
    
    return 0;
}
