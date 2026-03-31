/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_memcpy_len = 256;
static volatile size_t g_memset_len = 128;
static volatile size_t g_memmove_len = 192;

/* Recursive AST-like structure */
typedef struct ast_node {
    int type;
    char *data;
    size_t data_len;
    struct ast_node *left;
    struct ast_node *right;
} ast_node_t;

/* Constructor function to force early initialization */
__attribute__((constructor)) static void init_asan_early(void) {
    volatile char buffer[64];
    /* Force builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 32);
}

/* Destructor for cleanup coordination */
__attribute__((destructor)) static void cleanup_asan(void) {
    volatile char dummy[8];
    __builtin_memset(dummy, 0xFF, sizeof(dummy));
}

/* Recursive AST manipulation with memory operations */
static ast_node_t* create_ast(int depth, const char *base_data) {
    if (depth <= 0) return NULL;
    
    ast_node_t *node = malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    node->type = depth;
    node->data_len = g_memcpy_len % 128 + 64;
    node->data = malloc(node->data_len);
    
    /* Use all three builtins in AST construction */
    __builtin_memset(node->data, depth, node->data_len);
    
    if (base_data) {
        size_t copy_len = node->data_len < 96 ? node->data_len : 96;
        __builtin_memcpy(node->data, base_data, copy_len);
    }
    
    /* Create children with goto-based control flow */
    node->left = NULL;
    node->right = NULL;
    
    if (depth > 1) {
        /* Jump into memory operation block */
        goto create_left;
        
        create_left:
        node->left = create_ast(depth - 1, node->data);
        
        /* Jump out and back in */
        if (depth > 2) {
            goto create_right;
        }
    }
    
    if (depth > 2) {
        create_right:
        node->right = create_ast(depth - 2, node->data + 16);
    }
    
    return node;
}

/* Complex memory operation with goto edge cases */
static void process_with_goto(ast_node_t *node, char *output) {
    if (!node) return;
    
    volatile int use_memmove = 1;
    char temp_buffer[256];
    
    /* Goto jumping into memmove block */
    if (node->type % 2 == 0) {
        goto memmove_block;
    } else {
        goto memcpy_block;
    }
    
memmove_block:
    {
        /* Force memmove with overlapping regions */
        __builtin_memmove(temp_buffer + 32, temp_buffer, g_memmove_len % 128);
        goto after_memmove;
    }
    
memcpy_block:
    {
        __builtin_memcpy(temp_buffer, node->data, 
                        node->data_len < 256 ? node->data_len : 256);
        goto after_memmove;
    }
    
after_memmove:
    /* Jump back for memset */
    if (node->left) {
        goto memset_block;
    }
    
memset_block:
    {
        __builtin_memset(output + node->type * 8, node->type, 64);
        goto continue_processing;
    }
    
continue_processing:
    /* Process children */
    process_with_goto(node->left, output);
    process_with_goto(node->right, output + 128);
}

/* OpenMP parallel memory dispatch */
static void parallel_memory_ops(ast_node_t **nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buffer[512];
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Mixed builtin usage in parallel region */
                __builtin_memset(local_buffer, tid, sizeof(local_buffer));
                
                /* Force all three builtins per thread */
                size_t len = (g_memcpy_len + tid * 16) % 256;
                __builtin_memcpy(local_buffer + 64, nodes[i]->data, 
                                len < nodes[i]->data_len ? len : nodes[i]->data_len);
                
                /* Overlapping memmove */
                __builtin_memmove(local_buffer + 128, local_buffer + 96, 64);
                
                /* Copy back to node */
                __builtin_memcpy(nodes[i]->data + 32, local_buffer, 128);
            }
        }
    }
}

/* Main execution flow */
int main(void) {
    const int num_nodes = 8;
    ast_node_t *nodes[num_nodes];
    char final_buffer[1024] = {0};
    unsigned long hash = 0;
    
    /* Initialize AST forest */
    for (int i = 0; i < num_nodes; i++) {
        char base[128];
        __builtin_memset(base, i + 1, sizeof(base));
        nodes[i] = create_ast(4 + (i % 3), base);
    }
    
    /* Process with goto-based control flow */
    for (int i = 0; i < num_nodes; i++) {
        process_with_goto(nodes[i], final_buffer + i * 64);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(nodes, num_nodes);
    
    /* Compute verification hash */
    for (int i = 0; i < 1024; i++) {
        hash = hash * 31 + final_buffer[i];
    }
    
    /* Additional builtin calls in main */
    volatile char verify_buf[256];
    __builtin_memset(verify_buf, 0xCC, sizeof(verify_buf));
    __builtin_memcpy(verify_buf + 128, verify_buf, 64);
    __builtin_memmove(verify_buf, verify_buf + 64, 128);
    
    /* Print result */
    printf("Result hash: %lu\n", hash);
    
    /* Cleanup */
    for (int i = 0; i < num_nodes; i++) {
        if (nodes[i]) {
            free(nodes[i]->data);
            free(nodes[i]);
        }
    }
    
    return 0;
}
