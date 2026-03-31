/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_selector = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
char global_tokens[1024];
int token_hash = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force initialization of memcpy redirection */
    char src[32] = "constructor_data";
    char dest[32];
    __builtin_memcpy(dest, src, sizeof(src));
    
    /* Initialize global tokens with pattern */
    for (int i = 0; i < sizeof(global_tokens); i++) {
        global_tokens[i] = (i % 26) + 'A';
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    /* Use memset in destructor */
    char buffer[128];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Recursive parser with memory operations */
ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in data using memcpy */
    char pattern[32];
    snprintf(pattern, sizeof(pattern), "AST_%d_%d", depth, id);
    __builtin_memcpy(node->data, pattern, strlen(pattern) + 1);
    
    node->id = id;
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_memmove = 0;
        
    create_children:
        node->left = create_ast(depth - 1, id * 2);
        node->right = create_ast(depth - 1, id * 2 + 1);
        
        /* Conditional memmove between nodes */
        if (node->left && node->right && use_memmove == 0) {
            use_memmove = 1;
            __builtin_memmove(node->left->data + 32, 
                            node->right->data, 
                            volatile_len % 64);
            goto create_children; /* Jump back to test flow sensitivity */
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Process AST with memory operations */
int process_ast(ASTNode* node, int* sum) {
    if (!node) return 0;
    
    int local_sum = node->id;
    
    /* Use volatile length for memory operations */
    size_t len = volatile_len % sizeof(node->data);
    
    /* Copy data to temporary buffer */
    char temp[256];
    __builtin_memcpy(temp, node->data, len);
    
    /* Modify and move back */
    temp[0] = (temp[0] + 1) % 128;
    __builtin_memmove(node->data, temp, len);
    
    /* Recursive processing */
    local_sum += process_ast(node->left, sum);
    local_sum += process_ast(node->right, sum);
    
    *sum += local_sum;
    return local_sum;
}

/* Free AST with memory clearing */
void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

/* Parallel memory dispatch logic */
void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buf[512];
        char src_buf[512];
        
        /* Initialize source buffer */
        for (int i = 0; i < sizeof(src_buf); i++) {
            src_buf[i] = (i + thread_id) % 256;
        }
        
        /* Use all three builtins in parallel region */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        /* Conditional memcpy based on thread */
        if (thread_id % 2 == 0) {
            __builtin_memcpy(local_buf + 128, src_buf, 256);
        } else {
            __builtin_memmove(local_buf, src_buf + 128, 256);
        }
        
        /* Compute hash */
        #pragma omp critical
        {
            for (int i = 0; i < sizeof(local_buf); i++) {
                token_hash = (token_hash * 31 + local_buf[i]) % 1000000007;
            }
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create recursive AST */
    ASTNode* root = create_ast(4, 1);
    
    /* Process AST */
    int ast_sum = 0;
    process_ast(root, &ast_sum);
    printf("AST processed, sum: %d\n", ast_sum);
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    printf("Parallel ops complete, hash: %d\n", token_hash);
    
    /* Additional builtin usage with volatile control */
    char final_buffer[1024];
    size_t op_len = volatile_len;
    
    /* Chain of memory operations */
    __builtin_memset(final_buffer, 0xAA, op_len % sizeof(final_buffer));
    
    if (volatile_selector > 0) {
        __builtin_memcpy(final_buffer + 256, global_tokens, 512);
        __builtin_memmove(final_buffer, final_buffer + 128, 384);
    }
    
    /* Verify final buffer */
    int final_check = 0;
    for (int i = 0; i < 100; i++) {
        final_check += final_buffer[i];
    }
    printf("Final buffer check: %d\n", final_check);
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
