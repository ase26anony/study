/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* AST-like recursive structure */
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
    
    /* Force ASAN to see this memset */
    volatile char* vptr = (volatile char*)token_pool;
    for (int i = 0; i < 256; i++) {
        vptr[i] = i;
    }
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_verify(void) {
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += token_pool[i];
    }
    printf("Destructor: Token pool sum = %d\n", sum);
}

/* Recursive AST creation with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtin memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Fill with pattern using volatile control */
    for (int i = 0; i < volatile_len && i < 255; i++) {
        node->data[i] = (char)((id + i) & 0xFF);
    }
    
    node->id = id;
    
    /* Create children with goto-based flow control */
    if (depth > 1) {
        int use_goto = volatile_flag;
        
        if (use_goto) {
            /* Jump into block with memmove */
            goto create_left;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        node->right = create_ast(depth - 1, id * 2 + 1);
        return node;
        
    create_left:
        /* This block contains builtin memmove */
        ASTNode* temp = create_ast(depth - 1, id * 2);
        
        /* Copy data between nodes using builtin memcpy */
        if (temp) {
            __builtin_memcpy(node->data + 128, temp->data, 
                           volatile_len > 128 ? 128 : volatile_len);
        }
        node->left = temp;
        
        /* Jump out to create right */
        goto create_right;
        
    create_right:
        node->right = create_ast(depth - 1, id * 2 + 1);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Complex memory operation with OpenMP */
static void parallel_memory_operations(ASTNode* root) {
    if (!root) return;
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread performs different memory operations */
        char local_buffer[512];
        
        /* Use builtin memset with volatile length */
        __builtin_memset(local_buffer, thread_id, 
                        volatile_len > 512 ? 512 : volatile_len);
        
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            /* Mix of memory operations */
            if (i % 3 == 0) {
                /* memcpy between buffers */
                __builtin_memcpy(local_buffer + (i * 4), 
                               root->data, 
                               volatile_len > 64 ? 64 : volatile_len);
            } else if (i % 3 == 1) {
                /* memset pattern */
                __builtin_memset(local_buffer + (i * 4), 
                               i & 0xFF, 
                               volatile_len > 32 ? 32 : volatile_len);
            } else {
                /* memmove with overlap */
                if (i > 10) {
                    __builtin_memmove(local_buffer + (i * 4) - 8,
                                    local_buffer + (i * 4),
                                    volatile_len > 16 ? 16 : volatile_len);
                }
            }
        }
        
        /* Copy result back to AST node */
        #pragma omp critical
        {
            __builtin_memcpy(root->data + (thread_id * 16),
                           local_buffer,
                           volatile_len > 64 ? 64 : volatile_len);
        }
    }
}

/* Recursive tree traversal with memory operations */
static int traverse_and_hash(ASTNode* node, int depth) {
    if (!node) return 0;
    
    int hash = node->id;
    
    /* Process node data with builtin memcpy */
    char temp_buffer[256];
    __builtin_memcpy(temp_buffer, node->data, sizeof(temp_buffer));
    
    /* Modify using builtin memset in parts */
    for (int i = 0; i < 4; i++) {
        __builtin_memset(temp_buffer + (i * 64), 
                       (hash + i) & 0xFF,
                       volatile_len > 64 ? 64 : volatile_len);
    }
    
    /* Copy back with overlap using memmove */
    __builtin_memmove(node->data + 32, temp_buffer + 64, 128);
    
    /* Recursive traversal */
    int left_hash = traverse_and_hash(node->left, depth + 1);
    int right_hash = traverse_and_hash(node->right, depth + 1);
    
    return hash + left_hash * 3 + right_hash * 7;
}

/* Function with switch-based dispatch */
static void dispatch_memory_ops(int op_type, void* dest, const void* src, size_t n) {
    switch (op_type) {
        case 0:
            __builtin_memcpy(dest, src, n);
            break;
        case 1:
            __builtin_memset(dest, 0xCC, n);
            break;
        case 2:
            __builtin_memmove(dest, src, n);
            break;
        default:
            /* Fallback to regular functions */
            memcpy(dest, src, n);
    }
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Create and initialize AST */
    ASTNode* root = create_ast(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Parallel memory operations */
    parallel_memory_operations(root);
    
    /* Phase 3: Dispatch various memory operations */
    char buffer1[1024], buffer2[1024];
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memset(buffer2, 0x55, sizeof(buffer2));
    
    /* Test all three builtins in sequence */
    for (int i = 0; i < 3; i++) {
        dispatch_memory_ops(i, buffer1, buffer2, 
                          volatile_len > 256 ? 256 : volatile_len);
        
        /* Overlap test with memmove */
        if (i == 2) {
            __builtin_memmove(buffer1 + 128, buffer1, 384);
        }
    }
    
    /* Phase 4: Recursive traversal with hash computation */
    int final_hash = traverse_and_hash(root, 0);
    
    /* Phase 5: Complex token processing */
    int token_sum = 0;
    for (int i = 0; i < sizeof(token_pool); i += 64) {
        /* Copy tokens to local buffer */
        char local[64];
        __builtin_memcpy(local, token_pool + i, 64);
        
        /* Process with memset */
        __builtin_memset(local + 32, i & 0xFF, 32);
        
        /* Move data around */
        __builtin_memmove(token_pool + i, local, 64);
        
        /* Accumulate sum */
        for (int j = 0; j < 64; j++) {
            token_sum += token_pool[i + j];
        }
    }
    
    /* Print verification results */
    printf("AST hash result: %d\n", final_hash);
    printf("Token pool sum: %d\n", token_sum);
    printf("Volatile length used: %d\n", volatile_len);
    
    /* Cleanup */
    /* Note: Proper tree freeing omitted for brevity */
    
    return 0;
}
