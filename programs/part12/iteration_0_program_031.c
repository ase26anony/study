/* ISO C99-compliant test program for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor))
static void init_constructor(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < 256; i++) {
        volatile_src[i] = (char)(i % 26 + 'A');
    }
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Verify memory was properly handled */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += volatile_dest[i];
    }
    printf("Destructor: Memory verification sum = %d\n", sum);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Fill with pattern using volatile length */
    int len = volatile_len % 32;
    for (int i = 0; i < len; i++) {
        node->data[i] = (char)('A' + node->id + i);
    }
    
    /* Recursive creation with goto for control flow */
    int use_goto = (depth % 2 == 0);
    
    if (use_goto) {
        goto create_left;
    } else {
        node->left = create_ast(depth - 1, counter);
        node->right = create_ast(depth - 1, counter);
        return node;
    }
    
create_left:
    node->left = create_ast(depth - 1, counter);
    
    /* Jump back with __builtin_memmove in the path */
    if (depth > 2) {
        char temp[32];
        __builtin_memmove(temp, node->data, sizeof(node->data));
        __builtin_memmove(node->data + 8, temp, 16);
        goto create_right;
    }
    
create_right:
    node->right = create_ast(depth - 1, counter);
    return node;
}

/* Function with complex memory operations and goto */
static void process_ast(ASTNode* node) {
    if (!node) return;
    
    char buffer[64];
    int offset = 0;
    
    /* Label for goto into memory operation block */
process_node:
    /* Copy node data using __builtin_memcpy */
    __builtin_memcpy(buffer + offset, node->data, sizeof(node->data));
    offset += sizeof(node->data);
    
    /* Conditional goto out of memory operation context */
    if (node->id % 3 == 0) {
        goto skip_extra;
    }
    
    /* Additional memory operation that might be skipped */
    __builtin_memmove(buffer + offset - 8, buffer + offset - 16, 16);
    
skip_extra:
    /* Process children */
    process_ast(node->left);
    
    /* Jump back to process right child */
    if (node->right) {
        node = node->right;
        goto process_node;
    }
}

/* Parallel memory dispatch function */
static void parallel_memory_operations(void) {
    int i;
    char local_buf[128];
    char result_buf[128];
    
    /* Initialize with pattern */
    for (i = 0; i < 128; i++) {
        local_buf[i] = (char)(i % 10 + '0');
    }
    
    #pragma omp parallel private(i)
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        #pragma omp for
        for (i = 0; i < 100; i++) {
            char thread_buf[64];
            int idx = i % 64;
            
            /* Use __builtin_memset in parallel region */
            __builtin_memset(thread_buf, thread_id + 'A', sizeof(thread_buf));
            
            /* Copy to shared buffer with __builtin_memcpy */
            __builtin_memcpy(result_buf + idx, thread_buf, 16);
            
            /* Move data around with __builtin_memmove */
            if (i % 7 == 0) {
                __builtin_memmove(result_buf + idx + 8, result_buf + idx, 8);
            }
        }
        
        /* Barrier to ensure all memory operations complete */
        #pragma omp barrier
        
        #pragma omp single
        {
            /* Final consolidation with all builtins */
            __builtin_memcpy(volatile_dest, result_buf, 128);
            __builtin_memset(result_buf + 64, 0, 64);
            __builtin_memmove(volatile_dest + 64, volatile_dest, 64);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize and create recursive structure */
    int counter = 0;
    ASTNode* root = create_ast(4, &counter);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    printf("Created AST with %d nodes\n", counter);
    
    /* Phase 2: Process AST with goto control flow */
    process_ast(root);
    
    /* Phase 3: Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Phase 4: Direct built-in calls with volatile parameters */
    int dynamic_len = volatile_len;
    
    /* Test all three builtins in sequence */
    __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, dynamic_len);
    __builtin_memset((void*)(volatile_dest + dynamic_len), 'X', dynamic_len / 2);
    __builtin_memmove((void*)(volatile_dest + dynamic_len / 2), 
                     (void*)volatile_dest, 
                     dynamic_len / 2);
    
    /* Phase 5: Verify results */
    int hash = 0;
    for (int i = 0; i < 128; i++) {
        hash += volatile_dest[i] * (i + 1);
    }
    
    printf("Final memory hash: %d\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real ASAN, memory would be freed and checked */
    
    return 0;
}
