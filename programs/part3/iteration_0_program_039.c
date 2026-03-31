/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ast_node {
    int type;
    char *data;
    size_t len;
    struct ast_node *left;
    struct ast_node *right;
} ast_node_t;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_memory_pool(void) {
    printf("Constructor: Initializing memory pool\n");
    /* Force early initialization of ASAN structures */
    char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_memory_pool(void) {
    printf("Destructor: Cleaning up memory pool\n");
}

/* Recursive AST manipulation with memory operations */
static ast_node_t* create_ast_node(const char *data, size_t len) {
    ast_node_t *node = malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    node->type = 1;
    node->len = len;
    node->data = malloc(len + 1);
    node->left = NULL;
    node->right = NULL;
    
    if (node->data) {
        /* Use __builtin_memcpy with volatile length */
        volatile size_t copy_len = len;
        __builtin_memcpy(node->data, data, copy_len);
        node->data[len] = '\0';
    }
    
    return node;
}

/* Function with goto jumps around memory operations */
static void process_with_goto(ast_node_t *dest, ast_node_t *src) {
    int state = 0;
    
    /* Jump into memory operation block */
    goto entry_point;
    
memory_operation:
    /* This block contains __builtin_memmove */
    if (dest && src && dest->data && src->data) {
        size_t move_len = dest->len < src->len ? dest->len : src->len;
        __builtin_memmove(dest->data, src->data, move_len);
    }
    state = 1;
    goto exit_point;
    
entry_point:
    if (state == 0) {
        goto memory_operation;
    }
    
exit_point:
    /* Jump out of memory operation context */
    if (state) {
        __builtin_memset(&state, 0, sizeof(state));
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buf[128];
        char shared_buf[256];
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(local_buf, thread_id, sizeof(local_buf));
                break;
            case 1:
                __builtin_memcpy(local_buf, shared_buf, 
                                g_mem_size % sizeof(local_buf));
                break;
            case 2:
                __builtin_memmove(local_buf, shared_buf + 64,
                                 g_mem_size % (sizeof(local_buf) / 2));
                break;
        }
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Collective memory operation */
        #pragma omp single
        {
            __builtin_memset(shared_buf, 0xFF, sizeof(shared_buf));
        }
    }
}

/* Multi-stage processing with varied memory built-ins */
static unsigned long process_tokens(const char **tokens, int count) {
    unsigned long hash = 0xDEADBEEF;
    char buffer[512];
    char temp[512];
    
    for (int i = 0; i < count; i++) {
        size_t token_len = strlen(tokens[i]);
        volatile size_t op_len = token_len % 256;
        
        /* Alternate between different memory operations */
        switch (i % 3) {
            case 0:
                __builtin_memcpy(buffer + (i * 16), tokens[i], op_len);
                break;
            case 1:
                __builtin_memset(temp, i, op_len);
                __builtin_memcpy(buffer + (i * 16), temp, op_len);
                break;
            case 2:
                if (i > 0) {
                    __builtin_memmove(buffer, buffer + 128, op_len);
                }
                break;
        }
        
        /* Update hash using memory content */
        for (size_t j = 0; j < op_len && j < sizeof(buffer); j++) {
            hash = (hash * 31) + buffer[j];
        }
    }
    
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Initialize token array */
    const char *tokens[] = {
        "memcpy_test_token_1",
        "memset_test_token_2",
        "memmove_test_token_3",
        "asan_instrumentation",
        "hwasan_kernel_mode",
        "builtin_redirection",
        "rtl_modification",
        "symbol_interception"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Create AST structures */
    ast_node_t *ast_root = create_ast_node("ROOT_NODE", 9);
    ast_node_t *ast_child = create_ast_node("CHILD_NODE", 10);
    
    if (!ast_root || !ast_child) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Test goto flow with memory operations */
    process_with_goto(ast_root, ast_child);
    
    /* Execute parallel memory operations */
    #ifdef _OPENMP
    parallel_memory_ops();
    #endif
    
    /* Process tokens with varied memory built-ins */
    unsigned long final_hash = process_tokens(tokens, token_count);
    
    /* Additional memory operations to ensure coverage */
    char final_buffer[1024];
    volatile size_t final_size = g_mem_size * 2;
    
    __builtin_memset(final_buffer, 0xAA, final_size % sizeof(final_buffer));
    __builtin_memcpy(final_buffer + 256, ast_root->data, 
                    ast_root->len % 256);
    __builtin_memmove(final_buffer, final_buffer + 512, 128);
    
    /* Verify operations by computing checksum */
    unsigned long checksum = final_hash;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        checksum ^= (final_buffer[i] << ((i % 8) * 8));
    }
    
    printf("Final hash: 0x%08lX\n", final_hash);
    printf("Checksum: 0x%08lX\n", checksum);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(ast_root->data);
    free(ast_root);
    free(ast_child->data);
    free(ast_child);
    
    return 0;
}
