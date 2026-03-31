#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Complex AST-like structure for recursive operations */
typedef struct ASTNode {
    int type;
    int value;
    volatile int volatile_marker;  /* Prevent optimization */
    struct ASTNode *left;
    struct ASTNode *right;
    char data[64];
} ASTNode;

/* Global token array for initialization */
static volatile int global_token_array[256];

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_constructor(void) {
    volatile int i;
    /* Use builtins in constructor */
    __builtin_memset(global_token_array, 0xAA, sizeof(global_token_array));
    
    /* Force multiple builtin calls */
    for (i = 0; i < 16; i++) {
        volatile int temp[4];
        __builtin_memcpy(temp, &global_token_array[i*4], sizeof(temp));
        __builtin_memset(&global_token_array[i*4], i, sizeof(temp));
    }
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_destructor(void) {
    volatile int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += global_token_array[i];
    }
    /* Use builtin in destructor */
    __builtin_memset(global_token_array, 0, sizeof(global_token_array));
}

/* Recursive AST creation with memory operations */
static ASTNode* create_ast(int depth, int value) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    node->value = value;
    node->volatile_marker = depth * 1000 + value;
    
    /* Fill data with pattern */
    volatile int pattern = value * 31;
    __builtin_memset(node->data, pattern & 0xFF, sizeof(node->data));
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_goto = (value % 3 == 0);
        
        if (use_goto) {
            goto create_left;
        }
        
        node->left = create_ast(depth - 1, value * 2);
        
        create_left:
        node->right = create_ast(depth - 1, value * 2 + 1);
        
        /* Copy between nodes using builtin */
        if (node->left && node->right) {
            volatile size_t copy_size = sizeof(node->data) / 2;
            __builtin_memcpy(node->left->data + 32, 
                           node->right->data, 
                           copy_size);
        }
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void process_with_goto(ASTNode* node, int iterations) {
    volatile char buffer[128];
    volatile int use_memmove = 0;
    int i = 0;
    
    start_loop:
    if (i >= iterations) goto end_processing;
    
    /* Fill buffer with pattern */
    __builtin_memset(buffer, i, sizeof(buffer));
    
    /* Conditional goto around memmove */
    if (i % 2 == 0) {
        goto skip_memmove;
    }
    
    /* This memmove should be intercepted */
    use_memmove = 1;
    __builtin_memmove(buffer + 32, buffer, 64);
    
    skip_memmove:
    /* Copy to node data */
    if (node) {
        volatile size_t copy_len = sizeof(buffer) < sizeof(node->data) ? 
                                 sizeof(buffer) : sizeof(node->data);
        __builtin_memcpy(node->data, buffer, copy_len);
    }
    
    i++;
    goto start_loop;
    
    end_processing:
    /* Final memory operation */
    if (use_memmove) {
        __builtin_memset(buffer, 0xFF, sizeof(buffer));
    }
}

/* OpenMP parallel section with memory operations */
static long long parallel_memory_ops(ASTNode** nodes, int count) {
    volatile long long total_sum = 0;
    volatile int sync_buffer[1024];
    
    /* Initialize sync buffer */
    __builtin_memset(sync_buffer, 0, sizeof(sync_buffer));
    
    #pragma omp parallel reduction(+:total_sum)
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        
        /* Thread-local operations with builtins */
        volatile char thread_buffer[256];
        __builtin_memset(thread_buffer, thread_id, sizeof(thread_buffer));
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Mix of memory operations */
                if (i % 3 == 0) {
                    __builtin_memcpy(nodes[i]->data, thread_buffer, 
                                   sizeof(nodes[i]->data) < sizeof(thread_buffer) ?
                                   sizeof(nodes[i]->data) : sizeof(thread_buffer));
                } else if (i % 3 == 1) {
                    __builtin_memset(nodes[i]->data, i, sizeof(nodes[i]->data));
                } else {
                    /* Self-overlapping copy */
                    __builtin_memmove(nodes[i]->data + 16, nodes[i]->data, 32);
                }
                
                /* Calculate sum from node */
                for (int j = 0; j < 64; j++) {
                    total_sum += nodes[i]->data[j];
                }
            }
        }
        
        /* Thread synchronization with memory barrier */
        #pragma omp barrier
        
        /* Update global sync buffer */
        __builtin_memset(&sync_buffer[thread_id * 16], thread_id, 16 * sizeof(int));
    }
    
    /* Verify sync buffer */
    volatile int verify_sum = 0;
    for (int i = 0; i < 1024; i++) {
        verify_sum += sync_buffer[i];
    }
    total_sum += verify_sum;
    
    return total_sum;
}

/* Main execution flow */
int main(void) {
    const int NUM_NODES = 8;
    const int AST_DEPTH = 4;
    ASTNode* nodes[NUM_NODES];
    long long final_hash = 0;
    
    /* Phase 1: Initialize AST nodes */
    for (int i = 0; i < NUM_NODES; i++) {
        nodes[i] = create_ast(AST_DEPTH, i * 100);
        if (!nodes[i]) {
            fprintf(stderr, "Failed to create AST node %d\n", i);
            return 1;
        }
    }
    
    /* Phase 2: Process with goto control flow */
    for (int i = 0; i < NUM_NODES; i += 2) {
        process_with_goto(nodes[i], 5);
    }
    
    /* Phase 3: Parallel memory operations */
    final_hash = parallel_memory_ops(nodes, NUM_NODES);
    
    /* Phase 4: Final verification with mixed builtins */
    volatile char final_buffer[512];
    volatile int temp_sum = 0;
    
    /* Chain of memory operations */
    __builtin_memset(final_buffer, 0xCC, sizeof(final_buffer));
    __builtin_memcpy(final_buffer + 128, final_buffer, 256);
    __builtin_memmove(final_buffer + 256, final_buffer + 128, 128);
    __builtin_memset(final_buffer + 384, final_hash & 0xFF, 128);
    
    /* Calculate final verification */
    for (int i = 0; i < sizeof(final_buffer); i++) {
        temp_sum += final_buffer[i];
    }
    final_hash ^= (long long)temp_sum << 32;
    
    /* Cleanup */
    for (int i = 0; i < NUM_NODES; i++) {
        if (nodes[i]) {
            free(nodes[i]);
        }
    }
    
    /* Print result for verification */
    printf("Final hash: %lld\n", final_hash);
    
    return 0;
}
