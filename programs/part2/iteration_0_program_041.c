/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan",
    "instrument", "redzone", "shadow", "builtin"
};
static const int token_count = sizeof(tokens)/sizeof(tokens[0]);

/* Constructor/destructor functions */
__attribute__((constructor)) static void init_asan_env(void) {
    printf("Constructor: Initializing ASAN environment\n");
}

__attribute__((destructor)) static void cleanup_asan_env(void) {
    printf("Destructor: Cleaning up ASAN environment\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = depth;
    
    /* Copy token data using memcpy */
    const char* token = tokens[depth % token_count];
    size_t len = strlen(token);
    if (len > 255) len = 255;
    __builtin_memcpy(node->data, token, len);
    node->data[len] = '\0';
    
    /* Recursive creation */
    node->left = create_ast(depth + 1, max_depth);
    node->right = create_ast(depth + 2, max_depth);
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int use_memmove = g_use_memmove;
    
    if (use_memmove) goto use_memmove_block;
    
    /* Normal memcpy path */
    __builtin_memcpy(dst->data, src->data, sizeof(src->data));
    goto done;
    
use_memmove_block:
    /* Jump target with memmove */
    __builtin_memmove(dst->data, src->data, sizeof(src->data));
    
    /* Jump out of block */
    if (dst->id % 2) goto skip_adjust;
    
    /* Additional processing */
    dst->data[0] = 'X';
    
skip_adjust:
    /* Continue execution */
    dst->id += 1000;
    
done:
    return;
}

/* Parallel memory dispatch */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i] && nodes[i]->left) {
            /* Force built-in usage with volatile size */
            size_t size = g_mem_size;
            char buffer[256];
            
            /* Use all three built-ins */
            __builtin_memset(buffer, 0, sizeof(buffer));
            __builtin_memcpy(buffer, nodes[i]->data, size % 256);
            
            if (i % 3 == 0) {
                /* Conditional memmove */
                __builtin_memmove(nodes[i]->right->data, buffer, size % 256);
            }
        }
    }
}

/* Calculate hash from AST */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    /* DJB2 hash algorithm */
    while (*ptr) {
        hash = ((hash << 5) + hash) + *ptr++;
    }
    
    /* Recursive hash combination */
    hash += compute_ast_hash(node->left);
    hash ^= compute_ast_hash(node->right);
    
    return hash;
}

/* Free AST */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Create complex AST */
    ASTNode* root = create_ast(0, 5);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create copy for memmove operations */
    ASTNode* copy = (ASTNode*)malloc(sizeof(ASTNode));
    if (!copy) {
        free_ast(root);
        return 1;
    }
    
    /* Initialize copy with memset */
    __builtin_memset(copy, 0, sizeof(ASTNode));
    
    /* Test goto with memmove */
    process_with_goto(root, copy);
    
    /* Create node array for parallel processing */
    ASTNode* nodes[10];
    nodes[0] = root;
    nodes[1] = copy;
    nodes[2] = root->left;
    nodes[3] = root->right;
    nodes[4] = copy->left;
    
    /* Parallel memory operations */
    parallel_memory_ops(nodes, 5);
    
    /* Compute verification hash */
    unsigned long hash1 = compute_ast_hash(root);
    unsigned long hash2 = compute_ast_hash(copy);
    unsigned long total_hash = hash1 ^ hash2;
    
    printf("Result hash: %lu\n", total_hash);
    printf("Hash1: %lu, Hash2: %lu\n", hash1, hash2);
    
    /* Cleanup */
    free_ast(root);
    free(copy);
    
    printf("Test completed successfully\n");
    return 0;
}
