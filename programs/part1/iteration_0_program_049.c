/* ISO C99-compliant test program for ASAN/HWASAN built-in redirection */
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
static char g_tokens[][32] = {
    "token1", "token2", "token3", "token4",
    "token5", "token6", "token7", "token8"
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    volatile char init_buf[128];
    
    /* Force builtin initialization in constructor */
    __builtin_memset(init_buf, 0xAA, sizeof(init_buf));
    __builtin_memcpy(init_buf + 64, g_tokens[0], 32);
    
    printf("Constructor: ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[64];
    
    /* Force builtin in destructor */
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
    
    printf("Destructor: ASAN environment cleaned up\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Use all three builtins with volatile control */
    volatile size_t copy_size = 32;
    
    /* memset pattern based on depth */
    __builtin_memset(node->data, depth % 256, sizeof(node->data));
    
    /* memcpy token into node */
    __builtin_memcpy(node->data, g_tokens[node->id % 8], copy_size);
    
    /* Create children with goto-controlled flow */
    int use_goto = (depth % 3 == 0);
    
    if (use_goto) {
        goto create_left;
    }
    
    node->left = create_ast(depth - 1, counter);
    
create_left:
    /* Jump target for goto */
    if (!use_goto) {
        node->right = create_ast(depth - 1, counter);
    } else {
        /* memmove with goto context */
        char temp[64];
        __builtin_memcpy(temp, node->data, sizeof(temp));
        __builtin_memmove(node->data, temp + 16, 32);
        node->right = NULL;
    }
    
    return node;
}

/* Function with complex control flow and goto */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int do_copy = 1;
    
    if (src->id % 2 == 0) {
        goto skip_memcpy;
    }
    
    /* This memcpy should be redirected */
    __builtin_memcpy(dst->data, src->data, g_mem_size % 64);
    
skip_memcpy:
    /* Jump target */
    if (do_copy) {
        /* memmove with goto context */
        char buffer[128];
        __builtin_memcpy(buffer, src->data, 64);
        __builtin_memmove(dst->data + 16, buffer, 48);
    }
    
    /* Another goto back */
    if (dst->id > 100) {
        goto finalize;
    }
    
    /* memset after goto */
    __builtin_memset(dst->data + 32, 0xCC, 16);
    
finalize:
    return;
}

/* Parallel processing function */
static void parallel_memory_operations(void) {
    volatile char buffers[4][256];
    volatile int indices[4] = {0, 64, 128, 192};
    
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        volatile size_t op_size = g_mem_size / 4;
        
        /* Each thread uses different builtins */
        switch (tid % 3) {
            case 0:
                __builtin_memset(buffers[tid] + indices[tid], tid, op_size);
                break;
            case 1:
                __builtin_memcpy(buffers[tid] + indices[tid], 
                               buffers[(tid + 1) % 4] + indices[(tid + 1) % 4],
                               op_size);
                break;
            case 2:
                __builtin_memmove(buffers[tid] + indices[tid],
                                buffers[(tid + 2) % 4] + indices[(tid + 2) % 4],
                                op_size);
                break;
        }
    }
}

/* Calculate hash of AST */
static unsigned long long hash_ast(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long long hash = 0;
    volatile char* data = node->data;
    
    /* Use builtins in hash calculation */
    for (int i = 0; i < 64; i += 16) {
        char block[16];
        __builtin_memcpy(block, data + i, 16);
        
        for (int j = 0; j < 16; j++) {
            hash = (hash * 31) + block[j];
        }
    }
    
    hash += hash_ast(node->left);
    hash += hash_ast(node->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Initialize counters */
    int node_counter = 0;
    
    /* Create recursive AST */
    ASTNode* root = create_ast(4, &node_counter);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    printf("Created AST with %d nodes\n", node_counter);
    
    /* Process with goto control flow */
    process_with_goto(root, root->left ? root->left : root);
    
    /* Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Calculate and print verification hash */
    unsigned long long hash = hash_ast(root);
    printf("AST hash: %llu\n", hash);
    
    /* Additional builtin calls in main */
    volatile char final_buffer[512];
    volatile size_t final_size = g_mem_size * 2;
    
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, root->data, 64);
    __builtin_memmove(final_buffer + 128, final_buffer, 64);
    
    /* Force all three builtins one more time */
    char temp[256];
    __builtin_memcpy(temp, final_buffer, 128);
    __builtin_memset(final_buffer + 256, 0xAA, 128);
    __builtin_memmove(final_buffer, temp, 128);
    
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real code, you'd want to free the AST properly */
    
    return 0;
}
