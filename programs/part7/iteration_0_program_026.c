/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_constructor(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_init_flag = 1;
}

/* Destructor for cleanup verification */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with volatile size */
    size_t copy_size = (g_mem_size < 256) ? g_mem_size : 255;
    __builtin_memcpy(node->data, base_data, copy_size);
    node->size = copy_size;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_left = (depth % 2 == 0);
        
        if (use_left) {
            node->left = create_ast(depth - 1, base_data);
            goto skip_right;
        }
        
        node->right = create_ast(depth - 1, base_data);
        skip_right:
        /* Empty label for goto target */
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int mode = src->size % 3;
    
    if (mode == 0) {
        goto direct_copy;
    } else if (mode == 1) {
        goto memset_first;
    } else {
        goto memmove_case;
    }
    
direct_copy:
    /* This block is entered via goto */
    __builtin_memcpy(dst->data, src->data, src->size);
    goto end_processing;
    
memset_first:
    __builtin_memset(dst->data, 0, dst->size);
    __builtin_memcpy(dst->data, src->data, src->size);
    goto end_processing;
    
memmove_case:
    /* Test overlapping memory with __builtin_memmove */
    if (dst->size >= src->size * 2) {
        __builtin_memcpy(dst->data, src->data, src->size);
        __builtin_memmove(dst->data + src->size, dst->data, src->size);
    }
    /* fall through */
    
end_processing:
    return;
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Force different builtins based on thread ID */
            int thread_mod = i % 3;
            volatile size_t local_size = nodes[i]->size;
            
            switch (thread_mod) {
                case 0:
                    __builtin_memset(nodes[i]->data, i, local_size);
                    break;
                case 1:
                    if (i > 0 && nodes[i-1]) {
                        __builtin_memcpy(nodes[i]->data, nodes[i-1]->data, 
                                       (local_size < nodes[i-1]->size) ? 
                                       local_size : nodes[i-1]->size);
                    }
                    break;
                case 2:
                    if (i > 1 && nodes[i-2]) {
                        __builtin_memmove(nodes[i]->data, nodes[i-2]->data, 
                                        (local_size < nodes[i-2]->size) ? 
                                        local_size : nodes[i-2]->size);
                    }
                    break;
            }
        }
    }
}

/* Complex token processing with memory builtins */
static size_t process_token_array(const char** tokens, int token_count) {
    char buffer[1024];
    size_t total_hash = 0;
    volatile size_t buffer_pos = 0;
    
    for (int i = 0; i < token_count; i++) {
        size_t token_len = strlen(tokens[i]);
        
        /* Use different builtins based on token properties */
        if (token_len > 0) {
            if (i % 4 == 0) {
                /* memset pattern */
                __builtin_memset(buffer + buffer_pos, tokens[i][0], token_len);
            } else if (i % 4 == 1) {
                /* memcpy from previous token if available */
                if (i > 0) {
                    __builtin_memcpy(buffer + buffer_pos, 
                                   tokens[i-1], 
                                   (token_len < strlen(tokens[i-1])) ? 
                                   token_len : strlen(tokens[i-1]));
                }
            } else if (i % 4 == 2) {
                /* memmove with potential overlap */
                if (buffer_pos >= token_len) {
                    __builtin_memmove(buffer + buffer_pos - token_len/2, 
                                    buffer + buffer_pos, 
                                    token_len);
                }
            } else {
                /* Direct memcpy */
                __builtin_memcpy(buffer + buffer_pos, tokens[i], token_len);
            }
            
            buffer_pos += token_len;
            if (buffer_pos >= sizeof(buffer) - 256) {
                buffer_pos = 0;
            }
        }
        
        /* Compute simple hash */
        for (size_t j = 0; j < token_len; j++) {
            total_hash += (size_t)tokens[i][j];
        }
    }
    
    return total_hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Initialize token array */
    const char* tokens[] = {
        "memcpy", "memset", "memmove", "asan", "hwasan",
        "instrumentation", "redzone", "builtin", "coverage",
        "optimization", "parallel", "recursive", "volatile"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Phase 1: Token processing */
    size_t hash1 = process_token_array(tokens, token_count);
    printf("Token processing hash: %zu\n", hash1);
    
    /* Phase 2: AST creation and manipulation */
    ASTNode* ast_root = create_ast(4, "AST_Base_Data");
    if (ast_root) {
        ASTNode* ast_copy = create_ast(3, "Copy_Data");
        
        /* Test goto flow control with memory ops */
        process_with_goto(ast_root, ast_copy);
        
        /* Create array of nodes for parallel processing */
        ASTNode* node_array[8];
        node_array[0] = ast_root;
        node_array[1] = ast_copy;
        for (int i = 2; i < 8; i++) {
            node_array[i] = create_ast(2, "Parallel_Node");
        }
        
        /* Phase 3: OpenMP parallel memory operations */
        parallel_memory_ops(node_array, 8);
        
        /* Cleanup */
        for (int i = 2; i < 8; i++) {
            free(node_array[i]);
        }
        free(ast_copy);
        free(ast_root);
    }
    
    /* Phase 4: Direct builtin calls with volatile control */
    char final_buffer[512];
    volatile size_t final_size = g_mem_size % 512;
    
    __builtin_memset(final_buffer, 0xA5, final_size);
    __builtin_memcpy(final_buffer + 128, final_buffer, 64);
    __builtin_memmove(final_buffer + 64, final_buffer + 128, 32);
    
    /* Verify operations by computing checksum */
    size_t final_checksum = 0;
    for (size_t i = 0; i < final_size; i++) {
        final_checksum += (size_t)final_buffer[i];
    }
    
    printf("Final checksum: %zu\n", final_checksum);
    printf("Test completed successfully\n");
    
    return 0;
}
