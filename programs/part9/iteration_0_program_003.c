/* asan_coverage.c - Comprehensive test for ASAN memory built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_selector = 0;

/* AST-like recursive structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char global_tokens[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_constructor(void) {
    /* Initialize with pattern to detect corruption */
    for (int i = 0; i < sizeof(global_tokens); i++) {
        global_tokens[i] = (char)(i % 256);
    }
    printf("Constructor: Global tokens initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Verify data integrity */
    int errors = 0;
    for (int i = 0; i < 256; i++) {
        if (global_tokens[i] != (char)(i % 256)) {
            errors++;
        }
    }
    printf("Destructor: Found %d memory errors\n", errors);
}

/* Recursive AST creation and manipulation */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node->data, depth % 256, sizeof(node->data));
    node->size = sizeof(node->data);
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    return node;
}

/* Complex function with goto and memory operations */
static void complex_memory_operations(void* dest, void* src, size_t len) {
    void* temp = malloc(len);
    if (!temp) return;
    
    /* Use goto to create non-linear control flow */
    int use_memmove = 0;
    
    if (volatile_selector > 0) {
        use_memmove = 1;
        goto memmove_block;
    }
    
    /* Regular memcpy path */
    __builtin_memcpy(temp, src, len);
    goto after_copy;
    
memmove_block:
    /* This block tests memmove with overlapping regions */
    __builtin_memmove(temp, src, len);
    
after_copy:
    /* Modify data to ensure it's not optimized away */
    for (size_t i = 0; i < len && i < 16; i++) {
        ((char*)temp)[i] ^= 0x55;
    }
    
    __builtin_memcpy(dest, temp, len);
    free(temp);
}

/* OpenMP parallel memory operations */
static void parallel_memory_dispatch(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i] && nodes[i]->left && nodes[i]->right) {
                /* Copy between child nodes using builtins */
                size_t copy_len = nodes[i]->left->size;
                if (copy_len > sizeof(nodes[i]->left->data)) {
                    copy_len = sizeof(nodes[i]->left->data);
                }
                
                /* Force different builtins based on thread */
                switch (thread_id % 3) {
                    case 0:
                        __builtin_memcpy(nodes[i]->right->data, 
                                        nodes[i]->left->data, 
                                        copy_len);
                        break;
                    case 1:
                        __builtin_memset(nodes[i]->right->data, 
                                        thread_id, 
                                        copy_len / 2);
                        break;
                    case 2:
                        /* Create overlapping region for memmove */
                        __builtin_memmove(nodes[i]->data + 128,
                                         nodes[i]->data,
                                         copy_len);
                        break;
                }
            }
        }
    }
}

/* Recursive parser simulation */
static int recursive_parser(ASTNode* node, int depth) {
    if (!node || depth <= 0) return 0;
    
    int result = 0;
    
    /* Jump label for control flow testing */
    if (depth % 3 == 0) {
        goto process_data;
    }
    
    /* Normal processing */
    for (size_t i = 0; i < node->size && i < 32; i++) {
        result += node->data[i];
    }
    
    goto continue_recursion;

process_data:
    /* Process with memmove on overlapping data */
    __builtin_memmove(node->data + 16, node->data, 32);
    for (size_t i = 0; i < 32; i++) {
        result += node->data[i + 16];
    }

continue_recursion:
    result += recursive_parser(node->left, depth - 1);
    result += recursive_parser(node->right, depth - 1);
    
    return result;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Create AST structure */
    ASTNode* root = create_ast(4);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    ASTNode* nodes[8];
    nodes[0] = root;
    for (int i = 1; i < 8; i++) {
        nodes[i] = create_ast(3);
    }
    
    /* Test 1: Direct builtin calls with volatile lengths */
    size_t test_len = volatile_len;
    char buffer1[256], buffer2[256];
    
    __builtin_memset(buffer1, 0xAA, test_len);
    __builtin_memcpy(buffer2, buffer1, test_len);
    __builtin_memmove(buffer1 + 32, buffer1, test_len - 32);
    
    /* Test 2: Complex operations with goto */
    complex_memory_operations(buffer2, buffer1, test_len);
    
    /* Test 3: OpenMP parallel operations */
    parallel_memory_dispatch(nodes, 8);
    
    /* Test 4: Recursive parsing with control flow */
    int hash_result = recursive_parser(root, 4);
    
    /* Test 5: Global token manipulation */
    #pragma omp parallel for
    for (int i = 0; i < 256; i++) {
        size_t offset = (i * 13) % sizeof(global_tokens);
        size_t len = (i % 32) + 1;
        
        if (i % 3 == 0) {
            __builtin_memset(global_tokens + offset, i, len);
        } else if (i % 3 == 1) {
            __builtin_memcpy(global_tokens + offset, 
                           global_tokens + ((i * 17) % sizeof(global_tokens)), 
                           len);
        } else {
            __builtin_memmove(global_tokens + offset + len/2,
                            global_tokens + offset,
                            len/2);
        }
    }
    
    /* Calculate final verification hash */
    unsigned long final_hash = 0;
    for (int i = 0; i < 256; i++) {
        final_hash = final_hash * 31 + buffer1[i];
        final_hash = final_hash * 31 + buffer2[i];
        final_hash = final_hash * 31 + global_tokens[i];
    }
    final_hash += hash_result;
    
    printf("Test completed. Final hash: 0x%lx\n", final_hash);
    printf("Expected non-zero hash if operations executed: %s\n", 
           final_hash != 0 ? "PASS" : "FAIL");
    
    /* Cleanup */
    for (int i = 1; i < 8; i++) {
        free_ast(nodes[i]);
    }
    free_ast(root);
    
    return (final_hash != 0) ? 0 : 1;
}
