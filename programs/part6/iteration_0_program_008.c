/* ISO C99-compliant test program for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[128];
    /* Force memcpy built-in in constructor */
    __builtin_memcpy(buffer, "constructor_init", 16);
    buffer[16] = '\0';
}

/* Destructor for cleanup coordination */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char cleanup_buf[64];
    __builtin_memset(cleanup_buf, 0, 64);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use memset built-in for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data with memcpy built-in */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) 
        copy_len = sizeof(node->data);
    
    __builtin_memcpy(node->data, base_data, copy_len);
    node->size = copy_len;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 1) {
        create_left_label:
        if (create_left) {
            node->left = create_ast(depth - 1, "left_branch");
            create_left = 0;
            goto create_left_label; /* Jump back */
        } else {
            node->right = create_ast(depth - 1, "right_branch");
        }
    }
    
    return node;
}

/* Function with goto jumping around memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int use_memmove = g_use_memmove;
    
    if (use_memmove) {
        goto do_memmove;
    } else {
        goto do_memcpy;
    }
    
do_memmove:
    /* This should trigger the memmove built-in redirection */
    __builtin_memmove(dst->data, src->data, 
                     src->size < dst->size ? src->size : dst->size);
    goto after_ops;
    
do_memcpy:
    __builtin_memcpy(dst->data, src->data, 
                    src->size < dst->size ? src->size : dst->size);
    goto after_ops;
    
after_ops:
    /* Additional operation to prevent dead code elimination */
    dst->data[0] = src->data[0];
}

/* Parallel processing function */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        volatile char local_buf[256];
        volatile char src_buf[256];
        
        /* Initialize source with memset */
        __builtin_memset(src_buf, 'A', 256);
        
        /* Copy with memcpy in parallel region */
        __builtin_memcpy(local_buf, src_buf, g_mem_size);
        
        /* Potential overlap - use memmove */
        if (g_use_memmove) {
            __builtin_memmove(local_buf + 128, local_buf, 128);
        }
        
        #pragma omp barrier
        
        /* Verify copy */
        #pragma omp for
        for (int i = 0; i < (int)g_mem_size; i++) {
            local_buf[i] = src_buf[i] + 1;
        }
    }
}

/* Complex token processing */
static size_t process_tokens(char** tokens, int count) {
    size_t hash = 0;
    volatile char accum[512] = {0};
    size_t accum_pos = 0;
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        
        /* Use memcpy for token accumulation */
        if (accum_pos + len < sizeof(accum)) {
            __builtin_memcpy(accum + accum_pos, tokens[i], len);
            accum_pos += len;
            
            /* Use memset to clear section occasionally */
            if (i % 3 == 0) {
                __builtin_memset(accum + accum_pos - 8, 0, 8);
            }
        }
        
        /* Use memmove when buffer needs shifting */
        if (accum_pos > 400) {
            __builtin_memmove(accum, accum + 200, accum_pos - 200);
            accum_pos -= 200;
        }
        
        hash += len * (i + 1);
    }
    
    return hash;
}

int main(void) {
    /* Initialize token array */
    char* tokens[] = {
        "token1", "another_token", "test_string",
        "memory_operation", "asan_test", "builtin_redirect"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Create recursive structures */
    ASTNode* ast1 = create_ast(3, "root_node_1");
    ASTNode* ast2 = create_ast(3, "root_node_2");
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Process with goto jumps */
    process_with_goto(ast1, ast2);
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Process tokens with various memory operations */
    size_t token_hash = process_tokens(tokens, token_count);
    
    /* Additional memory operations in main */
    volatile char final_buf[1024];
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    
    /* Chain of memory operations to stress redirection */
    __builtin_memcpy(final_buf, ast1->data, ast1->size);
    __builtin_memmove(final_buf + 512, final_buf, 256);
    __builtin_memset(final_buf + 768, 0xFF, 128);
    
    /* Calculate verification result */
    size_t result = token_hash;
    for (size_t i = 0; i < sizeof(final_buf); i += 64) {
        result += final_buf[i];
    }
    
    printf("Result: %zu\n", result);
    
    /* Cleanup */
    free(ast1);
    free(ast2);
    
    return 0;
}
