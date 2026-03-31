/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ast_node {
    int type;
    char data[32];
    struct ast_node *left;
    struct ast_node *right;
} ast_node_t;

/* Global token array */
static char token_array[1024];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token array with pattern */
    for (int i = 0; i < sizeof(token_array); i++) {
        token_array[i] = (i % 26) + 'a';
    }
    
    /* Use builtin memset in constructor */
    __builtin_memset(volatile_dest, 0xAA, sizeof(volatile_dest));
    
    printf("Constructor initialized token array\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use builtin memcpy in destructor */
    char temp[256];
    __builtin_memcpy(temp, volatile_dest, sizeof(volatile_dest));
    printf("Destructor cleaned up\n");
}

/* Recursive parser with memory operations */
static ast_node_t* parse_expression(int depth) {
    if (depth <= 0) return NULL;
    
    ast_node_t* node = malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Initialize node with builtin memset */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    node->type = depth;
    
    /* Copy data using builtin memcpy with goto for edge case */
    int copy_len = volatile_len % 32;
    if (copy_len > 0) {
        goto copy_block;
    copy_block:
        __builtin_memcpy(node->data, &token_array[token_index], copy_len);
        token_index = (token_index + copy_len) % sizeof(token_array);
    }
    
    /* Recursive calls */
    node->left = parse_expression(depth - 1);
    
    /* Jump back for right child */
    if (depth > 2) {
        goto parse_right;
    }
    
    node->right = NULL;
    return node;
    
parse_right:
    node->right = parse_expression(depth - 2);
    return node;
}

/* Memory operation dispatcher with OpenMP */
static void dispatch_memory_ops(void) {
    int i;
    char buffer1[128], buffer2[128];
    
    /* Initialize buffers with volatile source */
    for (i = 0; i < sizeof(buffer1); i++) {
        buffer1[i] = i;
        buffer2[i] = 255 - i;
    }
    
    #pragma omp parallel private(i)
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (i = 0; i < 4; i++) {
            char local_buf[64];
            int op_type = (thread_id + i) % 3;
            
            switch (op_type) {
                case 0: /* memcpy */
                    __builtin_memcpy(local_buf, buffer1 + i * 16, 32);
                    __builtin_memcpy(buffer2 + i * 16, local_buf, 32);
                    break;
                    
                case 1: /* memset */
                    __builtin_memset(local_buf, thread_id + '0', 32);
                    __builtin_memcpy(volatile_dest + i * 16, local_buf, 16);
                    break;
                    
                case 2: /* memmove with overlap */
                    __builtin_memmove(buffer1 + 8, buffer1, 48);
                    __builtin_memcpy(local_buf, buffer1 + 8, 32);
                    break;
            }
        }
        
        /* Additional builtin usage in parallel region */
        #pragma omp single
        {
            char single_buf[32];
            __builtin_memset(single_buf, 0xCC, sizeof(single_buf));
            __builtin_memcpy(volatile_src, single_buf, sizeof(single_buf));
        }
    }
}

/* Complex memory pattern generator */
static void generate_memory_patterns(void) {
    char pattern_buf[512];
    char *ptr1, *ptr2;
    
    /* Multiple overlapping operations */
    ptr1 = pattern_buf;
    ptr2 = pattern_buf + 128;
    
    /* Chain of builtin calls */
    __builtin_memset(ptr1, 'A', 256);
    __builtin_memcpy(ptr2, ptr1, 128);
    
    /* memmove with overlapping regions */
    __builtin_memmove(ptr1 + 64, ptr1, 192);
    
    /* Nested calls with volatile lengths */
    int len1 = volatile_len % 128;
    int len2 = (volatile_len * 2) % 128;
    
    if (len1 > 0) {
        __builtin_memcpy(ptr2 + 64, ptr1, len1);
    }
    
    if (len2 > 0) {
        __builtin_memset(ptr1 + 128, 'B', len2);
    }
    
    /* Final memmove to ensure all builtins are used */
    __builtin_memmove(pattern_buf + 256, pattern_buf, 256);
}

/* AST traversal with memory operations */
static int traverse_ast(ast_node_t* node, int depth) {
    if (!node) return 0;
    
    int sum = node->type;
    char temp[32];
    
    /* Use goto for control flow edge case */
    if (depth % 3 == 0) {
        goto mem_op_block;
    }
    
    sum += traverse_ast(node->left, depth + 1);
    
mem_op_block:
    /* Builtin memcpy between nodes */
    if (node->left && node->right) {
        __builtin_memcpy(temp, node->left->data, 16);
        __builtin_memcpy(node->right->data, temp, 16);
    }
    
    sum += traverse_ast(node->right, depth + 1);
    return sum;
}

/* Free AST memory */
static void free_ast(ast_node_t* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear node data before free */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    free(node);
}

int main(void) {
    printf("Starting ASAN builtin redirection test\n");
    
    /* Phase 1: Initialize and parse */
    ast_node_t* ast_root = parse_expression(5);
    if (!ast_root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Generate memory patterns */
    generate_memory_patterns();
    
    /* Phase 3: OpenMP memory operations */
    dispatch_memory_ops();
    
    /* Phase 4: Traverse and compute */
    int ast_sum = traverse_ast(ast_root, 0);
    
    /* Phase 5: Additional builtin stress test */
    char final_buffer[1024];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    
    /* Chain all three builtins */
    __builtin_memcpy(final_buffer, token_array, 256);
    __builtin_memset(final_buffer + 256, 0xFF, 256);
    __builtin_memmove(final_buffer + 512, final_buffer, 512);
    
    /* Compute verification hash */
    unsigned int hash = 0;
    for (int i = 0; i < sizeof(final_buffer); i++) {
        hash = (hash * 31) + final_buffer[i];
    }
    
    /* Print results */
    printf("AST traversal sum: %d\n", ast_sum);
    printf("Final buffer hash: 0x%08X\n", hash);
    printf("Volatile dest[0]: 0x%02X\n", (unsigned char)volatile_dest[0]);
    printf("Volatile src[0]: 0x%02X\n", (unsigned char)volatile_src[0]);
    
    /* Cleanup */
    free_ast(ast_root);
    
    printf("Test completed successfully\n");
    return 0;
}
