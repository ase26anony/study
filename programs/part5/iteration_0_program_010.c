/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "test"
};
static const int g_token_count = 6;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    volatile char init_buf[32];
    /* Force __builtin_memset in constructor */
    __builtin_memset(init_buf, 0xAA, sizeof(init_buf));
    
    /* Initialize global with volatile memory operation */
    volatile int* p = (volatile int*)&g_use_hwasan;
    __builtin_memset((void*)p, 0, sizeof(g_use_hwasan));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[16];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with __builtin_memset */
    __builtin_memset(node->data, id, sizeof(node->data));
    node->id = id;
    
    /* Create left child with different pattern */
    node->left = create_ast(depth - 1, id * 2);
    if (node->left) {
        /* Copy data between nodes using __builtin_memcpy */
        __builtin_memcpy(node->data + 32, node->left->data, 32);
    }
    
    /* Create right child */
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    return node;
}

/* Function with goto statements and __builtin_memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst, int mode) {
    char temp_buf[128];
    int state = 0;
    
    /* Jump into block with memory operation */
    if (mode == 0) goto memmove_block;
    
    normal_path:
    __builtin_memset(temp_buf, 0xCC, sizeof(temp_buf));
    state = 1;
    goto exit;
    
    memmove_block:
    /* This tests flow-sensitivity of asan_memfn_rtls retrieval */
    if (src && dst) {
        __builtin_memmove(dst->data, src->data, sizeof(src->data));
    }
    
    /* Jump out to different context */
    if (state == 0) goto normal_path;
    
    exit:
    /* Final memory operation */
    volatile char exit_buf[64];
    __builtin_memcpy(exit_buf, temp_buf, sizeof(temp_buf));
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_operations(void) {
    const int num_threads = 4;
    char thread_buffers[4][256];
    int results[4] = {0};
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread uses different builtins */
        switch (tid % 3) {
            case 0:
                __builtin_memset(thread_buffers[tid], tid, g_mem_size);
                break;
            case 1:
                if (tid > 0) {
                    __builtin_memcpy(thread_buffers[tid], 
                                   thread_buffers[tid-1], 
                                   g_mem_size / 2);
                }
                break;
            case 2:
                __builtin_memmove(thread_buffers[tid] + 64,
                                thread_buffers[tid],
                                g_mem_size - 64);
                break;
        }
        
        /* Compute checksum */
        for (size_t i = 0; i < g_mem_size; i++) {
            results[tid] += thread_buffers[tid][i];
        }
    }
    
    /* Verify parallel execution */
    volatile int total = 0;
    for (int i = 0; i < num_threads; i++) {
        total += results[i];
    }
}

/* Multi-stage initialization with varied memory operations */
static int initialize_system(void) {
    volatile int stage = 0;
    char stage_buffers[3][128];
    int hash = 0;
    
    /* Stage 1: memset */
    stage = 1;
    __builtin_memset(stage_buffers[0], 0x11, sizeof(stage_buffers[0]));
    
    /* Stage 2: memcpy with goto */
    stage = 2;
    goto copy_stage;
    
    skip_copy:
    stage = 3;
    
    /* Stage 3: memmove */
    __builtin_memmove(stage_buffers[2], stage_buffers[1], 
                     sizeof(stage_buffers[1]));
    
    /* Compute final hash */
    for (int i = 0; i < 3; i++) {
        for (size_t j = 0; j < sizeof(stage_buffers[i]); j++) {
            hash = (hash * 31 + stage_buffers[i][j]) & 0xFFFF;
        }
    }
    
    return hash;
    
    copy_stage:
    __builtin_memcpy(stage_buffers[1], stage_buffers[0], 
                    sizeof(stage_buffers[0]));
    goto skip_copy;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* 1. Initialize AST structure */
    ASTNode* root = create_ast(3, 1);
    ASTNode* copy = (ASTNode*)malloc(sizeof(ASTNode));
    
    if (root && copy) {
        /* 2. Test goto with memmove */
        process_with_goto(root, copy, 0);
        process_with_goto(root, copy, 1);
        
        /* 3. OpenMP parallel section */
        parallel_memory_operations();
        
        /* 4. Multi-stage initialization */
        int system_hash = initialize_system();
        printf("System hash: %d\n", system_hash);
        
        /* 5. Token processing with memory operations */
        char token_buffer[512];
        size_t offset = 0;
        
        for (int i = 0; i < g_token_count; i++) {
            size_t len = strlen(g_tokens[i]);
            __builtin_memcpy(token_buffer + offset, g_tokens[i], len);
            offset += len;
            token_buffer[offset++] = ' ';
        }
        
        /* Final verification memset */
        __builtin_memset(token_buffer + offset, 0, 
                        sizeof(token_buffer) - offset);
        
        /* Compute final checksum */
        unsigned long final_sum = 0;
        for (size_t i = 0; i < sizeof(token_buffer); i++) {
            final_sum += token_buffer[i];
        }
        
        printf("Final checksum: %lu\n", final_sum);
        
        /* Cleanup */
        free(copy);
        /* Note: AST cleanup omitted for brevity */
    }
    
    printf("Test completed.\n");
    return 0;
}
