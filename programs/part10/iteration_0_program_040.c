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
    for (int i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i * 31) & 0xFF);
    }
    
    /* Force early built-in usage in constructor */
    char local_buf[128];
    __builtin_memset(local_buf, 0xAA, sizeof(local_buf));
    __builtin_memcpy(&g_token_pool[512], local_buf, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operations in destructor */
    char verify_buf[256];
    __builtin_memset(verify_buf, 0, sizeof(verify_buf));
    __builtin_memcpy(verify_buf, g_token_pool, 256);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Use all three built-ins with volatile control */
    size_t copy_size = g_mem_size % 64;
    __builtin_memset(node->data, node->id, sizeof(node->data));
    
    /* Conditional goto for flow sensitivity */
    if (depth > 3) {
        goto skip_memmove;
    }
    
    /* This memmove should be instrumented */
    char temp[64];
    __builtin_memcpy(temp, node->data, copy_size);
    __builtin_memmove(node->data, temp, copy_size);
    
skip_memmove:
    /* Jump back to normal flow */
    if (depth % 2 == 0) {
        /* Another memcpy with goto */
        goto do_copy;
    }
    
    node->left = create_ast(depth - 1, counter);
    node->right = create_ast(depth - 1, counter);
    
    return node;
    
do_copy:
    /* Copy between different parts of the structure */
    __builtin_memcpy(&node->data[32], &node->data[0], 32);
    node->left = create_ast(depth - 1, counter);
    node->right = create_ast(depth - 1, counter);
    return node;
}

/* Function with complex control flow */
static void process_with_gotos(ASTNode* node) {
    if (!node) return;
    
    volatile int flag = node->id % 3;
    
    switch (flag) {
        case 0:
            goto case_zero;
        case 1:
            goto case_one;
        default:
            goto case_default;
    }
    
case_zero:
    {
        char buffer[128];
        /* Built-in in goto target block */
        __builtin_memset(buffer, 0xCC, sizeof(buffer));
        __builtin_memcpy(node->data, buffer, 64);
        goto cleanup;
    }
    
case_one:
    {
        /* Different memory operation pattern */
        char src[96];
        __builtin_memset(src, node->id, sizeof(src));
        __builtin_memmove(node->data, src, 48);
        goto cleanup;
    }
    
case_default:
    {
        /* Overlapping copy */
        __builtin_memmove(&node->data[16], &node->data[0], 48);
        goto cleanup;
    }
    
cleanup:
    /* Process children */
    process_with_gotos(node->left);
    process_with_gotos(node->right);
}

/* OpenMP parallel section */
static void parallel_memory_operations(void) {
    int i;
    char parallel_buffers[8][256];
    
    #pragma omp parallel for private(i)
    for (i = 0; i < 8; i++) {
        /* Each thread uses built-ins */
        volatile size_t local_size = g_mem_size / (i + 1);
        
        __builtin_memset(parallel_buffers[i], i, sizeof(parallel_buffers[i]));
        
        if (i % 2 == 0) {
            __builtin_memcpy(&parallel_buffers[i][128], 
                           &parallel_buffers[(i + 1) % 8][0], 
                           local_size % 128);
        } else {
            __builtin_memmove(&parallel_buffers[i][64],
                            &parallel_buffers[i][0],
                            local_size % 64);
        }
        
        /* Copy back to global pool */
        #pragma omp critical
        {
            __builtin_memcpy(&g_token_pool[i * 256],
                           parallel_buffers[i],
                           256);
        }
    }
}

/* Multi-stage initialization */
static void initialize_stages(void) {
    /* Stage 1: Direct built-in calls */
    char stage1[512];
    __builtin_memset(stage1, 0xDE, sizeof(stage1));
    __builtin_memcpy(&g_token_pool[1024], stage1, 256);
    
    /* Stage 2: Volatile-controlled operations */
    volatile size_t stage2_size = 192;
    char stage2[256];
    __builtin_memset(stage2, 0xAD, stage2_size);
    __builtin_memmove(&stage2[64], &stage2[0], 128);
    
    /* Stage 3: Nested calls */
    char stage3[384];
    __builtin_memcpy(stage3, stage2, 192);
    __builtin_memset(&stage3[192], 0xBE, 192);
}

/* Main execution flow */
int main(void) {
    int counter = 0;
    unsigned long hash = 0;
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: AST creation and processing */
    ASTNode* root = create_ast(5, &counter);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    process_with_gotos(root);
    
    /* Phase 2: OpenMP parallel operations */
    #ifdef _OPENMP
    parallel_memory_operations();
    #endif
    
    /* Phase 3: Multi-stage initialization */
    initialize_stages();
    
    /* Phase 4: Final memory operations */
    char final_buffer[1024];
    volatile size_t final_size = g_mem_size;
    
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, g_token_pool, final_size % 1024);
    __builtin_memmove(&final_buffer[512], &final_buffer[0], 512);
    
    /* Compute verification hash */
    for (int i = 0; i < 1024; i++) {
        hash = (hash * 31) + (unsigned long)final_buffer[i];
    }
    
    printf("Verification hash: 0x%016lx\n", hash);
    printf("AST nodes created: %d\n", counter);
    
    /* Cleanup */
    /* Note: In real code, you'd need to properly free the AST */
    
    return 0;
}
