/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_array[1024];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(token_array, 0xAA, sizeof(token_array));
    printf("Constructor: Token array initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use builtin memcpy in destructor */
    char temp[64];
    __builtin_memcpy(temp, token_array, 64);
    printf("Destructor: Cleanup completed\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtin memset */
    __builtin_memset(node->data, id, sizeof(node->data));
    node->id = id;
    
    /* Create pattern in data */
    for (int i = 0; i < 16; i++) {
        node->data[i] = (char)(id + i);
    }
    
    /* Recursive creation with goto for control flow */
    int use_goto = (id % 3 == 0);
    
    if (use_goto) {
        goto create_children;
    }
    
    /* Normal path */
    node->left = create_ast_node(depth - 1, id * 2);
    node->right = create_ast_node(depth - 1, id * 2 + 1);
    return node;
    
create_children:
    /* Goto target with builtin memmove */
    ASTNode* temp_left = create_ast_node(depth - 1, id * 2);
    ASTNode* temp_right = create_ast_node(depth - 1, id * 2 + 1);
    
    /* Copy between nodes using builtin memcpy */
    if (temp_left && temp_right) {
        __builtin_memcpy(node->data + 16, temp_left->data, 16);
        __builtin_memmove(temp_right->data, node->data, 16);
    }
    
    node->left = temp_left;
    node->right = temp_right;
    return node;
}

/* Function with complex control flow using goto */
static void process_with_goto(ASTNode* node) {
    if (!node) return;
    
    char buffer[128];
    int do_copy = 1;
    
    /* Jump into memory operation block */
    if (node->id % 2 == 0) {
        goto perform_memmove;
    }
    
    /* Normal path with builtin memcpy */
    __builtin_memcpy(buffer, node->data, sizeof(node->data));
    goto after_operation;
    
perform_memmove:
    /* Goto target with builtin memmove */
    __builtin_memmove(buffer, node->data, volatile_len % 64);
    
    /* Jump out to different context */
    if (node->id % 3 == 0) {
        goto skip_additional;
    }
    
    /* Additional builtin memset */
    __builtin_memset(buffer + 32, 0xFF, 16);
    
skip_additional:
    /* Continue processing */
    ;
    
after_operation:
    /* Process buffer */
    for (int i = 0; i < 32; i++) {
        token_array[token_index++ % 1024] ^= buffer[i];
    }
}

/* Parallel memory dispatch function */
static void parallel_memory_dispatch(void) {
    int i;
    char local_buf[256];
    
    #pragma omp parallel private(i) shared(token_array, volatile_len)
    {
        #pragma omp for
        for (i = 0; i < 100; i++) {
            /* Each thread uses builtins with volatile lengths */
            int len = (volatile_len + i) % 128 + 1;
            
            /* Mix of builtin memory operations */
            if (i % 3 == 0) {
                __builtin_memset(local_buf, i, len);
                __builtin_memcpy((char*)volatile_dest, local_buf, len);
            } else if (i % 3 == 1) {
                __builtin_memcpy(local_buf, token_array + i, len);
                __builtin_memmove((char*)volatile_src, local_buf, len);
            } else {
                __builtin_memmove(local_buf, (char*)volatile_dest, len);
                __builtin_memset(local_buf + len/2, 0xCC, len/2);
            }
            
            /* Update token array */
            #pragma omp critical
            {
                __builtin_memcpy(token_array + (i * 8) % 1024, 
                               local_buf, 
                               len > 8 ? 8 : len);
            }
        }
    }
}

/* Function to compute hash from AST */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char temp[64];
    
    /* Process current node with builtins */
    __builtin_memcpy(temp, node->data, sizeof(node->data));
    
    for (int i = 0; i < 32; i++) {
        hash = ((hash << 5) + hash) + temp[i];
    }
    
    /* Recursive hash computation */
    hash ^= compute_ast_hash(node->left);
    hash ^= compute_ast_hash(node->right);
    
    return hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    /* Clear data before free with builtin memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Initialize volatile source */
    for (int i = 0; i < 256; i++) {
        ((char*)volatile_src)[i] = (char)(i % 256);
    }
    
    /* Create recursive AST structure */
    ASTNode* root = create_ast_node(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process AST with goto control flow */
    process_with_goto(root);
    process_with_goto(root->left);
    process_with_goto(root->right);
    
    /* Execute parallel memory operations */
    parallel_memory_dispatch();
    
    /* Additional builtin usage in main */
    char main_buffer[512];
    
    /* Force all three builtins to be used */
    __builtin_memset(main_buffer, 0, sizeof(main_buffer));
    __builtin_memcpy(main_buffer, token_array, 256);
    __builtin_memmove(main_buffer + 256, main_buffer, 128);
    
    /* Mix with volatile operations */
    int dynamic_len = volatile_len % 128;
    __builtin_memcpy((char*)volatile_dest, main_buffer, dynamic_len);
    __builtin_memmove(main_buffer, (char*)volatile_src, dynamic_len);
    
    /* Compute and print verification result */
    unsigned long hash = compute_ast_hash(root);
    unsigned long array_hash = 0;
    
    for (int i = 0; i < 1024; i++) {
        array_hash = (array_hash * 31) + token_array[i];
    }
    
    printf("AST Hash: %lu\n", hash);
    printf("Array Hash: %lu\n", array_hash);
    printf("Final XOR: %lu\n", hash ^ array_hash);
    
    /* Cleanup */
    free_ast(root);
    
    /* Final builtin usage */
    __builtin_memset(main_buffer, 0, sizeof(main_buffer));
    
    printf("Test completed successfully\n");
    return 0;
}
