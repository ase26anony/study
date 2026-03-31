/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    uint32_t hash;
};

/* __attribute__((constructor)) forces early initialization */
static void __attribute__((constructor)) init_globals(void) {
    /* Use builtins in constructor to trigger early redirection */
    char buffer[32];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 8);
}

/* __attribute__((destructor)) for cleanup coordination */
static void __attribute__((destructor)) cleanup(void) {
    /* Empty but forces linker coordination */
}

/* Recursive function with memory operations */
static uint32_t process_ast(struct ASTNode* node, int depth) {
    if (!node || depth >= 5) return 0;
    
    uint32_t hash = 0;
    
    /* Conditional goto to test flow sensitivity */
    if (depth & 1) goto process_right;
    
    /* Process left subtree with memcpy */
    if (node->left) {
        /* Use volatile length */
        volatile size_t copy_len = 32;
        __builtin_memcpy(node->data, node->left->data, copy_len);
        hash += process_ast(node->left, depth + 1);
    }
    
    /* Jump label for goto */
process_right:
    /* Process right subtree with memset */
    if (node->right) {
        volatile char fill = 0x55;
        __builtin_memset(node->right->data, fill, 48);
        hash += process_ast(node->right, depth + 1);
    }
    
    /* Jump back from goto */
    if (depth == 2) {
        volatile int use_goto = g_use_memmove;
        if (use_goto) goto finalize;
    }
    
    /* Memmove between node data sections */
    if (node->left && node->right) {
        volatile size_t move_len = g_mem_size % 40;
        if (move_len > 0) {
            __builtin_memmove(node->data + 16, node->left->data + 8, move_len);
        }
    }
    
finalize:
    /* Compute hash from data */
    for (int i = 0; i < 64; i++) {
        hash = (hash * 31) + (uint8_t)node->data[i];
    }
    node->hash = hash;
    return hash;
}

/* OpenMP parallel section with memory operations */
static uint32_t parallel_memory_ops(void) {
    uint32_t total_hash = 0;
    char buffer1[512];
    char buffer2[512];
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 0x11, sizeof(buffer1));
    __builtin_memset(buffer2, 0x22, sizeof(buffer2));
    
    #pragma omp parallel reduction(+:total_hash)
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-specific memory operations */
        char local_buf[128];
        volatile size_t op_size = (thread_id * 32 + 64) % 128;
        
        /* Mix of builtins in parallel region */
        __builtin_memset(local_buf, thread_id, op_size);
        
        if (thread_id % 2 == 0) {
            __builtin_memcpy(buffer1 + thread_id * 16, local_buf, op_size / 2);
        } else {
            __builtin_memmove(buffer2 + thread_id * 8, buffer1 + thread_id * 8, op_size / 4);
        }
        
        /* Compute hash from buffers */
        for (size_t i = 0; i < op_size; i++) {
            total_hash += (uint8_t)local_buf[i];
            total_hash += (uint8_t)buffer1[(thread_id * 16 + i) % 512];
        }
    }
    
    /* Final memmove across buffer boundaries */
    volatile size_t final_move = g_mem_size % 256;
    if (final_move > 128) {
        __builtin_memmove(buffer1 + 256, buffer2, final_move - 128);
    }
    
    return total_hash;
}

/* Complex initialization with nested memory ops */
static struct ASTNode* create_ast_tree(int depth) {
    if (depth >= 4) return NULL;
    
    struct ASTNode* node = (struct ASTNode*)malloc(sizeof(struct ASTNode));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(struct ASTNode));
    
    /* Fill data with pattern */
    volatile char pattern = 0x40 + depth;
    __builtin_memset(node->data, pattern, 64);
    
    /* Create subtrees */
    node->left = create_ast_tree(depth + 1);
    node->right = create_ast_tree(depth + 1);
    
    /* Copy between nodes if both exist */
    if (node->left && node->right) {
        volatile size_t copy_len = 24;
        __builtin_memcpy(node->left->data + 8, node->right->data + 16, copy_len);
    }
    
    return node;
}

/* Free AST tree */
static void free_ast_tree(struct ASTNode* node) {
    if (!node) return;
    free_ast_tree(node->left);
    free_ast_tree(node->right);
    
    /* Clear memory before free */
    volatile char clear = 0;
    __builtin_memset(node, clear, sizeof(struct ASTNode));
    free(node);
}

int main(void) {
    uint32_t final_hash = 0;
    
    printf("Starting ASAN/HWASAN built-in redirection test...\n");
    
    /* Phase 1: AST processing */
    struct ASTNode* root = create_ast_tree(0);
    if (root) {
        final_hash += process_ast(root, 0);
        
        /* Additional memmove between tree nodes */
        if (root->left && root->right) {
            volatile size_t move_size = 32;
            __builtin_memmove(root->left->data, root->right->data, move_size);
            final_hash += process_ast(root, 1);
        }
    }
    
    /* Phase 2: OpenMP parallel operations */
    final_hash += parallel_memory_ops();
    
    /* Phase 3: Direct builtin calls with volatile parameters */
    char final_buffer[1024];
    volatile size_t final_len = g_mem_size % 512;
    volatile char final_fill = 0x77;
    
    __builtin_memset(final_buffer, final_fill, final_len);
    __builtin_memcpy(final_buffer + 128, final_buffer, final_len / 2);
    __builtin_memmove(final_buffer + 256, final_buffer + 64, final_len / 4);
    
    /* Compute final verification hash */
    for (size_t i = 0; i < final_len && i < 512; i++) {
        final_hash = (final_hash * 37) + (uint8_t)final_buffer[i];
    }
    
    /* Cleanup */
    if (root) {
        free_ast_tree(root);
    }
    
    printf("Test completed. Final hash: 0x%08X\n", final_hash);
    return (final_hash != 0) ? 0 : 1;
}
