/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[32];
    int value;
} ASTNode;

/* Global token array */
static char g_tokens[256];
static volatile int g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Initialize token array with pattern */
    for (int i = 0; i < sizeof(g_tokens); i++) {
        g_tokens[i] = (char)((i * 7) & 0xFF);
    }
    
    /* Force initialization of memory built-ins */
    char buffer1[32];
    char buffer2[32];
    
    /* Use all three built-ins in constructor */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1, buffer2, sizeof(buffer1));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operations in destructor */
    char final_buf[16];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with built-in memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using built-in memcpy */
    size_t copy_len = (depth < 8) ? (size_t)depth * 4 : 32;
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Create children recursively */
    node->left = create_ast(depth - 1, base_data + 1);
    node->right = create_ast(depth - 1, base_data + 2);
    
    /* Compute value from children */
    node->value = depth;
    if (node->left) node->value += node->left->value;
    if (node->right) node->value += node->right->value;
    
    return node;
}

/* Function with goto and memory operations */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    volatile int use_memmove = 1;
    
    if (src && dst) {
        /* Jump into memory operation block */
        goto mem_block;
        
        /* This label should be reachable */
        normal_path:
            __builtin_memcpy(dst->data, src->data, sizeof(src->data));
            return;
        
        mem_block:
            if (use_memmove) {
                /* Use built-in memmove with goto */
                __builtin_memmove(dst->data, src->data, sizeof(src->data));
                goto normal_path;
            }
    }
}

/* Parallel memory dispatch logic */
static uint64_t parallel_memory_operations(void) {
    uint64_t hash = 0;
    volatile size_t block_size = g_mem_size;
    
    #pragma omp parallel reduction(+:hash)
    {
        /* Each thread gets its own buffers */
        char local_buf1[128];
        char local_buf2[128];
        volatile int thread_id = omp_get_thread_num();
        
        /* Initialize with pattern */
        for (size_t i = 0; i < sizeof(local_buf1); i++) {
            local_buf1[i] = (char)((i + thread_id) & 0xFF);
        }
        
        /* Use all three built-ins in parallel region */
        __builtin_memset(local_buf2, thread_id, block_size);
        __builtin_memcpy(local_buf2 + block_size/2, local_buf1, block_size/2);
        __builtin_memmove(local_buf1, local_buf2, block_size);
        
        /* Compute hash from buffer contents */
        #pragma omp for
        for (size_t i = 0; i < block_size; i++) {
            hash += (uint64_t)local_buf1[i];
        }
    }
    
    return hash;
}

/* Function with switch and memory operations */
static void dispatch_memory_operation(int op_type, void* dst, const void* src, size_t n) {
    volatile size_t safe_n = n;
    
    switch (op_type) {
        case 0:
            __builtin_memcpy(dst, src, safe_n);
            break;
        case 1:
            __builtin_memset(dst, 0xCC, safe_n);
            break;
        case 2:
            __builtin_memmove(dst, src, safe_n);
            break;
        default:
            __builtin_memset(dst, 0xFF, safe_n);
    }
}

/* Main test execution */
int main(void) {
    uint64_t total_hash = 0;
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Create and process AST */
    ASTNode* ast1 = create_ast(4, g_tokens);
    ASTNode* ast2 = create_ast(3, g_tokens + 32);
    
    if (ast1 && ast2) {
        /* Test goto with memory operations */
        process_with_goto(ast1, ast2);
        
        /* Dispatch different memory operations */
        for (int i = 0; i < 3; i++) {
            dispatch_memory_operation(i, ast2->data, ast1->data, 16);
        }
        
        /* Free AST nodes */
        free(ast1);
        free(ast2);
    }
    
    /* Phase 2: Parallel operations */
    total_hash = parallel_memory_operations();
    
    /* Phase 3: Direct built-in calls with volatile sizes */
    char final_buffer[256];
    volatile size_t final_size = 128;
    
    __builtin_memset(final_buffer, 0xAA, final_size);
    __builtin_memcpy(final_buffer + 64, g_tokens, 64);
    __builtin_memmove(g_tokens, final_buffer, 64);
    
    /* Add final buffer to hash */
    for (size_t i = 0; i < final_size; i++) {
        total_hash += (uint64_t)final_buffer[i];
    }
    
    printf("Test completed. Hash: 0x%016llX\n", (unsigned long long)total_hash);
    
    return (total_hash != 0) ? 0 : 1;
}
