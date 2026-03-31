/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char *volatile_dest = NULL;
static volatile char *volatile_src = NULL;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[32];
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Global token array */
static char token_array[256];

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token array with pattern */
    for (int i = 0; i < 256; i++) {
        token_array[i] = (char)((i * 7) & 0xFF);
    }
    
    /* Force early built-in usage in constructor */
    char local_buf[32];
    __builtin_memset(local_buf, 0xAA, sizeof(local_buf));
    __builtin_memcpy(&token_array[0], local_buf, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Final built-in usage in destructor */
    char final_buf[64];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static int process_ast(ASTNode *node, int depth) {
    if (node == NULL || depth > 5) return 0;
    
    int sum = node->type;
    
    /* Copy data between nodes using built-ins */
    if (node->left && node->right) {
        /* Use goto to create control flow edge case */
        if (depth % 2 == 0) {
            goto copy_block;
        }
        
        /* Normal path */
        __builtin_memcpy(node->right->data, node->left->data, 16);
        return sum;
        
    copy_block:
        /* Jumped-to block with memmove */
        __builtin_memmove(node->data, node->left->data, 16);
    }
    
    /* Recursive processing */
    sum += process_ast(node->left, depth + 1);
    sum += process_ast(node->right, depth + 1);
    
    return sum;
}

/* Function with complex memory operations */
static void memory_operations(void) {
    char buffer1[128];
    char buffer2[128];
    char buffer3[128];
    
    /* Varied built-in usage patterns */
    
    /* Pattern 1: Direct built-in calls */
    __builtin_memset(buffer1, 0x11, volatile_len % 64);
    __builtin_memcpy(buffer2, buffer1, 32);
    
    /* Pattern 2: Chained operations */
    __builtin_memset(buffer3, 0x22, 48);
    __builtin_memmove(buffer2 + 16, buffer3, 32);
    __builtin_memcpy(buffer1 + 32, buffer2, 16);
    
    /* Pattern 3: Overlapping regions (stress test) */
    __builtin_memmove(buffer1 + 16, buffer1, 48);
    
    /* Use volatile pointers */
    volatile_dest = buffer1;
    volatile_src = buffer2;
    __builtin_memcpy((char*)volatile_dest, (char*)volatile_src, 16);
}

/* OpenMP parallel section */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char local_buf[64];
        char shared_buf[256];
        
        /* Each thread uses built-ins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp critical
        {
            /* Copy to shared buffer with offset */
            __builtin_memcpy(shared_buf + (thread_id * 32), 
                           local_buf, 32);
            
            /* Move data within shared buffer */
            __builtin_memmove(shared_buf, 
                            shared_buf + (thread_id * 16), 16);
        }
    }
}

/* Function with goto jumping in/out of memory blocks */
static int goto_memory_test(void) {
    char data[4][32];
    int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 4; i++) {
        __builtin_memset(data[i], i + 1, 32);
    }
    
    /* Complex goto pattern */
    if (volatile_len > 32) {
        goto block_a;
    }
    
block_b:
    __builtin_memcpy(data[2], data[0], 16);
    goto block_c;
    
block_a:
    __builtin_memmove(data[1], data[3], 16);
    
    if (volatile_len < 100) {
        goto block_b;
    }
    
    __builtin_memset(data[0], 0x99, 16);
    
block_c:
    /* Calculate checksum */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 16; j++) {
            result += data[i][j];
        }
    }
    
    return result;
}

/* Main test driver */
int main(void) {
    int total_sum = 0;
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Basic memory operations */
    memory_operations();
    
    /* Phase 2: Create and process AST */
    ASTNode *root = malloc(sizeof(ASTNode));
    ASTNode *left = malloc(sizeof(ASTNode));
    ASTNode *right = malloc(sizeof(ASTNode));
    
    if (root && left && right) {
        __builtin_memset(root, 0, sizeof(ASTNode));
        __builtin_memset(left, 0, sizeof(ASTNode));
        __builtin_memset(right, 0, sizeof(ASTNode));
        
        root->type = 1;
        left->type = 2;
        right->type = 3;
        
        __builtin_memcpy(root->data, "RootNodeData", 13);
        __builtin_memcpy(left->data, "LeftChildData", 14);
        __builtin_memcpy(right->data, "RightChildData", 15);
        
        root->left = left;
        root->right = right;
        
        total_sum += process_ast(root, 0);
        
        /* Copy between AST nodes */
        __builtin_memmove(left->data, right->data, 15);
        __builtin_memcpy(right->data, root->data, 13);
    }
    
    /* Phase 3: Parallel operations */
    parallel_memory_ops();
    
    /* Phase 4: Goto edge cases */
    total_sum += goto_memory_test();
    
    /* Phase 5: Final built-in calls with volatile */
    char final_buffer[128];
    volatile_len = 64;
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, token_array, 
                    volatile_len > 128 ? 128 : volatile_len);
    __builtin_memmove(final_buffer + 32, final_buffer, 64);
    
    /* Calculate final hash */
    for (int i = 0; i < 64; i++) {
        total_sum += final_buffer[i];
    }
    
    /* Cleanup */
    free(root);
    free(left);
    free(right);
    
    printf("Test completed. Result checksum: %d\n", total_sum);
    printf("Expected range: 8000-12000 (implementation dependent)\n");
    
    return total_sum > 0 ? 0 : 1;
}
