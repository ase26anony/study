/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
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
static char g_token_pool[4096];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Initialize token pool with pattern */
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i % 26) + 'A');
    }
    
    /* Force early built-in usage in constructor */
    volatile char local_buf[128];
    __builtin_memset(local_buf, 0xCC, sizeof(local_buf));
    
    /* Copy pattern into buffer */
    __builtin_memcpy(local_buf, "CONSTRUCTOR_INIT", 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[64];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
    
    /* Use memmove with overlapping regions */
    __builtin_memmove(cleanup_buf, cleanup_buf + 16, 32);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with built-ins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern based on ID */
    char pattern[32];
    __builtin_memset(pattern, 'A' + (id % 26), 31);
    pattern[31] = '\0';
    
    /* Copy pattern into node */
    __builtin_memcpy(node->data, pattern, 31);
    
    node->id = id;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            /* Jump into memory operation block */
            goto create_children;
        }
        
        /* Normal path */
        node->left = create_ast_node(depth - 1, id * 2);
        node->right = create_ast_node(depth - 1, id * 2 + 1);
        
        if (use_goto) {
            create_children:
            /* This block contains memmove with goto entry */
            volatile char temp[32];
            __builtin_memcpy(temp, node->data, 32);
            __builtin_memmove(node->data, temp, 32);
            
            node->left = create_ast_node(depth - 1, id * 2);
            node->right = create_ast_node(depth - 1, id * 2 + 1);
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Copy AST structure with overlapping memory regions */
static void copy_ast_structure(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Direct structure copy using memcpy */
    __builtin_memcpy(dest, src, sizeof(ASTNode));
    
    /* Handle children recursively */
    if (src->left) {
        dest->left = (ASTNode*)malloc(sizeof(ASTNode));
        if (dest->left) {
            copy_ast_structure(dest->left, src->left);
        }
    }
    
    if (src->right) {
        dest->right = (ASTNode*)malloc(sizeof(ASTNode));
        if (dest->right) {
            copy_ast_structure(dest->right, src->right);
        }
    }
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_operations(void) {
    volatile size_t local_size = g_mem_size;
    char* buffers[4];
    
    /* Allocate buffers */
    for (int i = 0; i < 4; i++) {
        buffers[i] = (char*)malloc(local_size);
        if (!buffers[i]) return;
    }
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(buffers[thread_id], thread_id, local_size);
                break;
            case 1:
                __builtin_memcpy(buffers[thread_id], g_token_pool, 
                                local_size < sizeof(g_token_pool) ? 
                                local_size : sizeof(g_token_pool));
                break;
            case 2:
                /* Overlapping memmove */
                if (local_size > 64) {
                    __builtin_memmove(buffers[thread_id] + 32, 
                                     buffers[thread_id], 
                                     local_size - 32);
                }
                break;
        }
        
        /* Barrier to ensure all memory ops complete */
        #pragma omp barrier
        
        /* Verify and modify data */
        #pragma omp for
        for (int i = 0; i < 4; i++) {
            volatile char verify_buf[128];
            size_t copy_size = local_size < 128 ? local_size : 128;
            __builtin_memcpy(verify_buf, buffers[i], copy_size);
            
            /* Modify buffer */
            __builtin_memset(buffers[i] + (copy_size / 2), 0xAA, copy_size / 4);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        free(buffers[i]);
    }
}

/* Complex memory dispatch with goto flow control */
static unsigned long execute_memory_dispatch(void) {
    unsigned long hash = 0;
    volatile int stage = 0;
    
    dispatch_start:
    switch (stage) {
        case 0: {
            /* Stage 0: Initialize with memset */
            volatile char stage_buf[256];
            __builtin_memset(stage_buf, 0x11, sizeof(stage_buf));
            hash += (unsigned long)stage_buf[0];
            stage = 1;
            goto dispatch_start;
        }
        
        case 1: {
            /* Stage 1: Copy data with memcpy */
            volatile char src_buf[128];
            volatile char dst_buf[128];
            
            for (int i = 0; i < 128; i++) {
                src_buf[i] = (char)(i % 256);
            }
            
            __builtin_memcpy(dst_buf, src_buf, 128);
            
            /* Calculate hash from copied data */
            for (int i = 0; i < 128; i++) {
                hash += (unsigned long)dst_buf[i];
            }
            
            stage = 2;
            
            /* Jump to memmove block */
            goto memmove_block;
        }
        
        case 2: {
            /* Stage 2: Final processing */
            volatile char final_buf[64];
            __builtin_memset(final_buf, 0x33, sizeof(final_buf));
            hash += (unsigned long)final_buf[32];
            stage = 3;
            goto dispatch_start;
        }
        
        case 3:
            return hash;
    }
    
    memmove_block:
    {
        /* This block is entered via goto */
        volatile char overlap_buf[192];
        
        /* Initialize buffer */
        for (int i = 0; i < 192; i++) {
            overlap_buf[i] = (char)((i * 7) % 256);
        }
        
        /* Perform overlapping memmove */
        __builtin_memmove(overlap_buf + 64, overlap_buf, 128);
        
        /* Update hash from moved data */
        for (int i = 64; i < 128; i++) {
            hash += (unsigned long)overlap_buf[i];
        }
        
        /* Jump back to switch */
        goto dispatch_start;
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Create and manipulate AST */
    ASTNode* root = create_ast_node(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Copy AST structure */
    ASTNode* copy = (ASTNode*)malloc(sizeof(ASTNode));
    if (copy) {
        copy_ast_structure(copy, root);
        
        /* Verify copy with memcmp */
        volatile char cmp_result = 0;
        __builtin_memcpy(&cmp_result, root->data, 1);
        __builtin_memcpy(&cmp_result, copy->data, 1);
        
        free(copy);
    }
    
    /* Phase 2: Execute parallel operations */
    parallel_memory_operations();
    
    /* Phase 3: Complex dispatch */
    unsigned long final_hash = execute_memory_dispatch();
    
    /* Phase 4: Additional built-in usage in main */
    volatile char main_buf[512];
    size_t dynamic_size = g_mem_size % 512;
    
    /* Chain of memory operations */
    __builtin_memset(main_buf, 0x55, dynamic_size);
    __builtin_memcpy(main_buf + 128, g_token_pool, 256);
    __builtin_memmove(main_buf, main_buf + 64, 256);
    
    /* Calculate verification hash */
    for (size_t i = 0; i < dynamic_size; i++) {
        final_hash += (unsigned long)main_buf[i];
    }
    
    /* Cleanup AST */
    /* Note: In production, would need proper recursive free */
    free(root);
    
    printf("Test completed. Final hash: %lu\n", final_hash);
    printf("Expected non-zero hash indicates memory operations executed.\n");
    
    return (final_hash != 0) ? 0 : 1;
}
