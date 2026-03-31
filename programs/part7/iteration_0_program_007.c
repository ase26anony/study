/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    uint32_t hash;
} ASTNode;

/* Global token array */
static char g_token_array[1024];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token array with pattern */
    for (int i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)((i * 31) & 0xFF);
    }
    printf("Constructor: Token array initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with goto for flow control */
    size_t copy_len = (depth < 8) ? 32 : 64;
    
    /* Goto-based control flow around memcpy */
    if (depth % 2 == 0) {
        goto even_depth;
    }
    
    /* Odd depth path */
    __builtin_memcpy(node->data, base_data, copy_len);
    goto after_copy;
    
even_depth:
    /* Even depth path with different memcpy */
    __builtin_memcpy(node->data + 16, base_data + 16, copy_len - 16);
    
after_copy:
    /* Create recursive children */
    node->left = create_ast(depth - 1, node->data);
    node->right = create_ast(depth - 2, node->data + 32);
    
    /* Compute hash using volatile size */
    volatile size_t hash_size = g_mem_size % 64;
    for (size_t i = 0; i < hash_size; i++) {
        node->hash = (node->hash * 31) + (uint32_t)node->data[i];
    }
    
    return node;
}

/* Memory operation dispatcher with OpenMP */
static void dispatch_memory_ops(ASTNode* node1, ASTNode* node2) {
    volatile char buffer[512];
    volatile size_t op_size = g_mem_size;
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0: {
                /* Thread 0: memcpy between AST nodes */
                size_t copy_len = op_size % 128;
                if (node1 && node2 && copy_len > 0) {
                    __builtin_memcpy(node2->data, node1->data, copy_len);
                }
                break;
            }
            case 1: {
                /* Thread 1: memset buffer */
                size_t set_len = (op_size * 2) % 256;
                if (set_len > 0) {
                    __builtin_memset((void*)buffer, thread_id, set_len);
                }
                break;
            }
            case 2: {
                /* Thread 2: memmove with overlap */
                size_t move_len = (op_size / 2) % 128;
                if (move_len > 0) {
                    __builtin_memmove((void*)(buffer + 64), 
                                     (void*)buffer, 
                                     move_len);
                }
                break;
            }
        }
        
        /* Barrier to synchronize */
        #pragma omp barrier
        
        /* All threads update token array */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            int idx = (thread_id * 100 + i) % sizeof(g_token_array);
            g_token_array[idx] = (char)((g_token_array[idx] + thread_id) & 0xFF);
        }
    }
}

/* Complex memory operation sequence */
static uint32_t execute_complex_sequence(void) {
    uint32_t result_hash = 0;
    volatile char temp_buf[256];
    volatile size_t seq_size = g_mem_size;
    
    /* Phase 1: Initialize with memset */
    __builtin_memset((void*)temp_buf, 0xAA, seq_size % 128);
    
    /* Phase 2: Copy from token array with goto */
    if (seq_size > 100) {
        goto large_copy;
    }
    
    __builtin_memcpy((void*)temp_buf, g_token_array, seq_size);
    goto after_copy_phase;
    
large_copy:
    __builtin_memcpy((void*)temp_buf, g_token_array + 128, 128);
    
after_copy_phase:
    /* Phase 3: Move data around */
    __builtin_memmove((void*)(temp_buf + 64), (void*)temp_buf, 64);
    
    /* Compute final hash */
    for (size_t i = 0; i < (seq_size % 64); i++) {
        result_hash = (result_hash * 31) + (uint32_t)temp_buf[i];
    }
    
    return result_hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create AST structures */
    ASTNode* ast1 = create_ast(5, "BaseDataForASTConstruction1234567890");
    ASTNode* ast2 = create_ast(4, "SecondaryASTDataABCDEFGHIJKLMNOPQRST");
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Execute OpenMP memory operations */
    dispatch_memory_ops(ast1, ast2);
    
    /* Execute complex sequence */
    uint32_t final_hash = execute_complex_sequence();
    
    /* Additional memory operations in main */
    volatile char main_buf[128];
    size_t main_size = g_mem_size % 64;
    
    __builtin_memset((void*)main_buf, 0xCC, main_size);
    __builtin_memcpy((void*)(main_buf + 32), ast1->data, 32);
    __builtin_memmove((void*)main_buf, (void*)(main_buf + 16), 16);
    
    /* Update hash with main buffer */
    for (size_t i = 0; i < 16; i++) {
        final_hash = (final_hash * 31) + (uint32_t)main_buf[i];
    }
    
    /* Print verification result */
    printf("Test completed. Final hash: 0x%08X\n", final_hash);
    printf("Token array checksum: ");
    uint32_t token_sum = 0;
    for (size_t i = 0; i < sizeof(g_token_array); i++) {
        token_sum += (uint32_t)g_token_array[i];
    }
    printf("%u\n", token_sum);
    
    /* Cleanup */
    free(ast1);
    free(ast2);
    
    return 0;
}
