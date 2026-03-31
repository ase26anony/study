/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char *volatile_dest = NULL;
static volatile const char *volatile_src = NULL;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode *left;
    struct ASTNode *right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    printf("Constructor: Initializing ASAN environment\n");
    volatile_dest = malloc(volatile_len * 2);
    volatile_src = "Initial source data for testing memory operations";
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Destructor: Cleaning up ASAN environment\n");
    if (volatile_dest) free((void*)volatile_dest);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Create pattern in data using __builtin_memcpy */
    char pattern[32];
    snprintf(pattern, sizeof(pattern), "AST_Node_%d_Depth_%d", id, depth);
    __builtin_memcpy(node->data, pattern, strlen(pattern) + 1);
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
done:
    return node;
}

/* Function with goto jumping into memory operation block */
static void test_goto_memmove(void) {
    char buffer1[128];
    char buffer2[128];
    int use_memmove = 0;
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 'A', sizeof(buffer1));
    __builtin_memset(buffer2, 'B', sizeof(buffer2));
    
    /* Jump into memmove block */
    if (rand() % 2) {
        goto perform_memmove;
    }
    
    /* Normal path */
    __builtin_memcpy(buffer1, buffer2, 64);
    goto after_memmove;
    
perform_memmove:
    /* This block should trigger memmove redirection */
    __builtin_memmove(buffer1 + 10, buffer1, 50);
    
after_memmove:
    /* Verify operation */
    buffer1[63] = '\0';
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    const int num_threads = 4;
    char *thread_buffers[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread allocates and operates on its own buffer */
        thread_buffers[tid] = malloc(256);
        
        if (thread_buffers[tid]) {
            /* Use all three builtins in parallel */
            __builtin_memset(thread_buffers[tid], tid + '0', 256);
            
            #pragma omp barrier
            
            /* Copy between threads with memcpy */
            int src_tid = (tid + 1) % num_threads;
            if (thread_buffers[src_tid]) {
                __builtin_memcpy(thread_buffers[tid] + 128, 
                               thread_buffers[src_tid], 128);
            }
            
            /* Use memmove within same buffer */
            __builtin_memmove(thread_buffers[tid] + 64, 
                            thread_buffers[tid], 64);
        }
        
        #pragma omp barrier
        
        /* Cleanup */
        if (thread_buffers[tid]) {
            free(thread_buffers[tid]);
        }
    }
}

/* Complex token parsing with memory operations */
static int parse_tokens(const char **tokens, int count) {
    char parse_buffer[512];
    int hash = 0;
    int i = 0;
    
    __builtin_memset(parse_buffer, 0, sizeof(parse_buffer));
    
    /* Process tokens with goto for flow control */
process_loop:
    if (i >= count) goto parse_done;
    
    const char *token = tokens[i];
    size_t token_len = strlen(token);
    
    /* Copy token to buffer using memcpy */
    __builtin_memcpy(parse_buffer + hash % 256, token, 
                    token_len < 32 ? token_len : 32);
    
    /* Move data around with memmove */
    if (i % 3 == 0) {
        __builtin_memmove(parse_buffer + 128, parse_buffer, 64);
    }
    
    /* Update hash */
    for (size_t j = 0; j < token_len && j < 8; j++) {
        hash = (hash * 31 + token[j]) % 1000000;
    }
    
    i++;
    goto process_loop;
    
parse_done:
    /* Final memmove */
    __builtin_memmove(parse_buffer + 256, parse_buffer + 128, 128);
    return hash;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* 1. Test volatile variable memory operations */
    if (volatile_dest && volatile_src) {
        size_t len = volatile_len;
        __builtin_memcpy((void*)volatile_dest, volatile_src, 
                        len > 32 ? 32 : len);
        __builtin_memset((void*)(volatile_dest + 32), 'X', 16);
        __builtin_memmove((void*)(volatile_dest + 16), 
                         volatile_dest, 16);
    }
    
    /* 2. Create and manipulate AST */
    ASTNode *root = create_ast(4, 1);
    if (root && root->left && root->right) {
        /* Copy between AST nodes */
        __builtin_memcpy(root->right->data, root->left->data, 32);
        __builtin_memmove(root->data + 16, root->data, 16);
    }
    
    /* 3. Test goto with memmove */
    test_goto_memmove();
    
    /* 4. Parallel memory operations */
    parallel_memory_operations();
    
    /* 5. Token parsing with memory ops */
    const char *tokens[] = {
        "memcpy", "memset", "memmove", "asan", "hwasan",
        "builtin", "redirection", "coverage", "test"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    int final_hash = parse_tokens(tokens, token_count);
    
    /* 6. Additional built-in calls in different contexts */
    char final_buffer[256];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    
    for (int i = 0; i < 3; i++) {
        switch (i) {
            case 0:
                __builtin_memcpy(final_buffer, "COPY", 4);
                break;
            case 1:
                __builtin_memset(final_buffer + 64, i + 'A', 32);
                break;
            case 2:
                __builtin_memmove(final_buffer + 128, final_buffer, 64);
                break;
        }
    }
    
    /* Calculate verification result */
    int result = final_hash;
    if (root) {
        result = (result + root->id) % 10000;
        /* Cleanup AST */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    printf("Test completed. Result hash: %d\n", result);
    printf("All memory operations should have been redirected by ASAN\n");
    
    return 0;
}
