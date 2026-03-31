/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 256;

/* Recursive AST-like structure */
struct ast_node {
    int type;
    char data[256];
    struct ast_node *left;
    struct ast_node *right;
};

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_constructor(void) {
    printf("Constructor: Initializing ASAN/HWASAN environment\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_destructor(void) {
    printf("Destructor: Program completed\n");
}

/* Recursive function with memory operations */
static int process_ast(struct ast_node *node, int depth) {
    if (!node || depth > 5) return 0;
    
    int sum = node->type;
    
    /* Use __builtin_memcpy between nodes */
    if (node->left && node->right) {
        __builtin_memcpy(node->left->data, 
                        node->right->data, 
                        g_memcpy_len);
    }
    
    /* Recursive processing */
    sum += process_ast(node->left, depth + 1);
    sum += process_ast(node->right, depth + 1);
    
    return sum;
}

/* Function with goto control flow */
static void goto_memory_operations(char *buf1, char *buf2, size_t len) {
    int use_memmove = 0;
    
    /* Jump into memory operation block */
    if (len > 100) goto do_operation;
    
    /* Normal path */
    __builtin_memset(buf1, 0xAA, len);
    return;
    
do_operation:
    /* Overlapping memory operation */
    __builtin_memmove(buf1 + 10, buf1, len - 10);
    use_memmove = 1;
    
    /* Jump out */
    if (use_memmove) goto cleanup;
    
cleanup:
    __builtin_memset(buf2, 0, len);
}

/* OpenMP parallel memory dispatcher */
static void parallel_memory_dispatch(int num_threads) {
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        char local_buf[512];
        char shared_buf[1024];
        
        /* Thread-specific memory operations */
        __builtin_memset(local_buf, tid, sizeof(local_buf));
        
        #pragma omp barrier
        
        /* Collective memory copy */
        #pragma omp for
        for (int i = 0; i < 8; i++) {
            __builtin_memcpy(&shared_buf[i * 64], 
                           local_buf, 
                           g_memcpy_len);
        }
        
        /* Memory move with overlap */
        if (tid == 0) {
            __builtin_memmove(shared_buf + 128, 
                            shared_buf, 
                            g_memmove_len);
        }
    }
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Initialize complex data structures */
    struct ast_node *root = calloc(1, sizeof(struct ast_node));
    struct ast_node *left = calloc(1, sizeof(struct ast_node));
    struct ast_node *right = calloc(1, sizeof(struct ast_node));
    
    if (!root || !left || !right) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    root->type = 1;
    left->type = 2;
    right->type = 3;
    
    /* Initialize node data */
    __builtin_memset(root->data, 'R', sizeof(root->data));
    __builtin_memset(left->data, 'L', sizeof(left->data));
    __builtin_memset(right->data, 'R', sizeof(right->data));
    
    root->left = left;
    root->right = right;
    
    /* Test goto control flow */
    char buffer1[1024], buffer2[1024];
    goto_memory_operations(buffer1, buffer2, g_memmove_len);
    
    /* Process AST recursively */
    int ast_sum = process_ast(root, 0);
    
    /* Execute parallel operations */
    parallel_memory_dispatch(4);
    
    /* Final memory operations to ensure all built-ins are used */
    char final_buf[2048];
    __builtin_memcpy(final_buf, root->data, g_memcpy_len);
    __builtin_memset(final_buf + 512, 0xFF, g_memset_len);
    
    /* Overlapping operation */
    __builtin_memmove(final_buf + 256, final_buf, 512);
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(final_buf); i++) {
        hash = hash * 31 + final_buf[i];
    }
    
    printf("AST sum: %d\n", ast_sum);
    printf("Final hash: %lu\n", hash);
    printf("Verification: %s\n", (hash != 0 && ast_sum == 6) ? "PASS" : "FAIL");
    
    /* Cleanup */
    free(left);
    free(right);
    free(root);
    
    return 0;
}
