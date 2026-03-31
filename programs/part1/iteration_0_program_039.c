/* ISO C99-compliant test program for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    printf("Constructor: Initializing ASAN environment\n");
    /* Force early initialization of memory builtins */
    char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    printf("Destructor: Cleaning up ASAN environment\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = (*counter)++;
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[64];
    for (int i = 0; i < 64; i++) {
        pattern[i] = (char)(i + depth);
    }
    __builtin_memcpy(node->data, pattern, 64);
    
    /* Recursive creation with goto for control flow */
    int create_left = 1;
    
    if (depth > 2) {
        goto skip_left;
    }
    
    create_left_label:
    node->left = create_ast(depth - 1, counter);
    
    skip_left:
    if (!create_left) {
        create_left = 1;
        goto create_left_label;
    }
    
    node->right = create_ast(depth - 1, counter);
    
    return node;
}

/* Function with complex control flow and __builtin_memmove */
static void process_ast(ASTNode* node, char* output) {
    if (!node) return;
    
    char temp[128];
    int use_memmove = 0;
    
    /* Complex control flow with goto */
    if (node->type % 3 == 0) {
        goto use_memcpy_path;
    } else if (node->type % 3 == 1) {
        goto use_memmove_path;
    } else {
        goto use_memset_path;
    }
    
use_memcpy_path:
    __builtin_memcpy(temp, node->data, 64);
    goto process_children;
    
use_memmove_path:
    use_memmove = 1;
    /* Create overlapping regions for memmove */
    char overlap[96];
    __builtin_memcpy(overlap, node->data, 64);
    __builtin_memmove(overlap + 16, overlap, 64);
    __builtin_memcpy(temp, overlap + 16, 64);
    goto process_children;
    
use_memset_path:
    __builtin_memset(temp, node->value, 64);
    /* Fall through */
    
process_children:
    /* Process children recursively */
    process_ast(node->left, output);
    process_ast(node->right, output);
    
    /* Copy result to output with volatile size */
    size_t copy_size = g_mem_size % 128;
    if (copy_size > 64) copy_size = 64;
    
    if (use_memmove) {
        __builtin_memmove(output, temp, copy_size);
    } else {
        __builtin_memcpy(output, temp, copy_size);
    }
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear memory before free */
    __builtin_memset(node, 0, sizeof(ASTNode));
    free(node);
}

/* Main test function with OpenMP parallelization */
int main(void) {
    int counter = 0;
    char final_output[256] = {0};
    
    printf("Creating AST structure...\n");
    ASTNode* root = create_ast(4, &counter);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    printf("Processing AST with parallel memory operations...\n");
    
    /* OpenMP parallel region with memory operations */
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char thread_buffer[128];
        
        /* Each thread processes memory operations */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            /* Use all three builtins in different patterns */
            if (i % 3 == 0) {
                __builtin_memset(thread_buffer, i, sizeof(thread_buffer));
            } else if (i % 3 == 1) {
                char src[128];
                for (int j = 0; j < 128; j++) src[j] = (char)(j + i);
                __builtin_memcpy(thread_buffer, src, sizeof(thread_buffer));
            } else {
                /* Create overlapping copy for memmove */
                char overlap[160];
                for (int j = 0; j < 160; j++) overlap[j] = (char)(j * 2);
                __builtin_memmove(overlap + 32, overlap, 128);
                __builtin_memcpy(thread_buffer, overlap + 32, sizeof(thread_buffer));
            }
            
            /* Reduce results */
            #pragma omp critical
            {
                for (int j = 0; j < 64; j++) {
                    final_output[j] ^= thread_buffer[j];
                }
            }
        }
    }
    
    /* Process AST with main thread */
    process_ast(root, final_output + 128);
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < 256; i++) {
        hash = hash * 31 + (unsigned char)final_output[i];
    }
    
    printf("Result hash: 0x%08lx\n", hash);
    printf("Operations completed successfully\n");
    
    /* Cleanup */
    free_ast(root);
    
    return 0;
}
