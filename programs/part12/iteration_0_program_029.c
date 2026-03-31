/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

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
    /* Force early initialization of ASAN runtime */
    volatile char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    /* Final memory operation to ensure all paths are taken */
    volatile char final_buf[16];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtin memset */
    __builtin_memset(node->data, id % 256, sizeof(node->data));
    
    /* Copy data between different parts of the structure */
    if (depth > 1) {
        char temp[32];
        __builtin_memcpy(temp, node->data, 32);
        __builtin_memcpy(node->data + 32, temp, 32);
    }
    
    node->id = id;
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int use_copy = 1;
    
    if (src->id % 3 == 0) {
        goto skip_memmove;
    }
    
    /* This memmove should be intercepted */
    if (g_use_memmove) {
        __builtin_memmove(dst->data, src->data, sizeof(src->data));
        use_copy = 0;
    }
    
skip_memmove:
    if (use_copy) {
        /* Fallback to memcpy */
        __builtin_memcpy(dst->data, src->data, sizeof(src->data));
    }
    
    /* Jump back for overlap case */
    if (dst->id % 5 == 0) {
        goto do_overlap;
    }
    
    return;
    
do_overlap:
    /* Overlapping memory operation */
    __builtin_memmove(dst->data + 16, dst->data, 48);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Each thread uses different builtins */
                if (tid % 3 == 0) {
                    __builtin_memset(nodes[i]->data, tid, g_mem_size % 64);
                } else if (tid % 3 == 1) {
                    if (i > 0) {
                        __builtin_memcpy(nodes[i]->data, nodes[i-1]->data, 32);
                    }
                } else {
                    char temp[64];
                    __builtin_memcpy(temp, nodes[i]->data, 32);
                    __builtin_memmove(nodes[i]->data + 16, temp, 32);
                }
            }
        }
    }
}

/* Main execution flow */
int main(void) {
    const int NUM_NODES = 8;
    const int AST_DEPTH = 3;
    
    /* Create array of AST nodes */
    ASTNode* nodes[NUM_NODES];
    
    /* Initialize nodes with recursive creation */
    for (int i = 0; i < NUM_NODES; i++) {
        nodes[i] = create_ast(AST_DEPTH, i + 100);
        if (!nodes[i]) {
            fprintf(stderr, "Failed to create node %d\n", i);
            return 1;
        }
    }
    
    /* Force initialization of asan_memfn_rtls cache */
    volatile char init_buf[128];
    
    /* Call all three builtins to ensure cache initialization */
    __builtin_memset(init_buf, 0xAA, sizeof(init_buf));
    __builtin_memcpy(init_buf + 64, init_buf, 64);
    __builtin_memmove(init_buf + 32, init_buf, 64);
    
    /* Test goto flow control */
    for (int i = 0; i < NUM_NODES - 1; i++) {
        process_with_goto(nodes[i], nodes[i + 1]);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(nodes, NUM_NODES);
    
    /* Compute verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < NUM_NODES; i++) {
        if (nodes[i]) {
            for (size_t j = 0; j < sizeof(nodes[i]->data); j++) {
                hash = (hash * 31 + nodes[i]->data[j]) % 1000000007;
            }
            /* Recursive cleanup */
            free(nodes[i]->left);
            free(nodes[i]->right);
            free(nodes[i]);
        }
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Program completed successfully\n");
    
    return 0;
}
