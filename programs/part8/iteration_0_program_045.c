/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 96;

/* Recursive AST-like structure */
struct ast_node {
    int type;
    char data[256];
    struct ast_node *left;
    struct ast_node *right;
    struct ast_node *next;
};

/* Constructor function to force early initialization */
__attribute__((constructor)) static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 16);
    __builtin_memmove(buffer + 8, buffer, 8);
}

/* Destructor to test cleanup paths */
__attribute__((destructor)) static void cleanup_asan(void) {
    volatile char final_buf[16];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth, const char *base_data) {
    if (depth <= 0) return NULL;
    
    struct ast_node *node = malloc(sizeof(*node));
    if (!node) return NULL;
    
    /* Use all three builtins with volatile lengths */
    __builtin_memset(node, 0, sizeof(*node));
    __builtin_memcpy(node->data, base_data, g_memcpy_len % 256);
    
    node->type = depth;
    node->left = build_ast(depth - 1, base_data + 1);
    node->right = build_ast(depth - 2, base_data + 2);
    
    /* Memmove between node data fields */
    if (node->left && node->right) {
        __builtin_memmove(node->left->data + 32, 
                         node->right->data, 
                         g_memmove_len % 128);
    }
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(struct ast_node *node) {
    if (!node) return;
    
    volatile char temp[512];
    int state = 0;
    
    /* Jump into memory operation block */
    goto entry_point;
    
memory_ops:
    /* This block contains the critical builtins */
    __builtin_memset(temp, node->type, g_memset_len % 512);
    __builtin_memcpy(node->data, temp, g_memcpy_len % 256);
    
    if (node->left) {
        __builtin_memmove(temp + 256, node->left->data, g_memmove_len % 256);
        goto process_left;
    }
    
    goto exit_point;
    
entry_point:
    state = 1;
    goto memory_ops;
    
process_left:
    state = 2;
    if (node->right) {
        /* Another memcpy in goto path */
        __builtin_memcpy(node->right->data, temp + 128, 64);
    }
    
exit_point:
    /* Final memory operation after goto */
    __builtin_memset(temp + 384, 0xCC, 64);
}

/* OpenMP parallel section */
static void parallel_memory_ops(struct ast_node **nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            volatile char local_buf[1024];
            
            /* Each thread uses all three builtins */
            __builtin_memset(local_buf, tid, g_memset_len % 1024);
            
            if (nodes[i]) {
                __builtin_memcpy(nodes[i]->data + 64, 
                               local_buf, 
                               g_memcpy_len % 192);
                
                /* Conditional memmove */
                if (i > 0 && nodes[i-1]) {
                    __builtin_memmove(nodes[i]->data,
                                     nodes[i-1]->data,
                                     g_memmove_len % 128);
                }
            }
        }
        
        /* Thread-private memory operations */
        #pragma omp single
        {
            volatile char single_buf[256];
            __builtin_memset(single_buf, 0xDD, sizeof(single_buf));
            __builtin_memcpy(single_buf + 128, single_buf, 64);
        }
    }
}

/* Main execution flow */
int main(void) {
    const char *base_data = "TEST_DATA_FOR_AST_CONSTRUCTION_0123456789_ABCDEF";
    struct ast_node *root = build_ast(5, base_data);
    
    if (!root) {
        fprintf(stderr, "Failed to build AST\n");
        return 1;
    }
    
    /* Process with goto patterns */
    process_with_goto(root);
    
    /* Create array for parallel processing */
    struct ast_node *node_array[8];
    node_array[0] = root;
    for (int i = 1; i < 8; i++) {
        node_array[i] = build_ast(3, base_data + i * 3);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(node_array, 8);
    
    /* Compute verification hash */
    unsigned long hash = 0;
    struct ast_node *current = root;
    int depth = 0;
    
    while (current && depth < 100) {
        volatile char compute_buf[128];
        
        /* Use builtins in verification */
        __builtin_memset(compute_buf, current->type, sizeof(compute_buf));
        __builtin_memcpy(compute_buf + 64, current->data, 64);
        
        for (int i = 0; i < 128; i++) {
            hash = (hash * 31) + compute_buf[i];
        }
        
        /* Memmove during traversal */
        if (current->left && current->right) {
            __builtin_memmove(current->left->data,
                             current->right->data,
                             g_memmove_len % 64);
        }
        
        current = current->next ? current->next : current->left;
        depth++;
    }
    
    printf("Verification hash: %lu\n", hash);
    
    /* Cleanup */
    /* ... cleanup code would go here ... */
    
    return 0;
}
