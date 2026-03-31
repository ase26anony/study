/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)((i * 13) & 0xFF);
    }
    
    /* Use builtins in constructor */
    __builtin_memset(token_pool + 1024, 0xAA, 128);
    __builtin_memcpy(token_pool + 1152, token_pool, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Clear sensitive data */
    __builtin_memset(token_pool, 0, sizeof(token_pool));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = depth;
    
    /* Use builtin memcpy with volatile length */
    int copy_len = volatile_len % 128;
    if (copy_len > 255) copy_len = 255;
    
    __builtin_memcpy(node->data, base_data, copy_len);
    node->data[copy_len] = '\0';
    
    /* Create pattern in data */
    __builtin_memset(node->data + copy_len, depth, 32);
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = volatile_flag;
        
        if (use_goto) {
            /* Jump into block with memmove */
            goto create_left;
        }
        
        node->left = create_ast(depth - 1, node->data);
        node->right = NULL;
        return node;
        
    create_left:
        /* This block contains memmove with goto entry */
        char temp[256];
        __builtin_memmove(temp, node->data, 128);
        __builtin_memmove(node->data + 64, temp, 64);
        node->left = create_ast(depth - 2, node->data);
        
        goto create_right;
    }
    
    node->left = NULL;
    node->right = NULL;
    return node;
    
create_right:
    node->right = create_ast(depth - 1, node->data + 64);
    return node;
}

/* Complex memory operation with OpenMP */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[512];
        char shared_buf[1024];
        
        /* Initialize with builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        /* Copy between buffers with volatile control */
        int copy_size = (volatile_len + thread_id * 16) % 256;
        
        if (thread_id % 2 == 0) {
            __builtin_memcpy(shared_buf + thread_id * 64, local_buf, copy_size);
        } else {
            __builtin_memmove(shared_buf + thread_id * 64, local_buf, copy_size);
        }
        
        /* Additional memset in parallel region */
        #pragma omp single
        {
            __builtin_memset(shared_buf + 768, 0xFF, 256);
        }
    }
}

/* Function with goto jumping around memmove */
static void goto_memmove_test(void) {
    char buffer_a[256];
    char buffer_b[256];
    int i = 0;
    
    /* Initialize buffers */
    for (i = 0; i < 256; i++) {
        buffer_a[i] = (char)i;
        buffer_b[i] = (char)(255 - i);
    }
    
    /* Complex goto pattern */
    if (volatile_flag) {
        goto middle_block;
    }
    
start_block:
    __builtin_memcpy(buffer_a, buffer_b, 128);
    goto end_test;
    
middle_block:
    /* Jump into this block containing memmove */
    __builtin_memmove(buffer_a + 64, buffer_b, 128);
    
    if (volatile_len > 32) {
        goto start_block;
    }
    
    __builtin_memset(buffer_a, 0, 64);
    
end_test:
    /* Final memmove */
    __builtin_memmove(buffer_b, buffer_a, 256);
}

/* Main test execution */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Recursive AST creation with memory ops */
    ASTNode* root = create_ast(5, token_pool);
    
    if (root) {
        /* Traverse and process AST */
        ASTNode* nodes[32];
        int node_count = 0;
        ASTNode* current = root;
        
        /* Collect nodes with goto */
        collect_nodes:
        if (current && node_count < 32) {
            nodes[node_count++] = current;
            
            /* Copy between node data */
            if (current->left) {
                __builtin_memcpy(current->left->data + 128, 
                               current->data, 64);
            }
            
            if (current->right) {
                __builtin_memmove(current->right->data,
                                current->left ? current->left->data : current->data,
                                96);
            }
            
            current = current->left;
            goto collect_nodes;
        }
        
        /* Process nodes */
        for (int i = 0; i < node_count; i++) {
            __builtin_memset(nodes[i]->data + 192, i, 32);
        }
        
        /* Cleanup */
        for (int i = 0; i < node_count; i++) {
            free(nodes[i]);
        }
    }
    
    /* Phase 2: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Phase 3: Goto flow test */
    goto_memmove_test();
    
    /* Phase 4: Direct builtin calls with volatile parameters */
    char final_buffer[1024];
    volatile int dynamic_size = volatile_len % 512;
    
    __builtin_memset(final_buffer, 0xCC, dynamic_size);
    __builtin_memcpy(final_buffer + 256, token_pool + 512, 256);
    __builtin_memmove(final_buffer + 512, final_buffer, 256);
    
    /* Verify by computing checksum */
    unsigned long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += (unsigned char)final_buffer[i];
    }
    
    printf("Test completed. Checksum: %lu\n", checksum);
    printf("If compiled with -fsanitize=address or -fsanitize=kernel-hwaddress,\n");
    printf("ASAN should have redirected memcpy/memset/memmove builtins.\n");
    
    return 0;
}
