/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#endif

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char *data;
    size_t len;
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *parent;
} ASTNode;

/* Global token array */
static const char *tokens[] = {
    "memcpy", "memset", "memmove",
    "asan", "hwasan", "instrument",
    "redzone", "shadow", "poison"
};
static const size_t num_tokens = sizeof(tokens) / sizeof(tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_env(void) {
    printf("ASAN Environment Initialized\n");
    
    /* Force early initialization of memory functions */
    char buffer[64];
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, "init", 5);
    
    /* Volatile access to prevent dead code elimination */
    if (g_use_hwasan) {
        __builtin_memmove(buffer + 10, buffer, 5);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_env(void) {
    printf("ASAN Environment Cleanup\n");
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth, const char **token_ptr) {
    if (depth <= 0 || *token_ptr >= tokens + num_tokens) {
        return NULL;
    }
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    const char *current_token = *token_ptr;
    size_t token_len = strlen(current_token);
    
    /* Allocate and copy token data using builtins */
    node->data = malloc(token_len + 1);
    if (node->data) {
        __builtin_memcpy(node->data, current_token, token_len);
        node->data[token_len] = '\0';
        __builtin_memset(node->data + token_len, 0, 1); /* Null terminator */
    }
    node->len = token_len;
    
    (*token_ptr)++;
    
    /* Recursive calls with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 2 == 0);
        
        if (use_goto) {
            goto parse_left;
        } else {
            node->left = parse_expression(depth - 1, token_ptr);
            goto parse_right;
        }
        
    parse_left:
        node->left = parse_expression(depth - 1, token_ptr);
        
    parse_right:
        node->right = parse_expression(depth - 1, token_ptr);
        
        /* Cross-copy between nodes using __builtin_memmove */
        if (node->left && node->right && node->left->data && node->right->data) {
            size_t copy_len = (node->left->len < node->right->len) ? 
                             node->left->len : node->right->len;
            
            /* This memmove should trigger the redirection logic */
            __builtin_memmove(node->left->data, node->right->data, copy_len);
            
            /* Jump back to avoid optimization */
            if (copy_len > 10) {
                goto finish_node;
            }
        }
    }
    
finish_node:
    return node;
}

/* Parallel memory operations */
static void parallel_mem_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char src_buffer[512];
        char dst_buffer[512];
        
        /* Initialize with __builtin_memset */
        __builtin_memset(src_buffer, thread_id, sizeof(src_buffer));
        __builtin_memset(dst_buffer, 0, sizeof(dst_buffer));
        
        /* Varied memory operations based on thread ID */
        switch (thread_id % 3) {
            case 0:
                /* memcpy pattern */
                __builtin_memcpy(dst_buffer, src_buffer, 
                               g_mem_size % sizeof(dst_buffer));
                break;
            case 1:
                /* memset pattern */
                __builtin_memset(dst_buffer + 100, thread_id + 65, 50);
                break;
            case 2:
                /* memmove with overlap */
                __builtin_memmove(dst_buffer + 200, dst_buffer + 150, 100);
                /* Additional memcpy to ensure both are called */
                __builtin_memcpy(src_buffer + 300, dst_buffer + 200, 50);
                break;
        }
        
        /* Barrier to synchronize */
        #pragma omp barrier
        
        /* Cross-thread memory operation */
        #pragma omp single
        {
            char master_buffer[1024];
            __builtin_memset(master_buffer, 0xFF, sizeof(master_buffer));
            
            /* This should trigger ASAN redirection */
            __builtin_memmove(master_buffer + 512, master_buffer, 512);
        }
    }
}

/* Calculate hash of AST tree */
static size_t hash_ast(ASTNode *node) {
    if (!node) return 0;
    
    size_t hash = 5381;
    char *data = node->data;
    
    if (data) {
        /* Use __builtin_memcpy in hash calculation */
        char temp_buf[256];
        size_t copy_len = (node->len < sizeof(temp_buf)) ? 
                         node->len : sizeof(temp_buf);
        
        __builtin_memcpy(temp_buf, data, copy_len);
        
        for (size_t i = 0; i < copy_len; i++) {
            hash = ((hash << 5) + hash) + temp_buf[i];
        }
    }
    
    /* Recursive hash combination */
    size_t left_hash = hash_ast(node->left);
    size_t right_hash = hash_ast(node->right);
    
    /* Combine using bit operations */
    __builtin_memcpy(&hash, &left_hash, sizeof(size_t) > 8 ? 8 : sizeof(size_t));
    hash ^= right_hash;
    
    return hash;
}

/* Free AST tree */
static void free_ast(ASTNode *node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    if (node->data) {
        /* Clear sensitive data before free */
        __builtin_memset(node->data, 0, node->len);
        free(node->data);
    }
    
    __builtin_memset(node, 0, sizeof(ASTNode));
    free(node);
}

int main(void) {
    printf("Starting ASAN Built-in Redirection Test\n");
    
    /* Initialize token pointer */
    const char *token_ptr = tokens;
    
    /* Create recursive AST structure */
    ASTNode *root = parse_expression(4, &token_ptr);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Calculate and print hash */
    size_t hash = hash_ast(root);
    printf("AST Hash: 0x%zx\n", hash);
    
    /* Execute parallel memory operations */
    printf("Executing parallel memory operations...\n");
    parallel_mem_operations();
    
    /* Additional memory operations in main */
    char final_buffer[2048];
    
    /* Chain of memory operations to trigger all builtins */
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, "ASAN_TEST", 10);
    __builtin_memmove(final_buffer + 1024, final_buffer, 10);
    __builtin_memset(final_buffer + 1034, 0xAA, 100);
    __builtin_memcpy(final_buffer + 1134, final_buffer + 1024, 110);
    
    /* Complex goto pattern with memory operations */
    int counter = 0;
    
start_loop:
    if (counter++ < 3) {
        char loop_buffer[128];
        __builtin_memset(loop_buffer, counter, sizeof(loop_buffer));
        
        if (counter % 2 == 0) {
            goto even_case;
        } else {
            goto odd_case;
        }
    } else {
        goto finish;
    }

even_case:
    __builtin_memcpy(final_buffer + 1500, "EVEN", 5);
    goto continue_loop;

odd_case:
    __builtin_memmove(final_buffer + 1505, "ODD", 4);
    goto continue_loop;

continue_loop:
    __builtin_memset(final_buffer + 1510, counter, 10);
    goto start_loop;

finish:
    /* Verify final buffer */
    size_t verify_sum = 0;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        verify_sum += final_buffer[i];
    }
    printf("Buffer verification sum: %zu\n", verify_sum);
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
