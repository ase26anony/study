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
    int id;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_token_pool(void) {
    /* Initialize with pattern using builtin memset */
    __builtin_memset(token_pool, 0xAA, sizeof(token_pool));
    
    /* Create recognizable pattern */
    for (int i = 0; i < sizeof(token_pool); i += 64) {
        token_pool[i] = 'T';
        token_pool[i + 1] = 'K';
        token_pool[i + 2] = i / 64 + '0';
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_tokens(void) {
    /* Clear sensitive data using builtin memset */
    __builtin_memset(token_pool, 0, sizeof(token_pool));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtin memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in node data */
    snprintf(node->data, sizeof(node->data), 
             "AST_%d_%d", id, depth);
    
    node->id = id;
    
    /* Create children with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 3 == 0);
        
        if (use_goto) {
            /* Jump into block with memmove */
            goto create_left;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        node->right = create_ast(depth - 1, id * 2 + 1);
        
        if (use_goto) {
            create_left:
            /* This block contains memmove with goto entry */
            char temp[256];
            __builtin_memmove(temp, node->data, 
                            volatile_len % sizeof(node->data));
            node->left = create_ast(depth - 1, id * 2);
            
            /* Jump out of block */
            goto skip_right;
        }
        
        node->right = create_ast(depth - 1, id * 2 + 1);
        skip_right:;
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Copy data between AST nodes */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use builtin memcpy with volatile length */
    int len = volatile_len % sizeof(dest->data);
    if (len > 0) {
        __builtin_memcpy(dest->data, src->data, len);
    }
    
    /* Recursive copy */
    if (src->left && dest->left) {
        copy_ast_data(dest->left, src->left);
    }
    if (src->right && dest->right) {
        copy_ast_data(dest->right, src->right);
    }
}

/* Parallel memory operations */
static void parallel_mem_operations(void) {
    char buffer1[1024];
    char buffer2[1024];
    char buffer3[1024];
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 'A', sizeof(buffer1));
    __builtin_memset(buffer2, 'B', sizeof(buffer2));
    __builtin_memset(buffer3, 'C', sizeof(buffer3));
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                /* memcpy operations */
                __builtin_memcpy(buffer1 + thread_id * 16, 
                               token_pool + thread_id * 32,
                               volatile_len % 128);
                break;
            case 1:
                /* memset operations */
                __builtin_memset(buffer2 + thread_id * 8,
                               'X' + thread_id,
                               volatile_len % 256);
                break;
            case 2:
                /* memmove operations with overlap */
                __builtin_memmove(buffer3 + 128,
                                buffer3 + 64,
                                volatile_len % 512);
                break;
        }
        
        /* Barrier to synchronize */
        #pragma omp barrier
        
        /* All threads copy to shared buffer */
        int offset = thread_id * 64;
        if (offset + 64 <= sizeof(token_pool)) {
            __builtin_memcpy(token_pool + offset,
                           buffer1 + offset % sizeof(buffer1),
                           64);
        }
    }
}

/* Calculate hash of AST structure */
static unsigned long long hash_ast(const ASTNode* node) {
    if (!node) return 0;
    
    unsigned long long hash = 5381;
    const char* p = node->data;
    
    /* Simple hash calculation */
    while (*p) {
        hash = ((hash << 5) + hash) + *p++;
    }
    
    /* Add children hashes */
    hash += hash_ast(node->left) * 31;
    hash += hash_ast(node->right) * 37;
    
    return hash;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create AST structures */
    ASTNode* ast1 = create_ast(4, 1);
    ASTNode* ast2 = create_ast(3, 2);
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST structures\n");
        return 1;
    }
    
    /* Perform memory copies between ASTs */
    copy_ast_data(ast2, ast1);
    
    /* Execute parallel memory operations */
    parallel_mem_operations();
    
    /* Complex memory operation sequence */
    char dynamic_buffer[2048];
    char* aligned_ptr = (char*)(((size_t)dynamic_buffer + 63) & ~63);
    
    /* Sequence of builtin calls */
    __builtin_memset(aligned_ptr, 0, 1024);
    __builtin_memcpy(aligned_ptr + 512, token_pool, 512);
    __builtin_memmove(aligned_ptr + 256, aligned_ptr + 512, 256);
    
    /* Another goto-controlled block */
    if (volatile_flag) {
        goto mem_operation_block;
    }
    
    /* This should be skipped */
    __builtin_memset(aligned_ptr, 0xFF, 1024);
    
mem_operation_block:
    /* This block is entered via goto */
    __builtin_memcpy(aligned_ptr + 768, aligned_ptr + 256, 256);
    
    /* Calculate and print verification hash */
    unsigned long long hash1 = hash_ast(ast1);
    unsigned long long hash2 = hash_ast(ast2);
    
    /* Hash of token pool */
    unsigned long long pool_hash = 0;
    for (size_t i = 0; i < sizeof(token_pool); i++) {
        pool_hash = pool_hash * 31 + token_pool[i];
    }
    
    printf("AST1 Hash: %llu\n", hash1);
    printf("AST2 Hash: %llu\n", hash2);
    printf("Token Pool Hash: %llu\n", pool_hash);
    printf("Final aligned buffer[0]: %d\n", (int)aligned_ptr[0]);
    
    /* Cleanup */
    /* Helper function to free AST */
    void free_ast(ASTNode* node) {
        if (!node) return;
        free_ast(node->left);
        free_ast(node->right);
        free(node);
    }
    
    free_ast(ast1);
    free_ast(ast2);
    
    printf("Test completed successfully.\n");
    return 0;
}
