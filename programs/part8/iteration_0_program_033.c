#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    struct ASTNode* parent;
} ASTNode;

/* Global volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_init_value = 0x42;
volatile int g_copy_flag = 1;

/* Token array for parser simulation */
static const char* tokens[] = {
    "IDENT", "NUMBER", "STRING", "LPAREN", "RPAREN",
    "PLUS", "MINUS", "MULT", "DIV", "ASSIGN"
};
static const int token_count = 10;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_sanitizer_hook(void) {
    volatile char buffer[128];
    /* Force initialization of memcpy redirection */
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, "constructor_init", 16);
    printf("Constructor: Initialized sanitizer hooks\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_sanitizer_hook(void) {
    volatile char cleanup_buf[256];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
    printf("Destructor: Cleaning up\n");
}

/* Recursive AST creation with memory operations */
static ASTNode* create_ast_node(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile size */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = depth * 100;
    
    /* Fill data with pattern using memset */
    __builtin_memset(node->data, g_init_value, g_mem_size % 64);
    
    /* Create children recursively */
    node->left = create_ast_node(depth - 1);
    node->left = create_ast_node(depth - 1);
    
    /* Copy parent reference using memcpy */
    if (node->left) {
        __builtin_memcpy(&node->left->parent, &node, sizeof(ASTNode*));
    }
    
    return node;
}

/* Function with goto and memmove in different blocks */
static void process_with_goto(ASTNode* src, ASTNode* dst, int mode) {
    volatile char temp_buffer[256];
    int state = 0;
    
    if (mode == 0) goto copy_block;
    
init_block:
    __builtin_memset(temp_buffer, 0xAA, sizeof(temp_buffer));
    state = 1;
    
copy_block:
    if (src && dst) {
        /* Use memmove for overlapping regions */
        __builtin_memmove(dst->data, src->data, g_mem_size % 64);
        state = 2;
    }
    
    if (state == 0) goto init_block;
    
transform_block:
    /* Transform data using memcpy */
    __builtin_memcpy(temp_buffer + 128, temp_buffer, 128);
    state = 3;
    
    if (state == 2) goto transform_block;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        volatile char local_buf[512];
        ASTNode* node = nodes[i];
        
        if (!node) continue;
        
        /* Each thread uses builtins independently */
        __builtin_memset(local_buf, i, sizeof(local_buf));
        
        if (g_copy_flag) {
            __builtin_memcpy(node->data, local_buf, g_mem_size % 64);
        } else {
            __builtin_memmove(local_buf + 256, local_buf, 256);
        }
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Verify copy with another memcpy */
        if (i % 2 == 0) {
            char verify_buf[64];
            __builtin_memcpy(verify_buf, node->data, 64);
        }
    }
}

/* Recursive parser simulation */
static int parse_expression(ASTNode* node, int token_idx) {
    static volatile int recursion_depth = 0;
    volatile char parse_buf[128];
    int result = 0;
    
    if (!node || token_idx >= token_count) return 0;
    
    recursion_depth++;
    
    /* Initialize buffer with token name */
    const char* token = tokens[token_idx];
    size_t len = strlen(token);
    __builtin_memset(parse_buf, 0, sizeof(parse_buf));
    __builtin_memcpy(parse_buf, token, len);
    
    /* Process based on token type */
    switch (token_idx % 4) {
        case 0:
            __builtin_memset(node->data, parse_buf[0], g_mem_size % 64);
            break;
        case 1:
            __builtin_memcpy(node->data, parse_buf, len);
            break;
        case 2:
            /* Overlapping memmove */
            __builtin_memmove(parse_buf + 64, parse_buf, 64);
            __builtin_memcpy(node->data, parse_buf + 64, 64);
            break;
        case 3:
            /* Chain of operations */
            __builtin_memset(parse_buf, 0xCC, 64);
            __builtin_memcpy(node->data, parse_buf, 64);
            __builtin_memmove(node->data + 32, node->data, 32);
            break;
    }
    
    /* Recursive calls */
    if (node->left) {
        result += parse_expression(node->left, (token_idx + 1) % token_count);
    }
    
    recursion_depth--;
    return result + 1;
}

int main(void) {
    const int ast_depth = 4;
    const int node_count = 15;
    ASTNode* nodes[node_count];
    int i, hash = 0;
    
    printf("Starting ASAN redirection test...\n");
    
    /* Create AST nodes */
    for (i = 0; i < node_count; i++) {
        nodes[i] = create_ast_node(ast_depth - (i % 3));
        if (!nodes[i]) {
            fprintf(stderr, "Failed to create node %d\n", i);
            return 1;
        }
    }
    
    /* Test goto with memmove */
    for (i = 0; i < node_count - 1; i++) {
        process_with_goto(nodes[i], nodes[i + 1], i % 2);
    }
    
    /* Execute OpenMP parallel operations */
    #ifdef _OPENMP
    printf("Running OpenMP parallel section...\n");
    parallel_memory_ops(nodes, node_count);
    #endif
    
    /* Parse all nodes recursively */
    for (i = 0; i < node_count; i++) {
        hash += parse_expression(nodes[i], i % token_count);
    }
    
    /* Final memory operations */
    volatile char final_buffer[1024];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    
    /* Chain all builtins together */
    for (i = 0; i < 10; i++) {
        if (i % 3 == 0) {
            __builtin_memcpy(final_buffer + i * 64, nodes[i % node_count]->data, 64);
        } else if (i % 3 == 1) {
            __builtin_memset(final_buffer + i * 64, i, 64);
        } else {
            __builtin_memmove(final_buffer + i * 64 + 32, final_buffer + i * 64, 32);
        }
    }
    
    /* Calculate verification hash */
    for (i = 0; i < 1024; i++) {
        hash += final_buffer[i];
    }
    
    printf("Test completed. Hash: %d\n", hash);
    
    /* Cleanup */
    for (i = 0; i < node_count; i++) {
        if (nodes[i]) {
            free(nodes[i]);
        }
    }
    
    return 0;
}
