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
static struct ast_node* build_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Use builtins with volatile lengths */
    __builtin_memset(node, 0, sizeof(*node));
    __builtin_memcpy(node->data, base_data, 
                     g_memcpy_len < 256 ? g_memcpy_len : 255);
    
    node->type = depth;
    
    /* Recursive construction */
    char child_data[256];
    __builtin_memcpy(child_data, base_data, 128);
    child_data[127] = (char)depth;
    
    node->left = build_ast(depth - 1, child_data);
    node->right = build_ast(depth - 1, child_data + 64);
    
    return node;
}

/* Function with goto control flow */
static void process_with_goto(struct ast_node* src, struct ast_node* dst) {
    int use_memmove = 1;
    
    if (src && dst) {
        /* Jump into memory operation block */
        goto mem_operation;
    }
    
    skip_operation:
        return;
    
    mem_operation:
        if (use_memmove) {
            /* This tests flow-sensitive RTL handling */
            __builtin_memmove(dst->data, src->data, g_memmove_len);
        }
        goto skip_operation;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(struct ast_node** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Thread-specific memory operations */
                volatile char local_buf[512];
                
                /* Mix of builtins */
                __builtin_memset(local_buf, tid, 
                                g_memset_len < 512 ? g_memset_len : 511);
                
                __builtin_memcpy(nodes[i]->data + 128, local_buf, 128);
                
                if (i > 0 && nodes[i-1]) {
                    __builtin_memmove(nodes[i]->data, 
                                     nodes[i-1]->data, 64);
                }
            }
        }
    }
}

/* Compute verification hash */
static unsigned long compute_hash(struct ast_node* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    for (int i = 0; i < 256 && node->data[i]; i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    hash += compute_hash(node->left);
    hash += compute_hash(node->right);
    
    return hash;
}

int main(void) {
    const int NUM_NODES = 8;
    struct ast_node* nodes[NUM_NODES];
    
    /* Initialize with builtins */
    char init_data[256];
    __builtin_memset(init_data, 'A', sizeof(init_data));
    
    /* Build AST structures */
    for (int i = 0; i < NUM_NODES; i++) {
        init_data[i] = (char)('A' + i);
        nodes[i] = build_ast(3 + (i % 3), init_data);
    }
    
    /* Test goto flow control */
    for (int i = 1; i < NUM_NODES; i++) {
        process_with_goto(nodes[i-1], nodes[i]);
    }
    
    /* Parallel memory operations */
    parallel_memory_ops(nodes, NUM_NODES);
    
    /* Additional direct builtin calls */
    volatile char final_buffer[1024];
    __builtin_memset(final_buffer, 0xCC, 
                     g_memset_len < 1024 ? g_memset_len : 1023);
    
    __builtin_memcpy(final_buffer + 512, final_buffer, 256);
    __builtin_memmove(final_buffer + 768, final_buffer + 256, 128);
    
    /* Compute and print verification result */
    unsigned long total_hash = 0;
    for (int i = 0; i < NUM_NODES; i++) {
        total_hash ^= compute_hash(nodes[i]);
    }
    
    printf("Verification hash: 0x%08lx\n", total_hash);
    
    /* Cleanup */
    for (int i = 0; i < NUM_NODES; i++) {
        free(nodes[i]);
    }
    
    return (int)(total_hash & 0x7FFFFFFF);
}
