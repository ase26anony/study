/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 16;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

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
    
    /* Use builtins in constructor */
    __builtin_memset(token_array + 512, 0, 128);
    __builtin_memcpy(volatile_src, token_array, 256);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Cleanup with builtins */
    __builtin_memset(token_array, 0, sizeof(token_array));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in data */
    char pattern[32];
    __builtin_memset(pattern, 'A' + (id % 26), 31);
    pattern[31] = '\0';
    
    __builtin_memcpy(node->data, pattern, 31);
    node->id = id;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 3) {
        goto skip_left;
    }
    
    create_left = 0;
    
skip_left:
    if (create_left) {
        node->left = create_ast_node(depth - 1, id * 2);
    } else {
        node->left = NULL;
    }
    
    node->right = create_ast_node(depth - 1, id * 2 + 1);
    
    return node;
}

/* Function with goto jumping into memmove block */
static void process_with_goto(ASTNode* node1, ASTNode* node2) {
    if (!node1 || !node2) return;
    
    int use_memmove = 1;
    
    /* Jump into memmove block */
    if (node1->id % 2 == 0) {
        goto do_memmove;
    }
    
    /* Regular path */
    __builtin_memcpy(node1->data, node2->data, 32);
    return;
    
do_memmove:
    /* This block should be reached via goto */
    __builtin_memmove(node1->data + 16, node1->data, 16);
    __builtin_memcpy(node1->data, node2->data, 32);
    
    /* Jump out */
    goto finish;
    
    /* Unreachable code that compiler might analyze */
    __builtin_memset(node1->data, 0, 64);  /* Never executed */
    
finish:
    return;
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[128];
        char src_buf[128];
        
        /* Initialize with builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        __builtin_memset(src_buf, 'X', sizeof(src_buf));
        
        /* Use volatile length */
        int len = volatile_len + thread_id;
        if (len > 128) len = 128;
        
        /* Memory operations in parallel region */
        __builtin_memcpy(local_buf, src_buf, len);
        __builtin_memmove(local_buf + 32, local_buf, 32);
        __builtin_memset(local_buf + 64, 0, 32);
        
        /* Copy to volatile destination */
        #pragma omp critical
        {
            __builtin_memcpy(volatile_dest + (thread_id * 16), local_buf, 16);
        }
    }
}

/* Complex memory dispatch with all builtins */
static unsigned long complex_memory_dispatch(ASTNode* root) {
    if (!root) return 0;
    
    unsigned long hash = 0;
    ASTNode* nodes[10];
    int node_count = 0;
    
    /* Collect nodes */
    ASTNode* current = root;
    while (current && node_count < 10) {
        nodes[node_count++] = current;
        current = current->left;
    }
    
    /* Process nodes with different memory operations */
    for (int i = 0; i < node_count; i++) {
        char temp[64];
        
        /* Alternate between memcpy, memset, memmove */
        switch (i % 3) {
            case 0:
                __builtin_memcpy(temp, nodes[i]->data, 32);
                __builtin_memcpy(nodes[i]->data, nodes[(i+1)%node_count]->data, 32);
                break;
            case 1:
                __builtin_memset(nodes[i]->data + 16, i, 16);
                __builtin_memmove(nodes[i]->data, nodes[i]->data + 16, 16);
                break;
            case 2:
                __builtin_memmove(temp, nodes[i]->data, 32);
                __builtin_memcpy(nodes[i]->data, temp, 32);
                break;
        }
        
        /* Compute hash */
        for (int j = 0; j < 32; j++) {
            hash = hash * 31 + nodes[i]->data[j];
        }
    }
    
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize volatile source */
    for (int i = 0; i < 256; i++) {
        volatile_src[i] = (i % 10) + '0';
    }
    
    /* Create recursive AST */
    ASTNode* root = create_ast_node(5, 1);
    
    /* Process with goto flow control */
    if (root && root->left && root->right) {
        process_with_goto(root->left, root->right);
        process_with_goto(root->right, root->left);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Complex memory dispatch */
    unsigned long result_hash = complex_memory_dispatch(root);
    
    /* Additional builtin calls in main */
    char final_buffer[256];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, volatile_dest, 128);
    __builtin_memmove(final_buffer + 128, final_buffer, 128);
    
    /* Use all three builtins in sequence */
    __builtin_memset(final_buffer + 192, 0xFF, 32);
    __builtin_memcpy(final_buffer + 64, final_buffer + 192, 32);
    __builtin_memmove(final_buffer, final_buffer + 64, 64);
    
    /* Compute final verification */
    unsigned long final_sum = result_hash;
    for (int i = 0; i < 256; i++) {
        final_sum += final_buffer[i];
    }
    
    printf("Test completed. Result hash: %lu, Final sum: %lu\n", 
           result_hash, final_sum);
    
    /* Cleanup */
    /* Note: In real code, you'd want to free the AST recursively */
    
    return 0;
}
