/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    char buffer[128];
    volatile char* dest = buffer;
    volatile const char* src = "Constructor init";
    
    /* Force builtin memcpy in constructor */
    __builtin_memcpy((void*)dest, (void*)src, 16);
    
    /* Force builtin memset in constructor */
    __builtin_memset(buffer + 16, 0xAA, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    char cleanup_buf[256];
    
    /* Use all three builtins in destructor */
    __builtin_memset(cleanup_buf, 0xFF, 128);
    __builtin_memcpy(cleanup_buf + 128, cleanup_buf, 64);
    __builtin_memmove(cleanup_buf + 64, cleanup_buf, 32);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile-controlled length */
    size_t copy_len = (size_t)volatile_len;
    if (copy_len > 255) copy_len = 255;
    
    /* Force builtin memcpy with volatile length */
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, base_data, copy_len);
    
    node->size = copy_len;
    
    /* Recursive creation with goto for control flow */
    int use_left = volatile_flag & 1;
    
    if (use_left) {
        node->left = create_ast(depth - 1, "Left branch");
        node->right = NULL;
    } else {
        node->left = NULL;
        node->right = create_ast(depth - 1, "Right branch");
    }
    
    return node;
}

/* Function with goto jumping into memmove block */
static void goto_memmove_test(char* dest, char* src, size_t len) {
    int condition = volatile_flag;
    
    if (condition) {
        goto memmove_block;
    }
    
    /* Normal path */
    __builtin_memcpy(dest, src, len);
    return;
    
memmove_block:
    /* Jumped-into block with memmove */
    __builtin_memmove(dest, src, len);
    
    /* Jump out to another block */
    goto after_memmove;
    
after_memmove:
    /* Additional operation after jump */
    __builtin_memset(dest + len/2, 0xCC, len/4);
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        char local_buf[512];
        char src_buf[512];
        
        /* Initialize source with pattern */
        for (int i = 0; i < 512; i++) {
            src_buf[i] = (char)(i % 256);
        }
        
        /* Use all three builtins in parallel region */
        __builtin_memset(local_buf, 0, sizeof(local_buf));
        __builtin_memcpy(local_buf, src_buf, 256);
        __builtin_memmove(local_buf + 256, local_buf, 128);
        
        /* Volatile-controlled operation */
        size_t move_len = (size_t)(volatile_len % 128);
        if (move_len > 0) {
            __builtin_memmove(local_buf + 384, local_buf, move_len);
        }
    }
}

/* Complex token processing */
static unsigned long process_tokens(const char** tokens, int count) {
    unsigned long hash = 0xDEADBEEF;
    char buffer[1024];
    char* current = buffer;
    
    for (int i = 0; i < count; i++) {
        size_t token_len = strlen(tokens[i]);
        
        /* Mix of memory operations */
        if (i % 3 == 0) {
            __builtin_memcpy(current, tokens[i], token_len);
        } else if (i % 3 == 1) {
            __builtin_memmove(current, tokens[i], token_len);
        } else {
            __builtin_memset(current, '*', token_len);
            __builtin_memcpy(current, tokens[i], token_len > 8 ? 8 : token_len);
        }
        
        /* Update hash */
        for (size_t j = 0; j < token_len && j < 8; j++) {
            hash = (hash << 5) + hash + current[j];
        }
        
        current += token_len;
        if (current >= buffer + sizeof(buffer) - 256) {
            current = buffer;
        }
    }
    
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* 1. Initialize complex token array */
    const char* tokens[] = {
        "memcpy", "memset", "memmove", "asan", "hwasan",
        "redzone", "instrumentation", "builtin", "coverage",
        "volatile", "goto", "parallel", "recursive", "struct"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* 2. Create recursive AST structure */
    ASTNode* root = create_ast(4, "Root node");
    
    /* 3. Perform AST memory operations */
    if (root && root->left) {
        /* Copy between AST nodes */
        __builtin_memcpy(root->right ? root->right->data : root->data,
                        root->left->data,
                        root->left->size > root->size ? root->size : root->left->size);
    }
    
    /* 4. Execute goto-based memmove test */
    char goto_src[256];
    char goto_dest[256];
    for (int i = 0; i < 256; i++) {
        goto_src[i] = (char)i;
    }
    goto_memmove_test(goto_dest, goto_src, 128);
    
    /* 5. Run parallel memory operations */
    parallel_memory_ops();
    
    /* 6. Process tokens and compute result */
    unsigned long result = process_tokens(tokens, token_count);
    
    /* 7. Additional builtin calls in main */
    char final_buf[1024];
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf, goto_dest, 128);
    __builtin_memmove(final_buf + 256, final_buf, 64);
    
    /* 8. Cleanup */
    /* Recursive free omitted for brevity - would normally free AST */
    
    printf("Test completed. Result hash: 0x%08lX\n", result);
    printf("Volatile check: len=%d, flag=%d\n", 
           (int)volatile_len, (int)volatile_flag);
    
    return 0;
}
