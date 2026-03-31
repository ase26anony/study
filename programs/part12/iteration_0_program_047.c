/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
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
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    /* Force early initialization of memory functions */
    char buffer[32];
    __builtin_memset(buffer, 0xA5, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Use builtins with volatile-controlled sizes */
    size_t copy_size = g_mem_size % 128;
    __builtin_memset(node->data, node->id, copy_size);
    
    /* Create pattern for memcpy */
    char pattern[32];
    __builtin_memset(pattern, 0xCC, sizeof(pattern));
    __builtin_memcpy(node->data + 16, pattern, 16);
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    skip_left:
    node->left = NULL;
    goto create_right;
    
    create_children:
    if (create_left) {
        node->left = create_ast(depth - 1, counter);
        create_left = 0;
        goto skip_left;
    }
    
    create_right:
    node->right = create_ast(depth - 1, counter);
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int use_memmove = 0;
    
    /* Jump into memmove block */
    if (src->id % 3 == 0) {
        goto do_memmove;
    }
    
    /* Normal memcpy path */
    __builtin_memcpy(dst->data, src->data, 48);
    goto done;
    
    do_memmove:
    /* Overlapping memory regions to force memmove */
    __builtin_memmove(dst->data + 16, dst->data, 32);
    use_memmove = 1;
    
    done:
    /* Verify with memset */
    if (use_memmove) {
        __builtin_memset(dst->data + 40, 0xFF, 8);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        ASTNode* node = nodes[i];
        if (!node) continue;
        
        /* Force all three builtins in parallel context */
        char temp[64];
        
        /* memset in parallel */
        __builtin_memset(temp, i, sizeof(temp));
        
        /* memcpy between buffers */
        __builtin_memcpy(node->data, temp, 32);
        
        /* Conditional memmove for overlapping regions */
        if (i % 2 == 0) {
            __builtin_memmove(node->data + 16, node->data, 24);
        }
        
        /* Additional memset to ensure coverage */
        __builtin_memset(node->data + 56, 0xAA, 8);
    }
}

/* Multi-stage initialization with memory builtins */
static void initialize_token_array(char tokens[][64], int rows) {
    for (int i = 0; i < rows; i++) {
        /* Pattern initialization with memset */
        __builtin_memset(tokens[i], i + 0x30, 64);
        
        /* Copy pattern variations */
        if (i > 0) {
            __builtin_memcpy(tokens[i] + 32, tokens[i-1], 32);
        }
        
        /* Move data within the same row */
        if (i % 4 == 0) {
            __builtin_memmove(tokens[i] + 16, tokens[i], 32);
        }
    }
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST creation */
    int counter = 1;
    ASTNode* root = create_ast(4, &counter);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Complex token array */
    char tokens[8][64];
    initialize_token_array(tokens, 8);
    
    /* Phase 3: Goto-based memory operations */
    ASTNode* copy = (ASTNode*)malloc(sizeof(ASTNode));
    if (copy) {
        process_with_goto(root, copy);
        
        /* Additional builtin calls */
        __builtin_memset(copy->data + 48, 0xEE, 16);
        __builtin_memcpy(root->data, copy->data, 32);
        
        free(copy);
    }
    
    /* Phase 4: OpenMP parallel operations */
    ASTNode* node_array[4];
    node_array[0] = root;
    for (int i = 1; i < 4; i++) {
        node_array[i] = create_ast(3, &counter);
    }
    
    #ifdef _OPENMP
    parallel_memory_ops(node_array, 4);
    #endif
    
    /* Phase 5: Final verification with all builtins */
    unsigned long hash = 0;
    for (int i = 0; i < 64; i++) {
        hash += (unsigned long)root->data[i];
    }
    
    /* Use memmove for final rearrangement */
    __builtin_memmove(root->data, root->data + 32, 32);
    __builtin_memset(root->data + 32, hash & 0xFF, 32);
    
    printf("Test completed. Hash: 0x%lx\n", hash);
    
    /* Cleanup */
    free(root);
    for (int i = 1; i < 4; i++) {
        if (node_array[i]) free(node_array[i]);
    }
    
    return 0;
}
