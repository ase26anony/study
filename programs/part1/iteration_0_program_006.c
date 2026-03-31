/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int depth;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_asan_environment(void) {
    printf("Initializing ASAN environment...\n");
    /* Force initialization of memory function caches */
    char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_asan_environment(void) {
    printf("Cleaning up ASAN environment...\n");
}

/* Recursive AST creation with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for node initialization */
    __builtin_memcpy(node->data, base_data, strlen(base_data) + 1);
    node->depth = depth;
    
    /* Create children with goto-controlled flow */
    int create_left = 1;
    if (depth > 3) {
        goto skip_left;
    }
    
    node->left = create_ast(depth - 1, "left_child");
    create_left = 0;
    
skip_left:
    if (create_left) {
        node->left = create_ast(depth - 2, "skipped_left");
    }
    
    /* Control flow with goto around memmove */
    volatile int use_memmove = 1;
    if (depth % 2 == 0) {
        goto no_memmove;
    }
    
    if (use_memmove) {
        char temp[64];
        __builtin_memmove(temp, node->data, sizeof(node->data));
        __builtin_memmove(node->data, "moved_data", 11);
    }
    
no_memmove:
    node->right = create_ast(depth - 1, "right_child");
    
    return node;
}

/* Process AST with memory operations */
static size_t process_ast(ASTNode* node, size_t* hash) {
    if (!node) return 0;
    
    size_t local_hash = 0;
    
    /* Use volatile to prevent folding */
    volatile size_t copy_size = g_mem_size % 64;
    
    /* Memory operations on node data */
    char buffer[64];
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, node->data, copy_size);
    
    /* Complex goto pattern around memmove */
    int do_memmove = (node->depth % 3 == 0);
    
    if (do_memmove) {
        goto perform_memmove;
    } else {
        goto skip_memmove;
    }
    
perform_memmove:
    {
        char temp[64];
        __builtin_memmove(temp, buffer, copy_size);
        __builtin_memmove(buffer, temp, copy_size);
        goto after_memmove;
    }
    
skip_memmove:
    /* Alternative path */
    __builtin_memset(buffer + 10, 'X', 5);
    
after_memmove:
    /* Calculate hash */
    for (size_t i = 0; i < copy_size; i++) {
        local_hash = local_hash * 31 + buffer[i];
    }
    
    *hash += local_hash;
    
    /* Recursive processing */
    size_t left_count = process_ast(node->left, hash);
    size_t right_count = process_ast(node->right, hash);
    
    return 1 + left_count + right_count;
}

/* Free AST with memory sanitization */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear memory before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    const int num_arrays = 8;
    char* arrays[num_arrays];
    
    #pragma omp parallel for
    for (int i = 0; i < num_arrays; i++) {
        size_t size = (g_mem_size * (i + 1)) % 512 + 64;
        arrays[i] = (char*)malloc(size);
        
        if (arrays[i]) {
            /* Force built-in usage in parallel region */
            __builtin_memset(arrays[i], i, size);
            
            /* Conditional memcpy with goto */
            if (i % 2 == 0) {
                goto copy_section;
            } else {
                goto move_section;
            }
            
        copy_section:
            {
                char dest[256];
                __builtin_memcpy(dest, arrays[i], size > 256 ? 256 : size);
                goto after_ops;
            }
            
        move_section:
            {
                char temp[256];
                volatile size_t move_len = size > 128 ? 128 : size;
                __builtin_memmove(temp, arrays[i], move_len);
                __builtin_memmove(arrays[i] + 50, temp, move_len);
            }
            
        after_ops:
            /* Additional memset */
            __builtin_memset(arrays[i] + size - 16, 0xFF, 16);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_arrays; i++) {
        if (arrays[i]) {
            free(arrays[i]);
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize token array */
    char tokens[][32] = {
        "token1", "token2", "token3", "token4",
        "token5", "token6", "token7", "token8"
    };
    
    /* Process tokens with memory operations */
    for (size_t i = 0; i < sizeof(tokens)/sizeof(tokens[0]); i++) {
        char buffer[32];
        volatile size_t len = strlen(tokens[i]) + 1;
        
        __builtin_memset(buffer, 0, sizeof(buffer));
        __builtin_memcpy(buffer, tokens[i], len);
        
        /* Conditional memmove with goto */
        if (i % 3 == 0) {
            goto shuffle_token;
        }
        
        continue;
        
    shuffle_token:
        __builtin_memmove(buffer + 2, buffer, len - 2);
        __builtin_memmove(buffer, "SHF", 3);
    }
    
    /* Create and process AST */
    ASTNode* root = create_ast(5, "root_node");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    size_t ast_hash = 0;
    size_t node_count = process_ast(root, &ast_hash);
    printf("Processed %zu AST nodes, hash: %zu\n", node_count, ast_hash);
    
    /* Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Final built-in calls to ensure cache initialization */
    char final_buffer[128];
    volatile size_t final_size = g_mem_size % 128;
    
    __builtin_memset(final_buffer, 0xAA, final_size);
    __builtin_memcpy(final_buffer + 32, "FINAL", 6);
    __builtin_memmove(final_buffer, final_buffer + 16, 32);
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully.\n");
    return 0;
}
