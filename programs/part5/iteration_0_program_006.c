/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 256;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 192;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    char *data;
    size_t data_len;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[64];
    /* Force builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 32, buffer, 16);
}

/* Destructor to test cleanup paths */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char final_check[32];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = depth * 7;
    node->data_len = (size_t)(depth + 1) * 16;
    node->data = (char*)malloc(node->data_len);
    
    if (node->data) {
        /* Pattern fill with memset */
        __builtin_memset(node->data, 0xCC + depth, node->data_len);
        
        /* Copy pattern between halves */
        if (node->data_len > 32) {
            __builtin_memcpy(node->data + 16, node->data, 16);
        }
    }
    
    /* Recursive creation */
    node->left = create_ast(depth + 1, max_depth);
    node->right = create_ast(depth + 2, max_depth);
    
    /* Memmove between child nodes if both exist */
    if (node->left && node->right && node->left->data && node->right->data) {
        size_t copy_len = node->left->data_len < node->right->data_len ? 
                         node->left->data_len : node->right->data_len;
        if (copy_len > 0) {
            __builtin_memmove(node->right->data, node->left->data, copy_len);
        }
    }
    
    return node;
}

/* Function with goto edge cases */
static void test_goto_memops(void) {
    volatile char src[256], dst[256];
    int use_memcpy = 1;
    
    /* Initialize source */
    for (int i = 0; i < 256; i++) {
        src[i] = (char)(i % 256);
    }
    
    /* Goto into memory operation block */
    goto mem_block;
    
mem_block:
    if (use_memcpy) {
        __builtin_memcpy(dst, src, (size_t)g_memcpy_len);
        use_memcpy = 0;
        goto memset_block;
    }
    
memset_block:
    __builtin_memset(dst + 128, 0x5A, (size_t)g_memset_len);
    goto memmove_block;
    
memmove_block:
    /* Overlapping memmove */
    __builtin_memmove(dst + 64, dst + 32, (size_t)g_memmove_len);
}

/* OpenMP parallel memory operations */
static void parallel_mem_ops(void) {
    volatile size_t local_len = 1024;
    char *shared_buf = (char*)malloc(local_len * 2);
    char *private_copies[8];
    
    if (!shared_buf) return;
    
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread allocates private buffer */
        private_copies[tid] = (char*)malloc(local_len);
        
        #pragma omp barrier
        
        /* Initialize shared buffer */
        #pragma omp single
        {
            __builtin_memset(shared_buf, 0xAA, local_len);
            __builtin_memset(shared_buf + local_len, 0xBB, local_len);
        }
        
        #pragma omp barrier
        
        /* Copy from shared to private */
        __builtin_memcpy(private_copies[tid], 
                        shared_buf + (tid * (local_len / 4)), 
                        local_len / 4);
        
        /* Modify private copy */
        __builtin_memset(private_copies[tid] + 16, tid, 32);
        
        /* Copy back to different shared location */
        __builtin_memcpy(shared_buf + local_len + (tid * (local_len / 4)),
                        private_copies[tid],
                        local_len / 4);
        
        #pragma omp barrier
        
        /* Overlapping memmove in parallel region */
        if (tid == 0) {
            __builtin_memmove(shared_buf + 512, shared_buf + 256, 256);
        }
        
        free(private_copies[tid]);
    }
    
    free(shared_buf);
}

/* Complex token processing with memory operations */
static unsigned long process_tokens(const char *tokens[], size_t count) {
    unsigned long hash = 0xDEADBEEF;
    volatile char processing_buf[512];
    
    for (size_t i = 0; i < count; i++) {
        size_t token_len = strlen(tokens[i]);
        
        /* Clear buffer with memset */
        __builtin_memset(processing_buf, 0, sizeof(processing_buf));
        
        /* Copy token with memcpy */
        if (token_len < sizeof(processing_buf)) {
            __builtin_memcpy(processing_buf, tokens[i], token_len);
        }
        
        /* Process token - overlapping memmove for rotation */
        if (token_len > 16) {
            __builtin_memmove(processing_buf, processing_buf + 8, token_len - 8);
        }
        
        /* Update hash */
        for (size_t j = 0; j < token_len && j < sizeof(processing_buf); j++) {
            hash = (hash << 5) + hash + processing_buf[j];
        }
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* 1. Force builtin initialization */
    volatile char init_buf[128];
    __builtin_memset(init_buf, 0x11, sizeof(init_buf));
    __builtin_memcpy(init_buf + 64, init_buf, 32);
    __builtin_memmove(init_buf + 32, init_buf + 16, 48);
    
    /* 2. Test goto edge cases */
    test_goto_memops();
    
    /* 3. Create recursive AST structure */
    ASTNode *root = create_ast(0, 4);
    
    /* 4. Parallel memory operations */
    parallel_mem_ops();
    
    /* 5. Token processing */
    const char *tokens[] = {
        "memcpy", "memset", "memmove", "asan", "hwasan",
        "instrumentation", "redzone", "builtin", "coverage"
    };
    unsigned long result = process_tokens(tokens, 
                                        sizeof(tokens)/sizeof(tokens[0]));
    
    /* 6. Final verification with all three builtins */
    volatile char verify_buf[384];
    __builtin_memset(verify_buf, 0x33, sizeof(verify_buf));
    __builtin_memcpy(verify_buf + 128, verify_buf, 128);
    __builtin_memmove(verify_buf + 64, verify_buf + 192, 128);
    
    /* Calculate final checksum */
    unsigned long checksum = result;
    for (size_t i = 0; i < sizeof(verify_buf); i++) {
        checksum = (checksum * 31) + verify_buf[i];
    }
    
    printf("Result checksum: 0x%08lX\n", checksum);
    printf("Test completed - all builtins should have been redirected\n");
    
    /* Cleanup */
    /* Note: AST cleanup omitted for brevity - would need recursive free */
    
    return (checksum != 0) ? 0 : 1;
}
