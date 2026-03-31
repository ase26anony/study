/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Global volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_flag = 1;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    
    /* Force early built-in usage in constructor */
    char buffer1[128];
    char buffer2[128];
    
    /* Use all three built-ins in constructor */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 32, buffer1, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast_node(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with built-ins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    node->value = depth * 100;
    
    /* Fill data with pattern */
    for (int i = 0; i < 63; i++) {
        node->data[i] = 'A' + (i % 26);
    }
    node->data[63] = '\0';
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_goto = volatile_flag;
        
        if (use_goto) {
            goto create_left;
        }
        
        node->right = create_ast_node(depth - 1);
        
    create_left:
        node->left = create_ast_node(depth - 1);
        
        if (!use_goto) {
            node->right = create_ast_node(depth - 1);
        }
    } else {
        node->left = NULL;
        node->right = NULL;
    }
    
    return node;
}

/* Function with complex control flow using goto */
static void process_ast_with_goto(ASTNode* node, int* sum) {
    if (!node) return;
    
    int local_sum = 0;
    
    /* Jump into memory operation block */
    if (node->type % 2 == 0) {
        goto even_node;
    }
    
    /* Odd node processing */
    char temp[64];
    __builtin_memcpy(temp, node->data, volatile_len % 64);
    for (int i = 0; i < 64; i++) {
        local_sum += temp[i];
    }
    goto continue_processing;
    
even_node:
    /* Even node processing with memmove */
    char buffer[128];
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, node->data, 32);
    __builtin_memmove(buffer + 32, buffer, 32);
    
    for (int i = 0; i < 64; i++) {
        local_sum += buffer[i];
    }
    
    /* Jump out of block */
    if (node->value > 150) {
        goto skip_extra;
    }
    
    /* Extra processing */
    __builtin_memset(buffer + 64, 0xFF, 32);
    
skip_extra:
    /* Continue normal flow */
    
continue_processing:
    *sum += local_sum + node->value;
    
    /* Process children */
    process_ast_with_goto(node->left, sum);
    process_ast_with_goto(node->right, sum);
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    const int ARRAY_SIZE = 1024;
    char* src = (char*)malloc(ARRAY_SIZE);
    char* dst = (char*)malloc(ARRAY_SIZE);
    
    if (!src || !dst) {
        free(src);
        free(dst);
        return;
    }
    
    /* Initialize source with pattern */
    #pragma omp parallel for
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = (char)(i % 256);
    }
    
    /* Parallel memory operations */
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        size_t chunk_size = ARRAY_SIZE / omp_get_num_threads();
        size_t start = thread_id * chunk_size;
        size_t end = (thread_id == omp_get_num_threads() - 1) ? 
                     ARRAY_SIZE : start + chunk_size;
        
        /* Use all three built-ins in parallel region */
        __builtin_memset(dst + start, thread_id, end - start);
        __builtin_memcpy(src + start, dst + start, end - start);
        
        if (thread_id % 2 == 0) {
            __builtin_memmove(dst + start + (end - start)/2, 
                            dst + start, 
                            (end - start)/2);
        }
    }
    
    /* Verify by calculating checksum */
    unsigned long checksum = 0;
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (unsigned char)dst[i];
    }
    
    printf("Parallel checksum: %lu\n", checksum);
    
    free(src);
    free(dst);
}

/* Multi-stage initialization */
static void initialize_token_array(char tokens[][32], int count) {
    for (int i = 0; i < count; i++) {
        /* Use memset for initialization */
        __builtin_memset(tokens[i], 0, 32);
        
        /* Create pattern with memcpy */
        char pattern[] = "TOKEN_XXXX_XXXX_XXXX";
        pattern[6] = 'A' + (i % 26);
        pattern[11] = '0' + (i % 10);
        pattern[16] = 'a' + (i % 26);
        
        __builtin_memcpy(tokens[i], pattern, sizeof(pattern) - 1);
        
        /* Use memmove for in-place modification */
        if (i % 3 == 0) {
            __builtin_memmove(tokens[i] + 5, tokens[i] + 6, 10);
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Stage 1: Initialize token array */
    char tokens[8][32];
    initialize_token_array(tokens, 8);
    
    /* Stage 2: Create recursive AST */
    ASTNode* root = create_ast_node(4);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Stage 3: Process AST with goto control flow */
    int ast_sum = 0;
    process_ast_with_goto(root, &ast_sum);
    printf("AST processed sum: %d\n", ast_sum);
    
    /* Stage 4: Parallel memory operations */
    parallel_memory_operations();
    
    /* Stage 5: Additional built-in stress test */
    {
        char buffer1[256];
        char buffer2[256];
        char buffer3[256];
        
        /* Chain of memory operations */
        __builtin_memset(buffer1, 0xCC, sizeof(buffer1));
        __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
        __builtin_memmove(buffer3, buffer2, sizeof(buffer2));
        
        /* Overlapping regions */
        __builtin_memmove(buffer1 + 128, buffer1, 128);
        __builtin_memcpy(buffer2, buffer1 + 64, 128);
        
        /* Calculate final verification hash */
        unsigned int hash = 0;
        for (size_t i = 0; i < sizeof(buffer3); i++) {
            hash = (hash * 31) + buffer3[i];
        }
        printf("Final verification hash: %u\n", hash);
    }
    
    /* Cleanup */
    /* Note: In real code, you'd need to properly free the AST tree */
    
    printf("=== Test completed successfully ===\n");
    return 0;
}
