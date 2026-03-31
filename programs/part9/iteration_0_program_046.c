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
    size_t data_len;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[128];
    /* Force memcpy initialization in constructor */
    __builtin_memcpy(buffer, "constructor_init", 16);
    printf("Constructor: ASAN initialization triggered\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    printf("Destructor: ASAN cleanup completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->left = create_ast(depth - 1, base_data);
    node->right = create_ast(depth - 1, base_data);
    
    /* Use memset and memcpy with volatile length */
    size_t copy_len = g_mem_size % 64;
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, base_data, copy_len);
    node->data_len = copy_len;
    
    return node;
}

/* Function with goto statements for flow control */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int state = 0;
    
    if (!src || !dst) goto cleanup;
    
    /* Jump into memory operation block */
    if (src->data_len > 0) {
        goto copy_block;
    } else {
        goto skip_copy;
    }
    
copy_block:
    /* This tests flow-sensitivity of asan_memfn_rtls retrieval */
    __builtin_memmove(dst->data, src->data, src->data_len);
    state = 1;
    
skip_copy:
    /* Jump out of block */
    if (state) {
        goto finalize;
    }
    
    /* Another memory operation after goto */
    __builtin_memset(dst->data, 'X', sizeof(dst->data));
    
finalize:
    dst->data_len = src->data_len;
    
cleanup:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        char local_buf[256];
        char src_buf[256];
        
        /* Initialize source with pattern */
        #pragma omp for
        for (int i = 0; i < 256; i++) {
            src_buf[i] = (char)(i % 256);
        }
        
        /* Force memcpy in parallel region */
        #pragma omp single
        {
            __builtin_memcpy(local_buf, src_buf, 256);
        }
        
        /* Use memset in parallel */
        #pragma omp barrier
        __builtin_memset(local_buf + 128, 0, 128);
    }
}

/* Complex token processing with varied memory operations */
static size_t process_tokens(const char** tokens, int count) {
    char buffer[512];
    size_t total_hash = 0;
    
    for (int i = 0; i < count; i++) {
        size_t token_len = strlen(tokens[i]);
        volatile size_t copy_len = token_len % 256;
        
        /* Alternate between memory functions */
        if (i % 3 == 0) {
            __builtin_memcpy(buffer + i * 16, tokens[i], copy_len);
        } else if (i % 3 == 1) {
            __builtin_memset(buffer + i * 16, tokens[i][0], copy_len);
        } else {
            if (i > 0) {
                __builtin_memmove(buffer + i * 16, buffer + (i-1) * 16, copy_len);
            }
        }
        
        /* Compute simple hash */
        for (size_t j = 0; j < copy_len; j++) {
            total_hash += (size_t)buffer[i * 16 + j];
        }
    }
    
    return total_hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize complex token array */
    const char* tokens[] = {
        "memcpy_test_token_1",
        "memset_test_token_2",
        "memmove_test_token_3",
        "asan_instrumentation",
        "hwasan_kernel_mode",
        "builtin_redirection",
        "rtl_modification",
        "symbol_interception"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Create recursive AST structures */
    ASTNode* ast1 = create_ast(3, "AST_Node_Base_Data_1");
    ASTNode* ast2 = create_ast(3, "AST_Node_Base_Data_2");
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Test goto-based flow control with memmove */
    process_with_goto(ast1, ast2);
    
    /* Execute parallelized memory operations */
    parallel_memory_ops();
    
    /* Process tokens with varied memory built-ins */
    size_t final_hash = process_tokens(tokens, token_count);
    
    /* Additional explicit calls to ensure all built-ins are used */
    char final_buffer[1024];
    volatile size_t final_size = g_mem_size % 512;
    
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, ast1->data, ast1->data_len);
    __builtin_memmove(final_buffer + 512, final_buffer, final_size);
    
    /* Verify operations by printing hash */
    printf("Final computed hash: %zu\n", final_hash);
    printf("AST1 data length: %zu, AST2 data length: %zu\n", 
           ast1->data_len, ast2->data_len);
    
    /* Cleanup */
    free(ast1);
    free(ast2);
    
    printf("ASAN built-in redirection test completed successfully\n");
    return 0;
}
