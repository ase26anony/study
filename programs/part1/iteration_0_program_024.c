/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
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
    int value;
} ASTNode;

/* Global token array */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "test"
};
static const int g_token_count = 6;

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_test(void) {
    printf("ASAN Test Constructor: Initializing...\n");
    /* Force initialization of memory functions */
    char buf[32];
    __builtin_memset(buf, 0, sizeof(buf));
    __builtin_memcpy(buf, "init", 5);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    printf("ASAN Test Destructor: Cleaning up...\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* token) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data with goto for flow control */
    int copy_len = 0;
    if (token) {
        copy_len = __builtin_strlen(token);
        if (copy_len > 63) copy_len = 63;
        
        /* Use goto to jump into memory operation block */
        goto copy_block;
        
    copy_block:
        __builtin_memcpy(node->data, token, copy_len);
        node->data[copy_len] = '\0';
    }
    
    /* Recursive creation with different memory patterns */
    node->left = create_ast(depth - 1, g_tokens[(depth * 2) % g_token_count]);
    node->right = create_ast(depth - 1, g_tokens[(depth * 3) % g_token_count]);
    
    /* Set value using volatile-controlled size */
    node->value = (int)(g_mem_size % 100);
    
    return node;
}

/* Complex memory operation with goto jumps */
static void process_ast_with_goto(ASTNode* node) {
    if (!node) return;
    
    char temp[64];
    volatile int use_memmove = 1;
    
    /* Jump into different memory operation blocks */
    if (node->value % 3 == 0) {
        goto memcpy_block;
    } else if (node->value % 3 == 1) {
        goto memset_block;
    } else {
        goto memmove_block;
    }
    
memcpy_block:
    __builtin_memcpy(temp, node->data, sizeof(temp));
    goto after_memop;
    
memset_block:
    __builtin_memset(temp, node->value, sizeof(temp));
    goto after_memop;
    
memmove_block:
    if (use_memmove) {
        /* Self-overlapping memmove */
        __builtin_memmove(node->data + 10, node->data, 20);
    }
    goto after_memop;
    
after_memop:
    /* Process children */
    process_ast_with_goto(node->left);
    process_ast_with_goto(node->right);
}

/* Parallel memory dispatch logic */
static void parallel_memory_operations(void) {
    const int array_size = 1024;
    char* src = (char*)malloc(array_size);
    char* dst = (char*)malloc(array_size);
    
    if (!src || !dst) {
        free(src);
        free(dst);
        return;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < array_size; i++) {
        src[i] = (char)(i % 256);
    }
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int chunk_size = array_size / omp_get_num_threads();
        int start = thread_id * chunk_size;
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                __builtin_memcpy(dst + start, src + start, chunk_size);
                break;
            case 1:
                __builtin_memset(dst + start, thread_id, chunk_size);
                break;
            case 2:
                __builtin_memmove(dst + start + 10, src + start, chunk_size - 10);
                break;
        }
    }
    
    /* Verify by calculating checksum */
    unsigned long long checksum = 0;
    for (int i = 0; i < array_size; i++) {
        checksum += (unsigned char)dst[i];
    }
    
    printf("Parallel checksum: %llu\n", checksum);
    
    free(src);
    free(dst);
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Create recursive AST */
    ASTNode* root = create_ast(4, g_tokens[0]);
    
    /* Process with goto jumps */
    process_ast_with_goto(root);
    
    /* Perform parallel operations */
    parallel_memory_operations();
    
    /* Additional memory operations with volatile control */
    volatile size_t dynamic_size = g_mem_size;
    char* buffer1 = (char*)malloc(dynamic_size);
    char* buffer2 = (char*)malloc(dynamic_size);
    
    if (buffer1 && buffer2) {
        /* Chain of memory operations */
        __builtin_memset(buffer1, 0xAA, dynamic_size);
        __builtin_memcpy(buffer2, buffer1, dynamic_size);
        
        /* Overlapping memmove */
        __builtin_memmove(buffer1 + 50, buffer1, dynamic_size - 50);
        
        /* Calculate final hash */
        unsigned int hash = 0;
        for (size_t i = 0; i < dynamic_size; i++) {
            hash = (hash * 31) + (unsigned char)buffer1[i];
        }
        printf("Final hash: %u\n", hash);
    }
    
    free(buffer1);
    free(buffer2);
    
    /* Cleanup AST */
    /* Note: In real code, implement proper tree freeing */
    
    printf("=== Test Complete ===\n");
    return 0;
}
