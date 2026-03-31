/* ISO C99-compliant test program for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_token_pool[4096];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token pool with pattern */
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i * 13) & 0xFF);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->left = NULL;
    node->right = NULL;
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy pattern into node using __builtin_memcpy */
    size_t copy_size = (size_t)(id % 64);
    if (copy_size > 0) {
        __builtin_memcpy(node->data, &g_token_pool[g_token_index], copy_size);
        g_token_index = (g_token_index + copy_size) % sizeof(g_token_pool);
    }
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int left_id = id * 2;
        int right_id = id * 2 + 1;
        
        /* Use goto to create interesting control flow */
        if (id % 3 == 0) {
            goto create_left;
        } else if (id % 3 == 1) {
            goto create_right;
        } else {
            goto create_both;
        }
        
    create_left:
        node->left = create_ast(depth - 1, left_id);
        goto done;
        
    create_right:
        node->right = create_ast(depth - 1, right_id);
        goto done;
        
    create_both:
        node->left = create_ast(depth - 1, left_id);
        node->right = create_ast(depth - 1, right_id);
        goto done;
        
    done:;
    }
    
    return node;
}

/* Function with __builtin_memmove and goto */
static void rearrange_ast_data(ASTNode* node) {
    if (!node) return;
    
    char temp[64];
    volatile int use_memmove = 1;
    
    if (use_memmove) {
        /* Use goto to jump into memmove block */
        goto do_memmove;
        
    do_memmove:
        /* Force __builtin_memmove usage */
        __builtin_memmove(temp, node->data, sizeof(node->data));
        
        /* Jump out of block */
        goto after_memmove;
    }
    
after_memmove:
    /* Reverse copy using __builtin_memcpy */
    for (int i = 0; i < 32; i++) {
        char c = temp[i];
        temp[i] = temp[63 - i];
        temp[63 - i] = c;
    }
    __builtin_memcpy(node->data, temp, sizeof(node->data));
    
    /* Recursive processing */
    rearrange_ast_data(node->left);
    rearrange_ast_data(node->right);
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_ops(void) {
    const int num_arrays = 8;
    char* arrays[num_arrays];
    volatile size_t local_size = g_mem_size;
    
    /* Allocate arrays */
    for (int i = 0; i < num_arrays; i++) {
        arrays[i] = malloc(local_size);
        if (!arrays[i]) return;
    }
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        #pragma omp for
        for (int i = 0; i < num_arrays; i++) {
            /* Each thread uses builtins with volatile sizes */
            volatile size_t op_size = local_size / (thread_id + 2);
            if (op_size > local_size) op_size = local_size;
            
            /* Use all three builtins */
            __builtin_memset(arrays[i], thread_id, op_size);
            
            if (i > 0) {
                __builtin_memcpy(arrays[i], arrays[i-1], op_size);
            }
            
            /* Circular shift with memmove */
            if (op_size > 16) {
                __builtin_memmove(arrays[i], arrays[i] + 8, op_size - 8);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_arrays; i++) {
        free(arrays[i]);
    }
}

/* Compute hash from AST */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    
    /* Process node data */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        hash = ((hash << 5) + hash) + (unsigned long)node->data[i];
    }
    
    /* Recursive hash combination */
    hash += compute_ast_hash(node->left);
    hash ^= compute_ast_hash(node->right);
    
    return hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    /* Use goto for non-linear control flow */
    if (node->left) {
        goto free_left;
    }
    
free_left:
    free_ast(node->left);
    
    if (node->right) {
        goto free_right;
    }
    
free_right:
    free_ast(node->right);
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Create complex AST */
    ASTNode* root = create_ast(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Rearrange data with memmove */
    rearrange_ast_data(root);
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Additional memory operations in main */
    char buffer1[512];
    char buffer2[512];
    volatile size_t copy_len = g_mem_size % 512;
    
    /* Force all three builtins in main */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memset(buffer2, 0xBB, sizeof(buffer2));
    
    __builtin_memcpy(buffer1, buffer2, copy_len);
    
    /* Use memmove with overlapping regions */
    __builtin_memmove(buffer1 + 100, buffer1, 200);
    
    /* Compute and print result */
    unsigned long final_hash = compute_ast_hash(root);
    printf("AST Hash: %lu\n", final_hash);
    
    /* Cleanup */
    free_ast(root);
    
    /* Final memory operation */
    __builtin_memset(&final_hash, 0, sizeof(final_hash));
    
    printf("Test completed successfully\n");
    return 0;
}
