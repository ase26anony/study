/* asan_coverage.c - Comprehensive test for ASAN memory builtin redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char *data;
    size_t len;
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *parent;
} ASTNode;

/* Global token array */
static char g_token_pool[4096];
static volatile size_t g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_token_pool(void) {
    /* Fill with pattern to detect corruption */
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)(i % 256);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void verify_token_pool(void) {
    size_t errors = 0;
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        if (g_token_pool[i] != (char)(i % 256)) {
            errors++;
        }
    }
    if (errors > 0) {
        fprintf(stderr, "Token pool corruption detected: %zu errors\n", errors);
    }
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth, volatile int *counter) {
    if (depth <= 0 || *counter >= 100) {
        return NULL;
    }
    
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile to prevent folding */
    volatile size_t local_size = g_mem_size / (depth + 1);
    node->len = local_size;
    node->data = (char*)malloc(local_size);
    
    if (node->data) {
        /* Force builtin_memset initialization */
        __builtin_memset(node->data, depth, local_size);
        
        /* Fill with token data using memcpy */
        size_t copy_len = local_size;
        if (copy_len > sizeof(g_token_pool) - g_token_idx) {
            copy_len = sizeof(g_token_pool) - g_token_idx;
        }
        
        /* Label for goto testing */
        copy_start:
        __builtin_memcpy(node->data, &g_token_pool[g_token_idx], copy_len);
        
        /* Conditional goto to test flow sensitivity */
        if (depth % 3 == 0) {
            goto skip_memmove;
        }
        
        /* Create overlapping copy with memmove */
        if (copy_len > 16) {
            __builtin_memmove(node->data + 8, node->data, copy_len - 8);
        }
        
        skip_memmove:
        g_token_idx += copy_len;
        if (g_token_idx >= sizeof(g_token_pool)) {
            g_token_idx = 0;
        }
    }
    
    (*counter)++;
    node->left = parse_expression(depth - 1, counter);
    
    /* Another goto into memory operation block */
    if (depth % 2 == 0 && node->left && node->left->data) {
        goto copy_start;
    }
    
    node->right = parse_expression(depth - 2, counter);
    node->parent = NULL;
    
    return node;
}

/* Copy AST structure with memory operations */
static ASTNode* copy_ast(const ASTNode *src) {
    if (!src) return NULL;
    
    ASTNode *dst = (ASTNode*)malloc(sizeof(ASTNode));
    if (!dst) return NULL;
    
    /* Copy metadata */
    dst->len = src->len;
    dst->data = (char*)malloc(src->len);
    
    if (dst->data && src->data) {
        /* Use all three builtins in sequence */
        __builtin_memset(dst->data, 0, src->len);
        __builtin_memcpy(dst->data, src->data, src->len);
        
        /* Create overlapping region with memmove */
        if (src->len > 32) {
            __builtin_memmove(dst->data + 16, dst->data, src->len - 16);
        }
    }
    
    dst->left = copy_ast(src->left);
    dst->right = copy_ast(src->right);
    dst->parent = NULL;
    
    return dst;
}

/* Free AST recursively */
static void free_ast(ASTNode *node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    if (node->data) {
        /* Clear memory before free */
        if (node->len > 0) {
            __builtin_memset(node->data, 0, node->len);
        }
        free(node->data);
    }
    
    free(node);
}

/* Compute hash of AST */
static size_t hash_ast(const ASTNode *node) {
    if (!node) return 0;
    
    size_t hash = 0;
    if (node->data && node->len > 0) {
        /* Simple hash computation */
        for (size_t i = 0; i < node->len; i++) {
            hash = hash * 31 + (size_t)node->data[i];
        }
    }
    
    return hash + hash_ast(node->left) + hash_ast(node->right);
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char buf1[512];
        char buf2[512];
        
        /* Initialize with builtin memset */
        __builtin_memset(buf1, thread_id, sizeof(buf1));
        __builtin_memset(buf2, 0xFF, sizeof(buf2));
        
        /* Copy between buffers */
        size_t copy_size = sizeof(buf1) - thread_id * 16;
        if (copy_size > sizeof(buf1)) copy_size = sizeof(buf1);
        
        __builtin_memcpy(buf2, buf1, copy_size);
        
        /* Overlapping move */
        if (copy_size > 64) {
            __builtin_memmove(buf1 + 32, buf1, copy_size - 32);
        }
        
        /* Verify pattern */
        #pragma omp barrier
        
        #pragma omp critical
        {
            /* Use all builtins in critical section */
            static char shared_buf[1024];
            static volatile size_t shared_idx = 0;
            
            __builtin_memset(&shared_buf[shared_idx], thread_id, 64);
            __builtin_memcpy(&shared_buf[shared_idx + 64], buf1, 128);
            
            if (shared_idx + 256 < sizeof(shared_buf)) {
                __builtin_memmove(&shared_buf[shared_idx + 128], 
                                 &shared_buf[shared_idx], 128);
            }
            
            shared_idx = (shared_idx + 256) % (sizeof(shared_buf) - 256);
        }
    }
}

int main(void) {
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Phase 1: Recursive AST parsing */
    volatile int counter = 0;
    ASTNode *ast1 = parse_expression(5, &counter);
    printf("Created AST with %d nodes\n", counter);
    
    /* Phase 2: AST copying with memory operations */
    ASTNode *ast2 = copy_ast(ast1);
    printf("Copied AST structure\n");
    
    /* Phase 3: Parallel memory operations */
    printf("Running parallel memory operations...\n");
    parallel_memory_ops();
    
    /* Phase 4: Verification */
    size_t hash1 = hash_ast(ast1);
    size_t hash2 = hash_ast(ast2);
    printf("AST hash1: %zu\n", hash1);
    printf("AST hash2: %zu\n", hash2);
    printf("Hashes %s\n", (hash1 == hash2) ? "match" : "differ");
    
    /* Phase 5: Additional edge cases */
    {
        /* Variable length arrays with builtins */
        volatile int n = 128;
        char vla1[n];
        char vla2[n];
        
        __builtin_memset(vla1, 0xAA, n);
        __builtin_memcpy(vla2, vla1, n);
        
        /* Overlapping within same array */
        __builtin_memmove(vla1 + 10, vla1, n - 10);
        
        /* Zero-length operations */
        if (n > 0) {
            __builtin_memset(vla1, 0, 0);  /* Should still call builtin */
            __builtin_memcpy(vla2, vla1, 0);
            __builtin_memmove(vla2, vla1, 0);
        }
    }
    
    /* Cleanup */
    free_ast(ast1);
    free_ast(ast2);
    
    printf("Test completed successfully\n");
    return 0;
}
