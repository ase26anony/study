/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    int value;
} ASTNode;

/* Token array for parser simulation */
typedef struct {
    char tokens[32][16];
    int count;
} TokenArray;

/* Global cache for built-in redirection testing */
static unsigned char g_buffer_a[1024];
static unsigned char g_buffer_b[1024];
static unsigned char g_buffer_c[1024];

/* Constructor function to force early initialization */
__attribute__((constructor))
static void init_asan_globals(void) {
    /* Force initialization of memory buffers */
    __builtin_memset(g_buffer_a, 0xAA, sizeof(g_buffer_a));
    __builtin_memset(g_buffer_b, 0xBB, sizeof(g_buffer_b));
    __builtin_memset(g_buffer_c, 0xCC, sizeof(g_buffer_c));
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan_globals(void) {
    /* Verify buffers were modified */
    size_t sum = 0;
    for (size_t i = 0; i < sizeof(g_buffer_a); i++) {
        sum += g_buffer_a[i];
    }
    printf("Destructor: Buffer A checksum = %zu\n", sum);
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(TokenArray* tokens, int* pos) {
    if (*pos >= tokens->count) {
        return NULL;
    }
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        return NULL;
    }
    
    /* Initialize node with built-in memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data using built-in memcpy */
    size_t token_len = strlen(tokens->tokens[*pos]);
    if (token_len > sizeof(node->data) - 1) {
        token_len = sizeof(node->data) - 1;
    }
    
    __builtin_memcpy(node->data, tokens->tokens[*pos], token_len);
    node->data[token_len] = '\0';
    
    (*pos)++;
    
    /* Control flow with goto to test edge cases */
    if (node->data[0] == 'j') {
        goto jump_label;
    }
    
    /* Normal recursive parsing */
    node->left = parse_expression(tokens, pos);
    
jump_label:
    /* Memory move operation after goto */
    if (node->left) {
        ASTNode temp;
        __builtin_memmove(&temp, node->left, sizeof(ASTNode));
        __builtin_memcpy(node->left, &temp, sizeof(ASTNode));
    }
    
    node->right = parse_expression(tokens, pos);
    
    return node;
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_operations(void) {
    volatile size_t local_size = g_mem_size;
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        unsigned char local_buf[128];
        
        /* Each thread uses built-in memory functions */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp for
        for (int i = 0; i < 8; i++) {
            size_t offset = i * 32;
            if (offset + 32 <= sizeof(g_buffer_a)) {
                /* Mixed memory operations in parallel region */
                __builtin_memcpy(&g_buffer_a[offset], local_buf, 32);
                __builtin_memmove(&g_buffer_b[offset], &g_buffer_a[offset], 32);
                __builtin_memset(&g_buffer_c[offset], i, 32);
            }
        }
        
        /* Barrier to ensure all memory ops complete */
        #pragma omp barrier
        
        /* Verify with another memory operation */
        if (thread_id == 0) {
            unsigned char verify_buf[32];
            __builtin_memcpy(verify_buf, g_buffer_a, 32);
        }
    }
}

/* Complex memory dispatch with control flow */
static void memory_dispatch_logic(int mode) {
    volatile int use_memmove = (mode & 1);
    volatile int use_memcpy = (mode & 2);
    volatile int use_memset = (mode & 4);
    
    unsigned char* src = g_buffer_a;
    unsigned char* dst = g_buffer_b;
    
    /* Goto-based control flow around memory operations */
    if (use_memmove) {
        goto do_memmove;
    }
    
    if (use_memcpy) {
        __builtin_memcpy(dst, src, g_mem_size);
        goto after_ops;
    }
    
do_memmove:
    __builtin_memmove(dst, src, g_mem_size);
    
    if (use_memset) {
        goto do_memset;
    }
    
    goto after_ops;
    
do_memset:
    __builtin_memset(dst, 0x55, g_mem_size);
    
after_ops:
    /* Additional memory operation after label */
    if (mode > 0) {
        __builtin_memcpy(g_buffer_c, dst, g_mem_size / 2);
    }
}

/* Calculate hash of AST tree */
static size_t calculate_tree_hash(ASTNode* node) {
    if (!node) {
        return 0;
    }
    
    size_t hash = 5381;
    
    /* Process node data */
    for (int i = 0; node->data[i] != '\0'; i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    /* Recursive hash calculation */
    size_t left_hash = calculate_tree_hash(node->left);
    size_t right_hash = calculate_tree_hash(node->right);
    
    /* Combine hashes with memory operation */
    unsigned char hash_bytes[16];
    __builtin_memset(hash_bytes, 0, sizeof(hash_bytes));
    __builtin_memcpy(hash_bytes, &left_hash, sizeof(left_hash));
    __builtin_memcpy(hash_bytes + 8, &right_hash, sizeof(right_hash));
    
    for (int i = 0; i < 16; i++) {
        hash = ((hash << 5) + hash) + hash_bytes[i];
    }
    
    return hash;
}

/* Free AST tree with memory verification */
static void free_ast_tree(ASTNode* node) {
    if (!node) {
        return;
    }
    
    /* Recursive free */
    free_ast_tree(node->left);
    free_ast_tree(node->right);
    
    /* Verify node wasn't corrupted before free */
    unsigned char verify_buf[sizeof(ASTNode)];
    __builtin_memcpy(verify_buf, node, sizeof(ASTNode));
    
    /* Clear memory before free */
    __builtin_memset(node, 0, sizeof(ASTNode));
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize token array for parser */
    TokenArray tokens;
    tokens.count = 10;
    
    const char* sample_tokens[] = {
        "alpha", "beta", "gamma", "delta", "epsilon",
        "jump", "zeta", "eta", "theta", "iota"
    };
    
    for (int i = 0; i < tokens.count; i++) {
        size_t len = strlen(sample_tokens[i]);
        if (len > sizeof(tokens.tokens[0]) - 1) {
            len = sizeof(tokens.tokens[0]) - 1;
        }
        __builtin_memcpy(tokens.tokens[i], sample_tokens[i], len);
        tokens.tokens[i][len] = '\0';
    }
    
    /* Parse expression tree */
    int pos = 0;
    ASTNode* root = parse_expression(&tokens, &pos);
    
    if (!root) {
        fprintf(stderr, "Failed to parse expression tree\n");
        return 1;
    }
    
    /* Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Test various memory dispatch modes */
    for (int mode = 0; mode < 8; mode++) {
        memory_dispatch_logic(mode);
    }
    
    /* Calculate and print verification hash */
    size_t tree_hash = calculate_tree_hash(root);
    size_t buffer_hash = 0;
    
    /* Hash buffer contents */
    for (size_t i = 0; i < sizeof(g_buffer_a); i++) {
        buffer_hash = ((buffer_hash << 5) + buffer_hash) + g_buffer_a[i];
        buffer_hash = ((buffer_hash << 5) + buffer_hash) + g_buffer_b[i];
        buffer_hash = ((buffer_hash << 5) + buffer_hash) + g_buffer_c[i];
    }
    
    printf("Tree hash: %zu\n", tree_hash);
    printf("Buffer hash: %zu\n", buffer_hash);
    printf("Final checksum: %zu\n", tree_hash ^ buffer_hash);
    
    /* Cleanup */
    free_ast_tree(root);
    
    /* Final memory operation to ensure all paths are tested */
    __builtin_memset(g_buffer_a, 0xFF, sizeof(g_buffer_a) / 2);
    __builtin_memcpy(g_buffer_b, g_buffer_a, sizeof(g_buffer_a) / 2);
    __builtin_memmove(g_buffer_c, g_buffer_b, sizeof(g_buffer_b) / 2);
    
    printf("ASAN test completed successfully\n");
    return 0;
}
