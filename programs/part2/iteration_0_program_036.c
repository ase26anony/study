/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 256;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 192;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_token_array[1024];

/* Constructor attribute for early initialization */
__attribute__((constructor))
static void init_constructor(void) {
    /* Force early initialization of ASAN runtime */
    volatile char dummy[16];
    __builtin_memset(dummy, 0, sizeof(dummy));
    printf("Constructor: ASAN runtime should be initialized\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_destructor(void) {
    printf("Destructor: Program completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    node->left = NULL;
    node->right = NULL;
    
    /* Use builtins with volatile lengths */
    __builtin_memset(node->data, node->id % 256, sizeof(node->data));
    
    if (depth > 1) {
        node->left = create_ast(depth - 1, counter);
        node->right = create_ast(depth - 1, counter);
        
        /* Copy between nodes using builtin */
        if (node->left && node->right) {
            __builtin_memcpy(node->right->data, node->left->data, 
                           g_memcpy_len % sizeof(node->data));
        }
    }
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(ASTNode* nodes[], int count) {
    int i = 0;
    
    /* Jump into memory operation block */
    goto entry_point;
    
memory_operation:
    /* This block contains builtin_memmove with goto control flow */
    if (i < count - 1) {
        __builtin_memmove(nodes[i]->data, nodes[i+1]->data, 
                         g_memmove_len % sizeof(nodes[0]->data));
    }
    goto next_node;
    
entry_point:
    while (i < count) {
        if (i % 3 == 0) {
            goto memory_operation;
        }
next_node:
        i++;
    }
}

/* OpenMP parallel section */
static void parallel_memory_operations(ASTNode* nodes[], int count) {
    int sum = 0;
    
    #pragma omp parallel reduction(+:sum)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread performs different builtin operations */
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (tid % 3 == 0) {
                /* Thread 0,3,6,... use memcpy */
                __builtin_memcpy(nodes[i]->data, g_token_array, 
                               g_memcpy_len % sizeof(nodes[i]->data));
            } else if (tid % 3 == 1) {
                /* Thread 1,4,7,... use memset */
                __builtin_memset(nodes[i]->data, tid, 
                               g_memset_len % sizeof(nodes[i]->data));
            } else {
                /* Thread 2,5,8,... use memmove with overlap */
                size_t len = g_memmove_len % sizeof(nodes[i]->data);
                if (len > 32) {
                    __builtin_memmove(nodes[i]->data + 16, nodes[i]->data, len - 16);
                }
            }
            
            /* Compute checksum */
            for (size_t j = 0; j < sizeof(nodes[i]->data); j++) {
                sum += nodes[i]->data[j];
            }
        }
    }
    
    printf("Parallel checksum: %d\n", sum);
}

/* Main execution flow */
int main(void) {
    const int NUM_NODES = 8;
    const int AST_DEPTH = 4;
    ASTNode* nodes[NUM_NODES];
    int counter = 1;
    
    /* Initialize token array with pattern */
    for (size_t i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)(i % 256);
    }
    
    /* Create AST nodes */
    for (int i = 0; i < NUM_NODES; i++) {
        nodes[i] = create_ast(AST_DEPTH, &counter);
        if (!nodes[i]) {
            fprintf(stderr, "Failed to create AST node\n");
            return 1;
        }
    }
    
    /* Test builtins in main control flow */
    printf("Testing builtin memory functions...\n");
    
    /* Direct builtin calls */
    __builtin_memcpy(nodes[0]->data, nodes[1]->data, 
                    g_memcpy_len % sizeof(nodes[0]->data));
    
    __builtin_memset(nodes[2]->data, 0xAB, 
                    g_memset_len % sizeof(nodes[2]->data));
    
    /* Overlapping memmove */
    size_t move_len = g_memmove_len % sizeof(nodes[3]->data);
    if (move_len > 48) {
        __builtin_memmove(nodes[3]->data + 24, nodes[3]->data, move_len - 24);
    }
    
    /* Process with goto edge cases */
    process_with_goto(nodes, NUM_NODES);
    
    /* Execute parallel operations */
    parallel_memory_operations(nodes, NUM_NODES);
    
    /* Final verification hash */
    unsigned long final_hash = 0;
    for (int i = 0; i < NUM_NODES; i++) {
        for (size_t j = 0; j < sizeof(nodes[i]->data); j++) {
            final_hash = final_hash * 31 + nodes[i]->data[j];
        }
        free(nodes[i]);
    }
    
    printf("Final hash: %lu\n", final_hash);
    printf("Program completed successfully\n");
    
    return 0;
}
