/* ISO C99-compliant program targeting ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/* Constructor function with memory operations */
__attribute__((constructor))
static void init_asan_redirection(void) {
    char buffer[128];
    char* dest = buffer + 16;
    char* src = buffer + 32;
    
    /* Force initialization of asan_memfn_rtls cache */
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(dest, src, 32);
    __builtin_memmove(buffer, buffer + 8, 64);
    
    g_init_flag = 1;
}

/* Destructor for cleanup */
__attribute__((destructor))
static void cleanup_asan(void) {
    char cleanup_buf[256];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive function with memory operations */
static ast_node_t* create_ast(int depth, const char* pattern) {
    if (depth <= 0) return NULL;
    
    ast_node_t* node = malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    
    /* Copy pattern data with control flow */
    size_t len = strlen(pattern);
    if (len > 63) len = 63;
    
    /* Goto-based control flow for memmove */
    if (depth % 2 == 0) {
        goto even_depth;
    }
    
    __builtin_memcpy(node->data, pattern, len);
    node->data[len] = '\0';
    goto create_children;
    
even_depth:
    /* Use memmove with overlapping regions */
    char temp[64];
    __builtin_memcpy(temp, pattern, len);
    __builtin_memmove(node->data, temp, len);
    node->data[len] = '\0';
    
create_children:
    node->value = depth;
    node->left = create_ast(depth - 1, pattern);
    node->right = create_ast(depth - 2, pattern);
    
    return node;
}

/* Function with goto jumping into memory block */
static void process_with_goto(ast_node_t* a, ast_node_t* b) {
    int use_memmove = 0;
    
    if (a && b) {
        if (a->value > b->value) {
            use_memmove = 1;
            goto perform_copy;
        }
    }
    
    /* Regular path */
    if (a && b) {
        __builtin_memcpy(a->data, b->data, 64);
    }
    goto done;
    
perform_copy:
    /* Jumped into block with memmove */
    __builtin_memmove(a->data, b->data, 64);
    goto done;
    
done:
    return;
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char local_buf[512];
        char* volatile ptr = local_buf + thread_id * 64;
        volatile size_t size = g_mem_size / 4;
        
        /* All three builtins in parallel context */
        __builtin_memset(ptr, thread_id, size);
        
        if (thread_id % 2 == 0) {
            __builtin_memcpy(ptr + 32, ptr, 32);
        } else {
            __builtin_memmove(ptr, ptr + 16, 48);
        }
    }
}

/* Complex token processing */
static unsigned long process_tokens(const char** tokens, int count) {
    unsigned long hash = 0;
    char buffer[1024];
    char* current = buffer;
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        
        /* Mix of memory operations */
        if (i % 3 == 0) {
            __builtin_memcpy(current, tokens[i], len);
        } else if (i % 3 == 1) {
            __builtin_memset(current, i, len);
            __builtin_memcpy(current, tokens[i], len);
        } else {
            __builtin_memmove(current, tokens[i], len);
        }
        
        current += len;
        *current++ = ' ';
        
        /* Update hash */
        for (size_t j = 0; j < len; j++) {
            hash = hash * 31 + tokens[i][j];
        }
    }
    
    return hash;
}

int main(void) {
    /* Verify constructor ran */
    if (!g_init_flag) {
        fprintf(stderr, "Constructor not executed\n");
        return 1;
    }
    
    /* Initialize token array */
    const char* tokens[] = {
        "ASAN", "HWASAN", "MEMCPY", "MEMSET", "MEMMOVE",
        "REDZONE", "INSTRUMENT", "BUILTIN", "COVERAGE"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Create recursive structures */
    ast_node_t* ast1 = create_ast(5, "AST_NODE_1");
    ast_node_t* ast2 = create_ast(4, "AST_NODE_2");
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process with goto control flow */
    process_with_goto(ast1, ast2);
    
    /* Execute parallel operations */
    parallel_memory_ops();
    
    /* Process tokens */
    unsigned long result = process_tokens(tokens, token_count);
    
    /* Additional memory operations in main */
    char final_buffer[256];
    volatile char* vdest = final_buffer;
    volatile char* vsrc = final_buffer + 128;
    
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy((char*)vdest, (char*)vsrc, 64);
    __builtin_memmove(final_buffer + 32, final_buffer, 128);
    
    /* Print verification result */
    printf("Result hash: %lu\n", result);
    printf("AST values: %d, %d\n", ast1->value, ast2->value);
    printf("Final buffer[0]: %d\n", (int)final_buffer[0]);
    
    /* Cleanup */
    free(ast1);
    free(ast2);
    
    return 0;
}
