#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Complex AST-like structure for recursive parsing */
typedef struct ASTNode {
    int type;
    int value;
    volatile int volatile_marker;  /* Prevent optimization */
    struct ASTNode* left;
    struct ASTNode* right;
    char padding[32];  /* Ensure size for memcpy operations */
} ASTNode;

/* Global token array with volatile elements */
volatile int token_array[256];
volatile int token_index = 0;

/* Function prototypes */
ASTNode* create_ast_node(int type, int value);
void recursive_parser(ASTNode* node, int depth);
void parallel_memory_operations(void);
void memory_operation_with_goto(void);
int compute_hash(ASTNode* root);
void __attribute__((constructor)) init_tokens(void);
void __attribute__((destructor)) cleanup(void);

/* Constructor - runs before main() */
void __attribute__((constructor)) init_tokens(void) {
    volatile int i;
    for (i = 0; i < 256; i++) {
        token_array[i] = (i * 13) % 256;
    }
    token_index = 0;
    
    /* Early builtin usage in constructor */
    volatile char buffer[64];
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, "init", 5);
}

/* Create AST node with memory initialization */
ASTNode* create_ast_node(int type, int value) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins for memory operations */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = type;
    node->value = value;
    node->volatile_marker = 1;  /* Force volatile access */
    node->left = NULL;
    node->right = NULL;
    
    /* Copy padding with builtin */
    const char* pad = "AST_NODE_PADDING_DATA_1234567890";
    __builtin_memcpy(node->padding, pad, 32);
    
    return node;
}

/* Recursive parser with memory operations between nodes */
void recursive_parser(ASTNode* node, int depth) {
    if (depth >= 5 || !node) return;
    
    /* Create child nodes */
    node->left = create_ast_node(depth * 2, depth * 100);
    node->right = create_ast_node(depth * 2 + 1, depth * 100 + 1);
    
    /* Copy data between nodes using builtins */
    if (node->left && node->right) {
        volatile size_t copy_size = sizeof(ASTNode) - 16;
        __builtin_memcpy(node->right->padding, 
                        node->left->padding, 
                        copy_size);
        
        /* Move data around */
        char temp[32];
        __builtin_memcpy(temp, node->left->padding, 32);
        __builtin_memmove(node->left->padding, 
                         node->right->padding, 
                         32);
        __builtin_memcpy(node->right->padding, temp, 32);
    }
    
    /* Recursive calls */
    recursive_parser(node->left, depth + 1);
    recursive_parser(node->right, depth + 1);
}

/* Complex goto-based memory operation */
void memory_operation_with_goto(void) {
    volatile char src[128];
    volatile char dst[128];
    volatile int use_memmove = 0;
    
    /* Initialize source with pattern */
    for (int i = 0; i < 128; i++) {
        src[i] = (char)(i % 26 + 'A');
    }
    
    /* Jump into memory operation block */
    goto start_block;
    
memory_ops:
    /* This block contains builtin memory operations */
    if (use_memmove) {
        /* Overlapping regions force memmove */
        __builtin_memmove(&dst[32], &dst[0], 64);
    } else {
        /* Non-overlapping uses memcpy */
        __builtin_memcpy(dst, src, 64);
    }
    goto end_block;
    
start_block:
    /* First pass with memcpy */
    use_memmove = 0;
    goto memory_ops;
    
mid_block:
    /* Second pass with memmove */
    use_memmove = 1;
    goto memory_ops;
    
end_block:
    /* Jump back for second operation */
    if (use_memmove == 0) {
        /* Set up overlapping regions */
        __builtin_memcpy(dst, src, 128);
        goto mid_block;
    }
}

/* Parallel memory operations using OpenMP */
void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        volatile char local_buffer[1024];
        volatile char shared_buffer[1024];
        
        /* Each thread initializes its buffer differently */
        #pragma omp for
        for (int i = 0; i < 1024; i++) {
            local_buffer[i] = (char)((i + thread_id) % 256);
        }
        
        /* Barrier to ensure initialization */
        #pragma omp barrier
        
        /* Memory operations in parallel */
        #pragma omp single
        {
            /* Master thread sets up shared buffer */
            __builtin_memset(shared_buffer, 0, 1024);
        }
        
        #pragma omp barrier
        
        /* Each thread copies to shared buffer */
        volatile size_t chunk_size = 1024 / omp_get_num_threads();
        volatile size_t offset = thread_id * chunk_size;
        
        __builtin_memcpy(&shared_buffer[offset], 
                        &local_buffer[offset], 
                        chunk_size);
        
        /* Use memmove for overlapping within thread's region */
        if (chunk_size > 64) {
            __builtin_memmove(&shared_buffer[offset + 32],
                            &shared_buffer[offset],
                            64);
        }
        
        #pragma omp barrier
        
        /* Verify with memset pattern */
        __builtin_memset(&local_buffer[offset], thread_id, chunk_size);
    }
}

/* Compute hash from AST for verification */
int compute_hash(ASTNode* root) {
    if (!root) return 0;
    
    volatile int hash = root->value;
    char buffer[64];
    
    /* Use builtins for memory operations in hash computation */
    __builtin_memset(buffer, 0, 64);
    __builtin_memcpy(buffer, &root->type, sizeof(int));
    __builtin_memcpy(buffer + 4, &root->value, sizeof(int));
    
    for (int i = 0; i < 64; i++) {
        hash = (hash * 31 + buffer[i]) % 1000000007;
    }
    
    hash += compute_hash(root->left);
    hash += compute_hash(root->right);
    
    return hash;
}

/* Destructor - runs after main() */
void __attribute__((destructor)) cleanup(void) {
    volatile char cleanup_buf[128];
    __builtin_memset(cleanup_buf, 0xFF, 128);
    
    /* Final memory operations in destructor */
    char final_msg[] = "Cleanup completed";
    __builtin_memcpy(cleanup_buf, final_msg, sizeof(final_msg));
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN coverage test program\n");
    
    /* Initialize root AST node */
    ASTNode* root = create_ast_node(1, 1000);
    if (!root) {
        fprintf(stderr, "Failed to create AST root\n");
        return 1;
    }
    
    /* Build recursive AST structure */
    recursive_parser(root, 0);
    
    /* Execute goto-based memory operations */
    memory_operation_with_goto();
    
    /* Perform parallel memory operations */
    parallel_memory_operations();
    
    /* Compute verification hash */
    int result_hash = compute_hash(root);
    printf("Computed hash: %d\n", result_hash);
    
    /* Additional builtin usage in main */
    volatile int final_check[4] = {0};
    __builtin_memset(final_check, 0, sizeof(final_check));
    __builtin_memcpy(final_check, &result_hash, sizeof(int));
    
    /* Use all three builtins in sequence */
    volatile char verification[256];
    __builtin_memset(verification, 0, 256);
    __builtin_memcpy(verification, "VERIFY", 7);
    __builtin_memmove(verification + 32, verification, 64);
    
    printf("Program completed successfully\n");
    
    /* Cleanup */
    free(root);
    
    return 0;
}
