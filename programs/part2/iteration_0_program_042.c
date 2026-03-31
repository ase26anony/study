/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[128];
static volatile char volatile_src[128];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {
    "memcpy_test", "memset_test", "memmove_test",
    "recursive", "parallel", "asan", "hwasan"
};
static const int token_count = 7;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < 128; i++) {
        volatile_src[i] = (char)(i % 26 + 'A');
    }
    
    /* Force early built-in usage in constructor */
    __builtin_memset(volatile_dest, 0, sizeof(volatile_dest));
    __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final built-in usage in destructor */
    __builtin_memset(volatile_dest, 0xFF, 16);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Use built-ins with volatile lengths */
    int len = volatile_len % 32;
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, tokens[node->id % token_count], len);
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    if (depth > 2) {
        goto create_children;
    }
    
create_children:
    node->left = create_ast(depth - 1, counter);
    
    /* Jump back for right child */
    if (create_left) {
        create_left = 0;
        goto create_right;
    }
    
create_right:
    node->right = create_ast(depth - 1, counter);
    
    return node;
}

/* Function with goto jumping into memmove block */
static void test_goto_memmove(char* dest, char* src, int size) {
    int use_memmove = 1;
    
    if (size > 16) {
        goto perform_copy;
    }
    
    /* This block should be jumped into */
perform_copy:
    if (use_memmove) {
        /* Force memmove usage with overlapping regions */
        __builtin_memmove(dest, src, (size_t)size);
        use_memmove = 0;
        goto after_copy;
    }
    
after_copy:
    /* Additional operation after goto */
    __builtin_memset(dest + size/2, 'X', size/4);
}

/* Parallel memory dispatch logic */
static void parallel_memory_operations(void) {
    char buffer1[256];
    char buffer2[256];
    char buffer3[256];
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 'A', sizeof(buffer1));
    __builtin_memset(buffer2, 'B', sizeof(buffer2));
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                __builtin_memcpy(buffer3, buffer1, volatile_len % 128);
                break;
            case 1:
                __builtin_memset(buffer2 + thread_id * 16, thread_id, 32);
                break;
            case 2:
                /* Overlapping memmove */
                __builtin_memmove(buffer1 + 32, buffer1, 64);
                break;
        }
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Collective operation */
        #pragma omp for
        for (int i = 0; i < 64; i++) {
            __builtin_memcpy(&buffer3[i*4], &buffer1[i*4], 4);
        }
    }
    
    /* Final serial operations */
    __builtin_memmove(buffer1, buffer3, 128);
}

/* Compute hash from AST */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* data = node->data;
    
    /* Process node data */
    while (*data) {
        hash = ((hash << 5) + hash) + *data++;
    }
    
    /* Recursive hash computation */
    hash ^= compute_ast_hash(node->left);
    hash ^= compute_ast_hash(node->right);
    hash ^= (unsigned long)node->id;
    
    return hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Recursive AST operations */
    int counter = 0;
    ASTNode* root = create_ast(4, &counter);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Copy between AST nodes */
    if (root->left && root->right) {
        __builtin_memcpy(root->right->data, root->left->data, 
                        volatile_len % sizeof(root->left->data));
    }
    
    /* Phase 2: Goto flow control test */
    char test_buf1[100];
    char test_buf2[100];
    
    __builtin_memset(test_buf1, 'G', sizeof(test_buf1));
    __builtin_memset(test_buf2, 'H', sizeof(test_buf2));
    
    test_goto_memmove(test_buf1, test_buf2, volatile_len % 80);
    
    /* Phase 3: Parallel operations */
    parallel_memory_operations();
    
    /* Phase 4: Compute verification result */
    unsigned long hash = compute_ast_hash(root);
    
    /* Additional built-in usage in main */
    char final_buffer[64];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, test_buf1, 32);
    __builtin_memmove(final_buffer + 16, final_buffer, 16);
    
    /* Print results */
    printf("AST node count: %d\n", counter);
    printf("Computed hash: 0x%lx\n", hash);
    printf("Final buffer[0]: %c\n", final_buffer[0]);
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully.\n");
    return 0;
}
