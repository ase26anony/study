/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char g_token_array[4096];
static volatile size_t g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token array with pattern */
    for (size_t i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)((i * 13) % 256);
    }
    printf("Constructor: Token array initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(size_t depth, volatile size_t* counter) {
    if (depth == 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with goto for flow control */
    char temp[64];
    __builtin_memset(temp, 'A' + (depth % 26), sizeof(temp));
    
    /* Jump label for goto testing */
    copy_data:
    __builtin_memcpy(node->data, temp, sizeof(node->data));
    
    node->size = depth * 16;
    (*counter)++;
    
    /* Recursive creation with goto out of block */
    if (depth > 1) {
        node->left = create_ast(depth - 1, counter);
        if (node->left) {
            /* Move data between nodes using __builtin_memmove */
            goto move_block;
            move_block_return:
            ;
        }
        
        node->right = create_ast(depth - 1, counter);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
    
move_block:
    /* This tests flow-sensitivity with goto into memmove block */
    if (node->left) {
        __builtin_memmove(node->data + 32, node->left->data, 32);
    }
    goto move_block_return;
}

/* Parallel memory dispatch function */
static void parallel_memory_ops(volatile char* dest, volatile char* src, size_t size) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        size_t chunk = size / omp_get_num_threads();
        size_t start = tid * chunk;
        size_t end = (tid == omp_get_num_threads() - 1) ? size : start + chunk;
        
        /* Each thread performs memory operations */
        char local_buf[128];
        
        /* Copy from source to local buffer */
        __builtin_memcpy(local_buf, (char*)src + start, 
                        (end - start) < 128 ? (end - start) : 128);
        
        /* Process data */
        for (size_t i = 0; i < 128 && i < (end - start); i++) {
            local_buf[i] ^= 0x55;
        }
        
        /* Move back to destination */
        __builtin_memmove((char*)dest + start, local_buf, 
                         (end - start) < 128 ? (end - start) : 128);
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Final memset in parallel */
        if (tid == 0) {
            __builtin_memset((char*)dest + size - 64, 0xFF, 64);
        }
    }
}

/* Function with complex control flow and builtins */
static size_t process_ast(ASTNode* root, volatile size_t* hash) {
    if (!root) return 0;
    
    size_t local_hash = 0;
    char buffer[128];
    
    /* Jump into different processing blocks */
    if (root->size > 32) {
        goto process_large;
    } else {
        goto process_small;
    }
    
process_large:
    /* Large node processing with memcpy */
    __builtin_memcpy(buffer, root->data, 64);
    
    /* Compute hash */
    for (int i = 0; i < 64; i++) {
        local_hash = (local_hash * 31) + buffer[i];
    }
    
    goto continue_processing;
    
process_small:
    /* Small node processing with memset */
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, root->data, root->size);
    
    for (size_t i = 0; i < root->size; i++) {
        local_hash = (local_hash * 17) + buffer[i];
    }
    
    goto continue_processing;
    
continue_processing:
    /* Recursive processing */
    size_t left_hash = process_ast(root->left, hash);
    size_t right_hash = process_ast(root->right, hash);
    
    *hash = (*hash + local_hash + left_hash + right_hash) % 1000000007;
    return local_hash;
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    volatile size_t node_counter = 0;
    volatile size_t final_hash = 0;
    
    /* Create recursive AST */
    ASTNode* root = create_ast(5, &node_counter);
    printf("Created AST with %zu nodes\n", node_counter);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process AST to compute hash */
    process_ast(root, &final_hash);
    printf("AST hash: %zu\n", final_hash);
    
    /* Test parallel memory operations */
    char* src_buf = (char*)malloc(g_mem_size);
    char* dest_buf = (char*)malloc(g_mem_size);
    
    if (src_buf && dest_buf) {
        /* Initialize source with pattern */
        for (size_t i = 0; i < g_mem_size; i++) {
            src_buf[i] = (char)(i % 256);
        }
        
        /* Clear destination */
        __builtin_memset(dest_buf, 0, g_mem_size);
        
        /* Perform parallel memory operations */
        parallel_memory_ops((volatile char*)dest_buf, 
                           (volatile char*)src_buf, 
                           g_mem_size);
        
        /* Verify by computing checksum */
        size_t checksum = 0;
        for (size_t i = 0; i < g_mem_size; i++) {
            checksum += dest_buf[i];
        }
        printf("Parallel ops checksum: %zu\n", checksum);
        
        free(src_buf);
        free(dest_buf);
    }
    
    /* Test builtins with volatile control */
    volatile char test_buf[512];
    volatile char test_src[512];
    
    /* Initialize with different builtins */
    __builtin_memset((char*)test_src, 0xAA, sizeof(test_src));
    __builtin_memcpy((char*)test_buf, (char*)test_src, sizeof(test_buf));
    
    /* Move data around */
    __builtin_memmove((char*)test_buf + 256, (char*)test_buf, 256);
    
    /* Final verification sum */
    size_t final_sum = 0;
    for (size_t i = 0; i < sizeof(test_buf); i++) {
        final_sum += ((char*)test_buf)[i];
    }
    printf("Final buffer sum: %zu\n", final_sum);
    
    /* Cleanup */
    /* Note: In real code, you'd need a proper AST free function */
    
    printf("=== Test Complete ===\n");
    return 0;
}
