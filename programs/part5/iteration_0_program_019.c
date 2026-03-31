/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    struct ASTNode* parent;
} ASTNode;

/* Global token array */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "instrument"
};
static const int g_token_count = sizeof(g_tokens) / sizeof(g_tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_sanitizer_env(void) {
    printf("Initializing sanitizer environment...\n");
    /* Force initialization of sanitizer runtime */
    volatile char dummy[16];
    __builtin_memset(dummy, 0, sizeof(dummy));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_sanitizer_env(void) {
    printf("Cleaning up sanitizer environment...\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* data) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    __builtin_memset(node, 0, sizeof(ASTNode));
    __builtin_memcpy(node->data, data, strlen(data) + 1);
    
    return node;
}

static void copy_ast_data(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use goto for control flow edge cases */
    if (dest->type == src->type) {
        goto copy_block;
    } else {
        dest->type = src->type;
    }
    
copy_block:
    /* Built-in memcpy with goto into block */
    __builtin_memcpy(dest->data, src->data, sizeof(dest->data));
    
    /* Jump out of block */
    goto after_copy;
    
after_copy:
    return;
}

static void process_ast_recursive(ASTNode* node, int depth) {
    if (!node || depth > 3) return;
    
    /* Create child nodes */
    node->left = create_ast_node("left_child");
    node->right = create_ast_node("right_child");
    
    if (node->left && node->right) {
        /* Copy data between nodes */
        copy_ast_data(node->right, node->left);
        
        /* Use memmove for overlapping regions */
        volatile char overlap_buf[128];
        __builtin_memset(overlap_buf, 'A', sizeof(overlap_buf));
        __builtin_memmove(overlap_buf + 32, overlap_buf, 64);
    }
    
    process_ast_recursive(node->left, depth + 1);
    process_ast_recursive(node->right, depth + 1);
}

static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_operations(void) {
    const int buffer_count = 8;
    char* buffers[buffer_count];
    
    #pragma omp parallel for
    for (int i = 0; i < buffer_count; i++) {
        size_t size = g_mem_size * (i + 1);
        buffers[i] = (char*)malloc(size);
        
        if (buffers[i]) {
            /* Force all three built-ins in parallel regions */
            __builtin_memset(buffers[i], i, size);
            
            if (i > 0) {
                __builtin_memcpy(buffers[i], buffers[i-1], size > g_mem_size ? g_mem_size : size);
            }
            
            /* Create overlapping copy with memmove */
            if (size > 128) {
                __builtin_memmove(buffers[i] + 64, buffers[i], 64);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < buffer_count; i++) {
        free(buffers[i]);
    }
}

/* Complex dispatch with goto patterns */
static void memory_dispatch_logic(void) {
    char buffer1[512];
    char buffer2[512];
    volatile int use_memmove = 1;
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 'X', sizeof(buffer1));
    __builtin_memset(buffer2, 'Y', sizeof(buffer2));
    
    /* Goto-based dispatch */
    dispatch_start:
    if (use_memmove) {
        goto use_memmove_block;
    } else {
        goto use_memcpy_block;
    }
    
use_memmove_block:
    /* Built-in memmove with goto into block */
    __builtin_memmove(buffer1 + 128, buffer1, 256);
    goto after_dispatch;
    
use_memcpy_block:
    /* Built-in memcpy */
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    goto after_dispatch;
    
after_dispatch:
    /* Toggle for next iteration */
    use_memmove = !use_memmove;
    
    /* Verify with memset */
    __builtin_memset(buffer1 + 384, 'Z', 64);
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Initialize and parse tokens */
    ASTNode* root = create_ast_node("root");
    if (!root) {
        fprintf(stderr, "Failed to create AST root\n");
        return 1;
    }
    
    /* Copy token data into AST */
    for (int i = 0; i < g_token_count; i++) {
        size_t len = strlen(g_tokens[i]);
        if (len < sizeof(root->data)) {
            __builtin_memcpy(root->data + i * 16, g_tokens[i], len + 1);
        }
    }
    
    /* Phase 2: Recursive processing */
    process_ast_recursive(root, 0);
    
    /* Phase 3: Parallel operations */
    parallel_memory_operations();
    
    /* Phase 4: Dispatch logic with gotos */
    memory_dispatch_logic();
    
    /* Phase 5: Calculate verification hash */
    unsigned long hash = 0;
    char* ptr = (char*)root;
    for (size_t i = 0; i < sizeof(ASTNode); i++) {
        hash = hash * 31 + ptr[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
