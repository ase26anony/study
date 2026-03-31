/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[256];
    /* Force memcpy redirection early */
    __builtin_memset(buffer, 0, sizeof(buffer));
    printf("Constructor: Early ASAN initialization triggered\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    printf("Destructor: Program cleanup completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset with volatile size */
    size_t copy_size = g_mem_size % 64;
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy data with builtin memcpy */
    if (base_data) {
        __builtin_memcpy(node->data, base_data, 
                        copy_size < strlen(base_data) ? copy_size : strlen(base_data));
    }
    
    node->size = copy_size;
    node->left = create_ast(depth - 1, "left_data");
    node->right = create_ast(depth - 1, "right_data");
    
    return node;
}

/* Function with goto and memory operations */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int state = 0;
    
    goto start_block;
    
copy_block:
    /* This block contains builtin memmove with goto entry */
    __builtin_memmove(dst->data, src->data, src->size);
    state = 1;
    goto end_block;
    
start_block:
    /* Initialize with builtin memset */
    __builtin_memset(dst->data, 0xFF, sizeof(dst->data));
    
    /* Conditional goto into memory block */
    if (src->size > 32) {
        goto copy_block;
    }
    
end_block:
    /* Final touch with builtin memcpy */
    __builtin_memcpy(src->data + 16, dst->data + 16, 16);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            volatile size_t local_size = g_mem_size % 32;
            
            /* Mix of all three builtins in parallel region */
            __builtin_memset(nodes[i]->data, i, local_size);
            
            if (i > 0 && nodes[i-1]) {
                __builtin_memcpy(nodes[i]->data + 16, 
                               nodes[i-1]->data + 16, 16);
            }
            
            /* Self-overlap with memmove */
            __builtin_memmove(nodes[i]->data + 8, 
                            nodes[i]->data, 8);
        }
    }
}

/* Complex token processing with memory operations */
static size_t process_tokens(const char** tokens, int token_count) {
    char buffer[512];
    size_t hash = 0;
    volatile int offset = 0;
    
    for (int i = 0; i < token_count; i++) {
        size_t len = strlen(tokens[i]);
        
        /* Use all three builtins in sequence */
        __builtin_memset(buffer + offset, 0, 32);
        __builtin_memcpy(buffer + offset, tokens[i], len);
        
        /* Overlapping move */
        if (offset > 16) {
            __builtin_memmove(buffer + offset - 8, 
                            buffer + offset, 8);
        }
        
        /* Compute simple hash */
        for (size_t j = 0; j < len; j++) {
            hash = (hash * 31) + buffer[offset + j];
        }
        
        offset += len;
        if (offset > 480) offset = 0;
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize AST structures */
    ASTNode* root = create_ast(3, "root_data");
    ASTNode* copy = create_ast(3, NULL);
    
    if (!root || !copy) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Phase 2: Test goto flow with memory operations */
    process_with_goto(root, copy);
    
    /* Phase 3: Create node array for parallel processing */
    ASTNode* nodes[8];
    for (int i = 0; i < 8; i++) {
        nodes[i] = create_ast(2, "node_data");
    }
    
    /* Phase 4: Parallel memory operations */
    parallel_memory_ops(nodes, 8);
    
    /* Phase 5: Token processing */
    const char* tokens[] = {
        "memcpy", "memset", "memmove", "asan", "hwasan",
        "builtin", "redirection", "coverage", "test"
    };
    
    size_t token_hash = process_tokens(tokens, 
                                      sizeof(tokens)/sizeof(tokens[0]));
    
    /* Phase 6: Complex overlapping operations */
    char overlap_buf[256];
    volatile size_t op_size = g_mem_size % 128;
    
    /* Chain of builtin calls */
    __builtin_memset(overlap_buf, 0xAA, sizeof(overlap_buf));
    __builtin_memcpy(overlap_buf + 64, overlap_buf, 64);
    __builtin_memmove(overlap_buf + 32, overlap_buf + 96, 64);
    
    /* Final verification hash */
    size_t final_hash = token_hash;
    for (int i = 0; i < 256; i++) {
        final_hash = (final_hash * 31) + overlap_buf[i];
    }
    
    printf("Test completed. Final hash: %zu\n", final_hash);
    
    /* Cleanup */
    free(root);
    free(copy);
    for (int i = 0; i < 8; i++) {
        free(nodes[i]);
    }
    
    return 0;
}
