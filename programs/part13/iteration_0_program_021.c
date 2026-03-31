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
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    uint32_t hash;
} ASTNode;

/* Global token array */
static char g_tokens[][32] = {
    "memcpy_test", "memset_test", "memmove_test",
    "overlap_test", "recursive_test", "parallel_test"
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    volatile char init_buf[128];
    /* Force early built-in usage */
    __builtin_memset(init_buf, 0xAA, sizeof(init_buf));
    __builtin_memcpy(init_buf + 32, "constructor_init", 16);
    
    printf("[Constructor] ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[64];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
    printf("[Destructor] ASAN environment cleaned up\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* data, int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use all three built-ins in varied contexts */
    __builtin_memset(node, 0, sizeof(ASTNode));
    __builtin_memcpy(node->data, data, strlen(data) + 1);
    
    /* Compute hash using memcpy between buffers */
    uint32_t temp_hash = 0;
    for (size_t i = 0; i < sizeof(node->data); i += 4) {
        uint32_t chunk;
        __builtin_memcpy(&chunk, &node->data[i], sizeof(chunk));
        temp_hash ^= chunk;
    }
    node->hash = temp_hash;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        char left_data[64], right_data[64];
        __builtin_memset(left_data, 0, sizeof(left_data));
        __builtin_memset(right_data, 0, sizeof(right_data));
        
        /* Goto-based control flow */
        if (depth % 2 == 0) goto create_left;
        else goto create_right;
        
    create_left:
        __builtin_memcpy(left_data, "left_", 5);
        __builtin_memcpy(left_data + 5, data, strlen(data));
        node->left = create_ast_node(left_data, depth - 1);
        goto after_left;
        
    create_right:
        __builtin_memcpy(right_data, "right_", 6);
        __builtin_memcpy(right_data + 6, data, strlen(data));
        node->right = create_ast_node(right_data, depth - 1);
        goto after_right;
        
    after_left:
    after_right:
        /* Memmove with potential overlap */
        if (node->left && node->right) {
            char overlap_buf[128];
            __builtin_memcpy(overlap_buf, node->left->data, 64);
            __builtin_memmove(overlap_buf + 32, overlap_buf, 64);
            __builtin_memcpy(node->right->data, overlap_buf + 32, 64);
        }
    }
    
    return node;
}

/* Parallel memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[256];
        char src_buf[256];
        
        /* Initialize with built-ins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        __builtin_memset(src_buf, 0xCC, sizeof(src_buf));
        
        /* Varied memory operations per thread */
        switch (thread_id % 3) {
            case 0:
                __builtin_memcpy(local_buf + 64, src_buf, g_mem_size);
                break;
            case 1:
                __builtin_memset(local_buf + 128, 0xDD, g_mem_size);
                break;
            case 2:
                __builtin_memmove(local_buf, local_buf + 32, g_mem_size);
                break;
        }
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Verify operations */
        #pragma omp critical
        {
            volatile char verify_buf[128];
            __builtin_memcpy(verify_buf, local_buf, 64);
            __builtin_memset(verify_buf + 64, 0xEE, 64);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Initialize complex data structures */
    ASTNode* root = NULL;
    uint64_t total_hash = 0;
    
    for (size_t i = 0; i < sizeof(g_tokens)/sizeof(g_tokens[0]); i++) {
        ASTNode* node = create_ast_node(g_tokens[i], 3);
        if (node) {
            total_hash += node->hash;
            
            /* Chain memory operations between nodes */
            if (root) {
                char transfer_buf[512];
                __builtin_memcpy(transfer_buf, root->data, 256);
                __builtin_memcpy(transfer_buf + 256, node->data, 256);
                __builtin_memmove(root->data, transfer_buf, 256);
                __builtin_memmove(node->data, transfer_buf + 256, 256);
            }
            root = node;
        }
    }
    
    /* Phase 2: Execute parallel operations */
    printf("Executing parallel memory operations...\n");
    parallel_memory_operations();
    
    /* Phase 3: Stress test with volatile-controlled loops */
    volatile int iterations = 100;
    volatile char stress_buf[1024];
    volatile char src_buf[1024];
    
    __builtin_memset(src_buf, 0xAB, sizeof(src_buf));
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Alternate between built-ins */
        switch (i % 3) {
            case 0:
                __builtin_memcpy(stress_buf, src_buf, g_mem_size * (i + 1));
                break;
            case 1:
                __builtin_memset(stress_buf + 256, i, g_mem_size);
                break;
            case 2:
                __builtin_memmove(stress_buf, stress_buf + 128, g_mem_size);
                break;
        }
        
        /* Goto to create additional control flow edges */
        if (i == 50) goto special_case;
    }
    
    goto normal_exit;
    
special_case:
    /* Special path with overlapping memmove */
    __builtin_memmove(stress_buf + 384, stress_buf + 256, 256);
    __builtin_memset(stress_buf + 512, 0xFF, 128);
    
normal_exit:
    /* Phase 4: Verification and cleanup */
    printf("Total AST hash: %llu\n", (unsigned long long)total_hash);
    printf("Final memory size used: %zu\n", (size_t)g_mem_size);
    
    /* Final built-in calls to ensure coverage */
    volatile char final_buf[64];
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf, "TEST_COMPLETE", 13);
    __builtin_memmove(final_buf + 16, final_buf, 13);
    
    printf("=== Test Complete ===\n");
    return 0;
}
