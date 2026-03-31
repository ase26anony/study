/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 256;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 192;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char *data;
    size_t data_len;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Global token array */
static char g_token_array[4096];

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token array with pattern */
    for (size_t i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)((i * 13) & 0xFF);
    }
    printf("Constructor: Token array initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    printf("Destructor: Program cleanup complete\n");
}

/* Recursive parser with memory operations */
static ASTNode* parse_recursive(int depth, const char *src, size_t len) {
    if (depth <= 0 || len == 0) {
        return NULL;
    }
    
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        return NULL;
    }
    
    node->type = depth;
    node->data_len = len / 2;
    node->data = (char*)malloc(node->data_len);
    
    if (node->data) {
        /* Use __builtin_memcpy with volatile length */
        __builtin_memcpy(node->data, src, 
                        (node->data_len < (size_t)g_memcpy_len) ? 
                        node->data_len : (size_t)g_memcpy_len);
        
        /* Use __builtin_memset on part of the data */
        if (node->data_len > 16) {
            __builtin_memset(node->data + 8, 0xAA, 
                           (node->data_len - 8 < (size_t)g_memset_len) ?
                           node->data_len - 8 : (size_t)g_memset_len);
        }
    }
    
    /* Recursive calls with goto for flow control */
    int use_goto = (depth % 3 == 0);
    
    if (use_goto) {
        goto recursive_call;
    }
    
    node->left = parse_recursive(depth - 1, src, len / 3);
    
recursive_call:
    if (use_goto) {
        node->left = parse_recursive(depth - 2, src + len/4, len / 2);
    }
    
    node->right = parse_recursive(depth - 1, src + len/2, len / 2);
    
    return node;
}

/* Complex memory dispatch with OpenMP */
static void parallel_memory_dispatch(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        size_t buffer_size = 1024;
        char *src_buf = (char*)malloc(buffer_size);
        char *dst_buf = (char*)malloc(buffer_size);
        
        if (src_buf && dst_buf) {
            /* Initialize source with pattern */
            for (size_t i = 0; i < buffer_size; i++) {
                src_buf[i] = (char)((i + thread_id * 17) & 0xFF);
            }
            
            /* Use all three builtins with volatile lengths */
            __builtin_memcpy(dst_buf, src_buf, 
                           (buffer_size < (size_t)g_memcpy_len) ? 
                           buffer_size : (size_t)g_memcpy_len);
            
            __builtin_memset(dst_buf + 64, thread_id, 
                           (buffer_size - 64 < (size_t)g_memset_len) ?
                           buffer_size - 64 : (size_t)g_memset_len);
            
            /* memmove with overlapping regions */
            size_t move_len = (size_t)g_memmove_len;
            if (move_len > buffer_size / 2) {
                move_len = buffer_size / 2;
            }
            
            __builtin_memmove(dst_buf + buffer_size/4, dst_buf, move_len);
            
            /* Verify with regular memcmp (not builtin) */
            if (memcmp(src_buf, dst_buf, 16) == 0) {
                #pragma omp atomic
                g_memcpy_len++;
            }
        }
        
        free(src_buf);
        free(dst_buf);
    }
}

/* Free AST recursively */
static void free_ast(ASTNode *node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    if (node->data) {
        /* Clear sensitive data before free */
        __builtin_memset(node->data, 0, node->data_len);
        free(node->data);
    }
    free(node);
}

/* Main execution flow */
int main(void) {
    printf("=== ASAN/HWASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Recursive parsing with memory ops */
    ASTNode *root = parse_recursive(5, g_token_array, sizeof(g_token_array));
    
    /* Phase 2: Parallel memory operations */
    parallel_memory_dispatch();
    
    /* Phase 3: Additional builtin usage with goto */
    char buffer1[512], buffer2[512];
    int use_memmove = 1;
    
    __builtin_memset(buffer1, 0xCC, sizeof(buffer1));
    
    if (use_memmove) {
        goto do_memmove;
    }
    
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1) / 2);
    
do_memmove:
    if (use_memmove) {
        /* Force memmove path */
        __builtin_memmove(buffer2, buffer1, 
                         sizeof(buffer1) < (size_t)g_memmove_len ? 
                         sizeof(buffer1) : (size_t)g_memmove_len);
    }
    
    /* Phase 4: Compute verification hash */
    unsigned long hash = 0;
    if (root && root->data) {
        for (size_t i = 0; i < root->data_len && i < 256; i++) {
            hash = (hash * 31) + (unsigned char)root->data[i];
        }
    }
    
    /* Include buffer contents in hash */
    for (size_t i = 0; i < sizeof(buffer2) && i < 256; i++) {
        hash = (hash * 17) + (unsigned char)buffer2[i];
    }
    
    printf("Result hash: 0x%08lx\n", hash & 0xFFFFFFFF);
    printf("Mem lengths used: memcpy=%zu, memset=%zu, memmove=%zu\n",
           (size_t)g_memcpy_len, (size_t)g_memset_len, (size_t)g_memmove_len);
    
    /* Cleanup */
    free_ast(root);
    
    return 0;
}
