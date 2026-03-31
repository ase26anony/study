/* ISO C99-compliant test program for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

/* AST-like recursive structure */
struct ast_node {
    char data[256];
    struct ast_node* left;
    struct ast_node* right;
    int id;
};

/* Global token array */
static char token_pool[4096];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token pool with pattern */
    for (size_t i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)((i * 13) & 0xFF);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive AST manipulation with memory operations */
static struct ast_node* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(struct ast_node));
    node->id = id;
    
    /* Fill data with pattern using volatile size */
    size_t copy_size = g_mem_size % 256;
    for (size_t i = 0; i < copy_size; i++) {
        node->data[i] = (char)((id + i) & 0xFF);
    }
    
    /* Recursive creation with goto for control flow */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    return node;
    
create_children:
    /* Jump back into block with memory operation */
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    /* Copy data between nodes if both exist */
    if (node->left && node->right) {
        size_t copy_len = (g_mem_size % 128) + 1;
        
        /* Use __builtin_memcpy between nodes */
        __builtin_memcpy(node->right->data, 
                        node->left->data, 
                        copy_len);
        
        /* Use __builtin_memmove for overlapping regions */
        if (copy_len > 32) {
            __builtin_memmove(node->left->data + 16,
                             node->left->data,
                             copy_len - 16);
        }
    }
    
    return node;
}

/* Function with complex control flow and goto */
static void process_with_goto(struct ast_node* node) {
    if (!node) return;
    
    volatile int do_copy = 1;
    
    /* Jump into memory operation block */
    if (node->id % 3 == 0) {
        goto perform_memcpy;
    }
    
    /* Normal path */
    node->id += 1000;
    goto end_processing;
    
perform_memcpy:
    {
        char temp[128];
        size_t len = g_mem_size % 128;
        
        /* Force built-in usage with goto context */
        __builtin_memcpy(temp, node->data, len);
        
        /* Modify and copy back */
        for (size_t i = 0; i < len; i++) {
            temp[i] ^= 0x55;
        }
        
        __builtin_memcpy(node->data, temp, len);
    }
    
    /* Jump out of block */
    goto after_memcpy;
    
after_memcpy:
    node->id += 2000;
    
end_processing:
    /* Recursive processing */
    process_with_goto(node->left);
    process_with_goto(node->right);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char src_buf[256];
        char dst_buf[256];
        
        /* Initialize with pattern */
        for (int i = 0; i < 256; i++) {
            src_buf[i] = (char)((thread_id + i) & 0xFF);
        }
        
        /* Use all three built-ins in parallel region */
        __builtin_memset(dst_buf, 0, sizeof(dst_buf));
        __builtin_memcpy(dst_buf, src_buf, g_mem_size % 256);
        
        /* Overlapping memmove */
        if (thread_id % 2 == 0) {
            __builtin_memmove(dst_buf + 64, dst_buf + 32, 128);
        }
        
        /* Verify by computing checksum */
        unsigned long sum = 0;
        for (int i = 0; i < 256; i++) {
            sum += (unsigned char)dst_buf[i];
        }
        
        #pragma omp critical
        {
            printf("Thread %d: checksum = %lu\n", thread_id, sum);
        }
    }
}

/* Multi-stage initialization */
static void initialize_test_data(char* buffer, size_t size) {
    volatile size_t actual_size = size;
    
    /* Stage 1: Clear with memset */
    __builtin_memset(buffer, 0, actual_size);
    
    /* Stage 2: Fill pattern with memcpy from token pool */
    size_t copy_size = actual_size;
    if (copy_size > sizeof(token_pool)) {
        copy_size = sizeof(token_pool);
    }
    
    __builtin_memcpy(buffer, token_pool, copy_size);
    
    /* Stage 3: Rotate with memmove */
    if (actual_size > 128) {
        __builtin_memmove(buffer + 64, buffer, actual_size - 64);
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Initialize complex data structures */
    struct ast_node* root = create_ast(4, 1);
    
    /* Phase 2: Process with goto control flow */
    process_with_goto(root);
    
    /* Phase 3: OpenMP parallel operations */
    printf("\nParallel memory operations:\n");
    parallel_memory_ops();
    
    /* Phase 4: Multi-stage buffer processing */
    char test_buffer[512];
    initialize_test_data(test_buffer, sizeof(test_buffer));
    
    /* Phase 5: Additional built-in calls in varied contexts */
    {
        /* Array of function pointers pattern */
        char* buffers[3];
        for (int i = 0; i < 3; i++) {
            buffers[i] = malloc(256);
            if (buffers[i]) {
                /* Alternate between built-ins */
                switch (i) {
                    case 0:
                        __builtin_memset(buffers[i], i, 256);
                        break;
                    case 1:
                        __builtin_memcpy(buffers[i], test_buffer, 
                                       g_mem_size % 256);
                        break;
                    case 2:
                        __builtin_memmove(buffers[i], buffers[0], 128);
                        break;
                }
            }
        }
        
        /* Cleanup */
        for (int i = 0; i < 3; i++) {
            free(buffers[i]);
        }
    }
    
    /* Compute final verification hash */
    unsigned long final_hash = 0;
    if (root) {
        /* Use node data for hash computation */
        for (int i = 0; i < 256; i++) {
            final_hash += (unsigned char)root->data[i];
            final_hash = (final_hash << 3) | (final_hash >> 61);
        }
    }
    
    printf("\nFinal verification hash: %lu\n", final_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real code, would need proper AST cleanup */
    
    return 0;
}
