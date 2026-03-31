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

/* Global volatile variables to prevent constant folding */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 96;

/* Token array for parser simulation */
static const char* tokens[] = {"ADD", "SUB", "MUL", "DIV", "NUM", "VAR", "END"};
volatile int token_idx = 0;

/* Constructor function to force early initialization */
__attribute__((constructor))
static void init_asan_early(void) {
    /* Force initialization of ASAN memfn cache early */
    char buffer1[256];
    char buffer2[256];
    
    /* Use all three builtins in constructor */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1, buffer2, sizeof(buffer1));
    
    printf("Constructor: ASAN initialization forced\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    printf("Destructor: Program cleanup\n");
}

/* Recursive parser with goto for flow control */
static ASTNode* parse_expression(int depth) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with volatile sizes */
    node->size = (depth % 3 == 0) ? g_memcpy_len : 
                 (depth % 3 == 1) ? g_memset_len : g_memmove_len;
    
    if (depth > 0) {
        /* Use goto to create complex control flow */
        if (depth % 2 == 0) {
            goto build_left;
        } else {
            goto build_right;
        }
        
    build_left:
        node->left = parse_expression(depth - 1);
        node->right = NULL;
        goto node_complete;
        
    build_right:
        node->left = NULL;
        node->right = parse_expression(depth - 1);
        goto node_complete;
    } else {
        node->left = node->right = NULL;
    }
    
node_complete:
    /* Use builtins with volatile control */
    __builtin_memset(&node->value, token_idx, sizeof(node->value));
    
    /* Copy padding between nodes if siblings exist */
    if (node->left && node->right) {
        __builtin_memcpy(node->left->padding, node->right->padding, 
                        node->size % sizeof(node->padding));
    }
    
    token_idx = (token_idx + 1) % 7;
    return node;
}

/* Function with goto jumping into memmove block */
static void complex_mem_operations(char* dest, char* src, size_t len) {
    int use_memmove = 0;
    
    /* Jump into memmove block */
    if (len > 50) {
        goto do_memmove;
    }
    
    /* Regular memcpy path */
    __builtin_memcpy(dest, src, len);
    return;
    
do_memmove:
    /* This block should be reachable via goto */
    use_memmove = 1;
    char temp[256];
    
    /* Jump out to different operation */
    if (len > 100) {
        goto do_memset;
    }
    
    __builtin_memmove(temp, src, len);
    __builtin_memcpy(dest, temp, len);
    return;
    
do_memset:
    __builtin_memset(dest, 0xCC, len);
}

/* OpenMP parallel memory operations */
static uint64_t parallel_memory_dispatch(ASTNode** nodes, int count) {
    uint64_t hash = 0;
    
    #pragma omp parallel for reduction(+:hash)
    for (int i = 0; i < count; i++) {
        if (nodes[i]) {
            char buffer[256];
            volatile size_t op_len = nodes[i]->size % sizeof(buffer);
            
            /* Force all three builtins in parallel region */
            __builtin_memset(buffer, i, op_len);
            
            if (nodes[i]->left) {
                __builtin_memcpy(nodes[i]->left->padding, buffer, 
                                op_len % sizeof(nodes[i]->left->padding));
            }
            
            if (nodes[i]->right && i > 0) {
                /* Create overlapping regions for memmove */
                size_t move_len = op_len / 2;
                __builtin_memmove(nodes[i]->right->padding, 
                                 nodes[i]->padding + 16, 
                                 move_len);
            }
            
            /* Compute hash from node data */
            for (size_t j = 0; j < sizeof(nodes[i]->padding); j++) {
                hash += (uint64_t)nodes[i]->padding[j];
            }
        }
    }
    
    return hash;
}

/* Multi-stage initialization */
static void initialize_data_structures(ASTNode* nodes[], int count) {
    for (int i = 0; i < count; i++) {
        nodes[i] = parse_expression(3);
        
        /* Force memcpy between nodes at different levels */
        if (i > 0 && nodes[i-1]) {
            size_t copy_len = (g_memcpy_len + i) % 64;
            __builtin_memcpy(nodes[i]->padding, 
                           nodes[i-1]->padding, 
                           copy_len);
        }
    }
}

int main(void) {
    const int NODE_COUNT = 8;
    ASTNode* nodes[NODE_COUNT];
    uint64_t final_hash = 0;
    
    printf("Starting ASAN memory operation test...\n");
    
    /* Stage 1: Initialize complex data structures */
    initialize_data_structures(nodes, NODE_COUNT);
    
    /* Stage 2: Perform complex memory operations with goto */
    for (int i = 0; i < NODE_COUNT; i++) {
        if (nodes[i]) {
            complex_mem_operations(nodes[i]->padding + 16,
                                  nodes[i]->padding,
                                  nodes[i]->size % 128);
        }
    }
    
    /* Stage 3: OpenMP parallel operations */
    final_hash = parallel_memory_dispatch(nodes, NODE_COUNT);
    
    /* Stage 4: Final memory rearrangement */
    for (int i = NODE_COUNT - 1; i > 0; i--) {
        if (nodes[i] && nodes[i-1]) {
            size_t move_len = (g_memmove_len + i) % 48;
            __builtin_memmove(nodes[i-1]->padding + 24,
                             nodes[i]->padding,
                             move_len);
        }
    }
    
    /* Verify operations by computing final checksum */
    for (int i = 0; i < NODE_COUNT; i++) {
        if (nodes[i]) {
            for (size_t j = 0; j < sizeof(nodes[i]->padding); j++) {
                final_hash ^= (final_hash << 5) + (uint64_t)nodes[i]->padding[j];
            }
            free(nodes[i]);
        }
    }
    
    printf("Final hash: 0x%016llX\n", (unsigned long long)final_hash);
    printf("Test completed successfully\n");
    
    return final_hash != 0 ? 0 : 1;
}
