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
static char global_tokens[4096];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(global_tokens, 0xAA, sizeof(global_tokens));
    printf("Constructor: Initialized global tokens\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Use builtin memcpy in destructor */
    char local_buf[64];
    __builtin_memcpy(local_buf, global_tokens, 64);
    printf("Destructor: Cleaned up resources\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* src, size_t len) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy for node initialization */
    size_t copy_len = len < 256 ? len : 255;
    __builtin_memcpy(node->data, src, copy_len);
    node->data[copy_len] = '\0';
    node->size = copy_len;
    node->left = node->right = NULL;
    
    return node;
}

/* Function with goto statements for flow control */
static void process_with_goto(ASTNode* dest, ASTNode* src) {
    int use_memmove = 0;
    
    if (volatile_flag) {
        use_memmove = 1;
        goto memmove_block;
    }
    
    /* Regular memcpy path */
    __builtin_memcpy(dest->data, src->data, src->size);
    goto end_processing;
    
memmove_block:
    /* Jumped into memmove block */
    if (dest->data > src->data) {
        __builtin_memmove(dest->data, src->data, src->size);
    } else {
        char temp[256];
        __builtin_memcpy(temp, src->data, src->size);
        __builtin_memcpy(dest->data, temp, src->size);
    }
    
end_processing:
    /* Clear using memset */
    __builtin_memset(src->data + src->size/2, 0, src->size/4);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread uses builtins */
        char local_buffer[128];
        char source_buffer[128];
        
        /* Initialize with memset */
        __builtin_memset(source_buffer, thread_id, sizeof(source_buffer));
        
        /* Copy with memcpy */
        __builtin_memcpy(local_buffer, source_buffer, sizeof(local_buffer));
        
        /* Potential overlap - use memmove */
        if (thread_id % 2 == 0) {
            __builtin_memmove(local_buffer + 32, local_buffer, 64);
        }
        
        /* Store result in global array */
        size_t offset = (thread_id * 128) % sizeof(global_tokens);
        if (offset + 128 <= sizeof(global_tokens)) {
            __builtin_memcpy(global_tokens + offset, local_buffer, 128);
        }
    }
}

/* Multi-stage processing with different builtins */
static size_t process_tokens(void) {
    size_t hash = 0;
    char buffer[512];
    char* volatile ptr = buffer;
    
    /* Stage 1: Initialize with memset */
    __builtin_memset(buffer, 0, sizeof(buffer));
    
    /* Stage 2: Copy global tokens */
    size_t copy_size = volatile_len < 512 ? volatile_len : 512;
    __builtin_memcpy(buffer, global_tokens, copy_size);
    
    /* Stage 3: Process with overlapping moves */
    for (int i = 0; i < 4; i++) {
        size_t offset = i * 64;
        if (offset + 128 <= sizeof(buffer)) {
            __builtin_memmove(buffer + offset + 32, 
                            buffer + offset, 64);
        }
    }
    
    /* Calculate simple hash */
    for (size_t i = 0; i < sizeof(buffer); i++) {
        hash = (hash * 31) + buffer[i];
    }
    
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Initialize AST structures */
    ASTNode* nodes[4];
    char init_data[4][256];
    
    for (int i = 0; i < 4; i++) {
        __builtin_memset(init_data[i], 'A' + i, 255);
        init_data[i][255] = '\0';
        nodes[i] = create_ast_node(init_data[i], 256);
    }
    
    /* Test goto flow control */
    process_with_goto(nodes[0], nodes[1]);
    process_with_goto(nodes[2], nodes[3]);
    
    /* Execute OpenMP parallel section */
    #ifdef _OPENMP
    printf("Running OpenMP parallel section\n");
    #pragma omp parallel num_threads(4)
    parallel_memory_ops();
    #else
    parallel_memory_ops();
    #endif
    
    /* Process tokens and compute result */
    size_t final_hash = process_tokens();
    
    /* Additional builtin usage in main */
    char final_buffer[1024];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, global_tokens, 
                    sizeof(global_tokens) < 1024 ? 
                    sizeof(global_tokens) : 1024);
    
    /* One more memmove for good measure */
    __builtin_memmove(final_buffer + 256, final_buffer, 512);
    
    printf("Final hash: %zu\n", final_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        free(nodes[i]);
    }
    
    return 0;
}
