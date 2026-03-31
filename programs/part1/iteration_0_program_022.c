/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
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
    size_t size;
} ASTNode;

/* Global token array */
static char g_token_pool[1024];
static volatile size_t g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_sanitizer_hook(void) {
    /* Force initialization of sanitizer runtime */
    volatile char dummy[16];
    __builtin_memset(dummy, 0, sizeof(dummy));
    g_token_idx = 0;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_sanitizer_hook(void) {
    /* Final memory operation to ensure coverage */
    volatile char final_buf[32];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* src, size_t depth) {
    if (depth > 3) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy with volatile size */
    volatile size_t copy_size = sizeof(node->data) - 1;
    if (copy_size > 64) copy_size = 64;
    
    __builtin_memcpy(node->data, src, copy_size);
    node->data[copy_size] = '\0';
    
    /* Initialize with __builtin_memset */
    __builtin_memset(&node->size, 0, sizeof(node->size));
    node->size = copy_size;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
copy_left:
    if (create_left) {
        char left_data[64];
        __builtin_memset(left_data, 'L', sizeof(left_data));
        node->left = create_ast_node(left_data, depth + 1);
        create_left = 0;
        goto copy_right;  /* Jump to create right child */
    }
    
copy_right:
    {
        char right_data[64];
        __builtin_memset(right_data, 'R', sizeof(right_data));
        node->right = create_ast_node(right_data, depth + 1);
    }
    
    return node;
}

/* Memory operation between AST nodes */
static void copy_ast_data(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    volatile size_t op_size = dest->size;
    if (src->size < op_size) op_size = src->size;
    
    /* Use __builtin_memmove for overlapping regions */
    if (dest->data + 10 == src->data) {
        __builtin_memmove(dest->data, src->data, op_size);
    } else {
        __builtin_memcpy(dest->data, src->data, op_size);
    }
}

/* Parallel memory dispatch logic */
static size_t parallel_memory_ops(void) {
    size_t total_hash = 0;
    char buffers[4][128];
    
    #pragma omp parallel reduction(+:total_hash)
    {
        int tid = 0;
        #ifdef _OPENMP
        tid = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        volatile size_t local_size = g_mem_size / (tid + 2);
        
        /* Use all three builtins in parallel regions */
        __builtin_memset(buffers[tid], tid, local_size);
        
        if (tid > 0) {
            __builtin_memcpy(buffers[tid] + 32, buffers[tid-1], local_size/2);
        }
        
        /* Circular buffer with memmove */
        char temp[64];
        __builtin_memcpy(temp, buffers[tid], 64);
        __builtin_memmove(buffers[tid], buffers[tid] + 32, 64);
        __builtin_memcpy(buffers[tid] + 64, temp, 64);
        
        /* Compute hash */
        for (size_t i = 0; i < local_size && i < 128; i++) {
            total_hash += (size_t)buffers[tid][i];
        }
    }
    
    return total_hash;
}

/* Token array initialization with memory operations */
static void init_token_array(void) {
    volatile size_t init_size = sizeof(g_token_pool);
    
    /* Clear with memset */
    __builtin_memset(g_token_pool, 0, init_size);
    
    /* Fill pattern with memcpy */
    char pattern[32];
    for (int i = 0; i < 32; i++) {
        pattern[i] = (char)(i * 7);
    }
    
    for (size_t offset = 0; offset < init_size; offset += 32) {
        size_t remaining = init_size - offset;
        volatile size_t copy_len = (remaining > 32) ? 32 : remaining;
        
        /* Jump into copy block */
        if (offset % 64 == 0) {
            goto do_copy;
        }
        
        continue_copy:
        continue;
        
    do_copy:
        __builtin_memcpy(g_token_pool + offset, pattern, copy_len);
        goto continue_copy;
    }
    
    /* Final overlapping memmove */
    __builtin_memmove(g_token_pool + 16, g_token_pool, 256);
}

/* Main execution flow */
int main(void) {
    size_t final_result = 0;
    
    /* Initialize token array with builtins */
    init_token_array();
    
    /* Create recursive AST structure */
    ASTNode* root = create_ast_node("ROOT", 0);
    if (root && root->left) {
        copy_ast_data(root, root->left);
    }
    
    /* Execute parallel memory operations */
    final_result = parallel_memory_ops();
    
    /* Additional memory operations in main */
    char main_buffer[512];
    volatile size_t buf_size = sizeof(main_buffer);
    
    __builtin_memset(main_buffer, 0xAA, buf_size);
    
    /* Overlapping region operation */
    __builtin_memmove(main_buffer + 128, main_buffer, 256);
    
    /* Copy from token pool */
    __builtin_memcpy(main_buffer + 384, g_token_pool, 128);
    
    /* Compute final verification hash */
    for (size_t i = 0; i < buf_size; i++) {
        final_result += (size_t)main_buffer[i];
    }
    
    if (root) {
        for (size_t i = 0; i < root->size && i < 64; i++) {
            final_result += (size_t)root->data[i];
        }
        free(root->left);
        free(root->right);
        free(root);
    }
    
    /* Print result for verification */
    printf("Result: %zu\n", final_result % 1000000);
    
    return 0;
}
