/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor))
static void init_asan_early(void) {
    volatile char buffer[256];
    /* Force builtin memcpy initialization */
    __builtin_memcpy(buffer, "constructor_init", 16);
    printf("Constructor: Early ASAN init triggered\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    printf("Destructor: Program cleanup\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->size = sizeof(ASTNode);
    
    /* Fill data with pattern using memcpy */
    char pattern[32];
    __builtin_snprintf(pattern, sizeof(pattern), "AST_depth_%d", depth);
    __builtin_memcpy(node->data, pattern, strlen(pattern) + 1);
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
create_children:
    if (create_left) {
        node->left = create_ast(depth - 1);
        create_left = 0;
        goto create_children; /* Jump back to create right child */
    } else {
        node->right = create_ast(depth - 1);
    }
    
    return node;
}

/* Function with goto jumping into memory block */
static void goto_memmove_test(char* dest, const char* src, size_t n) {
    int use_memmove = 1;
    
    if (use_memmove) {
        goto perform_memmove;
    }
    
    /* This block should be jumped into */
perform_memmove:
    __builtin_memmove(dest, src, n);
    
    /* Jump out to different context */
    if (n > 100) {
        goto large_copy;
    }
    return;
    
large_copy:
    /* Additional operation after jump */
    __builtin_memset(dest + n/2, 0xFF, n/4);
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    const size_t buf_size = (size_t)g_mem_size;
    char* buffer1 = (char*)malloc(buf_size);
    char* buffer2 = (char*)malloc(buf_size);
    
    if (!buffer1 || !buffer2) return;
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread uses different builtins */
        #pragma omp for
        for (size_t i = 0; i < buf_size; i += 64) {
            size_t chunk = (buf_size - i > 64) ? 64 : buf_size - i;
            
            if (thread_id % 3 == 0) {
                __builtin_memset(buffer1 + i, thread_id, chunk);
            } else if (thread_id % 3 == 1) {
                __builtin_memcpy(buffer2 + i, buffer1 + i, chunk);
            } else {
                /* Use memmove with overlapping regions */
                size_t overlap = chunk / 2;
                __builtin_memmove(buffer1 + i + overlap, buffer1 + i, chunk - overlap);
            }
        }
        
        /* Barrier to ensure all memory ops complete */
        #pragma omp barrier
        
        /* Verify with another memcpy */
        #pragma omp single
        {
            __builtin_memcpy(buffer2, buffer1, buf_size / 2);
        }
    }
    
    free(buffer1);
    free(buffer2);
}

/* Complex token processing with varied memory operations */
static size_t process_tokens(char** tokens, int count) {
    size_t hash = 0;
    char combined[512];
    volatile size_t offset = 0; /* Volatile to prevent optimization */
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]) + 1;
        
        /* Conditional goto around memcpy */
        if (offset + len > sizeof(combined)) {
            goto reset_buffer;
        }
        
        __builtin_memcpy(combined + offset, tokens[i], len);
        offset += len - 1;
        continue;
        
    reset_buffer:
        /* Handle buffer reset with memmove */
        __builtin_memmove(combined, combined + sizeof(combined)/2, sizeof(combined)/2);
        offset = sizeof(combined)/2;
        __builtin_memset(combined + offset, 0, sizeof(combined)/2);
        i--; /* Retry current token */
    }
    
    /* Compute simple hash */
    for (size_t i = 0; i < offset; i++) {
        hash = (hash * 31) + combined[i];
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = create_ast(4);
    if (root) {
        /* Copy between AST nodes */
        ASTNode temp;
        __builtin_memcpy(&temp, root, sizeof(ASTNode));
        __builtin_memcpy(root->left, &temp, sizeof(ASTNode));
        
        /* Cleanup */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    /* Phase 2: Goto-based memory operations */
    char src[256], dest[256];
    for (int i = 0; i < sizeof(src); i++) {
        src[i] = i % 256;
    }
    
    goto_memmove_test(dest, src, sizeof(src));
    
    /* Verify with memcpy */
    char verify[256];
    __builtin_memcpy(verify, dest, sizeof(dest));
    
    /* Phase 3: Token processing */
    char* tokens[] = {
        "memcpy_test",
        "memset_operation",
        "memmove_overlap",
        "asan_redirect",
        "hwasan_check"
    };
    
    size_t token_hash = process_tokens(tokens, 5);
    printf("Token hash: %zu\n", token_hash);
    
    /* Phase 4: Parallel operations */
    parallel_memory_ops();
    
    /* Phase 5: Direct builtin calls with volatile sizes */
    volatile size_t final_size = 128;
    char* final_buf = (char*)malloc(final_size);
    if (final_buf) {
        __builtin_memset(final_buf, 0xAA, final_size);
        __builtin_memmove(final_buf + 32, final_buf, 64);
        __builtin_memcpy(final_buf, "ASAN_TEST_COMPLETE", 19);
        free(final_buf);
    }
    
    printf("ASAN test completed successfully\n");
    return 0;
}
