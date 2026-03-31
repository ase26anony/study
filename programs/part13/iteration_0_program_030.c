/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_token_pool[4096];
static volatile int g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize token pool with pattern */
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i % 26) + 'A');
    }
    
    /* Force early built-in usage in constructor */
    char local_buf[128];
    __builtin_memset(local_buf, 0xCC, sizeof(local_buf));
    __builtin_memcpy(local_buf, "CONSTRUCTOR_INIT", 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Use built-ins in destructor */
    char cleanup_buf[64];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast_tree(int depth, int* node_id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with built-ins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in data */
    char pattern[32];
    __builtin_memset(pattern, 'N' + depth, 31);
    pattern[31] = '\0';
    __builtin_memcpy(node->data, pattern, 31);
    
    node->id = (*node_id)++;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
create_children:
    node->left = create_ast_tree(depth - 1, node_id);
    
    /* Jump back and forth with goto */
    if (depth % 2 == 0) {
        goto create_right;
    } else {
        node->right = create_ast_tree(depth - 2, node_id);
        goto skip_label;
    }
    
create_right:
    node->right = create_ast_tree(depth - 1, node_id);
    
skip_label:
    return node;
}

/* Function with complex control flow and built-ins */
static void process_ast_nodes(ASTNode* node1, ASTNode* node2) {
    if (!node1 || !node2) return;
    
    volatile int use_memmove = g_use_memmove;
    volatile size_t copy_size = g_mem_size % 128;
    
    /* Complex control flow with goto */
    if (node1->id < node2->id) {
        goto copy_forward;
    } else {
        goto copy_backward;
    }
    
copy_forward:
    {
        char temp_buf[256];
        
        /* Use all three built-ins */
        __builtin_memset(temp_buf, 0, sizeof(temp_buf));
        __builtin_memcpy(temp_buf, node1->data, copy_size);
        
        if (use_memmove) {
            /* Force memmove usage */
            __builtin_memmove(node2->data, temp_buf, copy_size);
            goto process_children;
        } else {
            __builtin_memcpy(node2->data, temp_buf, copy_size);
        }
    }
    goto process_children;
    
copy_backward:
    {
        /* Different memory operation pattern */
        __builtin_memcpy(node2->data + 32, node1->data, copy_size);
        __builtin_memset(node1->data + 16, 0xAA, 48);
    }
    
process_children:
    /* Process children recursively */
    process_ast_nodes(node1->left, node2->left);
    process_ast_nodes(node1->right, node2->right);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char thread_buf[512];
        char thread_buf2[512];
        
        /* Initialize with built-ins */
        __builtin_memset(thread_buf, thread_id, sizeof(thread_buf));
        
        /* Copy between buffers */
        volatile size_t op_size = (g_mem_size + thread_id) % 256;
        __builtin_memcpy(thread_buf2, thread_buf, op_size);
        
        /* Conditional memmove */
        if (thread_id % 3 == 0) {
            __builtin_memmove(thread_buf + 64, thread_buf2 + 32, op_size / 2);
        }
        
        /* Barrier to ensure all threads execute */
        #pragma omp barrier
        
        /* Final memset */
        __builtin_memset(thread_buf, 0xFF, 128);
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize AST trees */
    int node_id1 = 0, node_id2 = 1000;
    ASTNode* tree1 = create_ast_tree(4, &node_id1);
    ASTNode* tree2 = create_ast_tree(4, &node_id2);
    
    if (!tree1 || !tree2) {
        fprintf(stderr, "Failed to create AST trees\n");
        return 1;
    }
    
    /* Process trees with complex control flow */
    process_ast_nodes(tree1, tree2);
    
    /* Execute OpenMP parallel section */
    #ifdef _OPENMP
    parallel_memory_operations();
    #endif
    
    /* Additional built-in usage in main */
    char final_buffer[1024];
    volatile size_t final_size = g_mem_size % 512;
    
    /* Chain of built-in calls */
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, g_token_pool, final_size);
    
    /* Force memmove with overlapping regions */
    __builtin_memmove(final_buffer + 256, final_buffer, final_size / 2);
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        hash = (hash * 31) + (unsigned char)final_buffer[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    /* Note: In real code, you'd want to properly free the tree structures */
    
    return 0;
}
