/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
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
    int depth;
} ASTNode;

/* Token array for parser simulation */
typedef struct {
    char tokens[32][16];
    int count;
} TokenArray;

/* Global memory buffers */
static char g_buffer1[512] __attribute__((aligned(64)));
static char g_buffer2[512] __attribute__((aligned(64)));
static char g_buffer3[512] __attribute__((aligned(64)));

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    /* Force initialization of memory buffers */
    __builtin_memset(g_buffer1, 0xAA, sizeof(g_buffer1));
    __builtin_memset(g_buffer2, 0xBB, sizeof(g_buffer2));
    __builtin_memset(g_buffer3, 0xCC, sizeof(g_buffer3));
    
    /* Volatile write to prevent dead code elimination */
    g_use_hwasan = 0;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_globals(void) {
    /* Final memory operations */
    __builtin_memset(g_buffer1, 0, sizeof(g_buffer1));
    __builtin_memset(g_buffer2, 0, sizeof(g_buffer2));
}

/* Recursive AST creation with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy for data initialization */
    size_t len = strlen(base_data);
    if (len > sizeof(node->data) - 1) len = sizeof(node->data) - 1;
    
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, base_data, len);
    
    node->depth = depth;
    
    /* Create children with goto-controlled flow */
    if (depth > 1) {
        int use_goto = (depth % 3 == 0);
        
        if (use_goto) {
            goto create_left;
        }
        
        node->left = create_ast(depth - 1, "LEFT");
        goto skip_left;
        
    create_left:
        node->left = create_ast(depth - 2, "LEFT_GOTO");
        
    skip_left:
        node->right = create_ast(depth - 1, "RIGHT");
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* AST copy function with complex memory operations */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use builtin memmove for overlapping copy test */
    if (dest->data + 16 > src->data && dest->data < src->data + 48) {
        __builtin_memmove(dest->data, src->data, sizeof(dest->data));
    } else {
        __builtin_memcpy(dest->data, src->data, sizeof(dest->data));
    }
    
    /* Recursive copy */
    if (src->left) {
        if (!dest->left) dest->left = (ASTNode*)malloc(sizeof(ASTNode));
        copy_ast_data(dest->left, src->left);
    }
    
    if (src->right) {
        if (!dest->right) dest->right = (ASTNode*)malloc(sizeof(ASTNode));
        copy_ast_data(dest->right, src->right);
    }
}

/* Token parser with memory operations */
static void parse_tokens(TokenArray* tokens) {
    char temp[256];
    volatile size_t offset = 0;  /* Prevent optimization */
    
    for (int i = 0; i < tokens->count; i++) {
        /* Complex memory pattern with goto */
        if (i % 4 == 0) {
            goto copy_block;
        }
        
        /* Normal memset path */
        __builtin_memset(temp + offset, i, 16);
        offset += 16;
        continue;
        
    copy_block:
        /* memcpy with goto entry */
        __builtin_memcpy(temp + offset, tokens->tokens[i], 
                        strlen(tokens->tokens[i]));
        offset += strlen(tokens->tokens[i]);
        
        /* memmove for overlapping regions */
        if (offset > 32) {
            __builtin_memmove(temp + offset - 16, temp + offset - 32, 16);
        }
    }
    
    /* Final buffer operation */
    __builtin_memcpy(g_buffer3, temp, offset < sizeof(g_buffer3) ? 
                    offset : sizeof(g_buffer3));
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_ops(void) {
    int i;
    const int num_ops = 100;
    volatile char local_buf[1024];  /* Prevent optimization */
    
    #pragma omp parallel for private(i) shared(g_buffer1, g_buffer2)
    for (i = 0; i < num_ops; i++) {
        size_t size = (i * 7) % 128 + 16;  /* Non-constant size */
        size_t offset = (i * 13) % 256;
        
        /* Mix of memory operations */
        if (i % 3 == 0) {
            __builtin_memset(g_buffer1 + offset, i, size);
        } else if (i % 3 == 1) {
            __builtin_memcpy(g_buffer2 + offset, g_buffer1 + offset, size);
        } else {
            /* memmove with potential overlap */
            size_t src_offset = (offset + 32) % 256;
            __builtin_memmove(g_buffer1 + offset, g_buffer1 + src_offset, size);
        }
        
        /* Local buffer operations */
        __builtin_memcpy(local_buf + (i % 512), g_buffer1 + offset, 
                        size < 512 - (i % 512) ? size : 512 - (i % 512));
    }
}

/* Calculate hash of buffer contents */
static unsigned int buffer_hash(const char* buf, size_t size) {
    unsigned int hash = 0;
    for (size_t i = 0; i < size; i++) {
        hash = (hash * 31) + (unsigned char)buf[i];
    }
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize token array */
    TokenArray tokens;
    tokens.count = 20;
    for (int i = 0; i < tokens.count; i++) {
        snprintf(tokens.tokens[i], sizeof(tokens.tokens[i]), 
                "TOKEN_%d_%zu", i, g_mem_size);
    }
    
    /* Create AST structure */
    ASTNode* ast1 = create_ast(5, "ROOT_NODE");
    ASTNode* ast2 = create_ast(3, "COPY_ROOT");
    
    if (ast1 && ast2) {
        /* Copy AST data with memory operations */
        copy_ast_data(ast2, ast1);
        
        /* Additional memory operations on AST */
        __builtin_memcpy(ast1->data + 32, ast2->data, 32);
        __builtin_memset(ast1->data + 16, 0xFF, 16);
    }
    
    /* Parse tokens with goto flow */
    parse_tokens(&tokens);
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Complex memory pattern with volatile control */
    volatile size_t final_size = g_mem_size;
    if (final_size > sizeof(g_buffer1)) final_size = sizeof(g_buffer1);
    
    /* Force all three builtins in sequence */
    __builtin_memset(g_buffer1, 0x11, final_size);
    __builtin_memcpy(g_buffer2, g_buffer1, final_size);
    __builtin_memmove(g_buffer3, g_buffer2, final_size / 2);
    
    /* Calculate and print verification hash */
    unsigned int hash1 = buffer_hash(g_buffer1, final_size);
    unsigned int hash2 = buffer_hash(g_buffer2, final_size);
    unsigned int hash3 = buffer_hash(g_buffer3, final_size / 2);
    
    printf("Hash verification:\n");
    printf("  Buffer1 hash: 0x%08X\n", hash1);
    printf("  Buffer2 hash: 0x%08X\n", hash2);
    printf("  Buffer3 hash: 0x%08X\n", hash3);
    printf("  Total sum: %u\n", hash1 + hash2 + hash3);
    
    /* Cleanup */
    free(ast1);
    free(ast2);
    
    printf("Test completed successfully.\n");
    return 0;
}
