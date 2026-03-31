/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static int token_index = 0;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_token_pool(void) {
    /* Initialize with pattern using builtin memset */
    __builtin_memset(token_pool, 0xAA, sizeof(token_pool));
    
    /* Force symbol initialization */
    volatile char* ptr = (char*)token_pool;
    for (int i = 0; i < 256; i++) {
        ptr[i] = i & 0xFF;
    }
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_verify(void) {
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += token_pool[i];
    }
    printf("Destructor checksum: %d\n", sum);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy for data initialization */
    size_t len = (volatile_len % 128) + 64;
    __builtin_memcpy(node->data, base_data, len);
    
    node->id = depth;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        char child_data[256];
        
        /* Label for goto testing */
        create_left:
        __builtin_memcpy(child_data, node->data, len);
        child_data[0] = 'L';
        node->left = create_ast(depth - 1, child_data);
        
        if (volatile_flag) {
            /* Jump to skip right creation */
            goto skip_right;
        }
        
        create_right:
        child_data[0] = 'R';
        node->right = create_ast(depth - 1, child_data);
        
        skip_right:
        /* Force memmove with goto */
        if (node->left && node->right) {
            char temp[256];
            __builtin_memmove(temp, node->left->data, len);
            __builtin_memmove(node->left->data, node->right->data, len);
            __builtin_memmove(node->right->data, temp, len);
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Complex memory dispatch with OpenMP */
static void parallel_memory_operations(void) {
    const int num_blocks = 8;
    char* blocks[num_blocks];
    size_t sizes[num_blocks];
    
    /* Initialize blocks with varying sizes */
    for (int i = 0; i < num_blocks; i++) {
        sizes[i] = (volatile_len * (i + 1)) % 512 + 64;
        blocks[i] = (char*)malloc(sizes[i]);
        if (blocks[i]) {
            __builtin_memset(blocks[i], i, sizes[i]);
        }
    }
    
    /* OpenMP parallel region */
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        #pragma omp for
        for (int i = 0; i < num_blocks - 1; i++) {
            if (blocks[i] && blocks[i + 1]) {
                /* Use all three builtins in parallel */
                size_t copy_len = sizes[i] < sizes[i + 1] ? sizes[i] : sizes[i + 1];
                
                /* Test memcpy */
                __builtin_memcpy(blocks[i + 1], blocks[i], copy_len);
                
                /* Test memset with thread-specific pattern */
                __builtin_memset(blocks[i], thread_id, copy_len);
                
                /* Test memmove with overlapping regions */
                if (copy_len > 32) {
                    __builtin_memmove(blocks[i] + 16, blocks[i], copy_len - 32);
                }
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_blocks; i++) {
        if (blocks[i]) {
            free(blocks[i]);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = create_ast(4, "BaseASTNodeData");
    
    if (root) {
        /* Traverse and modify AST */
        ASTNode* stack[16];
        int top = 0;
        stack[top++] = root;
        
        while (top > 0) {
            ASTNode* current = stack[--top];
            
            /* Copy between nodes */
            if (current->left && current->right) {
                size_t len = volatile_len % 128;
                __builtin_memcpy(current->data, current->left->data, len);
                __builtin_memmove(current->left->data, current->right->data, len);
            }
            
            /* Push children */
            if (current->right) stack[top++] = current->right;
            if (current->left) stack[top++] = current->left;
        }
        
        /* Free AST recursively */
        void free_ast(ASTNode* node) {
            if (!node) return;
            free_ast(node->left);
            free_ast(node->right);
            
            /* Clear node data before free */
            __builtin_memset(node->data, 0, sizeof(node->data));
            free(node);
        }
        free_ast(root);
    }
    
    /* Phase 2: Parallel memory operations */
    parallel_memory_operations();
    
    /* Phase 3: Direct built-in calls with volatile control */
    char buffer1[1024];
    char buffer2[1024];
    
    /* Force initialization of all three builtins */
    __builtin_memset(buffer1, 0xCC, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    
    /* Overlapping memmove test */
    __builtin_memmove(buffer1 + 256, buffer1, 512);
    
    /* Verify with checksum */
    unsigned long long checksum = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        checksum += buffer1[i];
    }
    
    printf("Final checksum: %llu\n", checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}
