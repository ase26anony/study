/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    uint32_t hash;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force builtin initialization early */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_late(void) {
    volatile char final_check[8];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static uint32_t process_ast(ASTNode* node, int depth) {
    if (!node || depth <= 0) return 0;
    
    uint32_t hash = 0;
    
    /* Use goto to create control flow complexity */
    if (depth % 3 == 0) goto copy_block;
    
    /* Normal memset path */
    __builtin_memset(node->data, depth, sizeof(node->data));
    hash += depth * 31;
    
copy_block:
    /* This label is jumped into */
    {
        volatile char temp[64];
        __builtin_memcpy(temp, node->data, sizeof(node->data));
        
        /* Conditional memmove with goto out */
        if (g_use_memmove) {
            __builtin_memmove(node->data + 16, node->data, 32);
            goto hash_calc;
        }
    }
    
    /* Alternative path */
    __builtin_memcpy(node->data + 8, node->data, 24);
    
hash_calc:
    /* Calculate hash from data */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        hash = (hash << 5) - hash + node->data[i];
    }
    
    /* Recursive processing */
    if (node->left) {
        hash ^= process_ast(node->left, depth - 1);
    }
    if (node->right) {
        hash += process_ast(node->right, depth - 1);
    }
    
    node->hash = hash;
    return hash;
}

/* OpenMP parallel memory operations */
static void parallel_mem_operations(void) {
    #pragma omp parallel
    {
        volatile char local_buf[128];
        volatile int thread_id = 0;
        
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread uses different builtins */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(local_buf, thread_id, g_mem_size % sizeof(local_buf));
                break;
            case 1:
                __builtin_memcpy(local_buf + 32, local_buf, 64);
                break;
            case 2:
                __builtin_memmove(local_buf + 16, local_buf + 8, 48);
                break;
        }
        
        /* Barrier with memory operation */
        #pragma omp barrier
        
        /* All threads do memmove after barrier */
        if (thread_id % 2 == 0) {
            __builtin_memmove(local_buf + 8, local_buf, 32);
        }
    }
}

/* Complex initialization with volatile control */
static ASTNode* create_ast_tree(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Fill data with pattern */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        node->data[i] = (depth + i) % 256;
    }
    
    /* Create children with memcpy between nodes */
    node->left = create_ast_tree(depth - 1);
    node->right = create_ast_tree(depth - 1);
    
    /* Copy data between nodes if both children exist */
    if (node->left && node->right) {
        volatile size_t copy_len = sizeof(node->data) / 2;
        __builtin_memcpy(node->right->data, node->left->data, copy_len);
        __builtin_memmove(node->left->data + copy_len, node->right->data, copy_len);
    }
    
    return node;
}

/* Free AST tree with memory sanitization */
static void free_ast_tree(ASTNode* node) {
    if (!node) return;
    
    free_ast_tree(node->left);
    free_ast_tree(node->right);
    
    /* Clear memory before free */
    __builtin_memset(node, 0, sizeof(ASTNode));
    free(node);
}

int main(void) {
    uint32_t total_hash = 0;
    
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Phase 1: Initialize and process AST */
    ASTNode* root = create_ast_tree(4);
    if (root) {
        total_hash = process_ast(root, 4);
        printf("AST hash: %u\n", total_hash);
    }
    
    /* Phase 2: OpenMP parallel operations */
    printf("Running parallel memory operations\n");
    parallel_mem_operations();
    
    /* Phase 3: Direct builtin calls with volatile control */
    volatile char buffer1[256], buffer2[256];
    volatile size_t op_size = g_mem_size;
    
    /* Chain of memory operations */
    __builtin_memset(buffer1, 0xCC, op_size % sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, op_size % sizeof(buffer2));
    
    /* Conditional memmove with goto */
    if (op_size > 128) {
        goto do_memmove;
    }
    
    __builtin_memcpy(buffer1 + 64, buffer2, 128);
    goto finalize;
    
do_memmove:
    __builtin_memmove(buffer2 + 32, buffer1, 192);
    
finalize:
    /* Final verification hash */
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        total_hash = (total_hash << 3) ^ buffer1[i] ^ buffer2[i];
    }
    
    /* Cleanup */
    if (root) {
        free_ast_tree(root);
    }
    
    printf("Final hash: %u\n", total_hash);
    printf("Test completed\n");
    
    return (total_hash != 0) ? 0 : 1;
}
