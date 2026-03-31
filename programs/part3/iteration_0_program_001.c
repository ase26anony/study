/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 16;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[32];
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force early initialization of memory functions */
    char buf1[64], buf2[64];
    __builtin_memset(buf1, 0xAA, sizeof(buf1));
    __builtin_memcpy(buf2, buf1, sizeof(buf1));
    __builtin_memmove(buf1 + 8, buf1, 32);
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_destructor(void) {
    volatile char cleanup_buf[128];
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile-controlled size */
    int copy_len = volatile_len % 32;
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, base_data, copy_len);
    
    node->type = depth;
    node->left = create_ast(depth - 1, base_data);
    node->right = create_ast(depth - 1, base_data + 1);
    
    return node;
}

/* Function with goto jumps around memmove */
static void goto_memmove_test(char* dest, char* src, int len) {
    int i = 0;
    
start_label:
    if (i >= 3) goto end_label;
    
    /* Jump into block with memmove */
    if (i == 1) {
        goto memmove_block;
    }
    
normal_path:
    __builtin_memcpy(dest, src, len);
    i++;
    goto start_label;
    
memmove_block:
    /* This tests flow sensitivity */
    __builtin_memmove(dest + 8, src, len - 8);
    i++;
    goto normal_path;
    
end_label:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char local_buf[256];
        char shared_buf[256];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        #pragma omp critical
        {
            __builtin_memcpy(shared_buf, local_buf, volatile_len);
            __builtin_memmove(shared_buf + 64, shared_buf, 128);
        }
    }
}

/* Multi-stage initialization */
static void initialize_token_array(char tokens[][64], int count) {
    for (int i = 0; i < count; i++) {
        __builtin_memset(tokens[i], 'A' + (i % 26), 64);
        
        /* Conditional memcpy with volatile */
        if (i % 2 == 0) {
            int len = volatile_len % 64;
            __builtin_memcpy(tokens[i] + 32, tokens[(i + 1) % count], len);
        }
    }
}

/* Complex memory dispatch logic */
static unsigned long memory_dispatch_logic(void) {
    unsigned long hash = 0;
    char buffer_pool[4][256];
    
    /* Initialize with different patterns */
    for (int i = 0; i < 4; i++) {
        __builtin_memset(buffer_pool[i], i * 0x40, sizeof(buffer_pool[i]));
    }
    
    /* Chain memory operations */
    for (int i = 0; i < 100; i++) {
        int src_idx = i % 4;
        int dst_idx = (i + 1) % 4;
        int op_len = (volatile_len + i) % 128 + 1;
        
        switch (i % 3) {
            case 0:
                __builtin_memcpy(buffer_pool[dst_idx], 
                               buffer_pool[src_idx], 
                               op_len);
                break;
            case 1:
                __builtin_memset(buffer_pool[dst_idx], 
                               i & 0xFF, 
                               op_len);
                break;
            case 2:
                __builtin_memmove(buffer_pool[dst_idx] + 32,
                                buffer_pool[src_idx],
                                op_len);
                break;
        }
        
        /* Compute simple hash */
        for (int j = 0; j < op_len; j++) {
            hash += buffer_pool[dst_idx][j];
        }
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* 1. Initialize complex token array */
    char tokens[8][64];
    initialize_token_array(tokens, 8);
    
    /* 2. Create recursive AST structure */
    ASTNode* root = create_ast(4, "TestData123");
    
    if (root) {
        /* Copy between AST nodes */
        __builtin_memcpy(root->left->data, root->right->data, 16);
        __builtin_memmove(root->data + 8, root->left->data, 24);
    }
    
    /* 3. Execute goto-based memmove test */
    goto_memmove_test((char*)volatile_dest, (char*)volatile_src, 128);
    
    /* 4. Run parallel memory operations */
    parallel_memory_ops();
    
    /* 5. Execute multi-stage memory dispatch */
    unsigned long result_hash = memory_dispatch_logic();
    
    /* 6. Final verification with all three builtins */
    char final_buf[512];
    __builtin_memset(final_buf, 0xCC, sizeof(final_buf));
    __builtin_memcpy(final_buf + 256, final_buf, 128);
    __builtin_memmove(final_buf, final_buf + 128, 256);
    
    /* Add final builtins to hash */
    for (int i = 0; i < 256; i++) {
        result_hash += final_buf[i];
    }
    
    printf("Test completed. Result hash: %lu\n", result_hash);
    
    /* Cleanup */
    if (root) {
        /* Recursive free - simplified for example */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    return 0;
}
