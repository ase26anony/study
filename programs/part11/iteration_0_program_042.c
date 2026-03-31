/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _OPENMP
#include <omp.h>
#endif

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    uint32_t hash;
} ASTNode;

/* Global token array */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "test"
};
static const int g_token_count = sizeof(g_tokens) / sizeof(g_tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_environment(void) {
    printf("Initializing ASAN test environment...\n");
    
    /* Force early initialization with builtins */
    char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, "init", 5);
    
    /* Create volatile copy to prevent dead code elimination */
    volatile char* vbuf = buffer;
    (void)vbuf;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    printf("Cleaning up ASAN test environment...\n");
    
    /* Final builtin calls in destructor */
    char final_buf[16];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, const char* token) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data with memcpy */
    size_t token_len = __builtin_strlen(token);
    size_t copy_len = token_len < sizeof(node->data) - 1 ? token_len : sizeof(node->data) - 1;
    __builtin_memcpy(node->data, token, copy_len);
    node->data[copy_len] = '\0';
    
    /* Create children recursively */
    node->left = create_ast_node(depth - 1, g_tokens[(depth * 7) % g_token_count]);
    node->right = create_ast_node(depth - 1, g_tokens[(depth * 13) % g_token_count]);
    
    return node;
}

/* Complex memory operation with goto flow control */
static void perform_memory_operations(ASTNode* node1, ASTNode* node2) {
    char temp_buffer[128];
    volatile int use_memmove = 1;
    
    /* Goto label before builtin */
    before_memcpy:
    __builtin_memcpy(temp_buffer, node1->data, sizeof(node1->data));
    
    if (use_memmove) {
        /* Jump into memmove block */
        goto do_memmove;
    }
    
    /* This should be skipped by goto */
    __builtin_memset(temp_buffer, 0, sizeof(temp_buffer));
    
    do_memmove:
    /* Force memmove with overlapping regions */
    char* overlap_src = temp_buffer + 32;
    char* overlap_dst = temp_buffer + 16;
    __builtin_memmove(overlap_dst, overlap_src, 64);
    
    /* Jump out of block */
    goto after_operations;
    
    /* Unreachable code with builtin (tests edge cases) */
    __builtin_memset(temp_buffer, 0xFF, sizeof(temp_buffer));
    
    after_operations:
    /* Copy back with volatile size */
    volatile size_t copy_size = g_mem_size % 64;
    __builtin_memcpy(node2->data, temp_buffer, copy_size);
}

/* Parallel memory dispatch logic */
static uint32_t parallel_memory_dispatch(ASTNode** nodes, int count) {
    uint32_t total_hash = 0;
    
    #pragma omp parallel reduction(+:total_hash)
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Thread-specific memory operations */
                char thread_buffer[256];
                
                /* Mix of builtins with volatile parameters */
                volatile int fill_value = (thread_id * 17) & 0xFF;
                __builtin_memset(thread_buffer, fill_value, sizeof(thread_buffer));
                
                /* Copy node data with overlapping regions */
                __builtin_memcpy(thread_buffer + 64, nodes[i]->data, sizeof(nodes[i]->data));
                
                /* Move data around */
                __builtin_memmove(thread_buffer, thread_buffer + 32, 128);
                
                /* Compute simple hash */
                uint32_t hash = 0;
                for (int j = 0; j < 64; j++) {
                    hash = (hash * 31) + thread_buffer[j];
                }
                nodes[i]->hash = hash;
                total_hash += hash;
            }
        }
    }
    
    return total_hash;
}

/* Function with switch-based builtin selection */
static void dispatch_builtin_by_index(int index, void* dst, const void* src, size_t n) {
    volatile int builtin_type = index % 3;
    
    switch (builtin_type) {
        case 0:
            __builtin_memcpy(dst, src, n);
            break;
        case 1:
            __builtin_memset(dst, (index * 7) & 0xFF, n);
            break;
        case 2:
            __builtin_memmove(dst, src, n);
            break;
    }
}

/* Main test execution */
int main(void) {
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Create AST structure */
    ASTNode* root = create_ast_node(4, g_tokens[0]);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create sibling node for memory operations */
    ASTNode* sibling = create_ast_node(3, g_tokens[1]);
    
    /* Perform memory operations with goto flow control */
    perform_memory_operations(root, sibling);
    
    /* Create node array for parallel processing */
    ASTNode* node_array[8];
    node_array[0] = root;
    node_array[1] = sibling;
    
    /* Fill remaining slots with new nodes */
    for (int i = 2; i < 8; i++) {
        node_array[i] = create_ast_node(2, g_tokens[i % g_token_count]);
    }
    
    /* Execute parallel memory dispatch */
    uint32_t final_hash = parallel_memory_dispatch(node_array, 8);
    
    /* Additional builtin calls in main */
    char main_buffer[512];
    volatile size_t dynamic_size = g_mem_size;
    
    /* Chain of builtin operations */
    __builtin_memset(main_buffer, 0xAA, dynamic_size);
    __builtin_memcpy(main_buffer + 128, root->data, sizeof(root->data));
    __builtin_memmove(main_buffer + 64, main_buffer + 128, 192);
    
    /* Dispatch builtins via function */
    for (int i = 0; i < 5; i++) {
        dispatch_builtin_by_index(i, main_buffer + (i * 32), 
                                 g_tokens[i % g_token_count], 16);
    }
    
    /* Compute final verification value */
    uint32_t verification = final_hash;
    for (int i = 0; i < 256; i++) {
        verification = (verification * 17) + main_buffer[i];
    }
    
    printf("Test completed. Verification hash: 0x%08X\n", verification);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        if (node_array[i]) {
            free(node_array[i]);
        }
    }
    
    return 0;
}
