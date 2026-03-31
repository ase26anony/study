#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Complex AST-like structure for data structure access */
typedef struct ASTNode {
    int type;
    int value;
    volatile int volatile_marker;  /* Prevent optimization */
    struct ASTNode* left;
    struct ASTNode* right;
    char padding[32];  /* Ensure size for memcpy operations */
} ASTNode;

/* Global token array */
static volatile int token_array[1024];
static int token_index = 0;

/* Function prototypes */
static void initialize_tokens(void);
static ASTNode* create_ast_node(int type, int value);
static void recursive_parser(ASTNode* node, int depth);
static void parallel_memory_operations(void);
static void memory_operation_with_goto(void);
static int compute_ast_hash(ASTNode* node);
static void cleanup_ast(ASTNode* node);

/* Constructor function (runs before main) */
__attribute__((constructor))
static void constructor_function(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    initialize_tokens();
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void destructor_function(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Initialize token array with non-foldable values */
static void initialize_tokens(void) {
    volatile int seed = 42;
    for (int i = 0; i < 1024; i++) {
        token_array[i] = (i * seed) ^ 0xDEADBEEF;
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
}

/* Create AST node with memory initialization */
static ASTNode* create_ast_node(int type, int value) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize the structure */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = type;
    node->value = value;
    node->volatile_marker = 0xCAFEBABE;
    node->left = NULL;
    node->right = NULL;
    
    /* Initialize padding with pattern */
    for (int i = 0; i < 32; i++) {
        node->padding[i] = (char)((value + i) & 0xFF);
    }
    
    return node;
}

/* Recursive parser with memory operations */
static void recursive_parser(ASTNode* node, int depth) {
    if (!node || depth >= 8) return;
    
    /* Create child nodes */
    node->left = create_ast_node(depth * 2, token_array[token_index++ % 1024]);
    node->right = create_ast_node(depth * 2 + 1, token_array[token_index++ % 1024]);
    
    /* Perform memory copy between nodes if both exist */
    if (node->left && node->right) {
        volatile size_t copy_size = sizeof(ASTNode) - offsetof(ASTNode, padding);
        
        /* Use __builtin_memcpy to copy padding region */
        __builtin_memcpy(node->right->padding, 
                        node->left->padding, 
                        copy_size);
        
        /* Use __builtin_memmove for overlapping regions */
        if (copy_size > 16) {
            __builtin_memmove(node->left->padding + 8,
                            node->left->padding,
                            16);
        }
    }
    
    /* Recursive calls */
    recursive_parser(node->left, depth + 1);
    recursive_parser(node->right, depth + 1);
}

/* Function with goto statements for control flow edge cases */
static void memory_operation_with_goto(void) {
    volatile char buffer1[256];
    volatile char buffer2[256];
    volatile int use_memmove = 0;
    
    /* Initialize buffers with pattern */
    for (int i = 0; i < 256; i++) {
        buffer1[i] = (char)(i & 0xFF);
        buffer2[i] = (char)((i + 128) & 0xFF);
    }
    
    /* Label for goto */
    use_memcpy:
    {
        volatile size_t len = 128;
        __builtin_memcpy((void*)buffer2, (void*)buffer1, len);
        
        if (use_memmove < 2) {
            use_memmove++;
            goto use_memmove;
        }
    }
    goto finish;
    
    use_memmove:
    {
        volatile size_t len = 64;
        /* Overlapping memory regions to force memmove */
        __builtin_memmove((void*)(buffer1 + 32), 
                         (void*)buffer1, 
                         len);
        goto use_memcpy;
    }
    
    finish:
    /* Final memset */
    __builtin_memset((void*)buffer1, 0xAA, 128);
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        volatile char local_buffer[512];
        volatile char shared_buffer[1024];
        
        /* Initialize with thread-specific pattern */
        #pragma omp for
        for (int i = 0; i < 512; i++) {
            local_buffer[i] = (char)((thread_id * 256 + i) & 0xFF);
        }
        
        /* Barrier to ensure initialization */
        #pragma omp barrier
        
        /* Perform memory operations in parallel */
        #pragma omp for
        for (int i = 0; i < 8; i++) {
            volatile size_t offset = i * 64;
            volatile size_t len = 48 + (i * 8);
            
            /* Mix of memory builtins */
            switch (i % 3) {
                case 0:
                    __builtin_memcpy((void*)(shared_buffer + offset),
                                    (void*)local_buffer,
                                    len);
                    break;
                case 1:
                    __builtin_memset((void*)(shared_buffer + offset),
                                    0xCC,
                                    len);
                    break;
                case 2:
                    __builtin_memmove((void*)(shared_buffer + offset + 16),
                                     (void*)(shared_buffer + offset),
                                     len);
                    break;
            }
        }
        
        /* Verify operations with another memset */
        #pragma omp single
        {
            __builtin_memset((void*)shared_buffer, 0xFF, 256);
        }
    }
}

/* Compute hash of AST for verification */
static int compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    int hash = node->type ^ node->value ^ node->volatile_marker;
    
    /* Include padding in hash */
    for (int i = 0; i < 32; i++) {
        hash = (hash * 31) + node->padding[i];
    }
    
    return hash + compute_ast_hash(node->left) + compute_ast_hash(node->right);
}

/* Cleanup AST recursively */
static void cleanup_ast(ASTNode* node) {
    if (!node) return;
    
    cleanup_ast(node->left);
    cleanup_ast(node->right);
    
    /* Clear memory before free */
    __builtin_memset(node, 0, sizeof(ASTNode));
    free(node);
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Create root AST node */
    ASTNode* root = create_ast_node(0, token_array[0]);
    if (!root) {
        fprintf(stderr, "Failed to create AST root\n");
        return 1;
    }
    
    /* Build recursive AST with memory operations */
    recursive_parser(root, 0);
    
    /* Execute control flow with goto */
    memory_operation_with_goto();
    
    /* Perform parallel memory operations */
    parallel_memory_operations();
    
    /* Compute and print verification hash */
    int final_hash = compute_ast_hash(root);
    printf("AST verification hash: 0x%08X\n", final_hash);
    
    /* Additional memory operations in main */
    volatile char final_buffer[1024];
    volatile size_t operations[] = {256, 512, 768, 1024};
    
    for (int i = 0; i < 4; i++) {
        volatile size_t len = operations[i] % 512;
        
        /* Cycle through all three builtins */
        if (i % 3 == 0) {
            __builtin_memcpy((void*)final_buffer,
                            (void*)token_array,
                            len);
        } else if (i % 3 == 1) {
            __builtin_memset((void*)final_buffer,
                            0x55,
                            len);
        } else {
            __builtin_memmove((void*)(final_buffer + len/2),
                             (void*)final_buffer,
                             len/2);
        }
    }
    
    /* Cleanup */
    cleanup_ast(root);
    
    printf("ASAN test completed successfully\n");
    return 0;
}
