/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* AST-like recursive structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    size_t hash;
} ASTNode;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_globals(void) {
    printf("Constructor: Initializing global state\n");
    g_mem_size = 256 + (rand() % 128);
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_globals(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Fill with pattern using __builtin_memcpy */
    char pattern[16] = "AST_NODE_DATA_";
    __builtin_memcpy(node->data, pattern, sizeof(pattern));
    
    node->hash = 0;
    return node;
}

/* Complex memory operation with goto flow control */
static void process_with_goto(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    int state = 0;
    
    /* Jump into memory operation block */
    if (dest->hash > src->hash) {
        goto copy_block;
    }
    
    /* Normal path */
    __builtin_memset(dest->data, 0xFF, g_mem_size % sizeof(dest->data));
    state = 1;
    
copy_block:
    /* This block is entered via goto */
    __builtin_memmove(dest->data, src->data, 
                     g_mem_size % sizeof(dest->data));
    
    if (state == 0) {
        /* Jump out of block */
        goto finalize;
    }
    
    /* Additional memcpy after memmove */
    char temp[64];
    __builtin_memcpy(temp, dest->data, sizeof(temp));
    
finalize:
    /* Compute hash using volatile-controlled size */
    size_t len = g_mem_size % sizeof(dest->data);
    for (size_t i = 0; i < len; i++) {
        dest->hash = dest->hash * 31 + dest->data[i];
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Thread-specific memory operations */
                char buffer[128];
                volatile size_t local_size = g_mem_size + tid;
                
                /* Mix of builtins */
                __builtin_memset(buffer, tid, 
                               local_size % sizeof(buffer));
                
                __builtin_memcpy(nodes[i]->data, buffer,
                               local_size % sizeof(nodes[i]->data));
                
                /* Conditional memmove */
                if (i > 0 && nodes[i-1]) {
                    __builtin_memmove(nodes[i]->data, 
                                     nodes[i-1]->data,
                                     (local_size / 2) % sizeof(nodes[i]->data));
                }
            }
        }
    }
}

/* Multi-stage initialization with memory builtins */
static void initialize_token_array(char** tokens, int token_count) {
    const char* base_tokens[] = {"TOKEN_A", "TOKEN_B", "TOKEN_C", 
                                 "TOKEN_D", "TOKEN_E"};
    
    for (int i = 0; i < token_count; i++) {
        tokens[i] = (char*)malloc(32);
        if (tokens[i]) {
            /* Clear with memset */
            __builtin_memset(tokens[i], 0, 32);
            
            /* Copy token data */
            const char* src = base_tokens[i % 5];
            __builtin_memcpy(tokens[i], src, strlen(src) + 1);
            
            /* Move data around */
            if (i > 0) {
                __builtin_memmove(tokens[i] + 8, tokens[i], 16);
            }
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Stage 1: Initialize complex data structures */
    const int NODE_COUNT = 8;
    const int TOKEN_COUNT = 10;
    
    ASTNode* nodes[NODE_COUNT];
    char* tokens[TOKEN_COUNT];
    
    /* Create AST nodes */
    for (int i = 0; i < NODE_COUNT; i++) {
        nodes[i] = create_ast(3);
    }
    
    /* Initialize token array with memory builtins */
    initialize_token_array(tokens, TOKEN_COUNT);
    
    /* Stage 2: Process with goto flow control */
    for (int i = 1; i < NODE_COUNT; i++) {
        process_with_goto(nodes[i], nodes[i-1]);
    }
    
    /* Stage 3: Parallel memory operations */
    #ifdef _OPENMP
    parallel_memory_ops(nodes, NODE_COUNT);
    #endif
    
    /* Stage 4: Compute final verification hash */
    size_t final_hash = 0;
    for (int i = 0; i < NODE_COUNT; i++) {
        if (nodes[i]) {
            /* Mix node hashes */
            char buffer[64];
            __builtin_memcpy(buffer, &nodes[i]->hash, sizeof(size_t));
            __builtin_memmove(buffer + 8, buffer, 8);
            
            for (int j = 0; j < 16; j++) {
                final_hash = final_hash * 37 + buffer[j];
            }
            
            free(nodes[i]);
        }
    }
    
    /* Cleanup tokens */
    for (int i = 0; i < TOKEN_COUNT; i++) {
        if (tokens[i]) free(tokens[i]);
    }
    
    printf("Test completed. Final hash: %zu\n", final_hash);
    printf("Expected coverage:\n");
    printf("1. BUILT_IN_MEMCPY redirection\n");
    printf("2. BUILT_IN_MEMSET redirection\n");
    printf("3. BUILT_IN_MEMMOVE redirection\n");
    printf("4. asan_memfn_rtls cache initialization\n");
    printf("5. Flow-sensitive goto handling\n");
    printf("6. OpenMP parallel instrumentation\n");
    
    return 0;
}
