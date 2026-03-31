/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
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
static char global_tokens[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(global_tokens, 'A', sizeof(global_tokens));
    printf("Constructor: Initialized global tokens\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use builtin memcpy in destructor */
    char temp[64];
    __builtin_memcpy(temp, global_tokens, 64);
    printf("Destructor: Cleaned up %d bytes\n", 64);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* src, size_t len) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy for node initialization */
    __builtin_memcpy(node->data, src, len);
    node->size = len;
    node->left = node->right = NULL;
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ASTNode* dest, ASTNode* src) {
    int use_memmove = 0;
    
    /* Jump into block with memmove */
    if (volatile_flag) {
        goto memmove_block;
    }
    
    normal_path:
    __builtin_memcpy(dest->data, src->data, src->size);
    return;
    
    memmove_block:
    use_memmove = 1;
    /* Jump out of block */
    if (dest == src) {
        goto skip_memmove;
    }
    
    /* This should trigger the memmove redirection */
    __builtin_memmove(dest->data + 10, dest->data, src->size - 10);
    
    skip_memmove:
    /* Jump back to normal path */
    if (!use_memmove) {
        goto normal_path;
    }
}

/* Parallel memory dispatch logic */
static void parallel_memory_ops(void) {
    char buffer1[1024];
    char buffer2[1024];
    char buffer3[1024];
    
    /* Initialize buffers with different patterns */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memset(buffer2, 0xBB, sizeof(buffer2));
    __builtin_memset(buffer3, 0xCC, sizeof(buffer3));
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                /* Force memcpy redirection */
                __builtin_memcpy(buffer1 + thread_id * 16, 
                               buffer2, 
                               volatile_len);
                break;
            case 1:
                /* Force memset redirection */
                __builtin_memset(buffer2 + thread_id * 32, 
                               thread_id, 
                               volatile_len / 2);
                break;
            case 2:
                /* Force memmove redirection with overlap */
                __builtin_memmove(buffer3 + 128, 
                                buffer3, 
                                volatile_len);
                break;
        }
        
        /* Additional memory operation in parallel region */
        if (thread_id == 0) {
            char temp[256];
            __builtin_memcpy(temp, buffer1, 256);
            __builtin_memset(temp + 128, 0xFF, 128);
            __builtin_memmove(buffer1, temp, 256);
        }
    }
    
    /* Verify operations didn't crash */
    __builtin_memcpy(buffer2, buffer1, 512);
}

/* Complex initialization with nested memory ops */
static void initialize_complex_structure(void) {
    ASTNode* nodes[8];
    char init_data[256];
    
    /* Create initialization pattern */
    for (int i = 0; i < 256; i++) {
        init_data[i] = (i * 7) & 0xFF;
    }
    
    /* Create tree of nodes with memory operations */
    for (int i = 0; i < 8; i++) {
        nodes[i] = create_ast_node(init_data + i * 32, 32);
        
        /* Process nodes with goto jumps */
        if (i > 0) {
            process_with_goto(nodes[i], nodes[i-1]);
        }
    }
    
    /* Chain memory operations between nodes */
    for (int i = 0; i < 7; i++) {
        __builtin_memcpy(nodes[i]->data + 16, 
                        nodes[i+1]->data, 
                        16);
        __builtin_memmove(nodes[i]->data, 
                         nodes[i]->data + 8, 
                         24);
    }
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        free(nodes[i]);
    }
}

/* Main execution flow */
int main(void) {
    unsigned long hash = 0;
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Stage 1: Initialize complex token array */
    for (int i = 0; i < sizeof(global_tokens); i++) {
        global_tokens[i] = (i * 13) & 0xFF;
    }
    
    /* Stage 2: Invoke recursive parser logic */
    initialize_complex_structure();
    
    /* Stage 3: Execute parallelized memory dispatch */
    parallel_memory_ops();
    
    /* Stage 4: Compute verification hash */
    for (int i = 0; i < sizeof(global_tokens); i++) {
        hash = (hash * 31) + global_tokens[i];
    }
    
    /* Additional builtin calls in main */
    char final_buffer[512];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, global_tokens, 512);
    __builtin_memmove(final_buffer + 256, final_buffer, 256);
    
    /* Mix in volatile length */
    __builtin_memset(final_buffer + 384, 0x42, volatile_len);
    
    printf("Test completed. Hash: 0x%08lx\n", hash & 0xFFFFFFFF);
    printf("Volatile operations: len=%d, flag=%d\n", 
           volatile_len, (int)volatile_flag);
    
    return 0;
}
