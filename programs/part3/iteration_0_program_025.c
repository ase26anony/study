/* ISO C99-compliant test program for ASAN built-in redirection */
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

/* Constructor function */
__attribute__((constructor))
static void init_constructor(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)(i % 256);
    }
}

/* Destructor function */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Clear sensitive data */
    __builtin_memset(token_pool, 0, sizeof(token_pool));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for initialization */
    __builtin_memcpy(node->data, base_data, 
                     volatile_len < 256 ? volatile_len : 256);
    
    node->id = depth;
    node->left = create_ast(depth - 1, base_data + 16);
    node->right = create_ast(depth - 1, base_data + 32);
    
    return node;
}

/* Function with goto and memory operations */
static void process_with_goto(ASTNode* dest, ASTNode* src) {
    int state = 0;
    
    if (!dest || !src) return;
    
start_label:
    if (state == 0) {
        /* Use __builtin_memmove with goto */
        __builtin_memmove(dest->data, src->data, 
                         volatile_len < 256 ? volatile_len : 256);
        state = 1;
        goto middle_label;
    }
    
middle_label:
    if (state == 1) {
        /* Use __builtin_memset */
        __builtin_memset(dest->data + 128, 0xAA, 
                        volatile_len / 2 < 128 ? volatile_len / 2 : 128);
        state = 2;
        goto end_label;
    }
    
end_label:
    /* Final memory operation */
    if (volatile_flag) {
        __builtin_memcpy(dest->data + 64, src->data + 64, 64);
    }
}

/* OpenMP parallel memory dispatch */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i] && nodes[(i + 1) % count]) {
            /* Mix of memory operations in parallel region */
            __builtin_memcpy(nodes[i]->data, 
                           nodes[(i + 1) % count]->data, 
                           128);
            
            __builtin_memset(nodes[i]->data + 128, i, 64);
            
            if (i % 3 == 0) {
                __builtin_memmove(nodes[i]->data + 64,
                                nodes[i]->data,
                                64);
            }
        }
    }
}

/* Calculate hash/sum of AST tree */
static unsigned long long compute_tree_hash(ASTNode* root) {
    if (!root) return 0;
    
    unsigned long long hash = 0;
    for (int i = 0; i < 256; i++) {
        hash += (unsigned long long)root->data[i] * (i + 1);
    }
    
    hash += compute_tree_hash(root->left);
    hash += compute_tree_hash(root->right);
    
    return hash;
}

/* Free AST tree */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear node data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create AST trees */
    ASTNode* tree1 = create_ast(4, token_pool);
    ASTNode* tree2 = create_ast(3, token_pool + 512);
    
    if (!tree1 || !tree2) {
        fprintf(stderr, "Failed to create AST trees\n");
        return 1;
    }
    
    /* Test goto flow with memory operations */
    process_with_goto(tree1, tree2);
    
    /* Create array of nodes for parallel processing */
    ASTNode* node_array[8];
    node_array[0] = tree1;
    node_array[1] = tree2;
    
    /* Create additional nodes */
    for (int i = 2; i < 8; i++) {
        node_array[i] = create_ast(2, token_pool + i * 128);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(node_array, 8);
    
    /* Compute and print verification hash */
    unsigned long long total_hash = 0;
    for (int i = 0; i < 8; i++) {
        if (node_array[i]) {
            total_hash += compute_tree_hash(node_array[i]);
        }
    }
    
    printf("Verification hash: %llu\n", total_hash);
    printf("Token pool checksum: ");
    
    /* Final memory operation in main */
    char checksum[64];
    __builtin_memset(checksum, 0, sizeof(checksum));
    __builtin_memcpy(checksum, token_pool, sizeof(checksum));
    
    for (int i = 0; i < 64; i++) {
        printf("%02x", (unsigned char)checksum[i]);
    }
    printf("\n");
    
    /* Cleanup */
    for (int i = 2; i < 8; i++) {
        free_ast(node_array[i]);
    }
    free_ast(tree1);
    free_ast(tree2);
    
    printf("Test completed successfully\n");
    return 0;
}
