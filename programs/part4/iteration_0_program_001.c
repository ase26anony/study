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

/* Global token array */
static char token_pool[4096];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)(i % 256);
    }
    volatile_flag = 0;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Clear sensitive data */
    __builtin_memset(token_pool, 0, sizeof(token_pool));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* src, size_t len, int depth) {
    if (depth <= 0 || len == 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for node initialization */
    size_t copy_len = len > sizeof(node->data) ? sizeof(node->data) : len;
    __builtin_memcpy(node->data, src, copy_len);
    node->size = copy_len;
    
    /* Recursive creation with goto for flow control */
    int split = len / 2;
    
    if (split > 0) {
        node->left = create_ast_node(src, split, depth - 1);
        
        /* Jump label for goto testing */
        process_right:
        node->right = create_ast_node(src + split, len - split, depth - 1);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with goto jumping into memmove block */
static void process_with_goto(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    int use_memmove = volatile_flag;
    
    if (use_memmove) {
        goto do_memmove;
    }
    
    /* Normal path with memcpy */
    __builtin_memcpy(dest->data, src->data, 
                    src->size < dest->size ? src->size : dest->size);
    return;
    
do_memmove:
    /* Goto jumps here - tests flow sensitivity */
    size_t move_size = src->size < dest->size ? src->size : dest->size;
    __builtin_memmove(dest->data, src->data, move_size);
    
    /* Jump back out */
    goto cleanup;
    
cleanup:
    /* Use memset for cleanup */
    __builtin_memset(src->data + move_size, 0, sizeof(src->data) - move_size);
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[128];
        char src_buf[128];
        
        /* Initialize with pattern */
        for (int i = 0; i < sizeof(src_buf); i++) {
            src_buf[i] = (char)((i + thread_id) % 256);
        }
        
        /* Force builtin calls in parallel region */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        __builtin_memcpy(local_buf + 32, src_buf + 32, 64);
        
        /* Conditional memmove */
        if (thread_id % 2 == 0) {
            __builtin_memmove(local_buf, local_buf + 16, 48);
        }
        
        /* Store result in global pool (with synchronization) */
        #pragma omp critical
        {
            size_t offset = (token_index * 128) % sizeof(token_pool);
            __builtin_memcpy(token_pool + offset, local_buf, 128);
            token_index = (token_index + 1) % 32;
        }
    }
}

/* Complex memory dispatch with varied contexts */
static size_t dispatch_memory_operations(ASTNode* nodes[], int count) {
    size_t total_hash = 0;
    
    for (int i = 0; i < count; i++) {
        if (!nodes[i]) continue;
        
        /* Vary operations based on index */
        switch (i % 3) {
            case 0: {
                /* memcpy between nodes */
                int next = (i + 1) % count;
                if (nodes[next]) {
                    size_t len = nodes[i]->size < nodes[next]->size ? 
                                nodes[i]->size : nodes[next]->size;
                    __builtin_memcpy(nodes[next]->data, nodes[i]->data, len);
                }
                break;
            }
            case 1: {
                /* memset node data */
                __builtin_memset(nodes[i]->data + 16, i, 
                               nodes[i]->size > 16 ? nodes[i]->size - 16 : 0);
                break;
            }
            case 2: {
                /* memmove within node */
                if (nodes[i]->size > 32) {
                    __builtin_memmove(nodes[i]->data, nodes[i]->data + 8, 
                                    nodes[i]->size - 8);
                }
                break;
            }
        }
        
        /* Compute simple hash */
        for (size_t j = 0; j < nodes[i]->size && j < 32; j++) {
            total_hash += (size_t)nodes[i]->data[j];
        }
    }
    
    return total_hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create AST nodes */
    ASTNode* nodes[8];
    int depth = 3;
    
    for (int i = 0; i < 8; i++) {
        size_t len = (size_t)(volatile_len + i * 8);
        nodes[i] = create_ast_node(token_pool + i * 128, len, depth);
    }
    
    /* Test goto with memmove */
    if (nodes[0] && nodes[1]) {
        volatile_flag = 1;
        process_with_goto(nodes[0], nodes[1]);
        volatile_flag = 0;
    }
    
    /* Execute parallel operations */
    parallel_memory_operations();
    
    /* Dispatch memory operations */
    size_t hash = dispatch_memory_operations(nodes, 8);
    
    /* Additional builtin calls in main */
    char temp_buf[256];
    __builtin_memset(temp_buf, 0xAA, sizeof(temp_buf));
    __builtin_memcpy(temp_buf + 64, token_pool + 512, 128);
    __builtin_memmove(temp_buf, temp_buf + 32, 192);
    
    /* Use volatile to prevent dead code elimination */
    for (int i = 0; i < 64; i++) {
        hash += (size_t)temp_buf[i];
    }
    
    printf("Computed hash: %zu\n", hash);
    printf("Token index: %d\n", token_index);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        free_ast(nodes[i]);
    }
    
    /* Final builtin call */
    __builtin_memset(temp_buf, 0, sizeof(temp_buf));
    
    return (hash > 0) ? 0 : 1;
}
