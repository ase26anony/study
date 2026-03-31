/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 256;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_asan_early(void) {
    volatile char buffer[16];
    /* Force early builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 8, buffer, 8);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_asan_late(void) {
    volatile char final_buf[32];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile lengths */
    __builtin_memset(node, 0, sizeof(*node));
    node->type = depth;
    
    /* Fill data with pattern */
    for (int i = 0; i < 31; i++) {
        node->data[i] = 'A' + (depth + i) % 26;
    }
    node->data[31] = '\0';
    
    /* Recursive creation */
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 2);
    
    /* Copy between nodes if both exist */
    if (node->left && node->right) {
        size_t copy_len = g_memcpy_len % 32;
        __builtin_memcpy(node->right->data, node->left->data, copy_len);
    }
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(ASTNode* node) {
    if (!node) return;
    
    volatile char temp[64];
    int state = 0;
    
    /* Jump into memory operation block */
    goto entry_point;
    
memory_block:
    /* This block contains builtin_memmove with goto control flow */
    __builtin_memmove(temp + 16, temp, 32);
    goto after_move;
    
entry_point:
    /* Initialize buffer */
    __builtin_memset(temp, 0xCC, sizeof(temp));
    
    if (node->type % 2) {
        goto memory_block;
    } else {
        /* Alternative path */
        __builtin_memcpy(temp, node->data, 31);
    }
    
after_move:
    /* Process result */
    for (int i = 0; i < 32; i++) {
        temp[i] ^= 0x55;
    }
    
    /* Jump out of scope */
    if (node->type > 5) {
        goto finalize;
    }
    
    /* Another memory operation */
    __builtin_memmove(node->data, temp, 16);
    
finalize:
    return;
}

/* OpenMP parallel section */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        volatile char thread_buf[128];
        
        /* Each thread uses builtins */
        __builtin_memset(thread_buf, tid, sizeof(thread_buf));
        
        #pragma omp barrier
        
        /* Memory move between positions */
        size_t len = (g_memmove_len + tid) % 64;
        __builtin_memmove(thread_buf + 64, thread_buf, len);
        
        #pragma omp critical
        {
            /* Inter-thread copy simulation */
            static volatile char shared_buf[256];
            __builtin_memcpy(shared_buf + tid * 32, thread_buf, 32);
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Create recursive structure */
    ASTNode* root = create_ast(6);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process with goto edge cases */
    process_with_goto(root);
    
    /* Execute parallel memory operations */
    #ifdef _OPENMP
    parallel_memory_ops();
    #endif
    
    /* Complex memory operation sequence */
    volatile char* buffers[4];
    for (int i = 0; i < 4; i++) {
        buffers[i] = (volatile char*)malloc(512);
        if (buffers[i]) {
            __builtin_memset(buffers[i], i * 0x40, 512);
        }
    }
    
    /* Chain memory operations */
    if (buffers[0] && buffers[1] && buffers[2]) {
        __builtin_memcpy(buffers[1], buffers[0], g_memcpy_len % 256);
        __builtin_memmove(buffers[2], buffers[1], g_memmove_len % 128);
        __builtin_memset(buffers[3], 0x7F, g_memset_len % 512);
    }
    
    /* Calculate verification hash */
    uint32_t hash = 0;
    for (int i = 0; i < 4; i++) {
        if (buffers[i]) {
            for (int j = 0; j < 64; j++) {
                hash = (hash * 31) + buffers[i][j];
            }
            free((void*)buffers[i]);
        }
    }
    
    /* Cleanup AST */
    free(root);
    
    printf("Test completed. Hash: 0x%08X\n", hash);
    return 0;
}
