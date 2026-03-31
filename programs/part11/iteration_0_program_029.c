/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
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
static void init_asan_early(void) {
    volatile char buffer[128];
    /* Force builtin calls in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 32, g_tokens[0], 32);
    
    printf("[Constructor] Initialized ASAN early buffers\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    volatile char final_buf[64];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
    printf("[Destructor] Cleaned up ASAN buffers\n");
}

/* Recursive parser with memory operations */
static ASTNode* parse_tokens(int depth, int token_idx) {
    if (depth <= 0 || token_idx >= 8) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = depth * 100 + token_idx;
    
    /* Copy token data with builtin memcpy */
    size_t copy_len = strlen(g_tokens[token_idx]) + 1;
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    __builtin_memcpy(node->data, g_tokens[token_idx], copy_len);
    
    /* Recursive calls */
    node->left = parse_tokens(depth - 1, token_idx + 1);
    node->right = parse_tokens(depth - 1, token_idx + 2);
    
    /* Conditional memmove between nodes */
    if (node->left && node->right && g_use_memmove) {
        volatile char temp[256];
        __builtin_memcpy(temp, node->left->data, sizeof(temp));
        __builtin_memmove(node->left->data, node->right->data, sizeof(node->left->data));
        __builtin_memmove(node->right->data, temp, sizeof(node->right->data));
    }
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(ASTNode* node) {
    if (!node) return;
    
    volatile char local_buf[128];
    int state = 0;
    
    /* Jump into memory operation block */
    goto setup;
    
mem_operation:
    /* This block contains builtin memmove */
    __builtin_memmove(local_buf + 32, local_buf, 64);
    state = 1;
    goto finish;
    
setup:
    __builtin_memset(local_buf, 0xCC, sizeof(local_buf));
    __builtin_memcpy(local_buf, node->data, strlen(node->data));
    
    if (node->id % 2 == 0) {
        goto mem_operation;
    }
    
finish:
    /* Copy back with builtin memcpy */
    __builtin_memcpy(node->data + 64, local_buf, 32);
}

/* Parallel memory dispatcher */
static unsigned long parallel_memory_ops(ASTNode* nodes[], int count) {
    unsigned long total_hash = 0;
    
    #pragma omp parallel reduction(+:total_hash)
    {
        int tid = omp_get_thread_num();
        volatile char thread_buf[256];
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Each thread uses builtins */
                __builtin_memset(thread_buf, tid, sizeof(thread_buf));
                __builtin_memcpy(thread_buf + 128, nodes[i]->data, 128);
                
                /* Conditional memmove based on thread ID */
                if (tid % 3 == 0) {
                    __builtin_memmove(thread_buf, thread_buf + 64, 128);
                }
                
                /* Compute simple hash */
                for (int j = 0; j < 256; j++) {
                    total_hash += (unsigned long)thread_buf[j];
                }
            }
        }
    }
    
    return total_hash;
}

/* Main execution flow */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Create AST tree */
    ASTNode* root = parse_tokens(4, 0);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Array of nodes for parallel processing */
    ASTNode* node_array[16];
    node_array[0] = root;
    
    /* Collect nodes recursively */
    int idx = 1;
    ASTNode* stack[32];
    int top = 0;
    stack[top++] = root;
    
    while (top > 0 && idx < 16) {
        ASTNode* current = stack[--top];
        if (current->left) {
            node_array[idx++] = current->left;
            stack[top++] = current->left;
        }
        if (current->right && idx < 16) {
            node_array[idx++] = current->right;
            stack[top++] = current->right;
        }
    }
    
    /* Process with goto edge cases */
    for (int i = 0; i < idx; i++) {
        process_with_goto(node_array[i]);
    }
    
    /* Execute parallel memory operations */
    unsigned long hash_result = parallel_memory_ops(node_array, idx);
    printf("Parallel hash result: %lu\n", hash_result);
    
    /* Final builtin operations */
    volatile char final_buffer[512];
    size_t dynamic_size = g_mem_size * 2;
    
    __builtin_memset(final_buffer, 0xAB, sizeof(final_buffer));
    __builtin_memcpy(final_buffer + 128, root->data, 128);
    __builtin_memmove(final_buffer + 256, final_buffer + 128, 128);
    
    /* Verify by printing last byte */
    printf("Verification byte: 0x%02X\n", (unsigned char)final_buffer[511]);
    
    /* Cleanup */
    /* Note: Real cleanup would need recursive free, omitted for brevity */
    
    printf("=== Test completed ===\n");
    return 0;
}
