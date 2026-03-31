/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
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
static char global_tokens[4096];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Force initialization of ASAN runtime */
    volatile char init_buf[128];
    __builtin_memset(init_buf, 0, sizeof(init_buf));
    
    /* Use all three builtins in constructor */
    char src[64] = "Constructor initialization data";
    __builtin_memcpy(global_tokens, src, sizeof(src));
    __builtin_memset(global_tokens + 32, 'X', 16);
    
    token_index = 48;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Use memmove in destructor */
    char cleanup_buf[256];
    __builtin_memmove(cleanup_buf, global_tokens, 128);
    __builtin_memset(global_tokens, 0, sizeof(global_tokens));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* data, size_t len) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile length to prevent folding */
    size_t copy_len = len;
    if (volatile_flag) copy_len = volatile_len % 128;
    
    /* Test all three builtins in AST creation */
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, data, copy_len);
    
    /* Self-referential memmove */
    if (copy_len > 32) {
        __builtin_memmove(node->data + 16, node->data, 32);
    }
    
    node->size = copy_len;
    node->left = NULL;
    node->right = NULL;
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(ASTNode* dest, ASTNode* src) {
    int state = 0;
    
    /* Jump into block with memmove */
    if (volatile_flag) goto memmove_block;
    
    normal_path:
    __builtin_memcpy(dest->data, src->data, src->size);
    state = 1;
    goto continue_processing;
    
    memmove_block:
    /* This tests flow sensitivity */
    __builtin_memmove(dest->data, src->data, src->size);
    if (state == 0) goto normal_path;
    
    continue_processing:
    /* Another memcpy after goto */
    __builtin_memcpy(dest->data + 128, src->data, 
                    src->size > 128 ? 128 : src->size);
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    int i;
    char parallel_buf[1024];
    
    #pragma omp parallel private(i)
    {
        #pragma omp for
        for (i = 0; i < 16; i++) {
            char local_buf[256];
            volatile int idx = i * 64;
            
            /* Use all three builtins in parallel region */
            __builtin_memset(local_buf, i, sizeof(local_buf));
            __builtin_memcpy(parallel_buf + idx, local_buf, 64);
            
            /* Conditional memmove */
            if (i % 2 == 0) {
                __builtin_memmove(parallel_buf + idx + 32, 
                                 parallel_buf + idx, 32);
            }
        }
        
        /* Barrier with memory operation */
        #pragma omp barrier
        
        #pragma omp single
        {
            /* Master thread does additional memcpy */
            __builtin_memcpy(global_tokens, parallel_buf, 256);
        }
    }
}

/* Multi-stage initialization */
static void initialize_complex_state(void) {
    /* Stage 1: Direct builtin calls */
    char stage1[512];
    __builtin_memset(stage1, 'A', sizeof(stage1));
    
    /* Stage 2: Volatile-controlled operations */
    for (volatile int i = 0; i < 8; i++) {
        size_t len = (volatile_len + i) % 256;
        __builtin_memcpy(stage1 + i * 32, global_tokens, len);
    }
    
    /* Stage 3: Overlapping memmove */
    __builtin_memmove(stage1 + 128, stage1, 256);
    
    /* Copy to global */
    __builtin_memcpy(global_tokens, stage1, sizeof(stage1));
}

/* Main execution flow */
int main(void) {
    ASTNode* nodes[4];
    unsigned long hash = 0;
    
    /* Phase 1: Initialize complex state */
    initialize_complex_state();
    
    /* Phase 2: Create AST structure */
    for (int i = 0; i < 4; i++) {
        char node_data[128];
        __builtin_memset(node_data, '0' + i, sizeof(node_data));
        nodes[i] = create_ast_node(node_data, sizeof(node_data));
        
        if (i > 0) {
            /* Link nodes with memory operations */
            nodes[i]->left = nodes[i-1];
            __builtin_memcpy(nodes[i]->data + 64, 
                           nodes[i-1]->data, 64);
        }
    }
    
    /* Phase 3: Goto-based processing */
    for (int i = 1; i < 4; i++) {
        process_with_goto(nodes[i], nodes[i-1]);
    }
    
    /* Phase 4: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Phase 5: Compute verification hash */
    for (int i = 0; i < 4; i++) {
        for (size_t j = 0; j < nodes[i]->size && j < 256; j++) {
            hash = (hash * 31 + nodes[i]->data[j]) % 1000000007;
        }
        
        /* Cleanup with memset */
        if (i % 2 == 0) {
            __builtin_memset(nodes[i]->data + 128, 0, 128);
        }
        
        free(nodes[i]);
    }
    
    /* Final memory operation */
    char final_buf[128];
    __builtin_memcpy(final_buf, global_tokens, sizeof(final_buf));
    __builtin_memset(global_tokens + 1024, 0, 512);
    
    printf("Verification hash: %lu\n", hash);
    printf("Token index: %d\n", token_index);
    
    return 0;
}
