/* ISO C99-compliant test program for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _OPENMP
#include <omp.h>
#endif

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ast_node {
    struct ast_node* left;
    struct ast_node* right;
    char data[64];
    int value;
    unsigned char padding[32]; /* Extra padding for redzone testing */
} ast_node_t;

/* Global token array */
static unsigned char g_token_pool[4096];
volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token pool with pattern */
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (unsigned char)(i % 256);
    }
    g_init_flag = 1;
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Verify memory wasn't corrupted */
    unsigned char sum = 0;
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        sum ^= g_token_pool[i];
    }
    printf("Destructor: Token pool checksum = 0x%02x\n", sum);
}

/* Recursive function with memory operations */
static ast_node_t* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) {
        return NULL;
    }
    
    ast_node_t* node = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!node) {
        return NULL;
    }
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    
    /* Set data with builtin memcpy */
    char temp_data[64];
    for (int i = 0; i < 64; i++) {
        temp_data[i] = (char)(depth * 64 + i);
    }
    __builtin_memcpy(node->data, temp_data, 64);
    
    /* Use volatile to control recursion */
    volatile int next_depth = depth + 1;
    
    /* Create children with goto for flow control */
    if (next_depth < max_depth) {
        goto create_children;
    } else {
        goto skip_children;
    }
    
create_children:
    node->left = create_ast(next_depth, max_depth);
    node->right = create_ast(next_depth, max_depth);
    goto after_children;
    
skip_children:
    node->left = NULL;
    node->right = NULL;
    
after_children:
    /* Copy between nodes if both children exist */
    if (node->left && node->right) {
        /* Use builtin memmove for overlapping regions */
        __builtin_memmove(node->left->data + 16, node->right->data, 32);
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ast_node_t* node) {
    if (!node) {
        return;
    }
    
    volatile int use_memmove = 1;
    
    if (use_memmove) {
        goto memmove_block;
    } else {
        goto skip_memmove;
    }
    
memmove_block:
    {
        /* This block tests goto into memory operation */
        char buffer[128];
        __builtin_memset(buffer, 0xA5, sizeof(buffer));
        
        /* Overlapping memmove */
        __builtin_memmove(buffer + 32, buffer, 64);
        
        /* Copy to node */
        __builtin_memcpy(node->data, buffer, 64);
        
        goto after_memmove;
    }
    
skip_memmove:
    /* Alternative path */
    __builtin_memset(node->data, 0xFF, 64);
    
after_memmove:
    /* Process children recursively */
    process_with_goto(node->left);
    process_with_goto(node->right);
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_ops(void) {
    #ifdef _OPENMP
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        
        /* Thread-local buffers */
        char local_buf[256];
        char shared_buf[1024];
        
        /* Initialize with builtin memset */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        /* Copy to shared buffer with offset */
        size_t offset = (thread_id * 256) % 768;
        __builtin_memcpy(shared_buf + offset, local_buf, 128);
        
        #pragma omp barrier
        
        /* Move data around in shared buffer */
        if (thread_id == 0) {
            __builtin_memmove(shared_buf + 512, shared_buf, 256);
        }
        
        #pragma omp barrier
        
        /* Verify copy */
        if (thread_id < 3) {
            char verify_buf[128];
            __builtin_memset(verify_buf, 0, sizeof(verify_buf));
            __builtin_memcpy(verify_buf, shared_buf + offset, 128);
        }
    }
    #endif
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Wait for constructor */
    while (!g_init_flag) {
        /* Spin */
    }
    
    /* Create recursive AST */
    ast_node_t* root = create_ast(0, 4);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process with goto jumps */
    process_with_goto(root);
    
    /* Execute parallel operations */
    parallel_memory_ops();
    
    /* Complex token processing */
    unsigned char* dynamic_buf = (unsigned char*)malloc(g_mem_size);
    if (!dynamic_buf) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Multiple builtin calls with volatile sizes */
    volatile size_t copy_size = g_mem_size / 2;
    __builtin_memset(dynamic_buf, 0xCC, g_mem_size);
    __builtin_memcpy(dynamic_buf, g_token_pool, copy_size);
    __builtin_memmove(dynamic_buf + 64, dynamic_buf, 128);
    
    /* Calculate verification hash */
    unsigned long long hash = 0;
    for (size_t i = 0; i < g_mem_size; i++) {
        hash = (hash * 31) + dynamic_buf[i];
    }
    
    printf("Verification hash: 0x%016llx\n", hash);
    
    /* Cleanup */
    free(dynamic_buf);
    
    /* Note: AST cleanup omitted for brevity - would need recursive free */
    
    printf("Test completed successfully\n");
    return 0;
}
