/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static const char* tokens[] = {"memcpy", "memset", "memmove", "test", "data"};
static const size_t num_tokens = sizeof(tokens)/sizeof(tokens[0]);

/* Constructor/destructor functions */
__attribute__((constructor)) static void init_globals(void) {
    g_init_flag = 1;
    printf("Constructor: Global initialization complete\n");
}

__attribute__((destructor)) static void cleanup_globals(void) {
    printf("Destructor: Program cleanup\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_node(const char* src, size_t len) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Use __builtin_memcpy with volatile length */
    volatile size_t copy_len = len < 63 ? len : 63;
    __builtin_memcpy(node->data, src, copy_len);
    node->data[copy_len] = '\0';
    node->size = copy_len;
    
    return node;
}

static void copy_node_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Complex control flow with goto */
    volatile int use_memmove = (dest->data < src->data + src->size && 
                               dest->data + dest->size > src->data);
    
    if (use_memmove) {
        goto use_move;
    }
    
    /* Normal memcpy path */
    __builtin_memcpy(dest->data, src->data, src->size);
    dest->size = src->size;
    goto done;
    
use_move:
    /* Memmove path with overlapping regions */
    __builtin_memmove(dest->data, src->data, src->size);
    dest->size = src->size;
    
done:
    return;
}

/* Parallel memory dispatch logic */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char buffer[512];
        char src_buffer[512];
        
        /* Initialize source with pattern */
        for (size_t i = 0; i < 512; i++) {
            src_buffer[i] = (char)((i + thread_id) % 256);
        }
        
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            volatile size_t op_size = g_mem_size + i;
            
            /* Mix different builtins */
            if (i % 3 == 0) {
                __builtin_memcpy(buffer, src_buffer, op_size % 512);
            } else if (i % 3 == 1) {
                __builtin_memset(buffer, i, op_size % 512);
            } else {
                /* Create overlapping scenario for memmove */
                __builtin_memmove(buffer + 10, buffer, op_size % 500);
            }
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Initialize AST structure */
    ASTNode* root = create_node(tokens[0], strlen(tokens[0]));
    ASTNode* child1 = create_node(tokens[1], strlen(tokens[1]));
    ASTNode* child2 = create_node(tokens[2], strlen(tokens[2]));
    
    if (!root || !child1 || !child2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    root->left = child1;
    root->right = child2;
    
    /* Phase 2: Perform memory copies between nodes */
    copy_node_data(child1, root);
    copy_node_data(child2, child1);
    
    /* Phase 3: Parallel memory operations */
    parallel_memory_ops();
    
    /* Phase 4: Complex array operations with builtins */
    char* dynamic_buf = (char*)malloc(1024);
    char* dynamic_buf2 = (char*)malloc(1024);
    
    if (dynamic_buf && dynamic_buf2) {
        volatile size_t dyn_size = g_mem_size * 2;
        
        /* Chain of memory operations */
        __builtin_memset(dynamic_buf, 0xAA, dyn_size);
        __builtin_memcpy(dynamic_buf2, dynamic_buf, dyn_size);
        __builtin_memmove(dynamic_buf + 100, dynamic_buf, dyn_size - 100);
        
        /* Verify with checksum */
        unsigned long sum = 0;
        for (size_t i = 0; i < dyn_size; i++) {
            sum += (unsigned char)dynamic_buf[i];
        }
        printf("Checksum: %lu\n", sum);
    }
    
    /* Cleanup */
    free(dynamic_buf);
    free(dynamic_buf2);
    free(child1);
    free(child2);
    free(root);
    
    printf("Test completed successfully\n");
    return 0;
}
