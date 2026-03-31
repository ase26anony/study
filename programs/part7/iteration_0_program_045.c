/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_memmove = 1;
volatile int g_use_memset = 1;

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
    printf("Constructor: Initializing ASAN globals\n");
    /* Force early initialization of memory functions */
    char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_globals(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data with __builtin_memcpy */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) 
        copy_len = sizeof(node->data);
    
    __builtin_memcpy(node->data, base_data, copy_len);
    node->size = copy_len;
    
    /* Recursive creation with goto for control flow */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    } else {
        node->left = NULL;
        node->right = NULL;
        goto done;
    }
    
create_children:
    node->left = create_ast(depth - 1, "left_child");
    node->right = create_ast(depth - 1, "right_child");
    
done:
    return node;
}

/* Function with goto jumping around memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int use_memmove = g_use_memmove;
    
    if (use_memmove) {
        goto do_memmove;
    } else {
        goto skip_memmove;
    }
    
do_memmove:
    /* Force memmove usage with goto */
    __builtin_memmove(dst->data, src->data, 
                     src->size < dst->size ? src->size : dst->size);
    goto after_memmove;
    
skip_memmove:
    /* Alternative path */
    __builtin_memcpy(dst->data, src->data, 
                    src->size < dst->size ? src->size : dst->size);
    
after_memmove:
    /* Additional operation */
    if (g_use_memset) {
        __builtin_memset(src->data + src->size/2, 0xFF, 16);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Force all three builtins in parallel context */
            char temp[128];
            
            /* memset in parallel region */
            __builtin_memset(temp, i, sizeof(temp));
            
            /* memcpy between node and temp */
            size_t len = nodes[i]->size;
            if (len > sizeof(temp)) len = sizeof(temp);
            __builtin_memcpy(nodes[i]->data, temp, len);
            
            /* Conditional memmove */
            if (i > 0 && nodes[i-1]) {
                __builtin_memmove(nodes[i-1]->data + 16, 
                                 nodes[i]->data, 
                                 len > 32 ? 32 : len);
            }
        }
    }
}

/* Complex token processing */
static size_t process_tokens(const char** tokens, int token_count) {
    size_t hash = 0;
    char buffer[256];
    
    for (int i = 0; i < token_count; i++) {
        /* Clear buffer with memset */
        __builtin_memset(buffer, 0, sizeof(buffer));
        
        /* Copy token with memcpy */
        size_t token_len = strlen(tokens[i]);
        if (token_len >= sizeof(buffer)) 
            token_len = sizeof(buffer) - 1;
        
        __builtin_memcpy(buffer, tokens[i], token_len);
        
        /* Move data around with memmove if overlapping */
        if (i > 0 && token_len > 16) {
            __builtin_memmove(buffer + 8, buffer, token_len - 8);
        }
        
        /* Accumulate hash */
        for (size_t j = 0; j < token_len; j++) {
            hash = (hash * 31) + buffer[j];
        }
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Initialize complex data structures */
    const char* tokens[] = {
        "memcpy_test", "memset_operation", "memmove_data",
        "asan_instrumentation", "hwasan_kernel", "builtin_redirect"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Process tokens with all three builtins */
    size_t token_hash = process_tokens(tokens, token_count);
    printf("Token hash: %zu\n", token_hash);
    
    /* Phase 2: Create and process AST */
    ASTNode* root = create_ast(4, "root_node");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    ASTNode* copy = (ASTNode*)malloc(sizeof(ASTNode));
    if (!copy) {
        free(root);
        return 1;
    }
    
    /* Initialize copy with memset */
    __builtin_memset(copy, 0, sizeof(ASTNode));
    
    /* Test goto control flow with memmove */
    process_with_goto(root, copy);
    
    /* Phase 3: Parallel operations */
    ASTNode* node_array[8];
    for (int i = 0; i < 8; i++) {
        char name[32];
        snprintf(name, sizeof(name), "node_%d", i);
        node_array[i] = create_ast(2, name);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(node_array, 8);
    
    /* Phase 4: Verify results */
    size_t final_hash = token_hash;
    for (int i = 0; i < 8; i++) {
        if (node_array[i]) {
            for (size_t j = 0; j < node_array[i]->size && j < 16; j++) {
                final_hash = (final_hash * 31) + node_array[i]->data[j];
            }
            free(node_array[i]);
        }
    }
    
    /* Final memory operation with volatile size */
    char final_buffer[512];
    size_t op_size = g_mem_size;
    if (op_size > sizeof(final_buffer)) 
        op_size = sizeof(final_buffer);
    
    __builtin_memset(final_buffer, 0xAA, op_size);
    __builtin_memcpy(final_buffer + 128, final_buffer, 64);
    __builtin_memmove(final_buffer + 256, final_buffer + 128, 128);
    
    /* Add final buffer to hash */
    for (size_t i = 0; i < 64; i++) {
        final_hash = (final_hash * 31) + final_buffer[i * 4];
    }
    
    printf("Final verification hash: %zu\n", final_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(root);
    free(copy);
    
    return 0;
}
