#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    volatile size_t size;  /* Prevent optimization */
} ASTNode;

/* Global volatile variables to prevent folding */
volatile size_t g_mem_size = 64;
volatile int g_init_flag = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force early initialization of memory builtins */
    char buffer[32];
    volatile char* volatile_ptr = buffer;
    
    /* Use all three builtins in constructor */
    __builtin_memset(volatile_ptr, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 8);
    __builtin_memmove(buffer + 8, buffer, 8);
    
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use builtins in destructor */
    char cleanup_buf[16];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile to prevent optimization */
    volatile size_t local_size = g_mem_size;
    
    /* Complex memory initialization with goto */
    int use_goto = (depth % 2 == 0);
    
    if (use_goto) {
        goto mem_init_block;
    } else {
        /* Direct initialization */
        __builtin_memset(node->data, 0, sizeof(node->data));
        goto skip_goto;
    }
    
mem_init_block:
    /* This block tests goto into memory operation */
    __builtin_memmove(node->data, base_data, local_size < sizeof(node->data) ? local_size : sizeof(node->data));
    /* Jump out of block */
    goto after_init;
    
skip_goto:
    /* Alternative path */
    __builtin_memcpy(node->data, base_data, 32);
    
after_init:
    /* Fill remaining with memset */
    __builtin_memset(node->data + 32, depth, sizeof(node->data) - 32);
    
    node->size = local_size;
    node->left = create_ast(depth - 1, node->data);
    node->right = create_ast(depth - 2, node->data + 16);
    
    return node;
}

/* Function with OpenMP parallel section */
static void parallel_memory_operations(ASTNode* nodes[], int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i] && nodes[(i + 1) % count]) {
            volatile size_t copy_size = nodes[i]->size;
            
            /* Test all three builtins in parallel region */
            __builtin_memcpy(nodes[(i + 1) % count]->data, 
                           nodes[i]->data, 
                           copy_size < sizeof(nodes[i]->data) ? copy_size : sizeof(nodes[i]->data));
            
            /* Clear part of destination */
            __builtin_memset(nodes[i]->data + 16, 0, 16);
            
            /* Overlapping move */
            __builtin_memmove(nodes[i]->data + 8, 
                            nodes[i]->data, 
                            24);
        }
    }
}

/* Complex token processing */
static uint32_t process_ast(ASTNode* node, uint32_t hash) {
    if (!node) return hash;
    
    /* Process data with memory operations */
    char temp[64];
    volatile size_t process_size = node->size;
    
    /* Copy with goto control flow */
    int use_memmove = (hash % 3 == 0);
    
    if (use_memmove) {
        goto use_move;
    }
    
    __builtin_memcpy(temp, node->data, 
                    process_size < sizeof(temp) ? process_size : sizeof(temp));
    goto after_copy;
    
use_move:
    __builtin_memmove(temp, node->data, 
                     process_size < sizeof(temp) ? process_size : sizeof(temp));
    
after_copy:
    /* Compute hash */
    for (size_t j = 0; j < sizeof(temp); j++) {
        hash = (hash * 31) + temp[j];
    }
    
    /* Recursive processing */
    hash = process_ast(node->left, hash);
    hash = process_ast(node->right, hash);
    
    return hash;
}

/* Main execution flow */
int main(void) {
    const int NUM_NODES = 8;
    ASTNode* nodes[NUM_NODES];
    uint32_t final_hash = 0x811C9DC5;  /* FNV-1a basis */
    
    /* Initialize complex token array */
    char base_data[64];
    for (int i = 0; i < sizeof(base_data); i++) {
        base_data[i] = (char)(i * 7 + 13);
    }
    
    /* Create AST nodes */
    for (int i = 0; i < NUM_NODES; i++) {
        nodes[i] = create_ast(3 + (i % 3), base_data + (i * 8));
    }
    
    /* Parallel memory operations */
    parallel_memory_operations(nodes, NUM_NODES);
    
    /* Process all nodes */
    for (int i = 0; i < NUM_NODES; i++) {
        final_hash = process_ast(nodes[i], final_hash);
    }
    
    /* Additional memory operations in main */
    char final_buffer[128];
    volatile size_t final_size = g_mem_size * 2;
    
    /* Chain of memory operations */
    __builtin_memset(final_buffer, 0xCC, sizeof(final_buffer));
    __builtin_memcpy(final_buffer + 32, base_data, 32);
    __builtin_memmove(final_buffer + 64, final_buffer + 32, 32);
    
    /* Add buffer to hash */
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        final_hash = (final_hash * 31) + final_buffer[i];
    }
    
    /* Print verification result */
    printf("Final hash: 0x%08X\n", final_hash);
    
    /* Cleanup */
    for (int i = 0; i < NUM_NODES; i++) {
        if (nodes[i]) {
            /* Clear before free */
            __builtin_memset(nodes[i]->data, 0, sizeof(nodes[i]->data));
            free(nodes[i]);
        }
    }
    
    return (final_hash != 0) ? 0 : 1;
}
