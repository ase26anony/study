/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 256;
volatile int use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buf[32];
    /* Force builtin initialization in constructor */
    __builtin_memset(buf, 0xAA, sizeof(buf));
    __builtin_memcpy(buf + 16, buf, 8);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    volatile char final_buf[16];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive tree manipulation with memory operations */
ASTNode* create_tree(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Initialize node data with builtins */
    __builtin_memset(node->data, node->id % 256, sizeof(node->data));
    
    /* Create pattern in first half */
    char pattern[32];
    __builtin_memset(pattern, 0xCC, sizeof(pattern));
    __builtin_memcpy(node->data, pattern, 16);
    
    /* Recursive creation */
    node->left = create_tree(depth - 1, counter);
    node->right = create_tree(depth - 1, counter);
    
    return node;
}

/* Complex memory operation with goto flow control */
void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int stage = 0;
    
    /* Jump into memory operation block */
    goto entry_point;
    
memory_operation:
    /* This block contains the critical builtin calls */
    if (use_memmove) {
        /* Force memmove redirection */
        __builtin_memmove(dst->data, src->data, 
                         volatile_len % sizeof(src->data));
    } else {
        /* Use memcpy */
        __builtin_memcpy(dst->data, src->data, 
                        volatile_len % sizeof(src->data));
    }
    stage = 1;
    goto exit_point;
    
entry_point:
    /* Prepare data before jump */
    __builtin_memset(src->data + 32, 0xDD, 16);
    goto memory_operation;
    
exit_point:
    /* Verify operation */
    __builtin_memset(dst->data + 48, stage, 8);
}

/* OpenMP parallel memory dispatch */
void parallel_memory_ops(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count - 1; i++) {
            volatile size_t len = (volatile_len + tid) % 64;
            
            /* Mix different builtins in parallel */
            switch (i % 3) {
                case 0:
                    __builtin_memcpy(nodes[i+1]->data, 
                                    nodes[i]->data, len);
                    break;
                case 1:
                    __builtin_memset(nodes[i]->data + 16, 
                                    tid, len);
                    break;
                case 2:
                    __builtin_memmove(nodes[i]->data, 
                                     nodes[i+1]->data, len);
                    break;
            }
        }
        
        /* Thread-private memory operation */
        char private_buf[128];
        __builtin_memset(private_buf, tid, sizeof(private_buf));
        __builtin_memcpy(private_buf + 64, private_buf, 32);
    }
}

/* Main execution flow */
int main(void) {
    int counter = 1;
    
    /* Create complex tree structure */
    ASTNode* root = create_tree(4, &counter);
    ASTNode* copy = create_tree(4, &counter);
    
    if (!root || !copy) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize token array */
    char tokens[8][256];
    for (int i = 0; i < 8; i++) {
        __builtin_memset(tokens[i], i * 32, sizeof(tokens[i]));
    }
    
    /* Process with goto flow control */
    process_with_goto(root, copy);
    
    /* Toggle memmove usage */
    use_memmove = 0;
    process_with_goto(copy, root);
    
    /* Prepare nodes for parallel processing */
    ASTNode* node_array[6];
    node_array[0] = root;
    node_array[1] = copy;
    node_array[2] = create_tree(2, &counter);
    node_array[3] = create_tree(2, &counter);
    node_array[4] = create_tree(2, &counter);
    node_array[5] = create_tree(2, &counter);
    
    /* Execute parallel memory operations */
    parallel_memory_ops(node_array, 6);
    
    /* Compute verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < 6 && node_array[i]; i++) {
        for (size_t j = 0; j < sizeof(node_array[i]->data); j++) {
            hash = (hash * 31 + node_array[i]->data[j]) % 1000000007;
        }
    }
    
    /* Final memory rearrangement */
    for (int i = 0; i < 8; i += 2) {
        __builtin_memmove(tokens[i], tokens[i+1], 
                         volatile_len % sizeof(tokens[i]));
    }
    
    /* Add tokens to hash */
    for (int i = 0; i < 8; i++) {
        for (size_t j = 0; j < sizeof(tokens[i]); j += 8) {
            hash = (hash * 17 + tokens[i][j]) % 1000000007;
        }
    }
    
    printf("Verification hash: %lu\n", hash);
    
    /* Cleanup */
    for (int i = 2; i < 6; i++) {
        free(node_array[i]);
    }
    free(root);
    free(copy);
    
    return 0;
}
