/* asan_coverage.c - Comprehensive test for ASAN memory built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[32];
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Global token array */
static const char *tokens[] = {
    "memcpy", "memset", "memmove", "test", "data", "asan", "hwasan"
};
static const int token_count = sizeof(tokens) / sizeof(tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < 256; i++) {
        volatile_src[i] = (char)(i % 256);
    }
    
    /* Use builtins in constructor to trigger early redirection */
    __builtin_memset(volatile_dest, 0, sizeof(volatile_dest));
    __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operations in destructor */
    char final_buf[128];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
    __builtin_memmove(final_buf + 16, final_buf, 64);
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth, const char **token_ptr) {
    if (depth <= 0 || *token_ptr == NULL) {
        return NULL;
    }
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data with builtin memcpy */
    const char *current_token = *token_ptr;
    size_t token_len = strlen(current_token);
    if (token_len > 31) token_len = 31;
    
    __builtin_memcpy(node->data, current_token, token_len);
    node->data[token_len] = '\0';
    node->type = depth;
    
    /* Move to next token with goto for flow control */
    token_ptr++;
    if (*token_ptr == NULL) {
        token_ptr = tokens; /* Wrap around */
    }
    
    /* Recursive calls with goto jumps */
    if (depth > 1) {
        int use_goto = (depth % 3 == 0);
        
        if (use_goto) {
            goto recursive_left;
        }
        
        node->left = parse_expression(depth - 1, token_ptr);
        
        if (use_goto) {
            goto recursive_right;
        }
        
        node->right = parse_expression(depth - 2, token_ptr);
        goto done;
        
    recursive_left:
        node->left = parse_expression(depth - 1, token_ptr);
        goto after_left;
        
    recursive_right:
        node->right = parse_expression(depth - 2, token_ptr);
        goto done;
        
    after_left:
        /* Memory move between left and right before creating right */
        if (node->left) {
            char temp[32];
            __builtin_memcpy(temp, node->left->data, 32);
            __builtin_memmove(node->data + 16, temp, 16);
        }
        node->right = parse_expression(depth - 2, token_ptr);
    }
    
done:
    return node;
}

/* Calculate hash of AST tree */
static unsigned long long hash_ast(ASTNode *node) {
    if (!node) return 0;
    
    unsigned long long hash = 0;
    
    /* Process node data with memory operations */
    for (int i = 0; i < 32; i++) {
        hash = (hash * 31) + (unsigned char)node->data[i];
    }
    
    /* Recursive hash calculation */
    unsigned long long left_hash = hash_ast(node->left);
    unsigned long long right_hash = hash_ast(node->right);
    
    /* Combine hashes with memory move simulation */
    char hash_buf[16];
    __builtin_memcpy(hash_buf, &left_hash, 8);
    __builtin_memmove(hash_buf + 8, &right_hash, 8);
    
    for (int i = 0; i < 16; i++) {
        hash ^= (unsigned long long)hash_buf[i] << ((i % 8) * 8);
    }
    
    return hash;
}

/* Free AST tree */
static void free_ast(ASTNode *node) {
    if (!node) return;
    
    /* Clear node data before freeing */
    __builtin_memset(node->data, 0, 32);
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Final memory operation before free */
    char final_clear[sizeof(ASTNode)];
    __builtin_memcpy(final_clear, node, sizeof(ASTNode));
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    free(node);
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_operations(void) {
    const int buffer_size = 1024;
    char *buffers[8];
    
    /* Allocate buffers */
    for (int i = 0; i < 8; i++) {
        buffers[i] = malloc(buffer_size);
        if (!buffers[i]) continue;
        
        /* Initialize with builtin memset */
        __builtin_memset(buffers[i], i, buffer_size);
    }
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs memory operations */
        char local_buf[256];
        
        /* Use volatile length to prevent optimization */
        int len = volatile_len + thread_id;
        if (len > 256) len = 256;
        
        /* Pattern of memory operations */
        __builtin_memset(local_buf, thread_id, len);
        
        if (thread_id % 3 == 0) {
            __builtin_memcpy(local_buf + 64, local_buf, 128);
        } else if (thread_id % 3 == 1) {
            __builtin_memmove(local_buf + 32, local_buf, 192);
        } else {
            /* Complex pattern with goto */
            int do_copy = 1;
            if (thread_id > 2) {
                goto skip_first_copy;
            }
            
            __builtin_memcpy(local_buf + 96, local_buf, 64);
            do_copy = 0;
            
        skip_first_copy:
            if (do_copy) {
                __builtin_memcpy(local_buf + 128, local_buf + 32, 64);
            }
            __builtin_memmove(local_buf, local_buf + 64, 128);
        }
        
        /* Copy result to shared buffer */
        if (buffers[thread_id % 8]) {
            __builtin_memcpy(buffers[thread_id % 8], local_buf, len);
        }
    }
    
    /* Clean up */
    for (int i = 0; i < 8; i++) {
        if (buffers[i]) {
            /* Final memory operation before free */
            __builtin_memset(buffers[i] + buffer_size - 64, 0, 64);
            free(buffers[i]);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Recursive AST parsing with memory operations */
    const char *token_ptr = tokens;
    ASTNode *ast_root = parse_expression(5, &token_ptr);
    
    if (!ast_root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Calculate and print hash */
    unsigned long long hash = hash_ast(ast_root);
    printf("AST Hash: %llu\n", hash);
    
    /* Phase 3: Parallel memory operations */
    printf("Running parallel memory operations...\n");
    parallel_memory_operations();
    
    /* Phase 4: Additional builtin stress tests */
    char stress_buf1[512];
    char stress_buf2[512];
    
    /* Varied memory operation patterns */
    for (int i = 0; i < 10; i++) {
        switch (i % 3) {
            case 0:
                __builtin_memset(stress_buf1, i, 256 + i * 16);
                __builtin_memcpy(stress_buf2, stress_buf1, 128);
                break;
            case 1:
                __builtin_memmove(stress_buf1 + 128, stress_buf1, 256);
                __builtin_memset(stress_buf2 + 64, 0xFF, 192);
                break;
            case 2:
                /* Complex goto pattern */
                if (i > 5) {
                    goto large_copy;
                }
                __builtin_memcpy(stress_buf1, stress_buf2, 64);
                goto after_copy;
                
            large_copy:
                __builtin_memcpy(stress_buf1, stress_buf2, 384);
                
            after_copy:
                __builtin_memmove(stress_buf2, stress_buf1, 256);
                break;
        }
    }
    
    /* Phase 5: Cleanup */
    free_ast(ast_root);
    
    /* Final verification */
    int final_check[4] = {0};
    __builtin_memset(final_check, 0x42, sizeof(final_check));
    
    unsigned int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += final_check[i];
    }
    
    printf("Final checksum: %u\n", sum);
    printf("Test completed successfully.\n");
    
    return 0;
}
