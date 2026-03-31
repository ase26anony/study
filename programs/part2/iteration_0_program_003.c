/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ast_node {
    int type;
    int value;
    char data[64];
    struct ast_node *left;
    struct ast_node *right;
    struct ast_node *next;
} ast_node_t;

/* Global token array */
static const char *tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan",
    "test", "coverage", "builtin", "volatile", "omp"
};
static const int token_count = 10;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_sanitizer_test(void) {
    printf("Initializing sanitizer test environment...\n");
    /* Force initialization of sanitizer runtime */
    volatile char dummy[16];
    __builtin_memset(dummy, 0, sizeof(dummy));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_sanitizer_test(void) {
    printf("Cleaning up sanitizer test...\n");
}

/* Recursive parser with memory operations */
static ast_node_t* parse_expression(int depth, const char **token_ptr) {
    if (depth <= 0 || *token_ptr == NULL) {
        return NULL;
    }
    
    ast_node_t *node = malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(*node));
    
    /* Copy token data with builtin memcpy */
    const char *current_token = *token_ptr;
    size_t token_len = strlen(current_token);
    if (token_len > sizeof(node->data) - 1) {
        token_len = sizeof(node->data) - 1;
    }
    __builtin_memcpy(node->data, current_token, token_len);
    node->data[token_len] = '\0';
    
    /* Move to next token with builtin memmove */
    char temp[64];
    __builtin_memcpy(temp, node->data, sizeof(node->data));
    __builtin_memmove(node->data, temp, sizeof(node->data));
    
    node->type = depth;
    node->value = token_len;
    
    /* Control flow with goto */
    if (depth > 1) {
        int use_goto = (depth % 2 == 0);
        
        if (use_goto) {
            goto parse_left;
        }
        
        node->left = parse_expression(depth - 1, token_ptr + 1);
        
        parse_left:
        node->right = parse_expression(depth - 2, token_ptr + 2);
    } else {
        node->left = NULL;
        node->right = NULL;
    }
    
    return node;
}

/* Function with complex memory operations and goto */
static void process_ast_with_goto(ast_node_t *node) {
    if (!node) return;
    
    volatile int stage = 0;
    char buffer[128];
    
    stage_start:
    stage++;
    
    switch (stage) {
        case 1:
            /* Builtin memcpy with volatile size */
            __builtin_memcpy(buffer, node->data, g_mem_size % sizeof(buffer));
            goto stage_next;
            
        stage_next:
        case 2:
            /* Builtin memset */
            __builtin_memset(buffer + 32, 0xA5, g_mem_size % 64);
            goto stage_final;
            
        stage_final:
        case 3:
            /* Builtin memmove with overlap */
            __builtin_memmove(buffer, buffer + 16, 48);
            break;
            
        default:
            goto stage_end;
    }
    
    if (stage < 3) {
        goto stage_start;
    }
    
    stage_end:
    /* Process children */
    process_ast_with_goto(node->left);
    process_ast_with_goto(node->right);
}

/* Parallel memory dispatch function */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf1[256];
        char local_buf2[256];
        
        /* Initialize with builtin memset */
        __builtin_memset(local_buf1, thread_id, sizeof(local_buf1));
        
        /* Copy between buffers with builtin memcpy */
        __builtin_memcpy(local_buf2, local_buf1, sizeof(local_buf1));
        
        /* Move data around with builtin memmove */
        __builtin_memmove(local_buf1 + 64, local_buf1, 128);
        
        /* Verify with volatile access */
        volatile char *check = local_buf1;
        (void)check;
    }
}

/* Calculate hash of AST */
static unsigned long calculate_ast_hash(ast_node_t *node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char *ptr = node->data;
    
    /* Process string with volatile control */
    volatile int i = 0;
    while (ptr[i] && i < sizeof(node->data)) {
        hash = ((hash << 5) + hash) + ptr[i];
        i++;
    }
    
    hash += calculate_ast_hash(node->left);
    hash += calculate_ast_hash(node->right);
    
    return hash;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test...\n");
    
    /* Create recursive AST */
    ast_node_t *ast_root = parse_expression(5, tokens);
    if (!ast_root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process with goto control flow */
    process_ast_with_goto(ast_root);
    
    /* Execute parallel memory operations */
    #ifdef _OPENMP
    printf("Running parallel memory operations...\n");
    #endif
    parallel_memory_operations();
    
    /* Additional memory operations in main */
    char main_buffer[512];
    volatile size_t op_size = g_mem_size;
    
    /* Test all three builtins */
    __builtin_memset(main_buffer, 0xCC, op_size % sizeof(main_buffer));
    __builtin_memcpy(main_buffer + 128, main_buffer, 256);
    __builtin_memmove(main_buffer, main_buffer + 64, 384);
    
    /* Calculate and print result */
    unsigned long final_hash = calculate_ast_hash(ast_root);
    printf("Test completed. AST hash: 0x%08lx\n", final_hash);
    
    /* Cleanup */
    /* Note: In real ASAN, this would detect leaks */
    
    return 0;
}
