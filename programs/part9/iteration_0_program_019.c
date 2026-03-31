/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    uint32_t hash;
} ASTNode;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force initialization of ASAN runtime before main */
    volatile char buffer[16];
    __builtin_memset(buffer, 0xA5, sizeof(buffer));
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char final_check[8];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* build_ast(int depth, const char* token) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token using builtin memcpy with volatile size */
    size_t copy_len = g_mem_size % 64;
    if (copy_len > 63) copy_len = 63;
    __builtin_memcpy(node->data, token, copy_len);
    
    /* Build children recursively */
    node->left = build_ast(depth - 1, "LEFT");
    node->right = build_ast(depth - 1, "RIGHT");
    
    /* Compute hash using memory operations */
    node->hash = 0;
    for (size_t i = 0; i < sizeof(node->data); i++) {
        node->hash = (node->hash * 31) + node->data[i];
    }
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    volatile int use_memmove = g_use_memmove;
    
    if (use_memmove) {
        goto use_memmove_block;
    } else {
        goto use_memcpy_block;
    }
    
use_memmove_block:
    /* Jump into block containing builtin memmove */
    __builtin_memmove(dst->data, src->data, sizeof(src->data));
    goto after_operation;
    
use_memcpy_block:
    __builtin_memcpy(dst->data, src->data, sizeof(src->data));
    goto after_operation;
    
after_operation:
    /* Modify source after move to test overlap */
    __builtin_memset(src->data + 32, 0xCC, 16);
}

/* OpenMP parallel section */
static uint32_t parallel_ast_process(ASTNode* nodes[], size_t count) {
    uint32_t total_hash = 0;
    
    #pragma omp parallel reduction(+:total_hash)
    {
        #pragma omp for
        for (size_t i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Force builtin usage in parallel context */
                char temp[64];
                __builtin_memcpy(temp, nodes[i]->data, sizeof(temp));
                
                /* Conditional memmove with volatile control */
                volatile int do_move = (i % 3 == 0);
                if (do_move) {
                    __builtin_memmove(nodes[i]->data + 16, 
                                     nodes[i]->data, 32);
                }
                
                /* Recompute hash */
                uint32_t hash = 0;
                for (size_t j = 0; j < sizeof(nodes[i]->data); j++) {
                    hash = (hash * 31) + nodes[i]->data[j];
                }
                nodes[i]->hash = hash;
                total_hash += hash;
            }
        }
    }
    
    return total_hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Phase 1: Build AST structures */
    ASTNode* root = build_ast(3, "ROOT");
    if (!root) {
        fprintf(stderr, "Failed to build AST\n");
        return 1;
    }
    
    /* Phase 2: Test all three builtins explicitly */
    char buffer1[256];
    char buffer2[256];
    
    /* memset with volatile size */
    __builtin_memset(buffer1, 0xAA, g_mem_size % 256);
    
    /* memcpy with overlapping regions */
    __builtin_memcpy(buffer2, buffer1, 128);
    
    /* memmove with potential overlap */
    __builtin_memmove(buffer1 + 64, buffer1, 128);
    
    /* Phase 3: Goto-controlled memory operations */
    ASTNode* node_copy = (ASTNode*)malloc(sizeof(ASTNode));
    if (node_copy) {
        process_with_goto(root, node_copy);
        
        /* Switch memmove mode */
        g_use_memmove = 0;
        process_with_goto(root, node_copy);
    }
    
    /* Phase 4: OpenMP parallel processing */
    ASTNode* node_array[8];
    node_array[0] = root;
    for (int i = 1; i < 8; i++) {
        char token[16];
        snprintf(token, sizeof(token), "NODE_%d", i);
        node_array[i] = build_ast(2, token);
    }
    
    uint32_t parallel_hash = parallel_ast_process(node_array, 8);
    printf("Parallel hash result: %u\n", parallel_hash);
    
    /* Phase 5: Complex nested memory operations */
    for (int i = 0; i < 4; i++) {
        /* Create overlapping memory regions */
        char* region1 = malloc(512);
        char* region2 = region1 + 128;
        
        if (region1) {
            __builtin_memset(region1, i, 512);
            
            /* Force memmove redirection */
            __builtin_memmove(region2, region1, 256);
            
            /* Verify with memcpy */
            char verify[256];
            __builtin_memcpy(verify, region2, 256);
            
            free(region1);
        }
    }
    
    /* Cleanup */
    free(node_copy);
    for (int i = 0; i < 8; i++) {
        free(node_array[i]);
    }
    
    printf("Test completed successfully\n");
    return 0;
}
