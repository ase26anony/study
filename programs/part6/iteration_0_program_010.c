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
static volatile size_t token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize token pool with pattern */
    for (size_t i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)(i % 256);
    }
    
    /* Use builtins in constructor to trigger early redirection */
    char local_buf[128];
    __builtin_memset(local_buf, 0xAA, sizeof(local_buf));
    __builtin_memcpy(local_buf + 32, token_pool, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    /* Final builtin usage in destructor */
    char final_buf[64];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* src, size_t depth) {
    if (depth > 3) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile-controlled length */
    size_t copy_len = volatile_len % 256;
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    
    /* Builtin memcpy with volatile length */
    __builtin_memcpy(node->data, src, copy_len);
    node->size = copy_len;
    
    /* Recursive creation with goto for flow control */
    if (depth < 3 && volatile_flag) {
        node->left = create_ast_node(src + 16, depth + 1);
        
        /* Jump label for goto testing */
        process_right:
        node->right = create_ast_node(src + 32, depth + 1);
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
    } else {
        /* Regular path */
        __builtin_memcpy(dest->data, src->data, 
                        dest->size < src->size ? dest->size : src->size);
        return;
    }
    
    /* Target label for goto - tests flow sensitivity */
    do_memmove:
    {
        size_t len = volatile_len % 128;
        if (len > sizeof(dest->data)) len = sizeof(dest->data);
        
        /* Builtin memmove with overlap */
        __builtin_memmove(dest->data + 16, dest->data, len - 16);
        __builtin_memmove(dest->data, src->data, len);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, size_t count) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (size_t i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Each thread uses builtins */
                char temp[128];
                
                /* Mixed builtin usage pattern */
                __builtin_memset(temp, thread_id, sizeof(temp));
                __builtin_memcpy(nodes[i]->data + 64, temp, 64);
                
                /* Conditional memmove */
                if (thread_id % 2 == 0) {
                    __builtin_memmove(nodes[i]->data, nodes[i]->data + 32, 32);
                }
            }
        }
        
        /* Barrier with additional builtin usage */
        #pragma omp barrier
        
        #pragma omp single
        {
            /* Master thread does final consolidation */
            char master_buf[256];
            __builtin_memset(master_buf, 0, sizeof(master_buf));
            for (size_t i = 0; i < (count < 4 ? count : 4); i++) {
                if (nodes[i]) {
                    __builtin_memcpy(master_buf + i * 64, nodes[i]->data, 64);
                }
            }
        }
    }
}

/* Complex initialization with multiple builtin calls */
static void initialize_complex_buffer(char* buf, size_t size) {
    /* Layered memory operations */
    __builtin_memset(buf, 0x00, size);
    
    /* Pattern setup with memcpy */
    for (size_t i = 0; i < size / 64; i++) {
        __builtin_memcpy(buf + i * 64, token_pool + (i * 32) % 4096, 64);
    }
    
    /* Overlapping memmove */
    if (size > 128) {
        __builtin_memmove(buf + 64, buf, size - 64);
    }
}

int main(void) {
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Phase 1: Create AST structure */
    ASTNode* root = create_ast_node(token_pool, 0);
    ASTNode* nodes[8] = {0};
    
    /* Create array of nodes */
    for (int i = 0; i < 8; i++) {
        nodes[i] = create_ast_node(token_pool + i * 128, 1);
    }
    
    /* Phase 2: Process with goto flow control */
    if (root && nodes[0]) {
        process_with_goto(root, nodes[0]);
        
        /* Additional goto test jumping back */
        if (volatile_flag) {
            volatile_flag = 0;
            goto process_right; /* Defined in create_ast_node */
        }
    }
    
    /* Phase 3: OpenMP parallel operations */
    #ifdef _OPENMP
    parallel_memory_ops(nodes, 8);
    #endif
    
    /* Phase 4: Complex buffer operations */
    char complex_buf[1024];
    initialize_complex_buffer(complex_buf, sizeof(complex_buf));
    
    /* Phase 5: Final verification with all builtins */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(complex_buf); i++) {
        hash = (hash * 31) + (unsigned char)complex_buf[i];
    }
    
    /* Use builtins in verification */
    char verify_buf[256];
    __builtin_memset(verify_buf, 0, sizeof(verify_buf));
    __builtin_memcpy(verify_buf, &hash, sizeof(hash));
    __builtin_memmove(verify_buf + 8, verify_buf, 8);
    
    printf("Verification hash: %lu\n", hash);
    printf("Builtin redirection test completed.\n");
    
    /* Cleanup */
    free(root);
    for (int i = 0; i < 8; i++) {
        free(nodes[i]);
    }
    
    return 0;
}
