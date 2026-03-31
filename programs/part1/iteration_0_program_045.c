/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char g_token_pool[4096];
static volatile size_t g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Force initialization of memory functions */
    char buffer[128];
    volatile char* volatile_ptr = buffer;
    
    /* Use all three builtins in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 64, buffer, 32);
    __builtin_memmove(buffer + 32, buffer + 16, 16);
    
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operations in destructor */
    volatile char final_buf[64];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* data, size_t len) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile-controlled sizes */
    volatile size_t copy_len = len > 63 ? 63 : len;
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, data, copy_len);
    node->data[63] = '\0';
    node->size = copy_len;
    node->left = node->right = NULL;
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(ASTNode* dest, ASTNode* src) {
    int state = 0;
    
    /* Jump into memory operation block */
    if (dest && src) {
        goto memory_operation;
    }
    
    skip_operation:
        state = 1;
        return;
    
    memory_operation:
        /* This block tests flow-sensitivity */
        volatile size_t op_size = dest->size < src->size ? dest->size : src->size;
        
        /* Use goto to jump out mid-operation */
        if (op_size < 10) goto skip_operation;
        
        __builtin_memmove(dest->data, src->data, op_size);
        
        /* Jump back in */
        if (state == 0) {
            goto memory_operation;
        }
}

/* OpenMP parallel memory dispatcher */
static void parallel_memory_dispatch(void) {
    #pragma omp parallel
    {
        /* Thread-local buffers */
        char local_buf[256];
        char local_buf2[256];
        volatile int thread_id = omp_get_thread_num();
        
        /* Initialize with builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        __builtin_memcpy(local_buf2, local_buf, sizeof(local_buf));
        
        /* Complex memory movement pattern */
        for (int i = 0; i < 10; i++) {
            volatile size_t offset = (thread_id * 16 + i) % 128;
            __builtin_memmove(local_buf + offset, 
                            local_buf2 + (offset ^ 0x3F), 
                            32);
        }
        
        #pragma omp barrier
        
        /* Copy to global pool */
        #pragma omp critical
        {
            volatile size_t idx = g_token_idx;
            __builtin_memcpy(g_token_pool + idx, local_buf, 64);
            g_token_idx += 64;
        }
    }
}

/* Multi-stage interaction function */
static size_t compute_hash_from_tokens(void) {
    size_t hash = 5381;
    volatile size_t limit = g_token_idx > 1024 ? 1024 : g_token_idx;
    
    for (volatile size_t i = 0; i < limit; i++) {
        hash = ((hash << 5) + hash) + g_token_pool[i];
        
        /* Occasional memory operations during hash computation */
        if (i % 128 == 0) {
            char temp[64];
            __builtin_memset(temp, g_token_pool[i], sizeof(temp));
            __builtin_memcpy(g_token_pool + ((i + 64) % 1024), 
                           temp, 
                           32);
        }
    }
    
    return hash;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize AST structures */
    ASTNode* nodes[4];
    const char* test_data[] = {"AST_Node_Alpha", "AST_Node_Beta", 
                               "AST_Node_Gamma", "AST_Node_Delta"};
    
    for (int i = 0; i < 4; i++) {
        nodes[i] = create_ast_node(test_data[i], strlen(test_data[i]));
        if (!nodes[i]) {
            fprintf(stderr, "Failed to create node %d\n", i);
            return 1;
        }
    }
    
    /* Phase 2: Test goto edge cases */
    for (int i = 0; i < 3; i++) {
        process_with_goto(nodes[i], nodes[i + 1]);
    }
    
    /* Phase 3: Copy between AST nodes using builtins */
    volatile size_t copy_sizes[] = {16, 32, 48};
    for (int i = 0; i < 3; i++) {
        __builtin_memcpy(nodes[i]->data + 16, 
                        nodes[i + 1]->data, 
                        copy_sizes[i]);
    }
    
    /* Phase 4: Parallel memory operations */
    parallel_memory_dispatch();
    
    /* Phase 5: Additional builtin usage with volatile control */
    char final_buffer[512];
    volatile size_t final_size = g_mem_size % 256;
    
    __builtin_memset(final_buffer, 0xCC, sizeof(final_buffer));
    __builtin_memcpy(final_buffer + 128, nodes[0]->data, 64);
    __builtin_memmove(final_buffer + 256, final_buffer + 128, 128);
    
    /* Phase 6: Compute and verify result */
    size_t computed_hash = compute_hash_from_tokens();
    printf("Computed hash: 0x%08lx\n", (unsigned long)computed_hash);
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        free(nodes[i]);
    }
    
    /* Final builtin call to ensure all paths are tested */
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    
    printf("Test completed successfully.\n");
    return 0;
}
