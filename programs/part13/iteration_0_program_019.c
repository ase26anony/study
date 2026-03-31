/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[32];
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *next;
} ASTNode;

/* Global token array */
static char token_array[1024];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token array with pattern */
    for (int i = 0; i < sizeof(token_array); i++) {
        token_array[i] = (char)((i * 13) % 256);
    }
    
    /* Use builtins in constructor to trigger early initialization */
    __builtin_memset(token_array, 0xAA, 128);
    __builtin_memcpy(volatile_dest, token_array, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Final builtin usage in destructor */
    __builtin_memset(volatile_dest, 0, sizeof(volatile_dest));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    
    /* Copy data using builtin */
    char temp[32];
    __builtin_memset(temp, 'A' + depth, sizeof(temp));
    __builtin_memcpy(node->data, temp, sizeof(node->data));
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 2 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast_node(depth - 1);
        node->right = create_ast_node(depth - 2);
        
        create_children:
        /* Jump target with memmove operation */
        if (node->left && node->right) {
            /* Move data between nodes */
            __builtin_memmove(node->right->data, node->left->data, 16);
        }
    }
    
    return node;
}

/* Function with goto jumping into/out of memory blocks */
static void goto_memory_operations(void) {
    char buffer1[128];
    char buffer2[128];
    int i = 0;
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 0xCC, sizeof(buffer1));
    __builtin_memset(buffer2, 0xDD, sizeof(buffer2));
    
    /* Jump into memory operation block */
    goto jump_in;
    
    normal_path:
    /* Normal path with memcpy */
    __builtin_memcpy(buffer1, buffer2, volatile_len);
    goto after_block;
    
    jump_in:
    /* Jump target - inside memory operation */
    __builtin_memmove(buffer2, buffer1, 32);
    
    /* Conditional jump out */
    if (i++ < 3) {
        goto normal_path;
    }
    
    after_block:
    /* Final operation */
    __builtin_memset(buffer1, 0, 64);
}

/* Parallel memory dispatch logic */
static void parallel_memory_dispatch(void) {
    int results[8] = {0};
    char work_buffer[8][256];
    
    #pragma omp parallel for
    for (int i = 0; i < 8; i++) {
        /* Each thread uses builtins independently */
        __builtin_memset(work_buffer[i], i, sizeof(work_buffer[i]));
        
        /* Copy between buffers with volatile length */
        int len = volatile_len + i;
        if (i > 0) {
            __builtin_memcpy(work_buffer[i], work_buffer[i-1], len % 256);
        }
        
        /* Compute simple hash */
        int hash = 0;
        for (int j = 0; j < 64; j++) {
            hash += work_buffer[i][j];
        }
        results[i] = hash;
    }
    
    /* Consolidate results with memmove */
    int final_result = 0;
    for (int i = 0; i < 8; i++) {
        final_result ^= results[i];
    }
    printf("Parallel hash result: %d\n", final_result);
}

/* Complex memory operation sequence */
static void complex_memory_sequence(void) {
    char seq_buffer[512];
    char temp_buffer[256];
    
    /* Sequence 1: memset -> memcpy -> memmove chain */
    __builtin_memset(seq_buffer, 0x11, sizeof(seq_buffer));
    __builtin_memcpy(temp_buffer, seq_buffer, 128);
    __builtin_memmove(seq_buffer + 128, temp_buffer, 128);
    
    /* Sequence 2: Overlapping operations */
    __builtin_memmove(seq_buffer, seq_buffer + 64, 192);
    
    /* Sequence 3: Volatile-controlled operations */
    for (int i = 0; i < 4; i++) {
        int offset = i * 64;
        __builtin_memcpy(seq_buffer + offset, token_array + offset, 
                        volatile_len + i * 8);
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN builtin redirection test...\n");
    
    /* 1. Initialize complex structures */
    ASTNode *root = create_ast_node(4);
    
    /* 2. Execute goto-based memory operations */
    goto_memory_operations();
    
    /* 3. Run parallel memory dispatch */
    parallel_memory_dispatch();
    
    /* 4. Execute complex memory sequences */
    complex_memory_sequence();
    
    /* 5. Process AST with memory operations */
    if (root) {
        ASTNode *current = root;
        int node_count = 0;
        
        while (current) {
            /* Copy node data to volatile buffer */
            __builtin_memcpy(volatile_dest, current->data, 
                           sizeof(current->data));
            
            /* Move to next node */
            if (current->left) {
                __builtin_memmove(current->data, current->left->data, 16);
            }
            
            current = current->next;
            node_count++;
            
            /* Limit traversal */
            if (node_count > 10) break;
        }
        
        printf("Processed %d AST nodes\n", node_count);
        
        /* Cleanup */
        while (root) {
            ASTNode *next = root->next;
            free(root);
            root = next;
        }
    }
    
    /* 6. Final verification with all three builtins */
    char final_buffer[1024];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, token_array, sizeof(token_array));
    __builtin_memmove(final_buffer + 512, final_buffer, 512);
    
    /* Compute final checksum */
    unsigned int checksum = 0;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        checksum += final_buffer[i];
    }
    
    printf("Final checksum: %u\n", checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}
