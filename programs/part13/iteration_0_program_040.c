/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_globals(void) {
    g_init_flag = 1;
    printf("Constructor: Initializing global state\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_globals(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with goto for flow control */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    
    /* Goto-based control flow around memcpy */
    goto copy_start;
    
copy_bypass:
    node->size = 0;
    goto after_copy;
    
copy_start:
    __builtin_memcpy(node->data, base_data, copy_len);
    node->size = copy_len;
    
after_copy:
    /* Create children with different memory patterns */
    char left_data[256];
    char right_data[256];
    
    /* Prepare data for children */
    __builtin_memset(left_data, 'L', sizeof(left_data));
    __builtin_memset(right_data, 'R', sizeof(right_data));
    left_data[sizeof(left_data)-1] = '\0';
    right_data[sizeof(right_data)-1] = '\0';
    
    node->left = create_ast(depth - 1, left_data);
    node->right = create_ast(depth - 1, right_data);
    
    return node;
}

/* Function with complex memory movement using goto */
static void process_ast_with_goto(ASTNode* node) {
    if (!node) return;
    
    ASTNode temp;
    
    /* Jump into memory operation block */
    goto enter_memmove;
    
skip_operation:
    printf("Skipped memmove for node\n");
    return;
    
enter_memmove:
    /* Use __builtin_memmove with goto control */
    if (node->size > 0) {
        __builtin_memmove(&temp, node, sizeof(ASTNode));
        
        /* Modify and move back */
        temp.data[0] = 'M';
        __builtin_memmove(node, &temp, sizeof(ASTNode));
    }
    
    /* Jump out to recursive calls */
    goto process_children;
    
process_children:
    process_ast_with_goto(node->left);
    process_ast_with_goto(node->right);
}

/* Calculate hash of AST data */
static size_t hash_ast(ASTNode* node) {
    if (!node) return 0;
    
    size_t hash = 0;
    volatile size_t i = 0;  /* Prevent optimization */
    
    /* Process data with volatile counter */
    for (i = 0; i < node->size && i < sizeof(node->data); i++) {
        hash = hash * 31 + node->data[i];
    }
    
    return hash + hash_ast(node->left) + hash_ast(node->right);
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

/* Main test function with OpenMP parallel section */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Create complex AST */
    ASTNode* root = create_ast(3, "RootNodeData");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    size_t total_hash = 0;
    
    /* OpenMP parallel region with memory operations */
    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("OpenMP: Running with %d threads\n", omp_get_num_threads());
        }
        
        /* Each thread processes memory operations */
        #pragma omp for reduction(+:total_hash)
        for (int i = 0; i < 10; i++) {
            /* Thread-local buffer operations */
            char buffer1[512];
            char buffer2[512];
            volatile size_t op_size = g_mem_size / (i + 1);
            
            /* Force all three built-ins in parallel context */
            __builtin_memset(buffer1, i, sizeof(buffer1));
            __builtin_memcpy(buffer2, buffer1, op_size);
            
            /* Use memmove with overlap */
            if (i % 2 == 0) {
                __builtin_memmove(buffer1 + 100, buffer1, 200);
            }
            
            /* Contribute to hash */
            #pragma omp critical
            {
                for (size_t j = 0; j < op_size && j < sizeof(buffer1); j++) {
                    total_hash += buffer1[j];
                }
            }
        }
    }
    
    /* Process AST with goto-based control flow */
    process_ast_with_goto(root);
    
    /* Calculate final hash */
    size_t ast_hash = hash_ast(root);
    total_hash += ast_hash;
    
    printf("Result hash: %zu\n", total_hash);
    printf("AST hash component: %zu\n", ast_hash);
    
    /* Cleanup */
    free_ast(root);
    
    /* Final memory operation to ensure all built-ins are used */
    char final_buf[64];
    volatile char* volatile_ptr = final_buf;
    
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
    __builtin_memcpy(final_buf + 16, final_buf, 32);
    __builtin_memmove(final_buf, final_buf + 8, 24);
    
    /* Access through volatile to prevent dead store elimination */
    for (volatile size_t i = 0; i < sizeof(final_buf); i++) {
        volatile_ptr[i] = volatile_ptr[i] + 1;
    }
    
    printf("Test completed successfully\n");
    return 0;
}
