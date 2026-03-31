/* ISO C99-compliant test program for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int depth;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_hooks(void) {
    /* Force initialization of ASAN runtime */
    volatile char init_buf[16];
    __builtin_memset(init_buf, 0, sizeof(init_buf));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_hooks(void) {
    /* Final memory operation to ensure cleanup path */
    volatile char cleanup_buf[8];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Complex token array initialization */
static void init_token_array(char* tokens, size_t size) {
    volatile size_t i = 0;
    
    /* Use goto for control flow edge cases */
    if (size > 0) {
        goto init_start;
    }
    
    return;
    
init_start:
    for (; i < size; i++) {
        tokens[i] = (char)(i % 256);
    }
    
    /* Jump back with goto */
    if (size > 100) {
        goto large_array;
    }
    return;
    
large_array:
    /* Additional memset for large arrays */
    __builtin_memset(tokens + 100, 0xAA, size - 100);
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in data */
    snprintf(node->data, sizeof(node->data), "AST%d", depth);
    node->depth = depth;
    
    /* Recursive creation */
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    /* Copy data between nodes if siblings exist */
    if (node->left && node->right) {
        /* Use memcpy to copy data between nodes */
        __builtin_memcpy(node->right->data, node->left->data, 
                        sizeof(node->data));
        
        /* Use memmove for overlapping regions */
        volatile char temp[64];
        __builtin_memcpy(temp, node->data, 32);
        __builtin_memmove(node->data + 16, node->data, 32);
        __builtin_memcpy(node->data, temp, 32);
    }
    
    return node;
}

/* Parallel memory operations using OpenMP */
static void parallel_mem_operations(char* buffer, size_t size) {
    /* Use volatile to prevent optimization */
    volatile size_t chunk_size = size / 4;
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        size_t offset = thread_id * chunk_size;
        
        if (offset + chunk_size <= size) {
            /* Each thread performs different memory operations */
            switch (thread_id % 3) {
                case 0:
                    __builtin_memset(buffer + offset, thread_id, chunk_size);
                    break;
                case 1:
                    __builtin_memcpy(buffer + offset, 
                                   buffer + (offset + chunk_size/2),
                                   chunk_size/2);
                    break;
                case 2:
                    __builtin_memmove(buffer + offset + chunk_size/4,
                                    buffer + offset,
                                    chunk_size/2);
                    break;
            }
        }
        
        /* Barrier to synchronize */
        #pragma omp barrier
        
        /* All threads verify their work */
        #pragma omp for
        for (size_t i = 0; i < size; i++) {
            buffer[i] ^= 0x55;  /* Simple transformation */
        }
    }
}

/* Complex control flow with goto around memory operations */
static void goto_memmove_test(char* buf1, char* buf2, size_t size) {
    volatile int use_memmove = 1;
    
    if (size < 10) {
        goto small_buffer;
    }
    
    /* Jump into memory operation block */
    goto perform_op;
    
small_buffer:
    __builtin_memset(buf1, 0, size);
    return;
    
perform_op:
    if (use_memmove) {
        /* This should trigger the memmove built-in */
        __builtin_memmove(buf2, buf1, size);
        
        /* Jump out of block */
        goto after_op;
    }
    
    __builtin_memcpy(buf2, buf1, size);
    
after_op:
    /* Verify by reversing */
    __builtin_memmove(buf1, buf2, size);
}

/* Main test driver */
int main(void) {
    const size_t buffer_size = (size_t)g_mem_size;
    char* buffer1 = (char*)malloc(buffer_size);
    char* buffer2 = (char*)malloc(buffer_size);
    
    if (!buffer1 || !buffer2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Stage 1: Initialize with complex patterns */
    init_token_array(buffer1, buffer_size);
    
    /* Stage 2: Create and manipulate AST */
    ASTNode* root = create_ast(4);
    
    if (root) {
        /* Copy AST data to buffer */
        __builtin_memcpy(buffer2, root->data, sizeof(root->data));
        
        /* Recursive cleanup would go here */
        free(root);
    }
    
    /* Stage 3: Parallel memory operations */
    parallel_mem_operations(buffer1, buffer_size);
    
    /* Stage 4: Goto-based memory move test */
    goto_memmove_test(buffer1, buffer2, buffer_size / 2);
    
    /* Stage 5: Final verification hash */
    unsigned long long hash = 0;
    for (size_t i = 0; i < buffer_size; i++) {
        hash = (hash * 31) + (unsigned char)buffer1[i];
    }
    
    printf("Final hash: %llu\n", hash);
    printf("Buffer size: %zu\n", buffer_size);
    
    /* Cleanup */
    free(buffer1);
    free(buffer2);
    
    return 0;
}
