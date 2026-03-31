/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* AST-like recursive structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)((i * 31) & 0xFF);
    }
    
    /* Force early built-in usage in constructor */
    char local_buf[128];
    __builtin_memset(local_buf, 0xAA, sizeof(local_buf));
    __builtin_memcpy(&token_pool[0], local_buf, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operation in destructor */
    char final_check[32];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with built-ins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy pattern into node */
    int copy_len = volatile_len % 128;
    if (copy_len > 0) {
        __builtin_memcpy(node->data, &token_pool[id % 4096], copy_len);
    }
    
    node->id = id;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = volatile_flag;
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast_node(depth - 1, id * 2);
        node->right = NULL;
        return node;
        
    create_children:
        node->left = create_ast_node(depth - 1, id * 2);
        node->right = create_ast_node(depth - 1, id * 2 + 1);
        
        /* Memory move between child nodes */
        if (node->left && node->right) {
            int move_len = 32 + (id % 32);
            __builtin_memmove(node->right->data, node->left->data, move_len);
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void memory_operation_with_goto(char* dest, const char* src, size_t len) {
    int stage = 0;
    
    /* Jump into memory operation block */
    if (len > 100) {
        goto large_operation;
    }
    
    stage = 1;
    
    /* Normal path */
    __builtin_memcpy(dest, src, len);
    return;
    
large_operation:
    /* Jump target with memmove */
    stage = 2;
    char temp_buf[256];
    
    /* Multi-stage memory operations */
    __builtin_memcpy(temp_buf, src, 256);
    
    if (volatile_flag) {
        goto finalize;
    }
    
    __builtin_memset(temp_buf + 128, 0xCC, 128);
    
finalize:
    __builtin_memmove(dest, temp_buf, len > 256 ? 256 : len);
    
    /* Jump out of block */
    if (stage == 2) {
        goto cleanup;
    }
    
cleanup:
    __builtin_memset(temp_buf, 0, sizeof(temp_buf));
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    const int num_workers = 4;
    char worker_buffers[num_workers][512];
    int results[num_workers];
    
    #pragma omp parallel for
    for (int i = 0; i < num_workers; i++) {
        /* Each thread uses built-ins independently */
        __builtin_memset(worker_buffers[i], i, sizeof(worker_buffers[i]));
        
        /* Copy from token pool with volatile length */
        int copy_len = (volatile_len + i) % 512;
        __builtin_memcpy(worker_buffers[i], token_pool, copy_len);
        
        /* Compute simple hash */
        int hash = 0;
        for (int j = 0; j < copy_len; j++) {
            hash = (hash * 31 + worker_buffers[i][j]) & 0xFFFF;
        }
        results[i] = hash;
        
        /* Memory move within buffer */
        if (copy_len > 256) {
            __builtin_memmove(worker_buffers[i] + 128, worker_buffers[i], 256);
        }
    }
    
    /* Verify results */
    int total_hash = 0;
    for (int i = 0; i < num_workers; i++) {
        total_hash ^= results[i];
    }
    
    /* Final memory operation after parallel section */
    char final_buffer[1024];
    __builtin_memset(final_buffer, total_hash & 0xFF, sizeof(final_buffer));
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: AST manipulation */
    ASTNode* root = create_ast_node(4, 1);
    
    if (root) {
        /* Memory operations between AST nodes */
        char node_buffer[512];
        
        /* Test all three built-ins in sequence */
        __builtin_memset(node_buffer, 0, sizeof(node_buffer));
        __builtin_memcpy(node_buffer, root->data, 256);
        
        if (root->left && root->right) {
            __builtin_memmove(root->right->data, root->left->data, 128);
        }
        
        /* Complex goto-based memory operation */
        memory_operation_with_goto(node_buffer, token_pool, 300);
    }
    
    /* Phase 2: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Phase 3: Direct built-in stress test */
    char stress_buf1[1024], stress_buf2[1024];
    
    /* Chain of memory operations */
    __builtin_memset(stress_buf1, 0xAA, sizeof(stress_buf1));
    __builtin_memcpy(stress_buf2, stress_buf1, sizeof(stress_buf1));
    __builtin_memmove(stress_buf1, stress_buf2, sizeof(stress_buf1));
    
    /* Varied lengths using volatile */
    for (int i = 0; i < 10; i++) {
        size_t len = (volatile_len + i * 17) % 1024;
        if (len > 0) {
            __builtin_memcpy(&stress_buf1[i * 64], token_pool, len);
        }
    }
    
    /* Compute verification hash */
    unsigned long long hash = 0;
    for (int i = 0; i < 1024; i++) {
        hash = hash * 31 + stress_buf1[i];
    }
    
    printf("Test completed. Hash: 0x%016llx\n", hash);
    printf("Expected: Built-in redirection to __asan_* or __hwasan_* functions\n");
    
    /* Cleanup */
    /* Note: AST cleanup omitted for brevity - would need recursive free */
    
    return 0;
}
