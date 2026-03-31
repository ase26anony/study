/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 16;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_pool[1024];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)((i * 31) & 0xFF);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    /* Clear sensitive data */
    __builtin_memset(token_pool, 0, sizeof(token_pool));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for node initialization */
    __builtin_memcpy(node->data, base_data, 
                     volatile_len < 64 ? volatile_len : 64);
    
    node->id = depth;
    node->left = node->right = NULL;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        char child_data[64];
        
        /* Label for goto testing */
        create_left_child:
        __builtin_memcpy(child_data, node->data, 32);
        child_data[0] = 'L';
        node->left = create_ast(depth - 1, child_data);
        
        if (volatile_flag) {
            /* Jump to avoid optimization */
            goto skip_right;
        }
        
        create_right_child:
        __builtin_memcpy(child_data, node->data, 32);
        child_data[0] = 'R';
        node->right = create_ast(depth - 1, child_data);
        
        skip_right:
        /* Use __builtin_memmove for overlapping regions */
        if (node->left && node->right) {
            __builtin_memmove(node->left->data + 16, 
                            node->right->data, 16);
        }
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void process_with_goto(ASTNode* node) {
    if (!node) return;
    
    char buffer[128];
    int use_memset = 0;
    
    /* Jump into memory operation block */
    if (node->id % 2 == 0) {
        goto memcpy_block;
    } else {
        goto memset_block;
    }
    
memcpy_block:
    {
        /* Force memcpy redirection */
        __builtin_memcpy(buffer, node->data, 
                        volatile_len < 64 ? volatile_len : 64);
        
        /* Jump out of block */
        if (node->left) goto process_left;
    }
    
memset_block:
    {
        /* Force memset redirection */
        __builtin_memset(buffer, node->id, 
                        volatile_len < 128 ? volatile_len : 128);
        use_memset = 1;
    }
    
process_left:
    if (node->left) {
        /* Copy between AST nodes */
        __builtin_memcpy(node->left->data, buffer, 32);
    }
    
    /* Jump back for memmove */
    if (node->right && !use_memset) {
        goto memmove_block;
    }
    
    return;
    
memmove_block:
    {
        /* Force memmove redirection with overlap */
        __builtin_memmove(node->data + 8, node->data, 24);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            char local_buf[256];
            
            /* Mixed built-in usage in parallel region */
            __builtin_memset(local_buf, i, 128);
            __builtin_memcpy(local_buf + 128, nodes[i]->data, 64);
            __builtin_memmove(nodes[i]->data, local_buf, 64);
            
            /* Token pool access */
            int idx = (i * 17) % sizeof(token_pool);
            __builtin_memcpy(nodes[i]->data + 32, 
                           token_pool + idx, 32);
        }
    }
}

/* Calculate hash from AST */
static unsigned long ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* p = node->data;
    
    /* Simple hash calculation */
    for (int i = 0; i < 64 && *p; i++) {
        hash = ((hash << 5) + hash) + *p++;
    }
    
    return hash + ast_hash(node->left) + ast_hash(node->right);
}

int main(void) {
    const int NUM_NODES = 8;
    ASTNode* nodes[NUM_NODES];
    unsigned long total_hash = 0;
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create AST nodes with different depths */
    for (int i = 0; i < NUM_NODES; i++) {
        char base_data[64];
        __builtin_memset(base_data, 'A' + i, 64);
        nodes[i] = create_ast(3 + (i % 3), base_data);
    }
    
    /* Process with goto flow control */
    for (int i = 0; i < NUM_NODES; i++) {
        process_with_goto(nodes[i]);
    }
    
    /* Parallel memory operations */
    parallel_memory_ops(nodes, NUM_NODES);
    
    /* Calculate verification hash */
    for (int i = 0; i < NUM_NODES; i++) {
        if (nodes[i]) {
            total_hash ^= ast_hash(nodes[i]);
            
            /* Additional memory operation */
            __builtin_memmove(nodes[i]->data, 
                            nodes[(i + 1) % NUM_NODES]->data, 32);
        }
    }
    
    /* Final built-in calls */
    char final_buffer[512];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, token_pool, 256);
    __builtin_memmove(final_buffer + 256, final_buffer, 128);
    
    printf("Total hash: 0x%08lx\n", total_hash);
    printf("Test completed.\n");
    
    /* Cleanup */
    for (int i = 0; i < NUM_NODES; i++) {
        if (nodes[i]) {
            free(nodes[i]);
        }
    }
    
    return (total_hash != 0) ? 0 : 1;
}
