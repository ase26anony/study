/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    uint32_t hash;
} ASTNode;

/* Global token array */
static char g_token_array[256];
static volatile int g_token_index = 0;

/* Constructor/destructor functions */
__attribute__((constructor)) static void init_tokens(void) {
    for (int i = 0; i < 256; i++) {
        g_token_array[i] = (char)((i * 13) & 0xFF);
    }
}

__attribute__((destructor)) static void cleanup(void) {
    /* Force memory operations in destructor */
    volatile char buf[16];
    __builtin_memset(buf, 0, sizeof(buf));
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data using builtin memcpy */
    size_t copy_len = (g_mem_size < 32) ? g_mem_size : 32;
    __builtin_memcpy(node->data, &g_token_array[g_token_index], copy_len);
    g_token_index = (g_token_index + 17) % 256;
    
    /* Recursive construction */
    node->left = parse_expression(depth - 1);
    node->right = parse_expression(depth - 1);
    
    /* Compute hash using memory operations */
    node->hash = 0;
    for (size_t i = 0; i < sizeof(node->data); i++) {
        node->hash = (node->hash * 31) + node->data[i];
    }
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int use_memmove = g_use_memmove;
    
    /* Jump into memory operation block */
    goto start_block;
    
mem_operation:
    if (use_memmove) {
        /* This should trigger BUILT_IN_MEMMOVE */
        __builtin_memmove(dst->data, src->data, g_mem_size % 32);
    } else {
        /* This should trigger BUILT_IN_MEMCPY */
        __builtin_memcpy(dst->data, src->data, g_mem_size % 32);
    }
    goto end_block;
    
start_block:
    /* Initialize with BUILT_IN_MEMSET */
    __builtin_memset(dst->data, 0xFF, sizeof(dst->data));
    goto mem_operation;
    
end_block:
    /* Update hash */
    dst->hash = 0;
    for (size_t i = 0; i < sizeof(dst->data); i++) {
        dst->hash = (dst->hash * 37) + dst->data[i];
    }
}

/* Parallel memory dispatch */
static uint32_t parallel_memory_ops(ASTNode* nodes[], size_t count) {
    uint32_t total_hash = 0;
    
    #pragma omp parallel reduction(+:total_hash)
    {
        #pragma omp for
        for (size_t i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Force all three builtins in parallel region */
                char temp[32];
                
                /* BUILT_IN_MEMCPY */
                __builtin_memcpy(temp, nodes[i]->data, 16);
                
                /* BUILT_IN_MEMMOVE with overlap */
                __builtin_memmove(nodes[i]->data + 8, nodes[i]->data, 16);
                
                /* BUILT_IN_MEMSET */
                __builtin_memset(nodes[i]->data + 24, i & 0xFF, 8);
                
                /* Update hash */
                uint32_t hash = 0;
                for (int j = 0; j < 32; j++) {
                    hash = (hash * 41) + nodes[i]->data[j];
                }
                nodes[i]->hash = hash;
                total_hash += hash;
            }
        }
    }
    
    return total_hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create AST */
    ASTNode* root = parse_expression(3);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create destination nodes */
    ASTNode* dest_nodes[4];
    for (int i = 0; i < 4; i++) {
        dest_nodes[i] = parse_expression(2);
    }
    
    /* Test goto edge cases */
    for (int i = 0; i < 4; i++) {
        g_use_memmove = i & 1;
        process_with_goto(root, dest_nodes[i]);
    }
    
    /* Test all three builtins in main */
    char buffer1[64], buffer2[64];
    
    /* BUILT_IN_MEMSET */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    
    /* BUILT_IN_MEMCPY */
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    
    /* BUILT_IN_MEMMOVE with overlap */
    __builtin_memmove(buffer1 + 16, buffer1, 32);
    
    /* Parallel operations */
    ASTNode* node_array[] = {root, dest_nodes[0], dest_nodes[1], dest_nodes[2], dest_nodes[3]};
    uint32_t parallel_hash = parallel_memory_ops(node_array, 5);
    
    /* Compute final verification hash */
    uint32_t final_hash = 0;
    for (int i = 0; i < 64; i++) {
        final_hash = (final_hash * 59) + buffer1[i];
        final_hash = (final_hash * 61) + buffer2[i];
    }
    
    final_hash ^= parallel_hash;
    final_hash ^= root->hash;
    
    for (int i = 0; i < 4; i++) {
        if (dest_nodes[i]) {
            final_hash ^= dest_nodes[i]->hash;
        }
    }
    
    printf("Verification hash: 0x%08X\n", final_hash);
    printf("Test completed\n");
    
    /* Cleanup */
    free_ast(root);
    for (int i = 0; i < 4; i++) {
        free_ast(dest_nodes[i]);
    }
    
    return 0;
}
