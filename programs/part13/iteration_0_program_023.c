/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 96;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Global token array */
static char g_token_array[4096];

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force initialization of builtin redirection cache early */
    char local_buf[32];
    __builtin_memset(local_buf, 0xA5, sizeof(local_buf));
    __builtin_memcpy(local_buf + 16, local_buf, 16);
    
    /* Initialize token array with pattern */
    for (size_t i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)(i % 256);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Final memory operations in destructor */
    volatile char final_buf[64];
    __builtin_memset((void*)final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* src) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use all three builtins with volatile lengths */
    __builtin_memset(node, 0, sizeof(*node));
    node->type = depth;
    
    /* Copy data with goto-based control flow */
    size_t copy_len = (size_t)g_memcpy_len;
    if (copy_len > 256) copy_len = 256;
    
    /* Jump into memory operation block */
    goto copy_block;
    
copy_block:
    __builtin_memcpy(node->data, src, copy_len);
    goto after_copy;
    
after_copy:
    /* Create recursive children with memmove between buffers */
    char temp_buf[512];
    __builtin_memcpy(temp_buf, node->data, copy_len);
    
    /* Use memmove with overlapping regions */
    __builtin_memmove(node->data + 32, node->data, copy_len > 32 ? 32 : copy_len);
    
    /* Recursive calls */
    node->left = create_ast(depth - 1, temp_buf);
    node->right = create_ast(depth - 1, temp_buf + 128);
    
    return node;
}

/* Calculate hash of AST */
static unsigned long hash_ast(const ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    for (size_t i = 0; i < sizeof(node->data); i++) {
        hash = ((hash << 5) + hash) + (unsigned long)node->data[i];
    }
    
    return hash + hash_ast(node->left) + hash_ast(node->right);
}

/* Free AST with memory clearing */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear memory before free */
    __builtin_memset(node, 0, sizeof(*node));
    free(node);
}

/* Parallel memory dispatch logic */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buffer[1024];
        
        /* Each thread uses all three builtins */
        __builtin_memset(local_buffer, thread_id, sizeof(local_buffer));
        
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            char temp[256];
            size_t len = (size_t)(g_memcpy_len + i) % 256;
            
            /* Force builtin calls with varying parameters */
            __builtin_memcpy(temp, g_token_array + i * 8, len);
            __builtin_memset(temp + len, i, 128 - len);
            
            /* Overlapping memmove */
            if (len > 64) {
                __builtin_memmove(temp + 32, temp, 64);
            }
        }
        
        /* Thread-private memory operations */
        volatile char thread_private[64];
        __builtin_memcpy((void*)thread_private, local_buffer + thread_id * 16, 64);
        __builtin_memset(local_buffer + thread_id * 16, 0, 64);
    }
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Phase 1: Create and process AST */
    ASTNode* root = create_ast(4, g_token_array);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    unsigned long hash = hash_ast(root);
    printf("AST hash: %lu\n", hash);
    
    /* Phase 2: Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Phase 3: Additional builtin calls with goto */
    char final_buffer[512];
    volatile int use_memmove = 1;
    
    __builtin_memset(final_buffer, 0xCC, sizeof(final_buffer));
    
    if (use_memmove) {
        goto do_memmove;
    }
    
do_memmove:
    __builtin_memmove(final_buffer + 128, final_buffer, 256);
    goto after_final;
    
after_final:
    /* Verify operations with checksum */
    unsigned long checksum = 0;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        checksum += (unsigned long)final_buffer[i];
    }
    printf("Final buffer checksum: %lu\n", checksum);
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
