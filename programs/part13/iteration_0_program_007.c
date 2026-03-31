/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
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
    char* data;
    size_t size;
    uint64_t hash;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_redirection(void) {
    printf("Constructor: Initializing ASAN redirection test environment\n");
    
    /* Force early calls to builtins */
    char buffer1[64];
    char buffer2[64];
    
    /* These should trigger ASAN built-in initialization */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1, buffer2, sizeof(buffer1));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    printf("Destructor: ASAN test completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, size_t data_size) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->size = data_size;
    node->data = (char*)malloc(data_size);
    
    /* Use volatile to prevent constant folding */
    volatile size_t fill_size = data_size;
    
    /* Force built-in memset through ASAN */
    __builtin_memset(node->data, depth, fill_size);
    
    /* Calculate hash using memory operations */
    node->hash = 0;
    for (size_t i = 0; i < data_size; i++) {
        node->hash = (node->hash * 31) + node->data[i];
    }
    
    /* Recursive creation with goto for control flow */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
create_children:
    node->left = create_ast(depth - 1, data_size / 2);
    
    /* Jump back to avoid optimization */
    if (create_left) {
        create_left = 0;
        goto create_right;
    }
    
create_right:
    node->right = create_ast(depth - 1, data_size / 2);
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void test_goto_memmove(ASTNode* src, ASTNode* dst) {
    if (!src || !dst || src->size != dst->size) return;
    
    int use_memmove = 1;
    
    if (src->data && dst->data) {
        goto perform_operation;
    }
    
perform_operation:
    /* This goto should test flow sensitivity */
    if (use_memmove) {
        /* Force built-in memmove through ASAN */
        __builtin_memmove(dst->data, src->data, src->size);
        use_memmove = 0;
        goto update_hash;
    }
    
update_hash:
    /* Recalculate hash after memmove */
    dst->hash = 0;
    for (size_t i = 0; i < dst->size; i++) {
        dst->hash = (dst->hash * 31) + dst->data[i];
    }
}

/* OpenMP parallel section with memory operations */
static uint64_t parallel_memory_operations(ASTNode** nodes, int count) {
    uint64_t total_hash = 0;
    
    #pragma omp parallel reduction(+:total_hash)
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Create temporary buffer */
                size_t buf_size = g_mem_size + thread_id;
                char* temp_buf = (char*)malloc(buf_size);
                
                if (temp_buf) {
                    /* Mix of built-in calls */
                    __builtin_memset(temp_buf, i, buf_size);
                    
                    /* Copy from node data */
                    size_t copy_size = nodes[i]->size < buf_size ? 
                                      nodes[i]->size : buf_size;
                    __builtin_memcpy(temp_buf, nodes[i]->data, copy_size);
                    
                    /* Move data back with memmove (overlapping possible) */
                    __builtin_memmove(nodes[i]->data, temp_buf, copy_size);
                    
                    /* Update hash */
                    uint64_t local_hash = 0;
                    for (size_t j = 0; j < copy_size; j++) {
                        local_hash = (local_hash * 31) + nodes[i]->data[j];
                    }
                    nodes[i]->hash = local_hash;
                    total_hash += local_hash;
                    
                    free(temp_buf);
                }
            }
        }
    }
    
    return total_hash;
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Stress Test ===\n");
    
    /* Create AST tree */
    const int ast_depth = 4;
    const size_t base_size = 128;
    
    ASTNode* root = create_ast(ast_depth, base_size);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    const int node_count = 8;
    ASTNode* nodes[node_count];
    
    for (int i = 0; i < node_count; i++) {
        nodes[i] = create_ast(ast_depth - 1, base_size / (i + 1));
    }
    
    /* Test goto with memmove */
    printf("Testing goto-controlled memmove...\n");
    for (int i = 0; i < node_count - 1; i++) {
        test_goto_memmove(nodes[i], nodes[i + 1]);
    }
    
    /* Parallel memory operations */
    printf("Running parallel memory operations...\n");
    uint64_t parallel_hash = parallel_memory_operations(nodes, node_count);
    
    /* Additional built-in calls in main */
    char final_buffer[512];
    volatile size_t final_size = sizeof(final_buffer);
    
    __builtin_memset(final_buffer, 0, final_size);
    
    /* Copy hashes into buffer */
    for (int i = 0; i < node_count; i++) {
        if (nodes[i]) {
            size_t offset = (i * sizeof(uint64_t)) % sizeof(final_buffer);
            size_t copy_len = sizeof(uint64_t);
            if (offset + copy_len > sizeof(final_buffer)) {
                copy_len = sizeof(final_buffer) - offset;
            }
            __builtin_memcpy(final_buffer + offset, &nodes[i]->hash, copy_len);
        }
    }
    
    /* Final memmove to shift data */
    __builtin_memmove(final_buffer + 64, final_buffer, 128);
    
    /* Calculate final checksum */
    uint64_t final_checksum = 0;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        final_checksum = (final_checksum * 31) + final_buffer[i];
    }
    
    printf("Results:\n");
    printf("  Root hash: %llu\n", (unsigned long long)root->hash);
    printf("  Parallel hash sum: %llu\n", (unsigned long long)parallel_hash);
    printf("  Final checksum: %llu\n", (unsigned long long)final_checksum);
    printf("  AST depth: %d\n", ast_depth);
    printf("  Memory size: %zu\n", (size_t)g_mem_size);
    
    /* Cleanup */
    /* Helper function to free AST recursively */
    void free_ast(ASTNode* node) {
        if (!node) return;
        free_ast(node->left);
        free_ast(node->right);
        free(node->data);
        free(node);
    }
    
    free_ast(root);
    for (int i = 0; i < node_count; i++) {
        free_ast(nodes[i]);
    }
    
    printf("=== Test completed successfully ===\n");
    return 0;
}
