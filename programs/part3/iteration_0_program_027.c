/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
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

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_tokens(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)((i * 13) & 0xFF);
    }
    
    /* Use __builtin_memset in constructor */
    if (volatile_flag) {
        __builtin_memset(token_pool + 1024, 0xAA, 128);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup(void) {
    /* Verify memory was properly handled */
    volatile char check = token_pool[0];
    (void)check; /* Suppress unused warning */
}

/* Recursive parser with memory operations */
static ASTNode* create_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->left = NULL;
    node->right = NULL;
    
    /* Initialize node data with __builtin_memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy pattern from token pool */
    int copy_len = volatile_len % 128;
    if (copy_len > 0) {
        __builtin_memcpy(node->data, 
                        token_pool + (id * 16) % sizeof(token_pool),
                        copy_len);
    }
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int left_id = id * 2;
        int right_id = id * 2 + 1;
        
        /* Jump label for goto testing */
        create_left:
        node->left = create_node(depth - 1, left_id);
        
        /* Use __builtin_memmove between nodes */
        if (node->left && depth > 2) {
            ASTNode* temp = node->left;
            __builtin_memmove(node->data + 64, 
                            temp->data, 
                            sizeof(temp->data) / 4);
        }
        
        /* Conditional goto */
        if (volatile_flag) {
            goto create_right;
        }
        
        create_right:
        node->right = create_node(depth - 1, right_id);
        
        /* Another memmove with overlap */
        if (node->right && node->left) {
            __builtin_memmove(node->left->data,
                            node->right->data,
                            volatile_len % 64);
        }
    }
    
    return node;
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[512];
        char src_buf[512];
        
        /* Initialize source buffer */
        for (int i = 0; i < sizeof(src_buf); i++) {
            src_buf[i] = (char)((i + thread_id * 17) & 0xFF);
        }
        
        /* Use all three builtins in parallel region */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        /* Conditional memcpy based on thread ID */
        if (thread_id % 2 == 0) {
            __builtin_memcpy(local_buf + 128, 
                           src_buf, 
                           volatile_len % 256);
        } else {
            __builtin_memmove(local_buf, 
                            src_buf + 64, 
                            volatile_len % 128);
        }
        
        /* Additional overlapping memmove */
        __builtin_memmove(local_buf + 32, 
                        local_buf, 
                        96);
        
        /* Store result back to global pool */
        int offset = (thread_id * 64) % sizeof(token_pool);
        __builtin_memcpy(token_pool + offset,
                        local_buf,
                        64);
    }
}

/* Multi-stage processing */
static int process_ast(ASTNode* node, int depth) {
    if (!node) return 0;
    
    int sum = node->id;
    
    /* Process data with memory operations */
    char temp[256];
    
    /* Copy node data to temp buffer */
    __builtin_memcpy(temp, node->data, sizeof(temp));
    
    /* Modify temp buffer */
    for (int i = 0; i < sizeof(temp); i++) {
        temp[i] ^= (char)depth;
    }
    
    /* Copy back with memmove (overlap test) */
    __builtin_memmove(node->data + 32, temp, 128);
    
    /* Recursive processing */
    sum += process_ast(node->left, depth + 1);
    sum += process_ast(node->right, depth + 1);
    
    return sum;
}

/* Free AST with memory verification */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Final memcpy to global pool before free */
    int offset = (node->id * 8) % sizeof(token_pool);
    __builtin_memcpy(token_pool + offset,
                    &node->id,
                    sizeof(node->id));
    
    free(node);
}

int main(void) {
    int result = 0;
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Stage 1: Create recursive AST */
    ASTNode* root = create_node(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Stage 2: Parallel memory operations */
    parallel_memory_ops();
    
    /* Stage 3: Process AST with memory operations */
    result = process_ast(root, 0);
    
    /* Stage 4: Additional built-in calls in main */
    char main_buf[1024];
    
    /* Chain of memory operations */
    __builtin_memset(main_buf, 0xCC, sizeof(main_buf));
    __builtin_memcpy(main_buf + 256, token_pool, 512);
    __builtin_memmove(main_buf, main_buf + 128, 384);
    
    /* Use volatile length for one more operation */
    __builtin_memcpy(main_buf + 512,
                    token_pool + 1024,
                    volatile_len % 512);
    
    /* Calculate verification hash */
    unsigned int hash = 0;
    for (size_t i = 0; i < sizeof(main_buf); i++) {
        hash = (hash * 31) + (unsigned int)main_buf[i];
    }
    
    /* Mix in AST result */
    result ^= (int)(hash & 0xFFFFFFFF);
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed. Result: %d (hash: %u)\n", result, hash);
    
    return 0;
}
