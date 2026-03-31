/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[256];
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor)) static void init_asan_early(void) {
    volatile char buffer[128];
    /* Force memcpy redirection during static initialization */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 64, buffer, 64);
}

/* Destructor for cleanup verification */
__attribute__((destructor)) static void cleanup_asan(void) {
    volatile char final_check[32];
    __builtin_memset(final_check, 0xFF, 32);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile control */
    size_t len = volatile_len / (depth + 1);
    __builtin_memset(node->data, depth, len);
    
    /* Create pattern with memcpy */
    if (depth > 1) {
        __builtin_memcpy(node->data + len, node->data, len / 2);
    }
    
    node->type = depth;
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 2);
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(ASTNode *a, ASTNode *b) {
    int state = 0;
    
    /* Jump into memory operation block */
    if (volatile_flag) goto mem_block;
    
    normal_path:
        __builtin_memset(a->data, 0x11, 32);
        return;
    
    mem_block:
        /* This tests flow sensitivity */
        __builtin_memmove(a->data, b->data, 48);
        
        if (state++ < 2) {
            /* Jump out and back in */
            goto normal_path;
        }
}

/* OpenMP parallel memory operations */
static void parallel_mem_ops(ASTNode **nodes, int count) {
    #pragma omp parallel for
    for (int i = 0; i < count; i++) {
        volatile int tid = omp_get_thread_num();
        
        /* Each thread uses builtins */
        __builtin_memset(nodes[i]->data, tid, 64);
        
        if (i > 0) {
            /* Inter-node copying */
            __builtin_memcpy(nodes[i]->data + 64, 
                           nodes[i-1]->data, 
                           32);
        }
        
        /* Conditional memmove */
        if (tid % 2 == 0) {
            __builtin_memmove(nodes[i]->data + 32,
                            nodes[i]->data,
                            16);
        }
    }
}

/* Main execution flow */
int main(void) {
    const int NODES = 8;
    ASTNode *nodes[NODES];
    unsigned long hash = 0;
    
    /* Initialize AST structures */
    for (int i = 0; i < NODES; i++) {
        nodes[i] = create_ast(4);
        if (!nodes[i]) {
            fprintf(stderr, "Failed to create AST node\n");
            return 1;
        }
    }
    
    /* Test goto edge cases */
    process_with_goto(nodes[0], nodes[1]);
    
    /* Execute parallel memory operations */
    parallel_mem_ops(nodes, NODES);
    
    /* Additional builtin calls in main flow */
    char temp_buffer[256];
    __builtin_memset(temp_buffer, 0xCC, sizeof(temp_buffer));
    __builtin_memcpy(temp_buffer + 128, temp_buffer, 128);
    __builtin_memmove(temp_buffer + 64, temp_buffer, 192);
    
    /* Compute verification hash */
    for (int i = 0; i < NODES; i++) {
        for (int j = 0; j < 256; j++) {
            hash = (hash * 31 + nodes[i]->data[j]) % 1000000007;
        }
        
        /* Cleanup */
        free(nodes[i]->left);
        free(nodes[i]->right);
        free(nodes[i]);
    }
    
    printf("Verification hash: %lu\n", hash);
    return 0;
}
