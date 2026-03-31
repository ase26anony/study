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
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 16);
}

/* Destructor for cleanup coordination */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char final_check[8];
    __builtin_memset(final_check, 0xFF, 8);
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth, const char *base_data) {
    if (depth <= 0) return NULL;
    
    struct ast_node *node = malloc(sizeof(*node));
    if (!node) return NULL;
    
    /* Use builtins with volatile lengths */
    __builtin_memset(node, 0, sizeof(*node));
    node->type = depth;
    
    /* Copy data with builtin */
    size_t copy_len = g_memcpy_len % 256;
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Recursive construction */
    node->left = build_ast(depth - 1, base_data + 1);
    node->right = build_ast(depth - 1, base_data + 2);
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(struct ast_node *src, struct ast_node *dst) {
    int use_memmove = 0;
    
    /* Jump into block with memmove */
    if (src && dst) {
        goto copy_block;
    }
    
    normal_path:
        __builtin_memset(dst->data, 0xCC, g_memset_len % 256);
        return;
    
    copy_block:
        /* This tests flow sensitivity */
        use_memmove = 1;
        if (use_memmove) {
            __builtin_memmove(dst->data, src->data, g_memmove_len % 256);
        }
        goto normal_path;
}

/* OpenMP parallel memory operations */
static void parallel_mem_operations(struct ast_node **nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            volatile char local_buf[512];
            
            /* Mixed builtin usage in parallel region */
            __builtin_memset(local_buf, tid, sizeof(local_buf));
            
            if (nodes[i]) {
                /* Copy to/from AST nodes */
                __builtin_memcpy(nodes[i]->data, local_buf + i * 16, 64);
                
                /* Conditional memmove with goto */
                if (i > 0 && nodes[i-1]) {
                    __builtin_memmove(nodes[i-1]->data + 32, 
                                     nodes[i]->data + 32, 32);
                }
            }
        }
    }
}

/* Multi-stage processing */
static unsigned long compute_ast_hash(struct ast_node *node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    volatile char tmp_buf[128];
    
    /* Process node data with builtins */
    __builtin_memcpy(tmp_buf, node->data, sizeof(tmp_buf));
    
    for (size_t i = 0; i < sizeof(tmp_buf); i++) {
        hash = ((hash << 5) + hash) + tmp_buf[i];
    }
    
    /* Recursive hash computation */
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

int main(void) {
    const int num_nodes = 8;
    struct ast_node *nodes[num_nodes];
    unsigned long final_hash = 0;
    
    /* Initialize AST structures */
    const char *base_data = "AST_Base_Data_String_For_Memory_Operations_1234567890";
    
    for (int i = 0; i < num_nodes; i++) {
        nodes[i] = build_ast(3, base_data + i);
        
        /* Additional builtin usage during initialization */
        volatile char init_buf[256];
        __builtin_memset(init_buf, i * 16, sizeof(init_buf));
        
        if (nodes[i]) {
            __builtin_memcpy(nodes[i]->data + 128, init_buf, 128);
        }
    }
    
    /* Process with goto edge cases */
    for (int i = 1; i < num_nodes; i++) {
        if (nodes[i-1] && nodes[i]) {
            process_with_goto(nodes[i-1], nodes[i]);
        }
    }
    
    /* Parallel memory operations */
    parallel_mem_operations(nodes, num_nodes);
    
    /* Compute verification hash */
    #pragma omp parallel for reduction(+:final_hash)
    for (int i = 0; i < num_nodes; i++) {
        if (nodes[i]) {
            final_hash += compute_ast_hash(nodes[i]);
        }
    }
    
    /* Final builtin usage */
    volatile char final_buf[64];
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf, &final_hash, sizeof(final_hash));
    __builtin_memmove(final_buf + 32, final_buf, 32);
    
    /* Print result for verification */
    printf("AST Hash Result: %lu\n", final_hash);
    
    /* Cleanup */
    for (int i = 0; i < num_nodes; i++) {
        free(nodes[i]);
    }
    
    return 0;
}
