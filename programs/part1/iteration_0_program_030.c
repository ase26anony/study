/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    int id;
    uint8_t padding[32];  /* Ensure size for memcpy operations */
} ASTNode;

/* Global token array */
static char g_token_array[4096];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    /* Initialize token array with pattern */
    for (int i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)((i * 31) & 0xFF);
    }
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_globals(void) {
    /* Clear sensitive data */
    __builtin_memset(g_token_array, 0, sizeof(g_token_array));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int* node_id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = (*node_id)++;
    
    /* Fill data with pattern using builtin memcpy */
    char pattern[64];
    for (int i = 0; i < 64; i++) {
        pattern[i] = (char)((node->id + i) & 0xFF);
    }
    __builtin_memcpy(node->data, pattern, 64);
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
create_children:
    if (create_left) {
        node->left = create_ast(depth - 1, node_id);
        create_left = 0;
        goto create_children;  /* Jump back to create right child */
    } else {
        node->right = create_ast(depth - 1, node_id);
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void process_ast_with_goto(ASTNode* node, char* buffer) {
    if (!node) return;
    
    volatile int use_memmove = 0;
    
    /* Jump into memory operation block */
    if (node->id % 3 == 0) {
        goto memmove_block;
    } else if (node->id % 3 == 1) {
        goto memcpy_block;
    } else {
        goto memset_block;
    }

memmove_block:
    {
        char temp[128];
        /* Use builtin memmove with overlapping regions */
        __builtin_memcpy(temp, node->data, 64);
        __builtin_memmove(node->data, node->data + 16, 48);
        __builtin_memmove(node->data + 48, temp, 16);
        use_memmove = 1;
        goto next_node;
    }

memcpy_block:
    {
        /* Copy data to buffer using builtin memcpy */
        size_t copy_size = g_mem_size % 64;
        __builtin_memcpy(buffer + g_token_index, node->data, copy_size);
        g_token_index = (g_token_index + copy_size) % 1024;
        goto next_node;
    }

memset_block:
    {
        /* Clear portion of data using builtin memset */
        __builtin_memset(node->data + 32, 0, 32);
        goto next_node;
    }

next_node:
    /* Process children */
    process_ast_with_goto(node->left, buffer);
    process_ast_with_goto(node->right, buffer);
}

/* Parallel memory dispatch function */
static void parallel_memory_dispatch(void) {
    char local_buf[2048];
    volatile size_t op_size = g_mem_size;
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char thread_buf[512];
        
        /* Each thread performs different memory operations */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            switch ((i + thread_id) % 3) {
                case 0:
                    __builtin_memset(thread_buf, thread_id, op_size % 512);
                    break;
                case 1:
                    __builtin_memcpy(thread_buf, g_token_array + (i * 4), 256);
                    break;
                case 2:
                    /* Overlapping memmove */
                    __builtin_memmove(thread_buf + 128, thread_buf, 384);
                    break;
            }
            
            /* Copy to shared buffer with synchronization */
            #pragma omp critical
            {
                __builtin_memcpy(local_buf + (thread_id * 512), 
                               thread_buf, 256);
            }
        }
    }
    
    /* Final consolidation */
    char final_buf[1024];
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    
    for (int i = 0; i < 4; i++) {
        __builtin_memcpy(final_buf + (i * 256), 
                        local_buf + (i * 512), 256);
    }
}

/* Main execution flow */
int main(void) {
    /* Wait for constructor */
    while (!g_init_flag) {}
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create recursive AST */
    int node_id = 0;
    ASTNode* root = create_ast(5, &node_id);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process AST with goto-based control flow */
    char process_buffer[4096];
    __builtin_memset(process_buffer, 0, sizeof(process_buffer));
    process_ast_with_goto(root, process_buffer);
    
    /* Execute parallel memory operations */
    parallel_memory_dispatch();
    
    /* Compute verification hash */
    uint32_t hash = 0;
    for (int i = 0; i < sizeof(g_token_array); i++) {
        hash = (hash * 31) + (uint32_t)g_token_array[i];
    }
    
    /* Additional builtin calls in main */
    char temp[256];
    __builtin_memcpy(temp, g_token_array, 256);
    __builtin_memset(g_token_array + 1024, 0xAA, 512);
    __builtin_memmove(g_token_array + 512, g_token_array + 256, 512);
    
    printf("Test completed. Hash: 0x%08X\n", hash);
    printf("Token index: %d\n", g_token_index);
    
    /* Cleanup */
    /* Recursive free would be here in real implementation */
    
    return 0;
}
