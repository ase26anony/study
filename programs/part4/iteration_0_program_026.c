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
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)(i % 256);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->left = NULL;
    node->right = NULL;
    
    /* Use __builtin_memset to initialize data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy pattern using __builtin_memcpy with goto for flow control */
    int copy_len = volatile_len % 128;
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    
    /* Goto-based control flow */
    if (volatile_flag) {
        goto copy_block;
    } else {
        goto skip_copy;
    }
    
copy_block:
    /* This forces the compiler to process the memcpy in a goto context */
    __builtin_memcpy(node->data, token_pool + token_index, copy_len);
    token_index = (token_index + copy_len) % sizeof(token_pool);
    goto after_copy;
    
skip_copy:
    /* Alternative path */
    node->data[0] = 'X';
    
after_copy:
    /* Recursive creation with memmove between nodes */
    if (depth > 1) {
        node->left = create_ast_node(depth - 1, id * 2);
        node->right = create_ast_node(depth - 1, id * 2 + 1);
        
        /* Move data between nodes using __builtin_memmove */
        if (node->left && node->right) {
            int move_len = (volatile_len % 64) + 32;
            if (move_len > sizeof(node->data)) move_len = sizeof(node->data);
            
            __builtin_memmove(node->right->data, node->left->data, move_len);
        }
    }
    
    return node;
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_operations(ASTNode* nodes[], int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Force ASAN to process builtins in parallel context */
            int op_len = (i * 16 + volatile_len) % 128;
            
            /* Use all three builtins with volatile control */
            if (volatile_flag & 1) {
                __builtin_memset(nodes[i]->data + 32, i, op_len % 96);
            }
            
            if (volatile_flag & 2) {
                __builtin_memcpy(nodes[i]->data + 64, 
                               token_pool + (i * 32), 
                               op_len % 64);
            }
            
            /* Goto jumping into memmove block */
            if (i % 3 == 0) {
                goto memmove_block;
            } else {
                goto skip_memmove;
            }
            
        memmove_block:
            __builtin_memmove(nodes[i]->data, 
                            nodes[i]->data + 16, 
                            op_len % 48);
            goto after_memmove;
            
        skip_memmove:
            /* Do nothing */
            ;
            
        after_memmove:
            /* Continue execution */
            ;
        }
    }
}

/* Calculate hash from AST structure */
static unsigned long calculate_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    /* Simple hash calculation */
    for (int i = 0; i < sizeof(node->data) && *ptr; i++) {
        hash = ((hash << 5) + hash) + *ptr++;
    }
    
    hash += calculate_ast_hash(node->left);
    hash += calculate_ast_hash(node->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Create array of AST nodes */
    const int node_count = 8;
    ASTNode* nodes[node_count];
    
    /* Initialize nodes with recursive structure */
    for (int i = 0; i < node_count; i++) {
        nodes[i] = create_ast_node(3, i + 1);
        if (!nodes[i]) {
            fprintf(stderr, "Failed to create AST node %d\n", i);
            return 1;
        }
    }
    
    /* Execute parallel memory operations */
    printf("Executing parallel memory operations...\n");
    parallel_memory_operations(nodes, node_count);
    
    /* Verify operations by calculating combined hash */
    unsigned long total_hash = 0;
    for (int i = 0; i < node_count; i++) {
        total_hash ^= calculate_ast_hash(nodes[i]);
    }
    
    printf("Result hash: 0x%08lx\n", total_hash);
    
    /* Additional stress test with direct builtin calls */
    char buffer1[256], buffer2[256];
    
    /* Chain of builtin calls */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 32, buffer1, 128);
    __builtin_memset(buffer2 + 64, 0xBB, 96);
    __builtin_memcpy(buffer1, buffer2, 192);
    
    /* Cleanup */
    for (int i = 0; i < node_count; i++) {
        if (nodes[i]) {
            free(nodes[i]);
        }
    }
    
    printf("Test completed successfully\n");
    return 0;
}
