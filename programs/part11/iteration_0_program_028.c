/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {"memcpy", "memset", "memmove", "test", "data"};
static const int token_count = sizeof(tokens)/sizeof(tokens[0]);

/* Constructor attribute for early initialization */
__attribute__((constructor))
static void init_constructor(void) {
    printf("Constructor: Initializing ASAN test environment\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_destructor(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[64];
    __builtin_memset(pattern, 'A' + (id % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(node->data));
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    if (depth > 2) {
        goto create_children;
    }
    
    create_children:
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    return node;
}

/* Function with goto jumping into memmove block */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int use_memmove = g_use_memmove;
    
    if (use_memmove) {
        goto do_memmove;
    } else {
        /* Regular path */
        __builtin_memcpy(dst->data, src->data, sizeof(src->data));
        return;
    }
    
    do_memmove:
    /* This block is entered via goto */
    __builtin_memmove(dst->data, src->data, sizeof(src->data));
    
    /* Jump out */
    goto finish;
    
    finish:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        size_t size = g_mem_size;
        char* buffer = (char*)malloc(size);
        char* buffer2 = (char*)malloc(size);
        
        if (buffer && buffer2) {
            /* Each thread uses different builtins */
            switch (tid % 3) {
                case 0:
                    __builtin_memset(buffer, tid, size);
                    break;
                case 1:
                    __builtin_memcpy(buffer2, buffer, size);
                    break;
                case 2:
                    __builtin_memmove(buffer, buffer2, size);
                    break;
            }
        }
        
        free(buffer);
        free(buffer2);
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create AST structures */
    ASTNode* ast1 = create_ast(3, 1);
    ASTNode* ast2 = create_ast(3, 100);
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Test 1: Direct built-in calls */
    printf("Test 1: Direct built-in calls\n");
    char buffer1[256], buffer2[256];
    
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1, buffer2, sizeof(buffer1));
    
    /* Test 2: Goto flow control with memmove */
    printf("Test 2: Goto flow control\n");
    process_with_goto(ast1, ast2);
    
    /* Test 3: OpenMP parallel operations */
    printf("Test 3: OpenMP parallel operations\n");
    parallel_memory_ops();
    
    /* Test 4: Token processing with memory ops */
    printf("Test 4: Token processing\n");
    char token_data[256];
    for (int i = 0; i < token_count; i++) {
        size_t len = strlen(tokens[i]);
        __builtin_memcpy(&token_data[i * 32], tokens[i], len + 1);
    }
    
    /* Test 5: Recursive AST copying */
    printf("Test 5: Recursive AST operations\n");
    if (ast1->left && ast2->left) {
        __builtin_memcpy(ast2->left->data, ast1->left->data, sizeof(ast1->left->data));
    }
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < sizeof(buffer1); i++) {
        hash = hash * 31 + buffer1[i];
    }
    
    /* Include AST data in hash */
    for (int i = 0; i < sizeof(ast1->data); i++) {
        hash = hash * 31 + ast1->data[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Helper function to free AST recursively */
    void free_ast(ASTNode* node) {
        if (!node) return;
        free_ast(node->left);
        free_ast(node->right);
        free(node);
    }
    
    free_ast(ast1);
    free_ast(ast2);
    
    return 0;
}
