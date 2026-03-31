/* asan_coverage.c - Comprehensive test for ASAN memory built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    uint32_t hash;
} ASTNode;

/* Global token array */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "test"
};
static const int g_token_count = 6;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_sanitizer_hook(void) {
    volatile char buffer[32];
    /* Force initialization of memcpy redirection */
    __builtin_memset(buffer, 0, sizeof(buffer));
    printf("Constructor: Initialized sanitizer hooks\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_sanitizer_hook(void) {
    volatile char buffer[16];
    /* Force use of memmove redirection */
    __builtin_memmove(buffer, buffer + 8, 8);
    printf("Destructor: Cleaned up sanitizer hooks\n");
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth, const char** tokens, int* index) {
    if (depth <= 0 || *index >= g_token_count) {
        return NULL;
    }
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data with memcpy */
    const char* token = tokens[(*index)++];
    size_t len = strlen(token);
    if (len > sizeof(node->data) - 1) {
        len = sizeof(node->data) - 1;
    }
    __builtin_memcpy(node->data, token, len);
    node->data[len] = '\0';
    
    /* Control flow with goto */
    if (depth > 2) {
        goto recursive_branch;
    }
    
    node->left = parse_expression(depth - 1, tokens, index);
    
recursive_branch:
    node->right = parse_expression(depth - 2, tokens, index);
    
    /* Calculate hash using memory operations */
    uint32_t hash = 0;
    for (size_t i = 0; i < len; i++) {
        hash = (hash * 31) + node->data[i];
    }
    node->hash = hash;
    
    return node;
}

/* Complex memory operation with goto edge cases */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int use_memmove = 0;
    
    /* Jump into memory operation block */
    if (src->hash % 3 == 0) {
        goto do_memcpy;
    } else if (src->hash % 3 == 1) {
        goto do_memset;
    } else {
        use_memmove = 1;
        goto do_memmove;
    }
    
do_memcpy:
    /* Copy data between nodes */
    __builtin_memcpy(dst->data, src->data, sizeof(src->data));
    goto done;
    
do_memset:
    /* Clear destination */
    __builtin_memset(dst->data, 0, sizeof(dst->data));
    goto done;
    
do_memmove:
    /* Overlapping memory operation */
    if (dst->data + 16 < src->data + sizeof(src->data)) {
        __builtin_memmove(dst->data, src->data + 16, 32);
    }
    goto done;
    
done:
    /* Update hash */
    dst->hash = src->hash ^ 0xDEADBEEF;
}

/* Parallel memory dispatch */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count - 1; i++) {
        volatile size_t local_size = g_mem_size;
        char buffer[512];
        
        /* Varied memory operations */
        switch (i % 4) {
            case 0:
                __builtin_memset(buffer, i, local_size % 256);
                break;
            case 1:
                __builtin_memcpy(buffer, nodes[i]->data, 
                               strlen(nodes[i]->data) + 1);
                break;
            case 2:
                if (i > 0) {
                    __builtin_memmove(buffer + 64, buffer, 128);
                }
                break;
            case 3:
                /* Combined operations */
                __builtin_memset(buffer, 0xFF, 64);
                __builtin_memcpy(buffer + 64, nodes[i]->data, 32);
                __builtin_memmove(buffer, buffer + 32, 96);
                break;
        }
        
        /* Update node with result */
        if (nodes[i] && nodes[i + 1]) {
            size_t copy_len = strlen(nodes[i]->data);
            if (copy_len > sizeof(nodes[i + 1]->data) - 1) {
                copy_len = sizeof(nodes[i + 1]->data) - 1;
            }
            __builtin_memcpy(nodes[i + 1]->data, nodes[i]->data, copy_len);
            nodes[i + 1]->data[copy_len] = '\0';
        }
    }
}

/* Calculate tree hash sum */
static uint32_t compute_tree_hash(ASTNode* root) {
    if (!root) return 0;
    
    uint32_t sum = root->hash;
    sum += compute_tree_hash(root->left);
    sum += compute_tree_hash(root->right);
    
    /* Additional memory operation in recursion */
    volatile char temp[16];
    __builtin_memset(temp, sum & 0xFF, sizeof(temp));
    
    return sum;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Initialize recursive parser */
    int token_index = 0;
    ASTNode* root = parse_expression(5, g_tokens, &token_index);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create node array for parallel processing */
    ASTNode* nodes[10];
    nodes[0] = root;
    
    /* Build additional nodes */
    for (int i = 1; i < 10; i++) {
        nodes[i] = (ASTNode*)malloc(sizeof(ASTNode));
        if (nodes[i]) {
            __builtin_memset(nodes[i], 0, sizeof(ASTNode));
            __builtin_memcpy(nodes[i]->data, g_tokens[i % g_token_count], 
                           strlen(g_tokens[i % g_token_count]));
            nodes[i]->hash = i * 1000;
        }
    }
    
    /* Test control flow with goto */
    for (int i = 0; i < 5; i++) {
        process_with_goto(nodes[i], nodes[i + 1]);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(nodes, 10);
    
    /* Compute final result */
    uint32_t total_hash = 0;
    for (int i = 0; i < 10; i++) {
        if (nodes[i]) {
            total_hash += compute_tree_hash(nodes[i]);
        }
    }
    
    printf("Final hash sum: 0x%08X\n", total_hash);
    
    /* Cleanup */
    for (int i = 0; i < 10; i++) {
        free(nodes[i]);
    }
    
    printf("Test completed successfully\n");
    return 0;
}
