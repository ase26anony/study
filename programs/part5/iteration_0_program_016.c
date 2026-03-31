/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_constructor(void) {
    printf("Constructor: Initializing ASAN/HWASAN test environment\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_destructor(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    __builtin_memcpy(node->data, base_data, copy_len);
    
    node->size = copy_len;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    node->left = create_ast(depth - 1, "left_child");
    node->right = create_ast(depth - 1, "right_child");
    
done:
    return node;
}

/* Function with goto jumping into memory operation block */
static void goto_memmove_test(char* dest, char* src, size_t len) {
    int use_copy = 1;
    
    if (use_copy) {
        goto perform_copy;
    }
    
    /* This won't execute but tests flow analysis */
    __builtin_memset(dest, 0, len);
    return;
    
perform_copy:
    /* Jump into memmove block */
    if (use_memmove) {
        __builtin_memmove(dest, src, len);
    } else {
        __builtin_memcpy(dest, src, len);
    }
    
    /* Jump out */
    goto cleanup;
    
cleanup:
    /* Verify with volatile */
    volatile char check = dest[0];
    (void)check;
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i] && nodes[i]->left) {
                /* Use volatile length */
                size_t len = volatile_len;
                if (len > sizeof(nodes[i]->data)) {
                    len = sizeof(nodes[i]->data);
                }
                
                /* Force all three builtins in parallel context */
                __builtin_memset(nodes[i]->data, thread_id, len);
                
                if (nodes[i]->right) {
                    __builtin_memcpy(nodes[i]->right->data, 
                                   nodes[i]->data, 
                                   len);
                }
                
                /* Create overlapping regions for memmove */
                char temp[256];
                __builtin_memcpy(temp, nodes[i]->data, len);
                __builtin_memmove(nodes[i]->data + 10, 
                                nodes[i]->data, 
                                len - 10);
                __builtin_memcpy(nodes[i]->data, temp, 10);
            }
        }
    }
}

/* Multi-stage initialization */
static void initialize_token_array(char tokens[][64], int rows) {
    for (int i = 0; i < rows; i++) {
        char pattern[64];
        __builtin_memset(pattern, 'A' + (i % 26), 63);
        pattern[63] = '\0';
        
        /* Use all three builtins in sequence */
        __builtin_memset(tokens[i], 0, 64);
        __builtin_memcpy(tokens[i], pattern, 64);
        
        if (i > 0) {
            __builtin_memmove(tokens[i] + 10, 
                            tokens[i-1], 
                            54);
        }
    }
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Stage 1: Initialize complex token array */
    char tokens[8][64];
    initialize_token_array(tokens, 8);
    
    /* Stage 2: Create recursive AST structures */
    ASTNode* root = create_ast(4, "root_node");
    ASTNode* nodes[4];
    
    nodes[0] = root;
    nodes[1] = root ? root->left : NULL;
    nodes[2] = root ? root->right : NULL;
    nodes[3] = root && root->left ? root->left->left : NULL;
    
    /* Stage 3: Goto-based memory operations */
    char buffer1[256];
    char buffer2[256];
    
    __builtin_memset(buffer1, 'X', 256);
    __builtin_memset(buffer2, 'Y', 256);
    
    for (int i = 0; i < 3; i++) {
        goto_memmove_test(buffer1, buffer2, volatile_len + i * 10);
    }
    
    /* Stage 4: Parallel memory operations */
    parallel_memory_ops(nodes, 4);
    
    /* Stage 5: Compute verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 64; j++) {
            hash = (hash * 31 + tokens[i][j]) % 1000000007;
        }
    }
    
    if (root) {
        for (int i = 0; i < 256 && i < (int)root->size; i++) {
            hash = (hash * 31 + root->data[i]) % 1000000007;
        }
    }
    
    printf("Verification hash: %lu\n", hash);
    
    /* Cleanup */
    if (root) {
        /* Recursive free */
        if (root->left) free(root->left);
        if (root->right) free(root->right);
        free(root);
    }
    
    printf("Test completed successfully\n");
    return 0;
}
