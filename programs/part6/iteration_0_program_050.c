/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    volatile char buffer[32];
    /* Force __builtin_memset in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    printf("Constructor: ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    volatile char buffer[16];
    __builtin_memset(buffer, 0xFF, sizeof(buffer));
    printf("Destructor: ASAN environment cleaned up\n");
}

/* Complex token array initialization */
static void initialize_token_array(char* tokens, size_t size) {
    volatile size_t i = 0;
    
    /* Use goto for control flow edge cases */
    if (size > 0) goto start_copy;
    
    fallback:
    __builtin_memset(tokens, 0, size);
    return;
    
    start_copy:
    /* Force __builtin_memcpy with goto */
    __builtin_memcpy(tokens, "INIT_TOKENS", 11);
    
    if (i < size - 11) {
        i += 11;
        goto start_copy;
    }
    
    goto fallback;
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* data, size_t depth) {
    if (depth == 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for node initialization */
    __builtin_memcpy(node->data, data, strlen(data) + 1);
    node->size = strlen(data) + 1;
    
    /* Recursive creation with different memory patterns */
    char left_data[64];
    char right_data[64];
    
    __builtin_memset(left_data, 'L', sizeof(left_data));
    __builtin_memset(right_data, 'R', sizeof(right_data));
    
    /* Use __builtin_memmove for overlapping regions */
    __builtin_memmove(left_data + 10, left_data, 20);
    __builtin_memmove(right_data + 5, right_data, 15);
    
    node->left = create_ast_node(left_data, depth - 1);
    node->right = create_ast_node(right_data, depth - 1);
    
    return node;
}

/* Memory operation between AST nodes */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    volatile size_t copy_size = dest->size < src->size ? dest->size : src->size;
    
    /* Use all three built-ins in sequence */
    __builtin_memset(dest->data, 0, sizeof(dest->data));
    __builtin_memcpy(dest->data, src->data, copy_size);
    
    /* Create overlapping scenario for memmove */
    if (copy_size > 10) {
        __builtin_memmove(dest->data + 5, dest->data, copy_size - 5);
    }
}

/* Parallel memory dispatch logic */
static void parallel_memory_operations(void) {
    const size_t buffer_size = 1024;
    char* buffers[4];
    
    #pragma omp parallel for
    for (int i = 0; i < 4; i++) {
        buffers[i] = (char*)malloc(buffer_size);
        if (buffers[i]) {
            /* Force built-in usage in OpenMP context */
            __builtin_memset(buffers[i], i, buffer_size);
            
            /* Conditional memcpy based on thread ID */
            if (i % 2 == 0) {
                char temp[256];
                __builtin_memset(temp, 0xCC, sizeof(temp));
                __builtin_memcpy(buffers[i] + 128, temp, sizeof(temp));
            }
            
            /* Use memmove for intra-buffer operations */
            __builtin_memmove(buffers[i] + 512, buffers[i] + 256, 128);
        }
    }
    
    /* Cleanup */
    #pragma omp parallel for
    for (int i = 0; i < 4; i++) {
        if (buffers[i]) {
            free(buffers[i]);
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Token array initialization */
    char tokens[512];
    initialize_token_array(tokens, sizeof(tokens));
    
    /* Phase 2: Recursive AST creation and manipulation */
    ASTNode* root = create_ast_node("ROOT_NODE", 3);
    if (root && root->left && root->right) {
        copy_ast_data(root->left, root->right);
        
        /* Additional memory operations with goto */
        volatile int use_memmove = 1;
        
        if (use_memmove) goto do_memmove;
        
        skip_memmove:
        __builtin_memcpy(root->data, "MODIFIED", 9);
        goto after_ops;
        
        do_memmove:
        __builtin_memmove(root->data + 4, root->data, 20);
        goto skip_memmove;
        
        after_ops:
        ;
    }
    
    /* Phase 3: Parallel memory operations */
    parallel_memory_operations();
    
    /* Phase 4: Complex memory pattern with volatile control */
    volatile char* dynamic_buf = (char*)malloc(g_mem_size);
    if (dynamic_buf) {
        volatile size_t offset = g_mem_size / 2;
        
        /* Mix of all three built-ins */
        __builtin_memset(dynamic_buf, 0xAA, g_mem_size);
        __builtin_memcpy(dynamic_buf + offset, dynamic_buf, offset);
        __builtin_memmove(dynamic_buf, dynamic_buf + offset, offset);
        
        /* Compute verification hash */
        unsigned long hash = 0;
        for (size_t i = 0; i < g_mem_size; i++) {
            hash += (unsigned long)dynamic_buf[i];
        }
        printf("Verification hash: %lu\n", hash);
        
        free((void*)dynamic_buf);
    }
    
    /* Cleanup AST */
    /* Note: In real code, implement proper recursive free */
    if (root) free(root);
    
    printf("=== Test completed successfully ===\n");
    return 0;
}
