#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    volatile int depth;
} ASTNode;

/* Global volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_init_flag = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    g_init_flag = 1;
    printf("Constructor: Initializing ASAN environment\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive AST creation with memory operations */
ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->depth = depth;
    
    /* Create pattern in data using __builtin_memcpy */
    char pattern[64];
    __builtin_memset(pattern, 'A' + (depth % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, g_mem_size);
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_goto = depth % 3;
        
        if (use_goto == 0) {
            goto create_left;
        } else if (use_goto == 1) {
            node->left = create_ast(depth - 1);
            goto create_right;
        } else {
            node->left = create_ast(depth - 1);
            node->right = create_ast(depth - 1);
            goto done;
        }
        
    create_left:
        node->left = create_ast(depth - 1);
        goto skip_right;
        
    create_right:
        node->right = create_ast(depth - 1);
        goto done;
        
    skip_right:
        node->right = NULL;
        
    done:
        ;
    }
    
    return node;
}

/* Complex memory operation between AST nodes */
void process_ast_nodes(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int direction = src->depth % 2;
    
    /* Use __builtin_memmove for overlapping/adjacent memory */
    if (direction == 0) {
        __builtin_memmove(dst->data, src->data, g_mem_size);
    } else {
        /* Create overlapping scenario */
        char temp[128];
        __builtin_memcpy(temp, src->data, g_mem_size);
        __builtin_memmove(dst->data, temp + 32, g_mem_size - 32);
    }
    
    /* Process children recursively */
    process_ast_nodes(src->left, dst->left);
    process_ast_nodes(src->right, dst->right);
}

/* OpenMP parallel memory operations */
#pragma omp declare simd
void parallel_memory_ops(char* buffer, size_t size) {
    volatile size_t local_size = size;
    char* local_buf = buffer;
    
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        size_t chunk = local_size / omp_get_num_threads();
        size_t start = tid * chunk;
        
        /* Each thread uses builtins with volatile control */
        volatile char fill_char = '0' + (tid % 10);
        
        if (tid % 3 == 0) {
            __builtin_memset(local_buf + start, fill_char, chunk);
        } else if (tid % 3 == 1) {
            char pattern[256];
            __builtin_memset(pattern, fill_char, sizeof(pattern));
            __builtin_memcpy(local_buf + start, pattern, chunk);
        } else {
            /* Use memmove with overlapping regions */
            size_t overlap = chunk / 2;
            __builtin_memmove(local_buf + start + overlap, 
                            local_buf + start, 
                            chunk - overlap);
        }
    }
}

/* Main execution with complex flow */
int main(void) {
    printf("Starting ASAN builtin redirection test\n");
    
    /* Phase 1: Initialize and create ASTs */
    ASTNode* ast1 = create_ast(4);
    ASTNode* ast2 = create_ast(4);
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create ASTs\n");
        return 1;
    }
    
    /* Phase 2: Process ASTs with memory operations */
    process_ast_nodes(ast1, ast2);
    
    /* Phase 3: OpenMP parallel operations */
    size_t buffer_size = 4096;
    char* buffer = (char*)malloc(buffer_size);
    
    if (buffer) {
        /* Initial memset */
        __builtin_memset(buffer, 0, buffer_size);
        
        /* Parallel operations */
        parallel_memory_ops(buffer, buffer_size);
        
        /* Verify with memcpy to temp buffer */
        char* verify_buf = (char*)malloc(buffer_size);
        if (verify_buf) {
            __builtin_memcpy(verify_buf, buffer, buffer_size);
            
            /* Calculate simple hash for verification */
            uint64_t hash = 0;
            for (size_t i = 0; i < buffer_size; i++) {
                hash = (hash * 31) + verify_buf[i];
            }
            printf("Buffer hash: 0x%016llx\n", (unsigned long long)hash);
            
            free(verify_buf);
        }
        
        free(buffer);
    }
    
    /* Phase 4: Edge case with goto and memmove */
    {
        char arr1[256], arr2[256];
        volatile int use_goto = 1;
        
        __builtin_memset(arr1, 'X', sizeof(arr1));
        
        if (use_goto) {
            goto do_memmove;
        }
        
        __builtin_memcpy(arr2, arr1, sizeof(arr1));
        goto skip_memmove;
        
    do_memmove:
        /* This should trigger the memmove redirection */
        __builtin_memmove(arr2, arr1, sizeof(arr1));
        
    skip_memmove:
        /* Verify the copy */
        int cmp = __builtin_memcmp(arr1, arr2, sizeof(arr1));
        printf("Memmove verification: %s\n", cmp == 0 ? "PASS" : "FAIL");
    }
    
    /* Cleanup */
    /* Note: In real code, we'd need to properly free the AST trees */
    
    printf("Test completed\n");
    return 0;
}
