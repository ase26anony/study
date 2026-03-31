#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile size_t size;  /* volatile to prevent optimization */
    struct ASTNode *left;
    struct ASTNode *right;
    unsigned char data[64];
} ASTNode;

/* Global volatile variables to prevent constant folding */
volatile size_t g_memcpy_len = 32;
volatile size_t g_memset_len = 48;
volatile size_t g_memmove_len = 24;

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[128];
    
    /* Force __builtin_memset in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    
    /* Use the buffer to prevent optimization */
    volatile char *ptr = buffer;
    while (*ptr == 0xAA) {
        ptr++;
    }
}

/* Destructor function for cleanup coordination */
__attribute__((destructor))
static void cleanup_asan_late(void) {
    volatile int dummy = 0;
    __builtin_memset(&dummy, 0, sizeof(dummy));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int value) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = depth;
    node->value = value;
    node->size = (size_t)(depth * 16);
    node->left = create_ast(depth - 1, value * 2);
    node->right = create_ast(depth - 1, value * 3);
    
    /* Initialize data with __builtin_memset */
    __builtin_memset(node->data, value, sizeof(node->data));
    
    return node;
}

/* Function with goto statements for flow control */
static void process_with_goto(ASTNode *src, ASTNode *dst) {
    if (!src || !dst) return;
    
    int state = 0;
    
    /* Jump into memory operation block */
    goto entry_point;
    
memory_operation:
    /* Use __builtin_memmove with goto */
    __builtin_memmove(dst->data, src->data, g_memmove_len);
    state = 1;
    goto exit_point;
    
entry_point:
    /* Jump to memory operation */
    goto memory_operation;
    
exit_point:
    /* Modify data after memmove */
    if (state) {
        __builtin_memset(dst->data + 16, 0xFF, 8);
    }
}

/* Function with varied memory built-in usage */
static unsigned long process_ast(ASTNode *node) {
    if (!node) return 0;
    
    unsigned long hash = 0;
    volatile unsigned char temp[128];
    
    /* Use all three built-ins in different contexts */
    
    /* 1. __builtin_memcpy between node data */
    if (node->left && node->right) {
        __builtin_memcpy(temp, node->left->data, g_memcpy_len);
        __builtin_memcpy(node->right->data, temp, g_memcpy_len);
    }
    
    /* 2. __builtin_memset with volatile length */
    __builtin_memset(temp, node->value, g_memset_len);
    
    /* 3. Calculate hash from data */
    for (size_t i = 0; i < sizeof(node->data) && i < 32; i++) {
        hash = (hash * 31) + node->data[i];
    }
    
    /* Recursive processing */
    hash += process_ast(node->left);
    hash += process_ast(node->right);
    
    return hash;
}

/* Parallel processing function */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        volatile char local_buf[256];
        volatile char shared_buf[256];
        
        /* Each thread uses memory built-ins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        /* Use __builtin_memcpy between buffers */
        if (thread_id % 2 == 0) {
            __builtin_memcpy((void*)shared_buf, (void*)local_buf, 128);
        }
        
        #pragma omp barrier
        
        /* Use __builtin_memmove for overlapping regions */
        if (thread_id < 4) {
            __builtin_memmove(local_buf + 64, local_buf + 32, 64);
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN coverage test...\n");
    
    /* Create complex AST structure */
    ASTNode *root = create_ast(4, 42);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process with goto flow control */
    process_with_goto(root->left, root->right);
    
    /* Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Process AST and compute result */
    unsigned long result = process_ast(root);
    
    /* Additional memory operations in main */
    volatile int final_buffer[1024];
    
    /* Use all three built-ins in sequence */
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer + 256, root->data, 64);
    __builtin_memmove(final_buffer + 512, final_buffer + 256, 128);
    
    /* Verify operations by computing checksum */
    unsigned long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += final_buffer[i];
    }
    
    result ^= checksum;
    
    printf("Result: 0x%lx\n", result);
    printf("Test completed.\n");
    
    /* Cleanup */
    /* Note: In real code, you'd need to properly free the AST */
    
    return 0;
}
