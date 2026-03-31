/* ISO C99-compliant program targeting ASAN built-in redirection logic */
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
static char g_tokens[][32] = {
    "memcpy_test", "memset_test", "memmove_test",
    "recursive", "parallel", "asan_redirect"
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    volatile char buffer[128];
    /* Force built-in initialization with volatile */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    g_use_hwasan = (buffer[0] == 0xAA) ? 0 : 1; /* Non-foldable */
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[64];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_recursive(int depth, const char* token) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use built-in memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data with built-in memcpy */
    size_t token_len = __builtin_strlen(token);
    if (token_len > sizeof(node->data) - 1)
        token_len = sizeof(node->data) - 1;
    
    __builtin_memcpy(node->data, token, token_len);
    node->data[token_len] = '\0';
    node->size = token_len;
    
    /* Create children with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    /* Jump into block with memmove */
    {
        char temp[64];
        __builtin_memcpy(temp, node->data, node->size + 1);
        
        /* Use goto to jump around memmove */
        if (depth % 2 == 0) {
            goto skip_memmove;
        }
        
        /* Force memmove redirection */
        __builtin_memmove(node->data + 10, node->data, 
                         node->size > 10 ? 10 : node->size);
        __builtin_memcpy(node->data, temp, node->size + 1);
        
    skip_memmove:
        node->left = create_ast_recursive(depth - 1, "left_branch");
        node->right = create_ast_recursive(depth - 2, "right_branch");
    }
    
done:
    return node;
}

/* Process AST with memory operations */
static size_t process_ast(ASTNode* node, char* output) {
    if (!node) return 0;
    
    size_t total = 0;
    volatile char local_buf[128];
    
    /* Use volatile to prevent optimization */
    volatile size_t copy_size = node->size;
    if (copy_size > sizeof(local_buf)) 
        copy_size = sizeof(local_buf);
    
    /* Force built-in memcpy with volatile size */
    __builtin_memcpy((void*)local_buf, node->data, copy_size);
    
    /* Complex control flow with goto */
    if (node->left && node->right) {
        goto both_children;
    }
    
    total = copy_size;
    goto continue_processing;
    
both_children:
    {
        /* Memory operation between child nodes */
        char temp[64];
        size_t left_size = node->left->size;
        size_t right_size = node->right->size;
        
        /* Use built-in memcpy with non-constant size */
        __builtin_memcpy(temp, node->left->data, 
                        left_size < sizeof(temp) ? left_size : sizeof(temp));
        
        /* Jump over memmove in some cases */
        if (left_size > right_size) {
            goto skip_interchild_memmove;
        }
        
        /* Force memmove redirection */
        __builtin_memmove(node->right->data + 5, node->right->data,
                         right_size > 5 ? right_size - 5 : 0);
        
    skip_interchild_memmove:
        __builtin_memcpy(node->right->data, temp, 
                        left_size < sizeof(node->right->data) ? 
                        left_size : sizeof(node->right->data));
        
        total = left_size + right_size;
    }
    
continue_processing:
    if (output) {
        __builtin_memcpy(output + total, node->data, 
                        node->size < 32 ? node->size : 32);
    }
    
    total += process_ast(node->left, output ? output + total : NULL);
    total += process_ast(node->right, output ? output + total : NULL);
    
    return total;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    /* Clear data before freeing */
    volatile char clear_buf[64];
    __builtin_memset(clear_buf, 0, sizeof(clear_buf));
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Use memset on node before free */
    __builtin_memset(node, 0xDD, sizeof(ASTNode));
    free(node);
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        volatile char thread_buf[256];
        volatile size_t buf_size = g_mem_size;
        
        /* Each thread uses built-ins */
        __builtin_memset((void*)thread_buf, thread_id, buf_size);
        
        #pragma omp barrier
        
        /* Memcpy between buffers with goto */
        volatile char dest_buf[256];
        
        if (thread_id % 3 == 0) {
            goto use_memcpy;
        } else if (thread_id % 3 == 1) {
            goto use_memmove;
        } else {
            goto use_memset;
        }
        
    use_memcpy:
        __builtin_memcpy((void*)dest_buf, thread_buf, buf_size);
        goto done;
        
    use_memmove:
        {
            /* Create overlapping regions for memmove */
            volatile char overlap_buf[512];
            __builtin_memcpy((void*)overlap_buf, thread_buf, buf_size);
            __builtin_memmove((void*)(overlap_buf + 128), overlap_buf, buf_size);
            __builtin_memcpy((void*)dest_buf, overlap_buf + 128, buf_size);
        }
        goto done;
        
    use_memset:
        __builtin_memset((void*)dest_buf, 0xFF, buf_size);
        
    done:
        /* Verify with volatile read */
        volatile char check = dest_buf[0];
        (void)check; /* Suppress unused warning */
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Initialize token processing */
    char result_buffer[4096] = {0};
    size_t total_processed = 0;
    
    /* Create AST for each token */
    for (size_t i = 0; i < sizeof(g_tokens)/sizeof(g_tokens[0]); i++) {
        ASTNode* root = create_ast_recursive(4, g_tokens[i]);
        if (root) {
            size_t processed = process_ast(root, 
                result_buffer + total_processed);
            total_processed += processed;
            free_ast(root);
        }
    }
    
    /* Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Final memory operations with control flow */
    volatile char final_buf[1024];
    volatile size_t final_size = g_mem_size * 2;
    
    if (final_size > sizeof(final_buf)) {
        goto final_memset;
    }
    
    /* Chain of built-in calls */
    __builtin_memset(final_buf, 0x11, final_size);
    
    {
        volatile char temp_buf[1024];
        __builtin_memcpy(temp_buf, final_buf, final_size);
        
        /* Jump target for memmove */
        if (total_processed > 100) {
            goto do_memmove;
        }
        
        __builtin_memcpy(final_buf + 256, temp_buf, final_size - 256);
        goto final_check;
        
    do_memmove:
        __builtin_memmove(final_buf + 128, final_buf, final_size - 128);
    }
    
    goto final_check;
    
final_memset:
    __builtin_memset(final_buf, 0x22, sizeof(final_buf));
    
final_check:
    /* Calculate simple hash of results */
    unsigned long hash = 5381;
    for (size_t i = 0; i < total_processed && i < sizeof(result_buffer); i++) {
        hash = ((hash << 5) + hash) + result_buffer[i];
    }
    
    printf("Processed %zu bytes, hash: %lu\n", total_processed, hash);
    printf("ASAN built-in redirection test completed\n");
    
    return 0;
}
