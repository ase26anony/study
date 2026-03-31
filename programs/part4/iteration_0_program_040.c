/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_switch = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char *data;
    size_t size;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force initialization of ASAN runtime before main */
    volatile char buffer[16];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    /* Final memory operation in destructor */
    volatile char final_buf[8];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, size_t base_size) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile length to prevent constant folding */
    size_t data_size = base_size + (volatile_len % 256);
    node->size = data_size;
    node->data = malloc(data_size);
    
    if (node->data) {
        /* Force memcpy/memset builtins */
        __builtin_memset(node->data, depth, data_size);
        
        /* Conditional memcpy based on volatile */
        char pattern[32];
        __builtin_memset(pattern, 0xAB, sizeof(pattern));
        __builtin_memcpy(node->data, pattern, 
                        (data_size < 32) ? data_size : 32);
    }
    
    /* Recursive creation with goto for flow control */
    int create_left = volatile_switch & 1;
    
    if (create_left) {
        node->left = create_ast(depth - 1, base_size * 2);
    } else {
        node->left = NULL;
        goto skip_right;
    }
    
    node->right = create_ast(depth - 1, base_size / 2);
    
skip_right:
    return node;
}

/* Function with goto jumping into memory operation block */
static void goto_mem_operations(char *dest, char *src, size_t len) {
    int use_memmove = volatile_switch & 2;
    
    if (use_memmove) {
        goto do_memmove;
    } else {
        goto do_memcpy;
    }

do_memmove:
    /* This goto target contains __builtin_memmove */
    __builtin_memmove(dest, src, len);
    goto after_ops;
    
do_memcpy:
    __builtin_memcpy(dest, src, len);
    /* Fall through */
    
after_ops:
    /* Verify with memset */
    __builtin_memset(src, 0, len);
}

/* Process AST with OpenMP parallelization */
static size_t process_ast_parallel(ASTNode *root) {
    size_t total = 0;
    
    if (!root) return 0;
    
    #pragma omp parallel reduction(+:total)
    {
        /* Each thread processes memory operations */
        char thread_buf[128];
        char src_buf[128];
        
        /* Initialize source buffer */
        for (int i = 0; i < 128; i++) {
            src_buf[i] = i;
        }
        
        #pragma omp for
        for (int i = 0; i < 10; i++) {
            /* Varied memory operations in parallel region */
            size_t op_len = volatile_len % 128;
            
            if (i % 3 == 0) {
                __builtin_memcpy(thread_buf, src_buf, op_len);
            } else if (i % 3 == 1) {
                __builtin_memset(thread_buf, i, op_len);
            } else {
                __builtin_memmove(thread_buf, src_buf, op_len);
            }
            
            /* Compute hash */
            for (size_t j = 0; j < op_len; j++) {
                total += thread_buf[j];
            }
        }
    }
    
    return total;
}

/* Complex token array initialization */
static void init_token_array(char **tokens, int count) {
    for (int i = 0; i < count; i++) {
        tokens[i] = malloc(volatile_len % 128 + 1);
        if (tokens[i]) {
            /* Use all three builtins */
            __builtin_memset(tokens[i], 0, volatile_len % 128 + 1);
            
            char pattern[] = "TOKEN_";
            __builtin_memcpy(tokens[i], pattern, 
                           (sizeof(pattern)-1 < volatile_len % 128 + 1) ? 
                            sizeof(pattern)-1 : volatile_len % 128 + 1);
            
            /* Overlap with memmove */
            if (i > 0) {
                __builtin_memmove(tokens[i] + 3, tokens[i-1], 
                                (volatile_len % 32) + 1);
            }
        }
    }
}

int main(void) {
    size_t hash_result = 0;
    
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* 1. Create recursive AST structure */
    ASTNode *ast_root = create_ast(4, 32);
    
    /* 2. Initialize token array */
    char *tokens[8];
    init_token_array(tokens, 8);
    
    /* 3. Perform goto-based memory operations */
    char buffer1[256];
    char buffer2[256];
    
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memset(buffer2, 0xBB, sizeof(buffer2));
    
    for (int i = 0; i < 3; i++) {
        goto_mem_operations(buffer1, buffer2, volatile_len % 256);
        /* Swap buffers */
        char *temp = buffer1;
        buffer1 = buffer2;
        buffer2 = temp;
    }
    
    /* 4. Process AST with OpenMP */
    hash_result = process_ast_parallel(ast_root);
    
    /* 5. Additional builtin calls in main */
    char final_buffer[512];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    
    /* Overlapping memmove */
    __builtin_memmove(final_buffer + 128, final_buffer, 256);
    
    /* Final memcpy */
    __builtin_memcpy(final_buffer + 384, buffer1, 128);
    
    /* Compute final hash */
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        hash_result += final_buffer[i];
    }
    
    printf("Hash result: %zu\n", hash_result);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        free(tokens[i]);
    }
    
    /* Note: AST cleanup omitted for brevity */
    
    return 0;
}
