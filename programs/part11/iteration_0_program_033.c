/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    volatile char buffer[128];
    /* Force __builtin_memset initialization */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    g_mem_size = sizeof(buffer) / 2;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile int cleanup_flag = 1;
    /* Use __builtin_memmove in destructor */
    int src = 0xDEADBEEF;
    int dst;
    __builtin_memmove(&dst, &src, sizeof(int));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy with volatile size */
    size_t copy_size = g_mem_size % 128;
    __builtin_memcpy(node->data, base_data, copy_size);
    
    /* Fill remainder with __builtin_memset */
    if (copy_size < sizeof(node->data)) {
        __builtin_memset(node->data + copy_size, depth, 
                        sizeof(node->data) - copy_size);
    }
    
    node->size = copy_size;
    node->left = create_ast(depth - 1, base_data);
    node->right = create_ast(depth - 1, base_data + 1);
    
    return node;
}

/* Function with goto flow control */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    volatile int condition = 1;
    
    if (condition) goto copy_block;
    
    /* Dead code that might still be analyzed */
    __builtin_memset(dst->data, 0, sizeof(dst->data));
    
copy_block:
    /* Jump target with __builtin_memmove */
    size_t move_size = g_mem_size % 128;
    __builtin_memmove(dst->data, src->data, move_size);
    
    if (condition) goto finish;
    
    /* Another memory operation after goto */
    __builtin_memcpy(src->data + 10, dst->data + 10, 16);
    
finish:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        volatile char local_buf[256];
        int thread_id = omp_get_thread_num();
        
        /* Each thread uses different builtins */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(local_buf, thread_id, sizeof(local_buf));
                break;
            case 1:
                __builtin_memcpy(local_buf + 64, local_buf, 64);
                break;
            case 2:
                __builtin_memmove(local_buf + 128, local_buf + 32, 64);
                break;
        }
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Collective operation */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            volatile int temp[4];
            __builtin_memset(temp, i, sizeof(temp));
        }
    }
}

/* Multi-stage initialization */
static int stage_initializer(void) {
    static volatile int stage = 0;
    volatile char stage_buffer[512];
    
    switch (stage++) {
        case 0:
            __builtin_memset(stage_buffer, 0x11, 256);
            break;
        case 1:
            __builtin_memcpy(stage_buffer + 256, stage_buffer, 128);
            break;
        case 2:
            __builtin_memmove(stage_buffer, stage_buffer + 128, 256);
            break;
    }
    
    return stage;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize AST structures */
    const char* base_pattern = "AST_NODE_DATA_PATTERN_0123456789_ABCDEF";
    ASTNode* root = create_ast(3, base_pattern);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create destination node */
    ASTNode* dest = (ASTNode*)malloc(sizeof(ASTNode));
    if (!dest) {
        free(root);
        return 1;
    }
    
    /* Test goto flow control with memory ops */
    process_with_goto(root, dest);
    
    /* Execute parallel operations */
    parallel_memory_ops();
    
    /* Multi-stage processing */
    int total_stages = 0;
    for (int i = 0; i < 5; i++) {
        total_stages += stage_initializer();
    }
    
    /* Complex memory operation sequence */
    volatile char final_buffer[1024];
    volatile size_t sizes[] = {32, 64, 128, 256};
    
    for (int i = 0; i < 4; i++) {
        size_t current_size = sizes[i] % 512;
        
        /* Alternate between builtins */
        switch (i % 3) {
            case 0:
                __builtin_memset(final_buffer, i, current_size);
                break;
            case 1:
                __builtin_memcpy(final_buffer + current_size, 
                               final_buffer, current_size / 2);
                break;
            case 2:
                __builtin_memmove(final_buffer, 
                                final_buffer + current_size / 2, 
                                current_size);
                break;
        }
    }
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        hash = (hash * 31) + final_buffer[i];
    }
    
    /* Also hash AST data */
    for (size_t i = 0; i < sizeof(root->data); i++) {
        hash = (hash * 31) + root->data[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Total stages executed: %d\n", total_stages);
    
    /* Cleanup */
    free(root->left);
    free(root->right);
    free(root);
    free(dest);
    
    printf("Test completed successfully.\n");
    return 0;
}
