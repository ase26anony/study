/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    size_t data_len;
} ASTNode;

/* Global token array */
static char g_token_pool[4096];
static volatile size_t g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Force initialization of memory functions */
    volatile char buffer[128];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(g_token_pool, buffer, 64);
    
    /* Create non-foldable pattern */
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i * 13 + 7) & 0xFF);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operations */
    volatile char final_check[32];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_recursive(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data with builtin memcpy */
    size_t copy_len = (depth * 7) % 64;
    if (copy_len > 63) copy_len = 63;
    
    __builtin_memcpy(node->data, base_data, copy_len);
    node->data_len = copy_len;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
create_children:
    if (create_left) {
        node->left = create_ast_recursive(depth - 1, base_data + 1);
        create_left = 0;
        goto create_children; /* Jump back to create right child */
    } else {
        node->right = create_ast_recursive(depth - 1, base_data + 2);
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void process_with_goto_flow(ASTNode* node1, ASTNode* node2) {
    if (!node1 || !node2) return;
    
    volatile int stage = 0;
    
stage_0:
    /* First memmove with goto */
    if (node1->data_len > 0 && node2->data_len > 0) {
        size_t move_len = (node1->data_len < node2->data_len) ? 
                          node1->data_len : node2->data_len;
        __builtin_memmove(node2->data, node1->data, move_len);
    }
    stage = 1;
    if (g_use_hwasan) goto stage_2;
    
stage_1:
    /* Memset after conditional jump */
    volatile char temp_buffer[128];
    __builtin_memset(temp_buffer, stage, sizeof(temp_buffer));
    stage = 2;
    
stage_2:
    /* Memcpy with volatile size */
    size_t copy_size = g_mem_size % 128;
    __builtin_memcpy(node1->data + 16, temp_buffer, copy_size);
    
    /* Jump back based on condition */
    if (stage == 2 && node1->left) {
        stage = 0;
        node1 = node1->left;
        goto stage_0;
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    volatile char parallel_buffers[4][256];
    volatile int results[4] = {0};
    
    #pragma omp parallel for
    for (int i = 0; i < 4; i++) {
        /* Each thread uses builtins */
        __builtin_memset(parallel_buffers[i], i + 1, 256);
        
        /* Memcpy between buffers */
        int src_idx = (i + 1) % 4;
        __builtin_memcpy(parallel_buffers[i] + 64, 
                        parallel_buffers[src_idx], 128);
        
        /* Memmove within buffer */
        __builtin_memmove(parallel_buffers[i] + 128,
                         parallel_buffers[i], 64);
        
        /* Compute checksum */
        for (int j = 0; j < 256; j++) {
            results[i] += parallel_buffers[i][j];
        }
    }
    
    /* Verify parallel results */
    volatile int total = 0;
    for (int i = 0; i < 4; i++) {
        total += results[i];
    }
    g_token_pool[0] = (char)(total & 0xFF);
}

/* Multi-stage initialization */
static void initialize_memory_functions(void) {
    /* Force all three builtins to be used */
    volatile char init_buf[512];
    
    /* Stage 1: memset */
    __builtin_memset(init_buf, 0xCC, sizeof(init_buf));
    
    /* Stage 2: memcpy to token pool */
    size_t copy_size = g_mem_size % 512;
    __builtin_memcpy(g_token_pool + 512, init_buf, copy_size);
    
    /* Stage 3: memmove overlapping regions */
    __builtin_memmove(g_token_pool + 256, g_token_pool + 128, 128);
    
    /* Stage 4: nested calls */
    volatile char nested_buf[64];
    __builtin_memset(nested_buf, 0xAA, 32);
    __builtin_memcpy(nested_buf + 32, nested_buf, 32);
    __builtin_memmove(nested_buf, nested_buf + 16, 48);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize memory functions */
    initialize_memory_functions();
    
    /* Phase 2: Create recursive structures */
    ASTNode* root1 = create_ast_recursive(4, g_token_pool);
    ASTNode* root2 = create_ast_recursive(3, g_token_pool + 128);
    
    if (!root1 || !root2) {
        fprintf(stderr, "Failed to create AST structures\n");
        return 1;
    }
    
    /* Phase 3: Execute goto-based flow */
    process_with_goto_flow(root1, root2);
    
    /* Phase 4: Parallel operations */
    parallel_memory_operations();
    
    /* Phase 5: Additional builtin usage in loops */
    for (int i = 0; i < 3; i++) {
        volatile char loop_buf[96];
        __builtin_memset(loop_buf, i * 50, sizeof(loop_buf));
        
        if (i == 1) {
            __builtin_memcpy(loop_buf + 32, g_token_pool + i * 64, 64);
        } else if (i == 2) {
            __builtin_memmove(loop_buf, loop_buf + 16, 80);
        }
        
        /* Update global token */
        g_token_pool[i * 32] = loop_buf[0];
    }
    
    /* Compute final verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        hash = (hash * 31 + g_token_pool[i]) & 0xFFFFFFFF;
    }
    
    printf("Test completed. Final hash: 0x%08lx\n", hash);
    printf("Token pool[0-3]: 0x%02x 0x%02x 0x%02x 0x%02x\n",
           (unsigned char)g_token_pool[0],
           (unsigned char)g_token_pool[1],
           (unsigned char)g_token_pool[2],
           (unsigned char)g_token_pool[3]);
    
    /* Cleanup */
    free(root1);
    free(root2);
    
    return 0;
}
