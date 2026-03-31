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
    uint32_t hash;
} ASTNode;

/* Global token array */
static char g_token_pool[4096];
static volatile size_t g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Initialize token pool with pattern */
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i * 31) & 0xFF);
    }
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Clear sensitive data */
    __builtin_memset(g_token_pool, 0, sizeof(g_token_pool));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using builtin memcpy with volatile length */
    size_t copy_len = g_mem_size % 64;
    if (copy_len > 0) {
        __builtin_memcpy(node->data, base_data, copy_len);
    }
    
    /* Create hash using memory operations */
    uint32_t hash = 0;
    for (size_t i = 0; i < copy_len; i++) {
        hash = (hash * 31) + node->data[i];
    }
    node->hash = hash;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, node->data);
        node->right = NULL;
        goto skip_right;
        
    create_children:
        node->left = create_ast(depth - 2, node->data);
        node->right = create_ast(depth - 2, node->data + 32);
        
        /* Move data between nodes using builtin memmove */
        if (node->left && node->right) {
            size_t move_len = (g_mem_size % 32) + 1;
            __builtin_memmove(node->right->data + 16, 
                            node->left->data, 
                            move_len);
        }
    skip_right:
        ; /* Empty statement for label */
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_operations(ASTNode* nodes[], size_t count) {
    #pragma omp parallel
    {
        #pragma omp for
        for (size_t i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Use all three builtins in parallel */
                char buffer[128];
                
                /* memset */
                __builtin_memset(buffer, i & 0xFF, sizeof(buffer));
                
                /* memcpy from node */
                size_t len = (nodes[i]->hash % 64) + 1;
                __builtin_memcpy(buffer + 32, nodes[i]->data, len);
                
                /* memmove within buffer */
                __builtin_memmove(buffer, buffer + 32, len);
                
                /* Update node hash */
                nodes[i]->hash ^= *(uint32_t*)buffer;
            }
        }
        
        /* Additional parallel section with goto */
        #pragma omp single
        {
            int do_jump = 1;
            
            if (do_jump) {
                goto parallel_cleanup;
            }
            
            /* This won't execute due to goto */
            __builtin_memset(g_token_pool, 0, 16);
            
        parallel_cleanup:
            /* Clear thread-local buffers */
            char local_buf[64];
            __builtin_memset(local_buf, 0, sizeof(local_buf));
        }
    }
}

/* Complex initialization with multiple stages */
static void initialize_token_array(void) {
    volatile size_t stage = 0;
    
    while (stage < 3) {
        switch (stage) {
            case 0:
                /* Stage 1: Pattern initialization */
                for (size_t i = 0; i < 1024; i += 64) {
                    __builtin_memset(g_token_pool + i, stage, 64);
                }
                break;
                
            case 1:
                /* Stage 2: Overwrite with memcpy */
                char pattern[64];
                __builtin_memset(pattern, 0xAA, sizeof(pattern));
                
                for (size_t i = 64; i < 1024; i += 128) {
                    __builtin_memcpy(g_token_pool + i, pattern, 
                                   sizeof(pattern));
                }
                break;
                
            case 2:
                /* Stage 3: Rearrange with memmove */
                for (size_t i = 0; i < 960; i += 192) {
                    __builtin_memmove(g_token_pool + i + 64,
                                    g_token_pool + i,
                                    64);
                }
                break;
        }
        stage++;
    }
}

/* Main execution flow */
int main(void) {
    /* Wait for constructor */
    while (!g_init_flag) {
        volatile int spin = 0;
        spin++; /* Prevent empty loop optimization */
    }
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize complex structures */
    initialize_token_array();
    
    /* Create AST tree */
    ASTNode* root = create_ast(5, g_token_pool);
    
    /* Create node array for parallel processing */
    ASTNode* node_array[8] = {0};
    
    /* Fill array with nodes at different depths */
    for (int i = 0; i < 8; i++) {
        char seed[32];
        size_t offset = (i * 137) % 2048; /* Prime-based offset */
        __builtin_memcpy(seed, g_token_pool + offset, 32);
        node_array[i] = create_ast(3 + (i % 3), seed);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_operations(node_array, 8);
    
    /* Calculate verification hash */
    uint64_t final_hash = 0;
    for (int i = 0; i < 8; i++) {
        if (node_array[i]) {
            final_hash += node_array[i]->hash;
            final_hash = (final_hash << 5) | (final_hash >> 59); /* Rotate */
            
            /* Additional memory operation in main */
            char temp[128];
            __builtin_memset(temp, 0, sizeof(temp));
            __builtin_memcpy(temp, node_array[i]->data, 32);
            __builtin_memmove(node_array[i]->data, temp + 16, 16);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        if (node_array[i]) {
            /* Clear node data before free */
            __builtin_memset(node_array[i]->data, 0, 64);
            free(node_array[i]);
        }
    }
    
    if (root) {
        __builtin_memset(root->data, 0, 64);
        free(root);
    }
    
    printf("Test completed. Final hash: 0x%016llx\n", 
           (unsigned long long)final_hash);
    
    return (final_hash != 0) ? 0 : 1;
}
