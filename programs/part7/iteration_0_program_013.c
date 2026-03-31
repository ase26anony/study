/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
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
    struct ASTNode *parent;
} ASTNode;

/* Global token array for parser */
static const char *tokens[] = {
    "memcpy", "memset", "memmove", "data", "test", "asan", "hwasan"
};
static const int token_count = 7;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < 256; i++) {
        volatile_src[i] = (char)(i % 26 + 'A');
    }
    
    /* Force early built-in usage in constructor */
    __builtin_memset(volatile_dest, 0, sizeof(volatile_dest));
    __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operation in destructor */
    __builtin_memset(volatile_dest, 0xFF, 16);
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth, int token_idx) {
    if (depth <= 0 || token_idx >= token_count) {
        return NULL;
    }
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node with built-in memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data with built-in memcpy */
    const char *token = tokens[token_idx % token_count];
    size_t len = strlen(token);
    if (len > 31) len = 31;
    __builtin_memcpy(node->data, token, len);
    node->data[len] = '\0';
    
    node->type = token_idx;
    
    /* Recursive parsing with goto for flow control */
    int use_goto = (depth % 3 == 0);
    
    if (use_goto) {
        goto recursive_call;
    }
    
    node->left = parse_expression(depth - 1, token_idx + 1);
    
recursive_call:
    node->right = parse_expression(depth - 2, token_idx + 2);
    
    /* Copy between nodes using built-in memmove */
    if (node->left && node->right) {
        __builtin_memmove(node->left->data + 16, node->right->data, 16);
    }
    
    return node;
}

/* Complex memory operation with goto edge cases */
static void perform_memory_operations(void) {
    char buffer1[128];
    char buffer2[128];
    char buffer3[128];
    
    /* Initialize buffers */
    for (int i = 0; i < 128; i++) {
        buffer1[i] = (char)(i % 10 + '0');
        buffer2[i] = (char)(i % 26 + 'a');
    }
    
    /* Goto into memory operation block */
    int operation = 0;
    
operation_start:
    switch (operation) {
        case 0:
            /* Force memcpy redirection */
            __builtin_memcpy(buffer3, buffer1, volatile_len);
            operation = 1;
            goto operation_start;
            
        case 1:
            /* Force memset redirection */
            __builtin_memset(buffer3 + 32, 0xAA, volatile_len / 2);
            operation = 2;
            goto operation_start;
            
        case 2:
            /* Force memmove redirection with overlapping regions */
            __builtin_memmove(buffer1 + 16, buffer1, 64);
            operation = 3;
            goto operation_start;
            
        case 3:
            /* Jump out of switch */
            goto operation_end;
            
        default:
            __builtin_memset(buffer3, 0, sizeof(buffer3));
    }
    
operation_end:
    /* Final overlapping move */
    __builtin_memmove(buffer2, buffer2 + 32, 64);
}

/* OpenMP parallel memory dispatcher */
static void parallel_memory_dispatch(void) {
    int i;
    const int num_operations = 100;
    char *results = malloc(num_operations * 64);
    
    if (!results) return;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < num_operations; i++) {
        char local_buf[64];
        char *target = results + (i * 64);
        
        /* Mix of built-ins in parallel region */
        __builtin_memset(local_buf, i, 64);
        
        if (i % 3 == 0) {
            __builtin_memcpy(target, local_buf, 32);
        } else if (i % 3 == 1) {
            __builtin_memset(target, 0xCC, 48);
        } else {
            __builtin_memmove(target, target + 16, 32);
        }
    }
    
    free(results);
}

/* Calculate hash of AST tree */
static unsigned long compute_tree_hash(ASTNode *node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char *ptr = node->data;
    
    /* DJB2 hash algorithm */
    while (*ptr) {
        hash = ((hash << 5) + hash) + *ptr++;
    }
    
    hash += compute_tree_hash(node->left);
    hash += compute_tree_hash(node->right);
    
    return hash;
}

/* Main execution flow */
int main(void) {
    unsigned long total_hash = 0;
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Recursive parser with AST operations */
    ASTNode *root = parse_expression(5, 0);
    if (root) {
        total_hash = compute_tree_hash(root);
        printf("AST tree hash: %lu\n", total_hash);
    }
    
    /* Phase 2: Control flow edge cases with goto */
    perform_memory_operations();
    
    /* Phase 3: OpenMP parallel memory operations */
    #ifdef _OPENMP
    parallel_memory_dispatch();
    printf("OpenMP parallel dispatch completed\n");
    #endif
    
    /* Phase 4: Direct built-in calls with volatile lengths */
    char final_buffer[256];
    volatile int dynamic_len = volatile_len + 32;
    
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, (void*)volatile_src, dynamic_len);
    __builtin_memmove(final_buffer + 64, final_buffer, 128);
    
    /* Verify operations by computing checksum */
    unsigned long checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += (unsigned char)final_buffer[i];
    }
    
    printf("Final buffer checksum: %lu\n", checksum);
    printf("Total operations hash: %lu\n", total_hash + checksum);
    
    /* Cleanup */
    /* Note: In real code, would need to free AST tree recursively */
    
    return 0;
}
