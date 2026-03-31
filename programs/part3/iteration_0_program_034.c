/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    int value;
} ASTNode;

/* Global token array */
static char g_tokens[8][32] = {
    "token_alpha", "token_beta", "token_gamma",
    "token_delta", "token_epsilon", "token_zeta",
    "token_eta", "token_theta"
};

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_environment(void) {
    /* Force early initialization of memory functions */
    char buffer[16];
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, "init", 5);
    
    printf("Constructor: ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    printf("Destructor: Cleaning up ASAN resources\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_name) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use built-in memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Build node name with memcpy */
    char node_name[64];
    __builtin_memcpy(node_name, base_name, strlen(base_name));
    __builtin_memcpy(node_name + strlen(base_name), "_node", 6);
    __builtin_memcpy(node->data, node_name, strlen(node_name) + 1);
    
    node->value = depth * 100;
    
    /* Recursive creation with goto for control flow */
    int create_left = 1;
    
create_children:
    if (create_left) {
        node->left = create_ast(depth - 1, node_name);
        create_left = 0;
        goto create_children;  /* Jump back to create right child */
    } else {
        node->right = create_ast(depth - 1, node_name);
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int use_memmove = 1;
    
    /* Jump into memory operation block */
    goto start_operation;
    
skip_operation:
    return;
    
start_operation:
    if (use_memmove) {
        /* Force memmove redirection */
        __builtin_memmove(dst->data, src->data, 
                         sizeof(src->data) < sizeof(dst->data) ? 
                         sizeof(src->data) : sizeof(dst->data));
        
        /* Jump out of block */
        goto skip_operation;
    }
    
    /* Alternative path with memcpy */
    __builtin_memcpy(dst->data + 16, src->data + 8, 32);
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    const int num_arrays = 8;
    char* arrays[num_arrays];
    size_t size = g_mem_size;
    
    /* Allocate arrays */
    for (int i = 0; i < num_arrays; i++) {
        arrays[i] = malloc(size);
        if (arrays[i]) {
            __builtin_memset(arrays[i], i, size);
        }
    }
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < num_arrays - 1; i++) {
            if (arrays[i] && arrays[i + 1]) {
                /* Use volatile to prevent optimization */
                volatile size_t copy_size = size / 2;
                
                /* Force built-in function calls */
                if (thread_id % 2 == 0) {
                    __builtin_memcpy(arrays[i + 1], arrays[i], copy_size);
                } else {
                    __builtin_memmove(arrays[i], arrays[i + 1], copy_size);
                }
                
                /* Additional memset in parallel region */
                __builtin_memset(arrays[i] + copy_size, thread_id, 
                                size - copy_size);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_arrays; i++) {
        free(arrays[i]);
    }
}

/* Multi-stage interaction function */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    /* Process string with volatile pointer */
    volatile char* vptr = ptr;
    while (*vptr) {
        hash = ((hash << 5) + hash) + *vptr;
        vptr++;
    }
    
    /* Recursive hash computation */
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    hash += node->value;
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Initialize complex structures */
    ASTNode* ast1 = create_ast(4, "main");
    ASTNode* ast2 = create_ast(3, "backup");
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST structures\n");
        return 1;
    }
    
    /* Phase 2: Control flow with goto */
    process_with_goto(ast1, ast2);
    
    /* Phase 3: Token array processing */
    char token_buffer[256];
    __builtin_memset(token_buffer, 0, sizeof(token_buffer));
    
    for (int i = 0; i < 8; i++) {
        size_t offset = i * 32;
        volatile size_t copy_len = strlen(g_tokens[i]) + 1;
        
        if (offset + copy_len < sizeof(token_buffer)) {
            __builtin_memcpy(token_buffer + offset, 
                           g_tokens[i], copy_len);
        }
    }
    
    /* Phase 4: Parallel operations */
    parallel_memory_operations();
    
    /* Phase 5: Compute verification result */
    unsigned long hash1 = compute_ast_hash(ast1);
    unsigned long hash2 = compute_ast_hash(ast2);
    unsigned long final_hash = hash1 ^ hash2;
    
    /* Additional built-in calls for coverage */
    char verify_buffer[128];
    __builtin_memset(verify_buffer, 0xA5, sizeof(verify_buffer));
    __builtin_memcpy(verify_buffer + 64, token_buffer, 64);
    __builtin_memmove(verify_buffer, verify_buffer + 32, 32);
    
    printf("Verification hash: %lu\n", final_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real code, you'd need recursive free function */
    
    return 0;
}
