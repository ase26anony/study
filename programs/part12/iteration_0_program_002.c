/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char *volatile_dest = NULL;
static volatile char *volatile_src = NULL;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[32];
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *parent;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_asan_environment(void) {
    printf("Constructor: Initializing ASAN environment\n");
    
    /* Force early initialization of memory builtins */
    char buffer1[128];
    char buffer2[128];
    
    /* Use __builtin_memset in constructor */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    
    /* Use __builtin_memcpy in constructor */
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    
    /* Use __builtin_memmove in constructor */
    __builtin_memmove(buffer1 + 32, buffer1, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_asan_environment(void) {
    printf("Destructor: Cleaning up ASAN environment\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with __builtin_memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    
    /* Fill data with pattern using __builtin_memset */
    __builtin_memset(node->data, 'A' + depth, 31);
    node->data[31] = '\0';
    
    /* Create children recursively */
    node->left = create_ast(depth + 1, max_depth);
    node->left = create_ast(depth + 1, max_depth);
    
    return node;
}

/* Function with goto statements and memory operations */
static void process_with_goto(ASTNode *node1, ASTNode *node2) {
    int state = 0;
    
    /* Jump into block with memory operation */
    goto start_block;
    
    /* Target label for goto */
    memory_block:
        /* Use __builtin_memmove with goto flow */
        if (node1 && node2) {
            __builtin_memmove(node1->data, node2->data, 16);
        }
        goto end_block;
    
    start_block:
        /* Use __builtin_memcpy before goto */
        if (node1) {
            char temp[32];
            __builtin_memcpy(temp, node1->data, 16);
            state = 1;
        }
        
        /* Conditional goto */
        if (state) {
            goto memory_block;
        }
    
    end_block:
        /* Final __builtin_memset */
        if (node1) {
            __builtin_memset(node1->data + 16, 0xCC, 8);
        }
}

/* Function with OpenMP parallel section */
static void parallel_memory_operations(void) {
    const int array_size = 1024;
    char *src_array = malloc(array_size);
    char *dest_array = malloc(array_size);
    
    if (!src_array || !dest_array) {
        free(src_array);
        free(dest_array);
        return;
    }
    
    /* Initialize source with pattern */
    for (int i = 0; i < array_size; i++) {
        src_array[i] = (char)(i % 256);
    }
    
    /* OpenMP parallel region with memory operations */
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int chunk_size = array_size / omp_get_num_threads();
        int start = thread_id * chunk_size;
        int end = (thread_id == omp_get_num_threads() - 1) ? 
                  array_size : start + chunk_size;
        
        /* Each thread uses builtins independently */
        __builtin_memcpy(dest_array + start, src_array + start, end - start);
        
        /* Additional memset in parallel */
        __builtin_memset(dest_array + start + (end - start)/2, 
                        thread_id, (end - start)/4);
    }
    
    /* Verify with memmove across thread boundaries */
    __builtin_memmove(dest_array, dest_array + array_size/2, array_size/4);
    
    free(src_array);
    free(dest_array);
}

/* Main execution flow */
int main(void) {
    printf("Main: Starting ASAN built-in redirection test\n");
    
    /* Initialize volatile variables */
    char buffer1[256];
    char buffer2[256];
    volatile_dest = buffer1;
    volatile_src = buffer2;
    
    /* Phase 1: Direct builtin calls with volatile lengths */
    printf("Phase 1: Direct builtin calls\n");
    
    __builtin_memset(volatile_dest, 0x55, volatile_len);
    __builtin_memcpy((char*)volatile_dest + 32, volatile_src, volatile_len/2);
    __builtin_memmove((char*)volatile_dest, (char*)volatile_dest + 64, 32);
    
    /* Phase 2: Recursive AST operations */
    printf("Phase 2: Recursive AST operations\n");
    
    ASTNode *root1 = create_ast(0, 4);
    ASTNode *root2 = create_ast(0, 3);
    
    if (root1 && root2) {
        /* Copy between AST nodes */
        __builtin_memcpy(root1->data, root2->data, sizeof(root1->data));
        
        /* Process with goto flow */
        process_with_goto(root1, root2);
        
        /* Cleanup */
        free(root1);
        free(root2);
    }
    
    /* Phase 3: OpenMP parallel operations */
    printf("Phase 3: OpenMP parallel operations\n");
    parallel_memory_operations();
    
    /* Phase 4: Complex token array processing */
    printf("Phase 4: Complex token processing\n");
    
    const int token_count = 1000;
    int *tokens = malloc(token_count * sizeof(int));
    int *tokens_copy = malloc(token_count * sizeof(int));
    
    if (tokens && tokens_copy) {
        /* Initialize tokens */
        for (int i = 0; i < token_count; i++) {
            tokens[i] = i * 3;
        }
        
        /* Use builtins on integer arrays */
        __builtin_memcpy(tokens_copy, tokens, token_count * sizeof(int));
        __builtin_memset(tokens + token_count/2, 0, (token_count/4) * sizeof(int));
        __builtin_memmove(tokens, tokens_copy + token_count/3, 
                         (token_count/3) * sizeof(int));
        
        /* Compute verification hash */
        unsigned long long hash = 0;
        for (int i = 0; i < token_count; i++) {
            hash = hash * 31 + tokens[i];
        }
        
        printf("Verification hash: %llu\n", hash);
        
        free(tokens);
        free(tokens_copy);
    }
    
    printf("Main: Test completed successfully\n");
    return 0;
}
