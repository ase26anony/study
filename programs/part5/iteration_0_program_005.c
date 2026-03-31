/* ISO C99-compliant program targeting ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    uint32_t hash;
} ASTNode;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    printf("Constructor: Initializing ASAN environment\n");
    g_mem_size = 256 + (rand() % 128);
}

/* Destructor for cleanup verification */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Destructor: ASAN environment cleanup complete\n");
}

/* Recursive function with memory operations */
static ASTNode* build_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with goto for flow control */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    
    /* Goto-based control flow around memcpy */
    if (depth % 2 == 0) {
        goto use_memcpy;
    } else {
        /* Alternative path */
        __builtin_memcpy(node->data, "default", 8);
        goto skip_memcpy;
    }
    
use_memcpy:
    __builtin_memcpy(node->data, base_data, copy_len);
    
skip_memcpy:
    /* Build left/right children recursively */
    char left_data[32], right_data[32];
    snprintf(left_data, sizeof(left_data), "%s_L%d", base_data, depth);
    snprintf(right_data, sizeof(right_data), "%s_R%d", base_data, depth);
    
    node->left = build_ast(depth - 1, left_data);
    node->right = build_ast(depth - 1, right_data);
    
    return node;
}

/* Function with goto jumping into memmove block */
static void process_with_goto_memmove(ASTNode* src, ASTNode* dst) {
    int condition = (src != NULL && dst != NULL);
    
    if (!condition) {
        goto cleanup;
    }
    
    /* Jump into memmove block */
    goto do_memmove;
    
do_memmove:
    /* Use __builtin_memmove for overlapping regions */
    __builtin_memmove(dst->data + 10, dst->data, 32);
    goto after_memmove;
    
after_memmove:
    /* Additional processing */
    dst->hash = 0;
    for (size_t i = 0; i < sizeof(dst->data); i++) {
        dst->hash = dst->hash * 31 + dst->data[i];
    }
    
cleanup:
    return;
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_operations(ASTNode** nodes, size_t count) {
    #pragma omp parallel
    {
        #pragma omp for
        for (size_t i = 0; i < count; i++) {
            if (nodes[i] && nodes[(i + 1) % count]) {
                /* Use volatile to prevent optimization */
                volatile size_t copy_size = g_mem_size % 64;
                
                /* Force all three built-ins in parallel context */
                __builtin_memset(nodes[i]->data, i, copy_size);
                
                if (i % 3 == 0) {
                    __builtin_memcpy(nodes[(i + 1) % count]->data, 
                                   nodes[i]->data, copy_size);
                } else if (i % 3 == 1) {
                    __builtin_memmove(nodes[i]->data + 16, 
                                    nodes[i]->data, copy_size);
                }
            }
        }
    }
}

/* Multi-stage processing with different memory operations */
static uint32_t process_ast_tree(ASTNode* root) {
    if (!root) return 0;
    
    uint32_t total_hash = 0;
    ASTNode* temp = (ASTNode*)malloc(sizeof(ASTNode));
    
    if (temp) {
        /* Stage 1: Copy root to temp */
        __builtin_memcpy(temp, root, sizeof(ASTNode));
        
        /* Stage 2: Process with overlapping move */
        __builtin_memmove(temp->data + 20, temp->data, 40);
        
        /* Stage 3: Clear part of data */
        __builtin_memset(temp->data + 30, 0, 20);
        
        total_hash = temp->hash;
        free(temp);
    }
    
    /* Recursively process children */
    total_hash += process_ast_tree(root->left);
    total_hash += process_ast_tree(root->right);
    
    return total_hash;
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
    
    /* Build complex AST structure */
    ASTNode* root = build_ast(4, "root");
    if (!root) {
        fprintf(stderr, "Failed to build AST\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    ASTNode* nodes[8];
    nodes[0] = root;
    
    /* Build additional nodes */
    for (int i = 1; i < 8; i++) {
        char name[32];
        snprintf(name, sizeof(name), "node%d", i);
        nodes[i] = build_ast(3, name);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_operations(nodes, 8);
    
    /* Process with goto-based memmove */
    for (int i = 0; i < 7; i++) {
        process_with_goto_memmove(nodes[i], nodes[i + 1]);
    }
    
    /* Final tree processing */
    uint32_t final_hash = process_ast_tree(root);
    printf("Final hash: %u\n", final_hash);
    
    /* Cleanup */
    for (int i = 1; i < 8; i++) {
        free_ast(nodes[i]);
    }
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
