/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
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
    size_t size;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_constructor(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (i % 256) ^ 0x55;
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_destructor(void) {
    /* Clear sensitive data */
    __builtin_memset(token_pool, 0, sizeof(token_pool));
    printf("Destructor: Token pool cleared\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for node initialization */
    size_t copy_len = volatile_len % 128;
    __builtin_memcpy(node->data, base_data, copy_len);
    node->size = copy_len;
    
    /* Create left subtree with goto control flow */
    int use_goto = volatile_flag;
    if (use_goto) {
        goto create_left;
    }
    
    node->left = NULL;
    goto skip_left;
    
create_left:
    node->left = create_ast(depth - 1, base_data + 16);
    
skip_left:
    /* Use __builtin_memset for right node initialization */
    __builtin_memset(node->data + 128, 0xAA, 64);
    
    /* Create right subtree */
    node->right = create_ast(depth - 1, base_data + 32);
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int mode = volatile_flag;
    
    if (mode == 0) {
        goto direct_copy;
    } else if (mode == 1) {
        goto memset_first;
    } else {
        goto memmove_ops;
    }

direct_copy:
    /* Jump into memcpy block */
    __builtin_memcpy(dst->data, src->data, src->size);
    goto after_ops;

memset_first:
    /* Clear destination first */
    __builtin_memset(dst->data, 0, sizeof(dst->data));
    goto memmove_ops;

memmove_ops:
    /* Use __builtin_memmove for overlapping regions */
    if (src && dst) {
        size_t move_len = volatile_len % 192;
        __builtin_memmove(dst->data + 32, dst->data, move_len);
        __builtin_memmove(dst->data, src->data, move_len);
    }
    goto after_ops;

after_ops:
    /* Verify operation */
    __builtin_memset(dst->data + 192, 0xCC, 32);
}

/* Parallel processing with OpenMP */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i] && nodes[(i + 1) % count]) {
            /* Mix of memory operations in parallel region */
            __builtin_memset(nodes[i]->data + 64, i, 32);
            
            if (i % 3 == 0) {
                __builtin_memcpy(nodes[i]->data, 
                               nodes[(i + 1) % count]->data, 
                               volatile_len % 128);
            } else if (i % 3 == 1) {
                __builtin_memmove(nodes[i]->data + 16,
                                nodes[i]->data,
                                volatile_len % 96);
            }
            
            /* Additional volatile-dependent operation */
            if (volatile_flag) {
                __builtin_memset(nodes[i]->data + 160, 0xFF, 16);
            }
        }
    }
}

/* Compute hash of AST structure */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    /* Process data with volatile length */
    for (int i = 0; i < (volatile_len % node->size); i++) {
        hash = ((hash << 5) + hash) + ptr[i];
    }
    
    /* Recursive hash computation */
    hash ^= compute_ast_hash(node->left);
    hash ^= compute_ast_hash(node->right);
    
    return hash;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Create AST structures */
    ASTNode* ast1 = create_ast(4, token_pool);
    ASTNode* ast2 = create_ast(3, token_pool + 128);
    ASTNode* ast3 = create_ast(2, token_pool + 256);
    
    if (!ast1 || !ast2 || !ast3) {
        fprintf(stderr, "Failed to create AST structures\n");
        return 1;
    }
    
    /* Array of nodes for parallel processing */
    ASTNode* nodes[] = {ast1, ast2, ast3};
    int node_count = sizeof(nodes) / sizeof(nodes[0]);
    
    /* Test goto control flow with memory operations */
    process_with_goto(ast1, ast2);
    process_with_goto(ast2, ast3);
    
    /* Execute parallel memory operations */
    parallel_memory_ops(nodes, node_count);
    
    /* Additional direct built-in calls */
    char buffer1[512], buffer2[512];
    
    __builtin_memset(buffer1, 0x11, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, volatile_len % 512);
    __builtin_memmove(buffer1 + 128, buffer1, volatile_len % 384);
    
    /* Process tokens with mixed operations */
    for (int i = 0; i < 8; i++) {
        size_t offset = i * 128;
        size_t len = (volatile_len + i) % 128;
        
        if (i % 2 == 0) {
            __builtin_memcpy(token_pool + offset, 
                           buffer2, 
                           len);
        } else {
            __builtin_memset(token_pool + offset, 
                           i, 
                           len);
        }
    }
    
    /* Compute and print verification hash */
    unsigned long total_hash = 0;
    total_hash ^= compute_ast_hash(ast1);
    total_hash ^= compute_ast_hash(ast2);
    total_hash ^= compute_ast_hash(ast3);
    
    /* Hash token pool */
    for (int i = 0; i < 256; i++) {
        total_hash = ((total_hash << 3) ^ total_hash) + token_pool[i];
    }
    
    printf("Verification hash: 0x%08lx\n", total_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(ast1);
    free(ast2);
    free(ast3);
    
    return 0;
}
