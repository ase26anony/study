/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    int value;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 256 + (rand() % 768);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: ASAN test completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* prefix) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Build node data with __builtin_memcpy */
    char buffer[64];
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, prefix, strlen(prefix));
    __builtin_memcpy(node->data, buffer, sizeof(buffer));
    
    node->value = depth;
    node->left = create_ast(depth - 1, "L-");
    node->right = create_ast(depth - 1, "R-");
    
    return node;
}

/* Function with goto and memory operations */
static void test_goto_memmove(void* dest, const void* src, size_t n) {
    int use_memmove = 1;
    
    if (n == 0) goto skip_operation;
    
    /* Jump into memory operation block */
    goto perform_copy;
    
perform_copy:
    if (use_memmove) {
        /* Force __builtin_memmove usage */
        __builtin_memmove(dest, src, n);
        goto operation_done;
    }
    
skip_operation:
    __builtin_memset(dest, 0, n);
    
operation_done:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        size_t local_size = g_mem_size / (thread_id + 2);
        
        /* Thread-local buffers */
        char src_buffer[512];
        char dst_buffer[512];
        
        /* Initialize with __builtin_memset */
        __builtin_memset(src_buffer, thread_id, sizeof(src_buffer));
        
        /* Copy with __builtin_memcpy */
        __builtin_memcpy(dst_buffer, src_buffer, local_size);
        
        /* Move with __builtin_memmove (overlapping regions) */
        __builtin_memmove(dst_buffer + 64, dst_buffer, local_size - 64);
        
        #pragma omp barrier
        
        /* Verify copy */
        for (size_t i = 0; i < local_size; i++) {
            if (dst_buffer[i + 64] != (char)thread_id) {
                printf("Thread %d: Memory verification failed at index %zu\n", 
                       thread_id, i);
            }
        }
    }
}

/* Complex token processing with varied memory operations */
static uint32_t process_tokens(const char** tokens, int count) {
    uint32_t hash = 0x811C9DC5;
    char buffer[256];
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        
        /* Clear buffer with __builtin_memset */
        __builtin_memset(buffer, 0, sizeof(buffer));
        
        /* Copy token with __builtin_memcpy */
        __builtin_memcpy(buffer, tokens[i], len);
        
        /* Process with overlapping memmove */
        if (len > 32) {
            __builtin_memmove(buffer + 16, buffer, len - 16);
        }
        
        /* Update hash */
        for (size_t j = 0; j < len && j < sizeof(buffer); j++) {
            hash ^= buffer[j];
            hash *= 0x01000193;
        }
    }
    
    return hash;
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Test 1: Basic built-in operations */
    char src[256], dst[256];
    volatile size_t copy_size = g_mem_size % 256;
    
    __builtin_memset(src, 0xAA, sizeof(src));
    __builtin_memcpy(dst, src, copy_size);
    __builtin_memmove(dst + 128, dst, copy_size);
    
    /* Test 2: Recursive AST operations */
    ASTNode* root = create_ast(4, "Root-");
    if (root) {
        /* Copy between AST nodes */
        ASTNode temp;
        __builtin_memcpy(&temp, root, sizeof(ASTNode));
        __builtin_memmove(root->data, temp.data, sizeof(temp.data));
        
        /* Cleanup */
        free(root);
    }
    
    /* Test 3: Goto-controlled memory operations */
    char goto_src[100], goto_dst[100];
    __builtin_memset(goto_src, 0xCC, sizeof(goto_src));
    test_goto_memmove(goto_dst, goto_src, sizeof(goto_dst));
    
    /* Test 4: Parallel operations */
    #ifdef _OPENMP
    parallel_memory_ops();
    #endif
    
    /* Test 5: Token processing */
    const char* tokens[] = {
        "memcpy", "memset", "memmove", 
        "asan", "hwasan", "instrumentation",
        "redzone", "shadow", "granule"
    };
    
    uint32_t final_hash = process_tokens(tokens, 
                                        sizeof(tokens)/sizeof(tokens[0]));
    
    printf("Final hash: 0x%08X\n", final_hash);
    printf("Test completed successfully\n");
    
    return 0;
}
