/* ISO C99-compliant test program for ASAN/HWASAN built-in redirection */
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
static void init_constructor(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)((i * 7) & 0xFF);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Clear sensitive data */
    __builtin_memset(token_pool, 0, sizeof(token_pool));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for initialization */
    __builtin_memcpy(node->data, base_data, 
                     volatile_len < 256 ? volatile_len : 256);
    
    node->id = depth;
    node->left = create_ast(depth - 1, base_data + 16);
    node->right = create_ast(depth - 1, base_data + 32);
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int state = 0;
    
    /* Jump into memory operation block */
    if (volatile_flag) goto mem_block;
    
    normal_path:
        __builtin_memset(dst->data, 0xAA, 128);
        return;
    
    mem_block:
        /* This tests flow-sensitivity of asan_memfn_rtls */
        __builtin_memmove(dst->data, src->data, 
                         volatile_len < 128 ? volatile_len : 128);
        
        if (state++ < 2) {
            goto normal_path;  /* Jump out */
        }
}

/* Parallel memory dispatch */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        char local_buf[512];
        char src_buf[512];
        
        /* Initialize source buffer */
        #pragma omp for
        for (int i = 0; i < 512; i++) {
            src_buf[i] = (char)(i & 0xFF);
        }
        
        /* Force built-in calls in parallel region */
        #pragma omp single
        {
            __builtin_memcpy(local_buf, src_buf, 256);
            __builtin_memset(local_buf + 256, 0xCC, 128);
            __builtin_memmove(local_buf + 128, local_buf, 64);
        }
        
        /* Verify copy */
        #pragma omp barrier
        #pragma omp critical
        {
            for (int i = 0; i < 64; i++) {
                token_pool[token_index++ % 4096] ^= local_buf[i];
            }
        }
    }
}

/* Multi-stage interaction function */
static void complex_memory_chain(void) {
    ASTNode* nodes[4];
    char init_data[1024];
    
    /* Initialize with pattern */
    for (int i = 0; i < 1024; i++) {
        init_data[i] = (char)((i * 13) & 0xFF);
    }
    
    /* Create AST nodes */
    for (int i = 0; i < 4; i++) {
        nodes[i] = create_ast(3, init_data + i * 64);
    }
    
    /* Chain memory operations between nodes */
    for (int i = 0; i < 3; i++) {
        /* Mixed built-in usage */
        if (i % 2 == 0) {
            __builtin_memcpy(nodes[i+1]->data, nodes[i]->data, 192);
        } else {
            __builtin_memmove(nodes[i]->data + 64, 
                             nodes[i]->data, 128);
        }
        
        /* Process with goto edge cases */
        process_with_goto(nodes[i], nodes[(i+1)%4]);
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        if (nodes[i]) {
            free(nodes[i]);
        }
    }
}

int main(void) {
    unsigned long hash = 0;
    
    printf("Starting ASAN/HWASAN built-in redirection test...\n");
    
    /* Stage 1: Basic built-in calls */
    char buffer1[1024];
    char buffer2[1024];
    
    __builtin_memset(buffer1, 0x55, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 512, buffer1, 256);
    
    /* Stage 2: Recursive structure operations */
    complex_memory_chain();
    
    /* Stage 3: OpenMP parallel operations */
    parallel_memory_ops();
    
    /* Stage 4: Volatile-controlled operations */
    for (int i = 0; i < volatile_len && i < 1024; i++) {
        __builtin_memset(buffer1 + i * 2, i, 8);
    }
    
    /* Calculate verification hash */
    for (int i = 0; i < 1024; i++) {
        hash = (hash * 31) + (unsigned char)buffer1[i];
        hash = (hash * 31) + (unsigned char)buffer2[i];
        hash = (hash * 31) + (unsigned char)token_pool[i % 4096];
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Test completed.\n");
    
    return (hash != 0) ? 0 : 1;
}
