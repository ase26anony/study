/* ISO C99-compliant test program for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ast_node {
    struct ast_node* left;
    struct ast_node* right;
    char data[64];
    int value;
} ast_node_t;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force early initialization of ASAN runtime */
    char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Final memory operation */
    volatile char final_buf[16];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ast_node_t* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ast_node_t* node = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    
    /* Fill data with pattern */
    for (int i = 0; i < 63; i++) {
        node->data[i] = (char)('A' + (depth + i) % 26);
    }
    node->data[63] = '\0';
    
    /* Recursive creation */
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ast_node_t* src, ast_node_t* dst, int mode) {
    if (!src || !dst) return;
    
    volatile int use_memmove = mode;
    
    if (use_memmove) {
        goto use_memmove_block;
    } else {
        goto use_memcpy_block;
    }
    
use_memmove_block:
    {
        /* This block tests memmove with goto entry */
        char temp[64];
        __builtin_memcpy(temp, src->data, 64);
        
        /* Jump back and forth */
        if (src->value > 100) goto skip_memmove;
        
        __builtin_memmove(dst->data, temp, 64);
        goto after_memmove;
        
    skip_memmove:
        __builtin_memset(dst->data, 0, 64);
        
    after_memmove:
        dst->value = src->value;
    }
    return;
    
use_memcpy_block:
    {
        /* Direct memcpy without move */
        __builtin_memcpy(dst->data, src->data, 64);
        dst->value = src->value;
    }
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[128];
        char shared_buf[128];
        
        /* Initialize with memset */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        /* Copy between buffers */
        __builtin_memcpy(shared_buf, local_buf, sizeof(local_buf));
        
        /* Move data around */
        if (thread_id % 2 == 0) {
            __builtin_memmove(local_buf + 64, local_buf, 64);
        }
        
        #pragma omp barrier
        
        /* Verify with volatile read */
        volatile char check = shared_buf[0];
        (void)check;
    }
}

/* Complex token processing */
static uint32_t process_tokens(const char** tokens, int count) {
    uint32_t hash = 0x811C9DC5; /* FNV-1a basis */
    
    for (int i = 0; i < count; i++) {
        char buffer[256];
        size_t len = strlen(tokens[i]);
        
        /* Copy token to buffer */
        __builtin_memcpy(buffer, tokens[i], len + 1);
        
        /* Process with memset if long enough */
        if (len > 32) {
            __builtin_memset(buffer + 32, '#', len - 32);
        }
        
        /* Move to aligned position if needed */
        if ((uintptr_t)buffer % 8 != 0) {
            char aligned[256];
            __builtin_memmove(aligned, buffer, len + 1);
            __builtin_memcpy(buffer, aligned, len + 1);
        }
        
        /* Simple hash calculation */
        for (size_t j = 0; j < len; j++) {
            hash ^= buffer[j];
            hash *= 0x01000193;
        }
    }
    
    return hash;
}

int main(void) {
    /* Initialize complex token array */
    const char* tokens[] = {
        "memcpy_test_token_1",
        "memset_operation_token_2",
        "memmove_processing_token_3",
        "asan_instrumentation_4",
        "hwasan_kernel_mode_5",
        "builtin_redirection_6",
        "goto_flow_control_7",
        "openmp_parallel_8"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Create AST structures */
    ast_node_t* ast1 = create_ast(3);
    ast_node_t* ast2 = create_ast(3);
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Set values for processing */
    ast1->value = 150;
    ast2->value = 50;
    
    /* Test goto with memmove */
    process_with_goto(ast1, ast2, 1);
    process_with_goto(ast2, ast1, 0);
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Process tokens with memory builtins */
    uint32_t final_hash = process_tokens(tokens, token_count);
    
    /* Additional volatile operations to ensure builtins aren't optimized */
    volatile char* dynamic_buf = (char*)malloc(g_mem_size);
    if (dynamic_buf) {
        __builtin_memset(dynamic_buf, 0xAA, g_mem_size);
        
        char* mid_point = dynamic_buf + g_mem_size / 2;
        __builtin_memcpy(mid_point, dynamic_buf, g_mem_size / 4);
        __builtin_memmove(dynamic_buf, mid_point, g_mem_size / 4);
        
        free(dynamic_buf);
    }
    
    /* Print verification result */
    printf("Final hash: 0x%08X\n", final_hash);
    printf("Init flag: %d\n", g_init_flag);
    
    /* Cleanup */
    /* Note: Proper AST cleanup omitted for brevity */
    
    return 0;
}
