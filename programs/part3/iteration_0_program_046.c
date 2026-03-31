/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 96;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force early builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 8);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    volatile char final_check[16];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use all three builtins in varied contexts */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    
    /* Fill data with pattern using memcpy */
    char pattern[32];
    __builtin_memset(pattern, 'A' + depth, sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(pattern));
    
    /* Create children recursively */
    node->left = create_ast(depth + 1, max_depth);
    node->left = create_ast(depth + 1, max_depth);
    
    /* Memmove between node fields */
    if (node->left && node->right) {
        __builtin_memmove(node->data + 128, node->left->data, 32);
    }
    
    return node;
}

/* Function with goto-based control flow */
static void goto_memory_operations(void) {
    volatile char src[256], dst[256];
    int use_memmove = 1;
    
    __builtin_memset(src, 0xCC, sizeof(src));
    
    /* Jump into block with memmove */
    if (use_memmove) goto do_memmove;
    
    normal_path:
    __builtin_memcpy(dst, src, g_memcpy_len);
    return;
    
    do_memmove:
    /* This tests flow-sensitivity of redirection logic */
    __builtin_memmove(dst, src, g_memmove_len);
    goto normal_path;
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_dispatch(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        volatile char thread_buf[512];
        
        /* Each thread uses different builtins */
        switch (tid % 3) {
            case 0:
                __builtin_memset(thread_buf, tid, g_memset_len);
                break;
            case 1:
                __builtin_memcpy(thread_buf + 64, thread_buf, 32);
                break;
            case 2:
                __builtin_memmove(thread_buf + 128, thread_buf + 32, 64);
                break;
        }
        
        /* Barrier to ensure all threads reach here */
        #pragma omp barrier
        
        /* All threads do final memcpy */
        volatile char final[64];
        __builtin_memcpy(final, thread_buf + tid * 16, 32);
    }
}

/* Complex initialization with token processing */
static unsigned long process_tokens(const char** tokens, int count) {
    unsigned long hash = 0;
    volatile char buffer[1024];
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        
        /* Mix of memory operations based on token length */
        if (len < 32) {
            __builtin_memset(buffer, tokens[i][0], len);
        } else if (len < 64) {
            __builtin_memcpy(buffer, tokens[i], len);
        } else {
            __builtin_memmove(buffer + 64, tokens[i], 64);
        }
        
        /* Update hash */
        for (size_t j = 0; j < len && j < 64; j++) {
            hash = hash * 31 + buffer[j];
        }
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* 1. Initialize complex token array */
    const char* tokens[] = {
        "memcpy_test_token_1",
        "memset_operation_token_2",
        "memmove_heavy_token_with_more_characters_3",
        "short",
        "very_long_token_that_exceeds_typical_buffers_and_requires_proper_handling"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* 2. Create recursive AST structure */
    ASTNode* root = create_ast(0, 4);
    
    /* 3. Process tokens with memory operations */
    unsigned long token_hash = process_tokens(tokens, token_count);
    
    /* 4. Execute goto-based control flow */
    goto_memory_operations();
    
    /* 5. Run parallel memory dispatch */
    parallel_memory_dispatch();
    
    /* 6. Additional builtin calls in main */
    volatile char main_buffer[512];
    __builtin_memset(main_buffer, 0x5A, sizeof(main_buffer));
    __builtin_memcpy(main_buffer + 256, main_buffer, 128);
    __builtin_memmove(main_buffer + 384, main_buffer + 128, 64);
    
    /* 7. Calculate final result */
    unsigned long final_result = token_hash;
    if (root) {
        for (int i = 0; i < 256; i++) {
            final_result += root->data[i];
        }
        /* Cleanup */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    printf("Final result: %lu\n", final_result);
    printf("Test completed - check ASAN/HWASAN instrumentation\n");
    
    return (final_result > 0) ? 0 : 1;
}
