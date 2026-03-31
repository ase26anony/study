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
    int id;
} ASTNode;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force early initialization of ASAN runtime */
    volatile char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    /* Verify ASAN cleanup */
    printf("ASAN cleanup completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[64];
    __builtin_memset(pattern, 'A' + (id % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(pattern) > sizeof(node->data) ? 
                    sizeof(node->data) : sizeof(pattern));
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        node->right = create_ast(depth - 1, id * 2 + 1);
        
        if (0) {
create_children:
            /* Alternative path with __builtin_memmove */
            ASTNode temp;
            __builtin_memcpy(&temp, node, sizeof(ASTNode));
            node->left = create_ast(depth - 1, id * 2);
            node->right = create_ast(depth - 1, id * 2 + 1);
            __builtin_memmove(node, &temp, sizeof(ASTNode));
        }
    }
    
    return node;
}

/* Function with complex memory operations and OpenMP */
static void process_ast_parallel(ASTNode* root, int iterations) {
    if (!root) return;
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buffer[512];
        
        #pragma omp for
        for (int i = 0; i < iterations; i++) {
            /* Use volatile length to prevent constant folding */
            size_t len = volatile_len + i;
            
            /* Initialize buffer with __builtin_memset */
            __builtin_memset(local_buffer, thread_id, 
                           len > sizeof(local_buffer) ? sizeof(local_buffer) : len);
            
            /* Copy AST data with __builtin_memcpy */
            __builtin_memcpy(local_buffer + 128, root->data, 
                           sizeof(root->data) > 256 ? 256 : sizeof(root->data));
            
            /* Conditional __builtin_memmove based on volatile */
            if (use_memmove) {
                char temp[256];
                __builtin_memcpy(temp, local_buffer, 256);
                __builtin_memmove(local_buffer + 64, temp, 192);
            }
            
            /* Process children recursively */
            if (root->left) {
                char child_buffer[256];
                __builtin_memcpy(child_buffer, root->left->data, 256);
                __builtin_memset(child_buffer + 128, i, 64);
                __builtin_memcpy(root->left->data, child_buffer, 256);
            }
        }
    }
}

/* Function with goto jumping into memory operation blocks */
static void test_goto_memmove(void) {
    char src[1024], dst[1024];
    int mode = 0;
    
    /* Initialize source */
    for (int i = 0; i < sizeof(src); i++) {
        src[i] = i % 256;
    }
    
    /* Complex goto pattern */
    if (mode == 0) {
        goto block1;
    }
    
    __builtin_memset(dst, 0, sizeof(dst));
    
block1:
    /* Jump into block with __builtin_memmove */
    __builtin_memmove(dst, src, 512);
    
    if (mode == 1) {
        goto block2;
    }
    
    __builtin_memcpy(dst + 512, src + 256, 256);
    
block2:
    /* Another memory operation after goto */
    __builtin_memset(dst + 768, 0xFF, 256);
    
    /* Jump back */
    if (mode == 2) {
        goto block1;
    }
}

/* Main verification function */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    
    /* Hash node data */
    for (size_t i = 0; i < sizeof(node->data) && node->data[i]; i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    /* Recursive hash computation */
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create complex AST */
    ASTNode* root = create_ast(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Test goto patterns with memory operations */
    test_goto_memmove();
    
    /* Process AST with OpenMP parallelism */
    process_ast_parallel(root, 100);
    
    /* Compute verification hash */
    unsigned long hash = compute_ast_hash(root);
    printf("AST hash: %lu\n", hash);
    
    /* Additional memory operations in main */
    char final_buffer[2048];
    volatile size_t final_len = volatile_len * 2;
    
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, root->data, sizeof(root->data));
    
    if (use_memmove) {
        __builtin_memmove(final_buffer + 1024, final_buffer, 512);
    }
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
