/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    uint32_t hash;
} ASTNode;

/* Global token array */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "instrument"
};
static const int token_count = sizeof(tokens)/sizeof(tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_hooks(void) {
    volatile char buffer[128];
    /* Force builtin initialization in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 64, buffer, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_hooks(void) {
    volatile char final_buf[32];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile size */
    size_t copy_len = g_mem_size % 64;
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Create hash using memory operations */
    node->hash = 0;
    for (int i = 0; i < (int)copy_len; i++) {
        node->hash = (node->hash * 31) + node->data[i];
    }
    
    /* Recursive creation with goto for flow control */
    int use_left = (depth % 2) == 0;
    
    if (use_left) {
        node->left = create_ast(depth - 1, base_data + 1);
        goto skip_right;
    }
    
    node->right = create_ast(depth - 1, base_data + 2);
    
skip_right:
    /* Memmove with goto jumping into block */
    if (g_use_memmove) {
        char temp[64];
        __builtin_memcpy(temp, node->data, sizeof(node->data));
        goto do_memmove;
    }
    
    return node;
    
do_memmove:
    __builtin_memmove(node->data + 16, node->data, 32);
    return node;
}

/* Parallel memory operations */
static void parallel_mem_ops(void) {
    const int num_ops = 1000;
    char* buffers[10];
    
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < 10; i++) {
            buffers[i] = (char*)malloc(g_mem_size);
            if (buffers[i]) {
                /* Mix of builtins in parallel region */
                __builtin_memset(buffers[i], i, g_mem_size);
                
                if (i % 3 == 0) {
                    __builtin_memcpy(buffers[i] + 64, buffers[i], 128);
                } else if (i % 3 == 1) {
                    __builtin_memmove(buffers[i] + 32, buffers[i], 96);
                }
            }
        }
        
        #pragma omp single
        {
            /* Cross-thread memory operation */
            if (buffers[0] && buffers[1]) {
                __builtin_memcpy(buffers[1], buffers[0], 64);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 10; i++) {
        free(buffers[i]);
    }
}

/* Complex control flow with builtins */
static uint32_t process_tokens(void) {
    char token_buffer[1024];
    uint32_t total_hash = 0;
    int offset = 0;
    
    for (int i = 0; i < token_count; i++) {
        size_t len = strlen(tokens[i]);
        
        /* Conditional builtin usage */
        if (i % 2 == 0) {
            __builtin_memcpy(token_buffer + offset, tokens[i], len);
        } else {
            __builtin_memset(token_buffer + offset, tokens[i][0], len);
        }
        
        /* Memmove with overlapping regions */
        if (i > 0 && offset > 16) {
            __builtin_memmove(token_buffer + offset - 8, 
                            token_buffer + offset - 16, 16);
        }
        
        offset += len;
        
        /* Goto jumping over builtin */
        if (i == token_count - 2) {
            goto skip_hash;
        }
        
        /* Hash calculation */
        for (int j = 0; j < (int)len; j++) {
            total_hash = (total_hash * 31) + token_buffer[offset - len + j];
        }
        
        continue;
        
    skip_hash:
        /* Still do memory op after goto */
        __builtin_memset(token_buffer + offset, 0xCC, 8);
    }
    
    return total_hash;
}

int main(void) {
    uint32_t final_result = 0;
    
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = create_ast(4, tokens[0]);
    if (root) {
        final_result ^= root->hash;
        
        /* Memory operations between nodes */
        if (root->left && root->right) {
            __builtin_memcpy(root->right->data, root->left->data, 32);
            __builtin_memmove(root->left->data + 16, root->right->data, 16);
        }
        
        /* Cleanup AST */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    /* Phase 2: Parallel memory operations */
    parallel_mem_ops();
    
    /* Phase 3: Token processing */
    final_result += process_tokens();
    
    /* Final builtin stress test */
    volatile char final_buffer[512];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    
    for (int i = 0; i < 5; i++) {
        size_t size = (g_mem_size * (i + 1)) % 512;
        __builtin_memcpy(final_buffer + i * 64, final_buffer, size);
        
        if (i == 3) {
            __builtin_memmove(final_buffer + 128, final_buffer + 64, 192);
        }
    }
    
    printf("Final hash result: %u\n", final_result);
    printf("Test completed - check ASAN/HWASAN instrumentation\n");
    
    return (final_result != 0) ? 0 : 1;
}
