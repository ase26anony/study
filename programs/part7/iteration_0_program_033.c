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

/* Global volatile variables to prevent constant folding */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

/* Token array for parser */
typedef struct {
    char tokens[256];
    volatile int pos;
} TokenStream;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_sanitizer_hook(void) {
    volatile char buffer[32];
    /* Force __builtin_memset in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    g_use_hwasan = (rand() % 2);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_sanitizer_hook(void) {
    volatile char buffer[32];
    /* Force __builtin_memcpy in destructor */
    char src[32] = "destructor_data";
    __builtin_memcpy(buffer, src, 16);
}

/* Recursive parser with goto statements */
static ASTNode* parse_expression(TokenStream* ts, int depth) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node || depth > 3) {
        goto cleanup;
    }
    
    /* Initialize node with volatile memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    node->size = g_mem_size;
    
    /* Jump into memory operation block */
    if (ts->pos % 2 == 0) {
        goto mem_operation;
    }
    
    normal_path:
    /* Copy token data using builtin memcpy */
    size_t copy_len = ts->pos % 32 + 1;
    __builtin_memcpy(node->data, &ts->tokens[ts->pos], copy_len);
    ts->pos += copy_len;
    
    /* Recursive calls */
    if (depth < 2) {
        node->left = parse_expression(ts, depth + 1);
        
        /* Jump out of block and back in */
        if (node->left) {
            goto mem_operation;
        }
    }
    
    return node;
    
    mem_operation:
    {
        /* Complex memory move with goto */
        char temp[64];
        volatile size_t move_size = node->size % 32 + 16;
        
        /* Force __builtin_memmove with overlapping regions */
        __builtin_memmove(&node->data[16], node->data, move_size);
        
        /* Copy to temp and back */
        __builtin_memcpy(temp, node->data, move_size);
        __builtin_memmove(node->data, temp, move_size);
        
        if (ts->pos % 3 == 0) {
            goto normal_path;
        }
    }
    
    cleanup:
    if (node) free(node);
    return NULL;
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_operations(ASTNode** nodes, int count) {
    volatile int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Mixed memory operations in parallel region */
            char buffer[128];
            volatile size_t op_size = (i * 16 + 32) % 64;
            
            /* All three builtins in parallel */
            __builtin_memset(buffer, i, op_size);
            __builtin_memcpy(nodes[i]->data, buffer, op_size % 32);
            
            /* Overlapping memmove */
            if (i > 0 && nodes[i-1]) {
                size_t move_len = (op_size % 16) + 8;
                __builtin_memmove(
                    &nodes[i]->data[8],
                    nodes[i-1]->data,
                    move_len
                );
            }
        }
    }
}

/* Complex initialization with volatile control */
static void initialize_tokens(TokenStream* ts) {
    volatile char init_pattern[] = "TEST_PATTERN_1234567890_ABCDEF";
    volatile size_t pattern_len = sizeof(init_pattern) - 1;
    
    /* Fill tokens with pattern using builtin memcpy */
    for (int i = 0; i < 256; i += pattern_len) {
        size_t copy_len = (256 - i) < pattern_len ? (256 - i) : pattern_len;
        __builtin_memcpy(&ts->tokens[i], init_pattern, copy_len);
    }
    
    ts->pos = 0;
    
    /* Additional memset for partial regions */
    volatile size_t clear_size = 128;
    __builtin_memset(&ts->tokens[128], 0, clear_size);
}

/* Compute hash from AST structure */
static uint64_t compute_ast_hash(ASTNode* node, int depth) {
    if (!node || depth > 10) return 0;
    
    uint64_t hash = 5381;
    volatile char* data = node->data;
    
    /* Process data with memory operations */
    for (int i = 0; i < 32; i++) {
        hash = ((hash << 5) + hash) + data[i];
        
        /* Interleave with memmove on small buffers */
        if (i % 8 == 0 && i > 0) {
            char temp[8];
            __builtin_memcpy(temp, &data[i-8], 8);
            __builtin_memmove(&data[i-4], temp, 4);
        }
    }
    
    /* Recursive hash computation */
    if (node->left) {
        hash ^= compute_ast_hash(node->left, depth + 1);
    }
    if (node->right) {
        hash ^= compute_ast_hash(node->right, depth + 1);
    }
    
    return hash;
}

int main(void) {
    TokenStream ts;
    ASTNode* nodes[8] = {0};
    uint64_t final_hash = 0;
    
    /* Initialize with volatile memory operations */
    initialize_tokens(&ts);
    
    /* Create AST nodes recursively */
    for (int i = 0; i < 4; i++) {
        nodes[i] = parse_expression(&ts, 0);
        
        /* Additional memory operations between nodes */
        if (i > 0 && nodes[i] && nodes[i-1]) {
            volatile size_t copy_size = (i * 8 + 16) % 32;
            __builtin_memcpy(
                nodes[i]->data,
                nodes[i-1]->data,
                copy_size
            );
        }
    }
    
    /* Parallel memory operations */
    parallel_memory_operations(nodes, 4);
    
    /* Compute final verification hash */
    for (int i = 0; i < 4; i++) {
        if (nodes[i]) {
            final_hash += compute_ast_hash(nodes[i], 0);
            
            /* Final memory move before cleanup */
            if (i < 3 && nodes[i+1]) {
                __builtin_memmove(
                    nodes[i]->data,
                    nodes[i+1]->data,
                    24
                );
            }
        }
    }
    
    /* Print result for verification */
    printf("Result hash: %llu\n", (unsigned long long)final_hash);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        if (nodes[i]) free(nodes[i]);
    }
    
    return 0;
}
