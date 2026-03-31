/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    /* Force early initialization of memory functions */
    char buffer[32];
    __builtin_memset(buffer, 0xA5, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 8);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_globals(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile-controlled sizes */
    size_t copy_size = g_mem_size % 128;
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, base_data, copy_size);
    
    node->size = copy_size;
    node->left = create_ast(depth - 1, base_data + 1);
    node->right = create_ast(depth - 1, base_data + 2);
    
    return node;
}

/* Function with goto-based control flow */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int state = 0;
    
    /* Jump into memory operation block */
    if (src->size > 32) goto memmove_block;
    
    /* Normal path */
    __builtin_memcpy(dst->data, src->data, src->size);
    state = 1;
    goto exit;
    
memmove_block:
    /* Goto target with memmove */
    __builtin_memmove(dst->data + 16, src->data, src->size);
    state = 2;
    
    /* Jump out to different context */
    if (dst->size > src->size) goto exit;
    
    /* Another memory operation after goto */
    __builtin_memset(dst->data + 32, 0xFF, 16);
    state = 3;
    
exit:
    /* Final memory operation */
    __builtin_memcpy(dst->data + 48, &state, sizeof(state));
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes)
    for (i = 0; i < count; i++) {
        if (nodes[i] && nodes[(i + 1) % count]) {
            volatile size_t op_size = g_mem_size % 64;
            
            /* Mix of memory builtins in parallel region */
            __builtin_memset(nodes[i]->data, i, op_size);
            
            if (i % 3 == 0) {
                __builtin_memcpy(nodes[i]->data + 32, 
                               nodes[(i + 1) % count]->data, 
                               op_size / 2);
            } else if (i % 3 == 1) {
                __builtin_memmove(nodes[i]->data + 16,
                                nodes[i]->data,
                                op_size / 4);
            }
        }
    }
}

/* Complex token processing with memory operations */
static size_t process_tokens(const char** tokens, int token_count) {
    char buffer[1024];
    size_t total_hash = 0;
    int i;
    
    /* Initialize with memset */
    __builtin_memset(buffer, 0, sizeof(buffer));
    
    for (i = 0; i < token_count; i++) {
        size_t token_len = strlen(tokens[i]);
        size_t offset = i * 32 % sizeof(buffer);
        
        /* Use memcpy for token placement */
        __builtin_memcpy(buffer + offset, tokens[i], 
                        token_len < 32 ? token_len : 32);
        
        /* Overlap handling with memmove */
        if (i > 0 && offset < 32) {
            __builtin_memmove(buffer + offset + 16,
                            buffer + offset - 16,
                            32);
        }
        
        /* Hash calculation */
        for (size_t j = 0; j < 32 && (offset + j) < sizeof(buffer); j++) {
            total_hash += buffer[offset + j];
        }
    }
    
    return total_hash;
}

int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Recursive AST operations */
    const char* base_data = "TestDataForASANRedirectionABCDEFGHIJKLMNOP";
    ASTNode* root = create_ast(3, base_data);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Goto-based control flow */
    ASTNode* copy_node = malloc(sizeof(ASTNode));
    if (copy_node) {
        process_with_goto(root, copy_node);
    }
    
    /* Phase 3: Array of nodes for parallel processing */
    ASTNode* node_array[8];
    node_array[0] = root;
    node_array[1] = copy_node;
    
    for (int i = 2; i < 8; i++) {
        node_array[i] = create_ast(2, base_data + i);
    }
    
    /* Phase 4: OpenMP parallel operations */
    #ifdef _OPENMP
    parallel_memory_ops(node_array, 8);
    #endif
    
    /* Phase 5: Token processing */
    const char* tokens[] = {
        "memcpy", "memset", "memmove", "asan", "hwasan",
        "builtin", "redirection", "coverage", "test"
    };
    
    size_t hash_result = process_tokens(tokens, 
                                      sizeof(tokens)/sizeof(tokens[0]));
    
    printf("Token processing hash: %zu\n", hash_result);
    
    /* Phase 6: Direct builtin calls with varying sizes */
    char final_buffer[256];
    volatile size_t dynamic_size = g_mem_size;
    
    __builtin_memset(final_buffer, 0xAA, dynamic_size % 128);
    __builtin_memcpy(final_buffer + 64, final_buffer, 32);
    __builtin_memmove(final_buffer + 32, final_buffer + 16, 48);
    
    /* Verify results */
    size_t verification_sum = 0;
    for (int i = 0; i < 128; i++) {
        verification_sum += final_buffer[i];
    }
    printf("Verification sum: %zu\n", verification_sum);
    
    /* Cleanup */
    free(copy_node);
    
    /* Recursive cleanup would be needed for full AST */
    printf("Test completed successfully\n");
    
    return 0;
}
