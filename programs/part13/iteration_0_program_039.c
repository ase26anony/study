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
    char padding[32];      /* Ensure size for memcpy operations */
} ASTNode;

/* Global volatile variables to control memory operations */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    volatile char buffer[128];
    /* Force builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 32, buffer, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    volatile int data[16];
    __builtin_memset(data, 0xFF, sizeof(data));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int value) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile to prevent constant folding */
    volatile size_t local_size = g_mem_size;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = value;
    node->size = local_size;
    
    /* Create children with goto for flow control */
    if (depth > 1) {
        int use_goto = (value % 3 == 0);
        
        if (use_goto) {
            goto create_left;
        }
        
        node->left = create_ast(depth - 1, value * 2);
        
        create_left:
        node->right = create_ast(depth - 1, value * 2 + 1);
        
        /* Copy data between nodes using memcpy/memmove */
        if (node->left && node->right) {
            volatile int use_memmove = g_use_memmove;
            
            if (use_memmove) {
                /* Jump into memmove block */
                goto do_memmove;
            } else {
                __builtin_memcpy(node->right, node->left, 
                               sizeof(ASTNode) < local_size ? 
                               sizeof(ASTNode) : local_size);
            }
            
            do_memmove:
            __builtin_memmove(node->left->padding, node->right->padding, 32);
            
            /* Jump out of memmove block */
            goto after_copy;
        }
    }
    
    after_copy:
    return node;
}

/* Function with complex control flow and builtins */
static void process_ast(ASTNode* root, int* result) {
    if (!root) return;
    
    volatile char temp_buffer[256];
    volatile size_t copy_size = root->size % 128;
    
    /* Multiple memory operations with different builtins */
    __builtin_memset(temp_buffer, root->value, sizeof(temp_buffer));
    
    /* Conditional goto around memcpy */
    if (root->type % 2 == 0) {
        goto skip_memcpy;
    }
    
    __builtin_memcpy(temp_buffer + 64, temp_buffer, copy_size);
    
    skip_memcpy:
    /* Always execute memmove */
    __builtin_memmove(temp_buffer + 128, temp_buffer + 32, copy_size);
    
    /* Compute hash from buffer */
    for (size_t i = 0; i < copy_size; i++) {
        *result ^= temp_buffer[i];
        *result = (*result << 3) | (*result >> 29);
    }
    
    process_ast(root->left, result);
    process_ast(root->right, result);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        volatile char thread_buffer[512];
        volatile size_t sizes[] = {32, 64, 128, 256};
        volatile size_t size_idx = thread_id % 4;
        
        /* Each thread uses different memory builtins */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(thread_buffer, thread_id, 
                               sizes[size_idx]);
                break;
            case 1:
                __builtin_memcpy(thread_buffer + 128, thread_buffer, 
                               sizes[size_idx]);
                break;
            case 2:
                __builtin_memmove(thread_buffer + 256, thread_buffer + 64, 
                                sizes[size_idx]);
                break;
        }
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Combined operation after barrier */
        #pragma omp single
        {
            volatile char combined[1024];
            __builtin_memset(combined, 0, sizeof(combined));
            __builtin_memcpy(combined + 512, thread_buffer, 256);
            __builtin_memmove(combined, combined + 256, 256);
        }
    }
}

/* Main execution flow */
int main(void) {
    int result = 0x12345678;
    
    printf("Starting ASAN coverage test...\n");
    
    /* Phase 1: Create and process AST */
    ASTNode* ast = create_ast(4, 42);
    if (ast) {
        process_ast(ast, &result);
        
        /* Additional memory operations on AST */
        volatile ASTNode temp_node;
        __builtin_memcpy(&temp_node, ast, sizeof(ASTNode));
        __builtin_memset(ast->padding, 0xCC, sizeof(ast->padding));
        __builtin_memmove(&temp_node.padding, ast->padding, 16);
        
        free(ast);
    }
    
    /* Phase 2: Parallel operations */
    parallel_memory_operations();
    
    /* Phase 3: Direct builtin calls with volatile control */
    {
        volatile char final_buffer[2048];
        volatile int* int_ptr = (volatile int*)final_buffer;
        volatile size_t operations = 8;
        
        for (size_t i = 0; i < operations; i++) {
            size_t offset = (i * 256) % 1536;
            size_t length = 64 + (i * 32) % 192;
            
            switch (i % 3) {
                case 0:
                    __builtin_memset(final_buffer + offset, i, length);
                    break;
                case 1:
                    __builtin_memcpy(final_buffer + offset + 128, 
                                   final_buffer + offset, length);
                    break;
                case 2:
                    __builtin_memmove(final_buffer + offset + 256, 
                                    final_buffer + offset + 64, length);
                    break;
            }
        }
        
        /* Compute final result */
        for (size_t i = 0; i < 512; i++) {
            result ^= int_ptr[i % 256];
            result = (result * 1103515245 + 12345) & 0x7fffffff;
        }
    }
    
    printf("Result: 0x%08X\n", result);
    printf("Test completed.\n");
    
    return 0;
}
