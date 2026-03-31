/* ISO C99-compliant test program for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
struct ast_node {
    int type;
    char data[256];
    struct ast_node *left;
    struct ast_node *right;
    struct ast_node *next;
};

/* Constructor function to force early initialization */
__attribute__((constructor))
static void init_asan_early(void) {
    volatile char buffer[64];
    /* Force memcpy built-in in constructor */
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, "constructor_init", 16);
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char final_check[32];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(*node));
    node->type = depth;
    
    /* Copy data with volatile length */
    volatile size_t copy_len = 128;
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Recursive construction */
    char child_data[256];
    __builtin_snprintf(child_data, sizeof(child_data), "%s_%d", base_data, depth);
    
    node->left = build_ast(depth - 1, child_data);
    node->right = build_ast(depth - 1, child_data);
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(struct ast_node* src, struct ast_node* dst) {
    volatile int use_memmove = 1;
    
    if (src && dst) {
        goto do_copy;
    } else {
        goto skip_copy;
    }
    
do_copy:
    {
        volatile size_t move_size = sizeof(src->data);
        /* This should trigger the memmove built-in redirection */
        __builtin_memmove(dst->data, src->data, move_size);
    }
    goto after_copy;
    
skip_copy:
    __builtin_memset(dst->data, 0, sizeof(dst->data));
    
after_copy:
    /* Additional operation after goto */
    if (dst->type > 0) {
        volatile char temp[64];
        __builtin_memcpy(temp, dst->data, 32);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(struct ast_node** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        volatile size_t op_size = g_mem_size % 256;
        
        if (i % 3 == 0) {
            /* memcpy pattern */
            __builtin_memset(nodes[i]->data, i, op_size);
        } else if (i % 3 == 1) {
            /* memmove pattern with overlap */
            if (i > 0) {
                __builtin_memmove(nodes[i]->data, 
                                 nodes[i-1]->data, 
                                 op_size);
            }
        } else {
            /* memset pattern */
            __builtin_memset(nodes[i]->data, 0xFF, op_size);
        }
    }
}

/* Complex token processing */
static unsigned long process_tokens(const char** tokens, int token_count) {
    unsigned long hash = 5381;
    char buffer[512];
    volatile size_t buf_pos = 0;
    
    for (int i = 0; i < token_count; i++) {
        size_t token_len = strlen(tokens[i]);
        volatile size_t copy_len = token_len;
        
        if (buf_pos + copy_len >= sizeof(buffer)) {
            /* Use memmove to shift buffer */
            __builtin_memmove(buffer, buffer + 256, 256);
            buf_pos -= 256;
        }
        
        /* Use all three builtins */
        __builtin_memcpy(buffer + buf_pos, tokens[i], copy_len);
        buf_pos += copy_len;
        
        if (i % 5 == 0) {
            /* Occasionally clear section */
            __builtin_memset(buffer + buf_pos - 16, 0, 16);
        }
        
        /* Update hash */
        for (size_t j = 0; j < copy_len; j++) {
            hash = ((hash << 5) + hash) + buffer[buf_pos - copy_len + j];
        }
    }
    
    return hash;
}

int main(void) {
    const char* tokens[] = {
        "memcpy", "memset", "memmove", "asan", "hwasan",
        "instrumentation", "redzone", "builtin", "coverage",
        "optimization", "parallel", "recursive", "volatile"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Phase 1: Token processing */
    unsigned long token_hash = process_tokens(tokens, token_count);
    printf("Token hash: %lu\n", token_hash);
    
    /* Phase 2: AST construction and processing */
    struct ast_node* root = build_ast(4, "root");
    if (!root) {
        fprintf(stderr, "Failed to build AST\n");
        return 1;
    }
    
    struct ast_node* copy = malloc(sizeof(struct ast_node));
    if (!copy) {
        free(root);
        return 1;
    }
    
    /* Test goto with memmove */
    process_with_goto(root, copy);
    
    /* Phase 3: Create array for parallel operations */
    struct ast_node* node_array[8];
    node_array[0] = root;
    for (int i = 1; i < 8; i++) {
        node_array[i] = build_ast(3, "parallel_node");
    }
    
    /* Parallel memory operations */
    parallel_memory_ops(node_array, 8);
    
    /* Phase 4: Final verification with all builtins */
    char final_buffer[1024];
    volatile size_t final_size = g_mem_size % 512;
    
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, root->data, 128);
    __builtin_memmove(final_buffer + 128, copy->data, 128);
    
    /* Calculate final checksum */
    unsigned long final_sum = 0;
    for (size_t i = 0; i < final_size && i < sizeof(final_buffer); i++) {
        final_sum += final_buffer[i];
    }
    
    printf("Final checksum: %lu\n", final_sum);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(copy);
    free(root);
    for (int i = 1; i < 8; i++) {
        free(node_array[i]);
    }
    
    return 0;
}
