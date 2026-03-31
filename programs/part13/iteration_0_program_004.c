/* ISO C99-compliant test program for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 1024;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
struct ast_node {
    char data[256];
    struct ast_node *left;
    struct ast_node *right;
    size_t size;
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    volatile char buffer[64];
    /* Force builtin initialization in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 32, buffer, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[128];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Use volatile to control memory operation sizes */
    volatile size_t copy_size = (depth * 16) % 256;
    
    /* Builtin memory operations with non-foldable sizes */
    __builtin_memset(node->data, depth, sizeof(node->data));
    __builtin_memcpy(node->data, base_data, copy_size);
    
    /* Control flow with goto around memmove */
    if (depth % 2 == 0) {
        goto skip_memmove;
    }
    
    /* This memmove should be instrumented */
    char temp[256];
    __builtin_memmove(temp, node->data, copy_size);
    __builtin_memmove(node->data + 128, temp, copy_size / 2);
    
skip_memmove:
    node->size = copy_size;
    node->left = build_ast(depth - 1, base_data);
    node->right = build_ast(depth - 2, base_data);
    
    return node;
}

/* Function with complex control flow */
static void process_ast_with_goto(struct ast_node* node) {
    if (!node) return;
    
    volatile int do_copy = 1;
    
copy_block:
    if (do_copy && node->left && node->right) {
        /* Cross-copy between AST nodes */
        __builtin_memcpy(node->left->data, node->right->data, 
                        node->size < node->right->size ? node->size : node->right->size);
        do_copy = 0;
        goto skip_operation;
    }
    
    /* Another memmove with goto entry */
    if (node->data[0] != 0) {
        char buffer[128];
        __builtin_memmove(buffer, node->data, 64);
        __builtin_memmove(node->data + 64, buffer, 64);
    }
    
skip_operation:
    /* Recursive processing */
    process_ast_with_goto(node->left);
    process_ast_with_goto(node->right);
}

/* Parallel memory operations */
static void parallel_memory_operations(void) {
    const int num_buffers = 8;
    char* buffers[num_buffers];
    
    #pragma omp parallel for
    for (int i = 0; i < num_buffers; i++) {
        buffers[i] = malloc(g_mem_size);
        if (buffers[i]) {
            /* Each thread uses builtins */
            __builtin_memset(buffers[i], i, g_mem_size);
            
            /* Conditional memcpy based on thread ID */
            if (i > 0) {
                __builtin_memcpy(buffers[i], buffers[i-1], 
                               g_mem_size / (i + 1));
            }
            
            /* Memmove within buffer */
            __builtin_memmove(buffers[i] + g_mem_size/2, 
                            buffers[i], g_mem_size/4);
        }
    }
    
    /* Cleanup */
    #pragma omp parallel for
    for (int i = 0; i < num_buffers; i++) {
        if (buffers[i]) free(buffers[i]);
    }
}

/* Multi-stage initialization */
static void initialize_token_array(char tokens[][64], int count) {
    volatile int init_val = 0xDEADBEEF;
    
    for (int i = 0; i < count; i++) {
        /* Use builtin memset with volatile-derived size */
        volatile size_t token_size = (i * 16 + 32) % 64;
        __builtin_memset(tokens[i], init_val & 0xFF, token_size);
        
        /* Copy between tokens */
        if (i > 0) {
            __builtin_memcpy(tokens[i], tokens[i-1], 32);
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Stage 1: Initialize complex token array */
    char tokens[16][64];
    initialize_token_array(tokens, 16);
    
    /* Stage 2: Build and process recursive AST */
    struct ast_node* root = build_ast(5, "AST_BASE_DATA");
    if (root) {
        process_ast_with_goto(root);
        
        /* Verify AST data through checksum */
        unsigned long checksum = 0;
        struct ast_node* stack[32];
        int top = 0;
        stack[top++] = root;
        
        while (top > 0) {
            struct ast_node* current = stack[--top];
            for (size_t i = 0; i < current->size; i++) {
                checksum += current->data[i];
            }
            if (current->right) stack[top++] = current->right;
            if (current->left) stack[top++] = current->left;
        }
        
        printf("AST checksum: %lu\n", checksum);
        
        /* Cleanup AST */
        /* ... (recursive free implementation omitted for brevity) */
    }
    
    /* Stage 3: Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Stage 4: Final memory operations with varied sizes */
    volatile size_t final_sizes[] = {16, 32, 64, 128, 256};
    for (int i = 0; i < 5; i++) {
        char src[256], dst[256];
        volatile size_t op_size = final_sizes[i];
        
        __builtin_memset(src, i + 1, op_size);
        __builtin_memcpy(dst, src, op_size);
        __builtin_memmove(src + op_size/2, dst, op_size/2);
    }
    
    printf("ASAN builtin test completed successfully.\n");
    return 0;
}
