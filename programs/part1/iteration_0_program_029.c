/* ISO C99-compliant test program for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    int value;
    uint8_t padding[32];  /* For redzone testing */
} ASTNode;

/* Global token array */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "test"
};
static const int g_token_count = 6;

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_environment(void) {
    /* Force initialization of ASAN runtime */
    volatile char dummy[16];
    __builtin_memset(dummy, 0, sizeof(dummy));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operation to ensure cleanup paths are taken */
    volatile int final_check[8];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int index) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Fill data with pattern using memcpy */
    char pattern[64];
    __builtin_memset(pattern, 'A' + (index % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(node->data));
    
    node->value = index;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (index % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, index * 2);
        node->right = create_ast(depth - 1, index * 2 + 1);
        
        if (use_goto) {
            skip_children:
            /* Placeholder for goto target */
            return node;
        }
        
        create_children:
        /* Copy between nodes using memmove */
        if (node->left && node->right) {
            /* Test memmove with overlapping regions */
            size_t copy_size = sizeof(node->left->data) / 2;
            __builtin_memmove(node->left->data + 10, 
                            node->left->data, 
                            copy_size);
            __builtin_memmove(node->right->data,
                            node->left->data + 20,
                            copy_size);
        }
        goto skip_children;
    }
    
    return node;
}

/* Complex memory operation function with OpenMP */
static void parallel_memory_operations(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Varied memory operations based on index */
            switch (i % 3) {
                case 0: {
                    /* Test memcpy with volatile size */
                    volatile size_t copy_len = g_mem_size % 128;
                    char buffer[256];
                    __builtin_memcpy(buffer, nodes[i]->data, copy_len);
                    
                    /* Chain operations */
                    __builtin_memcpy(nodes[i]->data, buffer + 32, copy_len / 2);
                    break;
                }
                case 1: {
                    /* Test memset with pattern */
                    volatile char fill_char = 0x42 + (i % 16);
                    __builtin_memset(nodes[i]->data, fill_char, 48);
                    
                    /* Follow with memmove */
                    __builtin_memmove(nodes[i]->data + 16,
                                    nodes[i]->data,
                                    32);
                    break;
                }
                case 2: {
                    /* Complex memmove scenario */
                    char temp[128];
                    __builtin_memset(temp, 0, sizeof(temp));
                    __builtin_memcpy(temp, nodes[i]->data, 64);
                    
                    /* Overlapping move */
                    __builtin_memmove(nodes[i]->data + 32,
                                    nodes[i]->data,
                                    64);
                    __builtin_memmove(nodes[i]->data,
                                    temp + 16,
                                    48);
                    break;
                }
            }
        }
    }
}

/* Function with goto jumping into memory operation block */
static void test_goto_flow(void) {
    volatile int trigger = 1;
    char buffer_a[256];
    char buffer_b[256];
    
    __builtin_memset(buffer_a, 'X', sizeof(buffer_a));
    
    if (trigger) {
        goto jump_into_memop;
    }
    
    normal_path:
    __builtin_memcpy(buffer_b, buffer_a, 128);
    return;
    
    jump_into_memop:
    /* Jump into the middle of memory operations */
    __builtin_memset(buffer_a + 64, 'Y', 64);
    goto normal_path;
}

/* Calculate hash of AST structure */
static uint32_t calculate_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    uint32_t hash = 0;
    int i;
    
    /* Hash data field */
    for (i = 0; i < sizeof(node->data); i++) {
        hash = (hash * 31) + node->data[i];
    }
    
    hash ^= node->value;
    hash ^= calculate_ast_hash(node->left);
    hash ^= calculate_ast_hash(node->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create AST structures */
    ASTNode* nodes[8];
    int i;
    
    for (i = 0; i < 8; i++) {
        nodes[i] = create_ast(3, i);
    }
    
    /* Test goto flow control */
    test_goto_flow();
    
    /* Execute parallel memory operations */
    parallel_memory_operations(nodes, 8);
    
    /* Additional builtin calls in main */
    volatile size_t dynamic_size = g_mem_size;
    char main_buffer[512];
    
    __builtin_memset(main_buffer, 0, sizeof(main_buffer));
    __builtin_memcpy(main_buffer, g_tokens[0], strlen(g_tokens[0]));
    __builtin_memmove(main_buffer + 64, main_buffer, 32);
    
    /* Process tokens with memory operations */
    for (i = 0; i < g_token_count; i++) {
        size_t token_len = strlen(g_tokens[i]);
        volatile size_t copy_len = token_len % 128;
        
        __builtin_memcpy(main_buffer + (i * 32),
                        g_tokens[i],
                        copy_len);
    }
    
    /* Calculate and print verification hash */
    uint32_t total_hash = 0;
    for (i = 0; i < 8; i++) {
        if (nodes[i]) {
            total_hash ^= calculate_ast_hash(nodes[i]);
            free(nodes[i]);
        }
    }
    
    /* Final memory operation sequence */
    char final_buffer[256];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, main_buffer, 128);
    __builtin_memmove(final_buffer + 64, final_buffer, 64);
    
    printf("Verification hash: 0x%08X\n", total_hash);
    printf("Test completed.\n");
    
    return 0;
}
