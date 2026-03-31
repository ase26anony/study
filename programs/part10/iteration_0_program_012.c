/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 256 + (rand() % 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    printf("Destructor: ASAN test completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Fill data with pattern using builtin memcpy */
    char pattern[64];
    for (int i = 0; i < 64; i++) {
        pattern[i] = (char)((depth * 17 + i) % 256);
    }
    __builtin_memcpy(node->data, pattern, 64);
    
    node->size = g_mem_size;
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    return node;
}

/* Function with goto and memory operations */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int state = 0;
    
    /* Jump into memory operation block */
    if (src->size > 128) {
        goto mem_op_block;
    }
    
    state = 1;
    goto skip_mem;
    
mem_op_block:
    /* This block tests flow sensitivity */
    __builtin_memmove(dst->data, src->data, 64);
    state = 2;
    
skip_mem:
    /* Jump out of block */
    if (state == 1) {
        __builtin_memset(dst->data, 0xFF, 32);
    }
}

/* Parallel memory dispatch function */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i] && nodes[(i + 1) % count]) {
            /* Force builtin calls in parallel context */
            size_t copy_size = g_mem_size % 64;
            if (copy_size > 0) {
                __builtin_memcpy(
                    nodes[i]->data, 
                    nodes[(i + 1) % count]->data, 
                    copy_size
                );
            }
            
            /* Alternate between memset patterns */
            if (i % 3 == 0) {
                __builtin_memset(nodes[i]->data + 32, i, 16);
            }
        }
    }
}

/* Complex token processing with varied memory operations */
static unsigned long process_tokens(const char** tokens, int token_count) {
    unsigned long hash = 0;
    char buffer[256];
    char temp[256];
    
    /* Initialize with builtin memset */
    __builtin_memset(buffer, 0, sizeof(buffer));
    
    for (int i = 0; i < token_count; i++) {
        size_t len = strlen(tokens[i]);
        if (len > 0) {
            /* Copy token with builtin memcpy */
            __builtin_memcpy(temp, tokens[i], len);
            temp[len] = '\0';
            
            /* Move data around with builtin memmove */
            __builtin_memmove(buffer + hash % 128, temp, len % 64);
            
            /* Update hash */
            for (size_t j = 0; j < len; j++) {
                hash = (hash * 31) + (unsigned char)temp[j];
            }
        }
        
        /* Conditional goto to test edge cases */
        if (i == token_count / 2) {
            goto mid_point;
        }
    }
    
    return hash;

mid_point:
    /* Special processing at midpoint */
    __builtin_memset(buffer + 64, 0xAA, 32);
    return hash * 2;
}

/* Main test driver */
int main(void) {
    const char* tokens[] = {
        "ASAN", "TEST", "MEMCPY", "MEMSET", "MEMMOVE",
        "BUILTIN", "REDIRECTION", "COVERAGE", "GCC",
        "INSTRUMENTATION", "SANITIZER", "HARDWARE"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Token processing */
    unsigned long token_hash = process_tokens(tokens, token_count);
    printf("Token hash: %lu\n", token_hash);
    
    /* Phase 2: AST creation and manipulation */
    ASTNode* ast1 = create_ast(3);
    ASTNode* ast2 = create_ast(3);
    
    if (ast1 && ast2) {
        /* Test goto flow with memory operations */
        process_with_goto(ast1, ast2);
        
        /* Create array for parallel operations */
        ASTNode* nodes[6];
        nodes[0] = ast1;
        nodes[1] = ast2;
        nodes[2] = create_ast(2);
        nodes[3] = create_ast(2);
        nodes[4] = create_ast(1);
        nodes[5] = NULL;
        
        /* Phase 3: Parallel memory operations */
        parallel_memory_ops(nodes, 6);
        
        /* Verify results with final memory operation */
        char verify_buffer[256];
        __builtin_memset(verify_buffer, 0, sizeof(verify_buffer));
        __builtin_memcpy(verify_buffer, ast1->data, 64);
        __builtin_memmove(verify_buffer + 64, ast2->data, 64);
        
        /* Calculate final checksum */
        unsigned long final_sum = token_hash;
        for (int i = 0; i < 128; i++) {
            final_sum += (unsigned char)verify_buffer[i];
        }
        
        printf("Final checksum: %lu\n", final_sum);
        
        /* Cleanup */
        for (int i = 0; i < 6; i++) {
            if (nodes[i]) free(nodes[i]);
        }
    }
    
    printf("ASAN test completed successfully\n");
    return 0;
}
