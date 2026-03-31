/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_token_array[1024];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Initialize token array with pattern */
    for (int i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)((i * 31) & 0xFF);
    }
    
    /* Force early built-in usage in constructor */
    volatile char local_buf[32];
    __builtin_memset(local_buf, 0xAA, sizeof(local_buf));
    __builtin_memcpy(&g_token_array[0], local_buf, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operations in destructor */
    volatile char final_buf[64];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with built-ins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy token data using memcpy */
    size_t copy_size = (size_t)(g_mem_size % 64);
    if (copy_size > 0) {
        __builtin_memcpy(node->data, 
                        &g_token_array[g_token_index], 
                        copy_size);
        g_token_index = (g_token_index + 32) % sizeof(g_token_array);
    }
    
    node->id = id;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 3) {
        goto skip_left;
    }
    
    create_left = 1;
    
skip_left:
    if (create_left) {
        node->left = create_ast_node(depth - 1, id * 2);
    } else {
        node->left = NULL;
    }
    
    /* Use goto to jump around memory operations */
    if (depth % 2 == 0) {
        goto create_right;
    }
    
    /* Intermediate memory move operation */
    char temp_buf[32];
    __builtin_memmove(temp_buf, node->data, 16);
    __builtin_memcpy(node->data + 16, temp_buf, 16);
    
create_right:
    node->right = create_ast_node(depth - 2, id * 2 + 1);
    
    return node;
}

/* Complex memory operation with goto jumps */
static void perform_memory_operations(volatile char* dest, volatile char* src) {
    volatile char intermediate[128];
    volatile int operation = 1;
    
    /* Jump table simulation with goto */
    if (operation == 1) {
        goto op_memset;
    } else {
        goto op_memcpy;
    }

op_memset:
    /* Force memset redirection */
    __builtin_memset(intermediate, 0xCC, (size_t)g_mem_size % 128);
    operation = 2;
    goto op_memmove;

op_memcpy:
    __builtin_memcpy(intermediate, src, 64);
    goto finish;

op_memmove:
    /* Complex memmove with overlapping regions */
    __builtin_memmove((void*)dest, intermediate, 64);
    __builtin_memmove(intermediate + 32, intermediate, 32);
    goto op_memcpy;

finish:
    /* Final memory copy */
    __builtin_memcpy((void*)dest, intermediate, 64);
}

/* OpenMP parallel memory dispatcher */
static void parallel_memory_dispatch(void) {
    volatile char parallel_buffers[4][256];
    volatile int results[4] = {0};
    
    #pragma omp parallel for
    for (int i = 0; i < 4; i++) {
        /* Each thread uses built-ins independently */
        __builtin_memset(parallel_buffers[i], i * 0x11, 256);
        
        /* Memory move between buffers */
        if (i > 0) {
            __builtin_memmove(parallel_buffers[i], 
                            parallel_buffers[i-1], 
                            128);
        }
        
        /* Compute simple hash */
        for (int j = 0; j < 256; j++) {
            results[i] += parallel_buffers[i][j];
        }
    }
    
    /* Verify parallel execution */
    volatile int total = 0;
    for (int i = 0; i < 4; i++) {
        total += results[i];
    }
    
    /* Use result to prevent dead code elimination */
    g_token_array[0] = (char)(total & 0xFF);
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = create_ast_node(5, 1);
    
    if (root) {
        /* Copy between AST nodes */
        volatile char node_copy[64];
        __builtin_memcpy(node_copy, root->data, sizeof(node_copy));
        
        if (root->left) {
            __builtin_memmove(root->left->data, node_copy, 32);
        }
        
        /* Free AST recursively */
        ASTNode* nodes[32];
        int node_count = 0;
        nodes[node_count++] = root;
        
        while (node_count > 0) {
            ASTNode* current = nodes[--node_count];
            if (current->left) nodes[node_count++] = current->left;
            if (current->right) nodes[node_count++] = current->right;
            free(current);
        }
    }
    
    /* Phase 2: Complex memory operations with goto */
    volatile char src_buffer[512];
    volatile char dst_buffer[512];
    
    /* Initialize source with pattern */
    for (int i = 0; i < 512; i++) {
        src_buffer[i] = (char)(i % 256);
    }
    
    perform_memory_operations(dst_buffer, src_buffer);
    
    /* Phase 3: OpenMP parallel section */
    parallel_memory_dispatch();
    
    /* Phase 4: Final verification */
    volatile int final_hash = 0;
    for (int i = 0; i < sizeof(g_token_array); i += 64) {
        char block_hash = 0;
        __builtin_memcpy(&block_hash, &g_token_array[i], 1);
        
        /* Use all three built-ins in verification */
        char temp[16];
        __builtin_memset(temp, block_hash, 16);
        __builtin_memcpy(&g_token_array[i], temp, 16);
        __builtin_memmove(&g_token_array[i+8], &g_token_array[i], 8);
        
        final_hash += block_hash;
    }
    
    printf("Test completed. Final hash: %d\n", final_hash);
    printf("Built-in redirection should have been triggered for:\n");
    printf("  - __builtin_memcpy\n");
    printf("  - __builtin_memset\n");
    printf("  - __builtin_memmove\n");
    
    return final_hash & 0xFF;
}
