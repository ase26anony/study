/* ISO C99-compliant test program for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ast_node {
    struct ast_node* left;
    struct ast_node* right;
    char data[64];
    size_t data_len;
} ast_node_t;

/* Constructor function with attribute */
__attribute__((constructor)) 
static void init_globals(void) {
    printf("Constructor: Initializing global state\n");
    g_mem_size = 256;
}

/* Destructor function */
__attribute__((destructor)) 
static void cleanup_globals(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive function that uses builtins */
static ast_node_t* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ast_node_t* node = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    node->left = create_ast(depth - 1, base_data);
    node->right = create_ast(depth - 1, base_data);
    
    /* Use __builtin_memset to initialize data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Use __builtin_memcpy to copy data with volatile length */
    volatile size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data))
        copy_len = sizeof(node->data);
    
    __builtin_memcpy(node->data, base_data, copy_len);
    node->data_len = copy_len;
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ast_node_t* src, ast_node_t* dst) {
    int state = 0;
    
    if (!src || !dst) goto cleanup;
    
    /* Jump into memory operation block */
    if (src->data_len > 0) {
        goto do_memmove;
    } else {
        goto skip_memmove;
    }
    
do_memmove:
    {
        /* Use __builtin_memmove with volatile size */
        volatile size_t move_size = src->data_len;
        if (move_size > sizeof(dst->data))
            move_size = sizeof(dst->data);
        
        __builtin_memmove(dst->data, src->data, move_size);
        dst->data_len = move_size;
    }
    state = 1;
    
skip_memmove:
    /* Jump out of block */
    if (state) {
        goto finalize;
    }
    
    /* Another goto target with different memory operation */
    {
        volatile char temp[128];
        __builtin_memset(temp, 'X', sizeof(temp));
        __builtin_memcpy(dst->data, temp, sizeof(temp) < sizeof(dst->data) ? sizeof(temp) : sizeof(dst->data));
    }
    
finalize:
    return;
    
cleanup:
    printf("Invalid nodes in process_with_goto\n");
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        char buffer1[512];
        char buffer2[512];
        volatile int thread_id = 0;
        
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread uses builtins with different patterns */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(buffer1, thread_id, sizeof(buffer1));
                __builtin_memcpy(buffer2, buffer1, g_mem_size % sizeof(buffer1));
                break;
            case 1:
                __builtin_memmove(buffer1, buffer2, sizeof(buffer1) / 2);
                __builtin_memset(buffer2, 0xFF, sizeof(buffer2));
                break;
            case 2:
                __builtin_memcpy(buffer1, "TEST_DATA", 10);
                __builtin_memmove(buffer2, buffer1, 10);
                break;
        }
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Verify operations */
        #pragma omp single
        {
            printf("Parallel operations completed by %d threads\n", 
                   #ifdef _OPENMP
                   omp_get_num_threads()
                   #else
                   1
                   #endif
                  );
        }
    }
}

/* Complex token processing with memory builtins */
static uint32_t process_tokens(const char** tokens, size_t count) {
    uint32_t hash = 0xDEADBEEF;
    char accum[256];
    volatile size_t accum_pos = 0;
    
    __builtin_memset(accum, 0, sizeof(accum));
    
    for (size_t i = 0; i < count; i++) {
        size_t token_len = strlen(tokens[i]);
        volatile size_t copy_len = token_len;
        
        if (accum_pos + copy_len >= sizeof(accum)) {
            /* Use memmove to shift data when buffer is full */
            size_t shift = sizeof(accum) / 2;
            __builtin_memmove(accum, accum + shift, sizeof(accum) - shift);
            accum_pos -= shift;
        }
        
        __builtin_memcpy(accum + accum_pos, tokens[i], copy_len);
        accum_pos += copy_len;
        
        /* Mix hash */
        for (size_t j = 0; j < copy_len; j++) {
            hash = (hash << 5) + hash + accum[accum_pos - copy_len + j];
        }
    }
    
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create recursive AST structure */
    ast_node_t* ast1 = create_ast(3, "AST_NODE_DATA_1");
    ast_node_t* ast2 = create_ast(2, "AST_NODE_DATA_2");
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Test goto flow with memmove */
    process_with_goto(ast1, ast2);
    
    /* Process token array */
    const char* tokens[] = {
        "TOKEN_A", "TOKEN_B", "TOKEN_C", "TOKEN_D", "TOKEN_E",
        "TOKEN_F", "TOKEN_G", "TOKEN_H", "TOKEN_I", "TOKEN_J"
    };
    
    uint32_t token_hash = process_tokens(tokens, sizeof(tokens)/sizeof(tokens[0]));
    printf("Token processing hash: 0x%08X\n", token_hash);
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Additional builtin usage in main */
    char final_buffer[1024];
    volatile size_t final_size = g_mem_size;
    
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, ast2->data, 
                    ast2->data_len < sizeof(final_buffer) ? ast2->data_len : sizeof(final_buffer));
    
    /* Use memmove to create overlap */
    __builtin_memmove(final_buffer + 128, final_buffer, 256);
    
    /* Calculate final checksum */
    uint32_t final_sum = 0;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        final_sum += (uint8_t)final_buffer[i];
    }
    
    printf("Final buffer checksum: %u\n", final_sum);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(ast1);
    free(ast2);
    
    return 0;
}
