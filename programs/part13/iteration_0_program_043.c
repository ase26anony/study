/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 16;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    
    /* Force builtin usage in constructor */
    __builtin_memset(volatile_dest, 0xA5, sizeof(volatile_dest));
    __builtin_memcpy((void*)volatile_src, "ASAN_TEST_SOURCE", 17);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Cleaning up ASAN test\n");
    
    /* Use memmove in destructor */
    char temp[32];
    __builtin_memmove(temp, volatile_dest, 16);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Use memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in data */
    char pattern[32];
    __builtin_memset(pattern, 'A' + (depth % 26), 31);
    pattern[31] = '\0';
    
    /* Use memcpy to copy pattern */
    __builtin_memcpy(node->data, pattern, 
                    (volatile_len < 31) ? volatile_len : 31);
    
    /* Recursive creation with goto for control flow */
    int use_goto = (depth % 3 == 0);
    
    if (use_goto) {
        goto create_children;
    }
    
    node->left = create_ast(depth - 1, counter);
    node->right = NULL;
    
    if (use_goto) {
        create_children:
        /* Jump back into normal flow with memmove */
        ASTNode* temp = node->left;
        if (temp) {
            /* Copy data between nodes */
            __builtin_memmove(node->right, temp, sizeof(ASTNode));
        }
        node->right = create_ast(depth - 2, counter);
    } else {
        node->right = create_ast(depth - 1, counter);
    }
    
    return node;
}

/* Function with complex control flow and builtins */
static void process_ast(ASTNode* root, int* sum) {
    if (!root) return;
    
    /* Variable length based on volatile */
    int len = volatile_len % 64;
    
    /* Process left subtree */
    process_ast(root->left, sum);
    
    /* Use goto to create interesting control flow */
    if (root->id % 7 == 0) {
        goto special_case;
    }
    
    /* Normal processing with memcpy */
    char buffer[64];
    __builtin_memcpy(buffer, root->data, len);
    
    /* Calculate sum of characters */
    for (int i = 0; i < len; i++) {
        *sum += buffer[i];
    }
    
    if (root->id % 3 == 0) {
        special_case:
        /* Use memset in goto target */
        __builtin_memset(buffer, 0xFF, len);
        for (int i = 0; i < len; i++) {
            *sum -= buffer[i];
        }
    }
    
    /* Process right subtree */
    process_ast(root->right, sum);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buf[128];
        char shared_buf[256];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        /* Use memcpy with volatile length */
        int copy_len = (volatile_len + thread_id) % 128;
        __builtin_memcpy(shared_buf + thread_id * 16, 
                        local_buf, copy_len);
        
        #pragma omp barrier
        
        /* Use memmove for overlapping regions */
        if (thread_id % 2 == 0) {
            __builtin_memmove(shared_buf + 32, 
                            shared_buf + 16, 64);
        }
    }
}

/* Multi-stage initialization function */
static void initialize_test_data(char* tokens[], int count) {
    /* Stage 1: Clear all tokens */
    for (int i = 0; i < count; i++) {
        __builtin_memset(tokens[i], 0, 32);
    }
    
    /* Stage 2: Copy patterns */
    const char* patterns[] = {"TOKEN_A", "TOKEN_B", "TOKEN_C"};
    for (int i = 0; i < count && i < 3; i++) {
        __builtin_memcpy(tokens[i], patterns[i], 8);
    }
    
    /* Stage 3: Move data around */
    for (int i = 1; i < count; i++) {
        __builtin_memmove(tokens[i-1] + 8, tokens[i], 8);
    }
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Initialize token array */
    char* tokens[5];
    for (int i = 0; i < 5; i++) {
        tokens[i] = (char*)malloc(32);
    }
    
    initialize_test_data(tokens, 5);
    
    /* Create recursive AST */
    int counter = 0;
    ASTNode* ast_root = create_ast(4, &counter);
    
    /* Process AST */
    int ast_sum = 0;
    process_ast(ast_root, &ast_sum);
    printf("AST processing sum: %d\n", ast_sum);
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    printf("Parallel operations completed\n");
    
    /* Final builtin usage in main */
    char final_buffer[256];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, tokens[0], 16);
    __builtin_memmove(final_buffer + 128, final_buffer, 64);
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < 256; i++) {
        hash = hash * 31 + final_buffer[i];
    }
    printf("Verification hash: %lu\n", hash);
    
    /* Cleanup */
    for (int i = 0; i < 5; i++) {
        free(tokens[i]);
    }
    
    /* TODO: Add AST cleanup function */
    
    printf("Test completed successfully\n");
    return 0;
}
