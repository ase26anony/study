/* ISO C99-compliant program targeting ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char *volatile_dest = NULL;
static volatile char *volatile_src = NULL;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode *left;
    struct ASTNode *right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    printf("Constructor: Initializing ASAN environment\n");
    
    /* Force early built-in usage in constructor */
    char buffer1[128];
    char buffer2[128];
    
    /* Use all three built-ins in constructor */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1, buffer2, sizeof(buffer1));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Destructor: Cleaning up ASAN environment\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with built-ins */
    __builtin_memset(node->data, id, sizeof(node->data));
    node->id = id;
    
    /* Create left child with different pattern */
    node->left = create_ast(depth - 1, id * 2);
    if (node->left) {
        /* Copy data between nodes */
        __builtin_memcpy(node->data + 16, node->left->data, 16);
    }
    
    /* Create right child */
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    return node;
}

/* Function with goto control flow */
static void complex_memory_operations(void) {
    char buffer1[256];
    char buffer2[256];
    int use_memmove = 0;
    
    /* Initial memset */
    __builtin_memset(buffer1, 0xCC, sizeof(buffer1));
    
    /* Goto into block with memmove */
    if (volatile_len > 32) {
        goto memmove_block;
    }
    
    /* Normal path */
    __builtin_memcpy(buffer2, buffer1, 128);
    goto end;
    
memmove_block:
    /* This block should trigger memmove redirection */
    __builtin_memmove(buffer2, buffer1, volatile_len);
    use_memmove = 1;
    
end:
    /* Verify the operation */
    if (use_memmove) {
        __builtin_memset(buffer1, 0xDD, 64);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_dispatch(void) {
    const int num_threads = 4;
    char *buffers[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        buffers[tid] = malloc(1024);
        
        if (buffers[tid]) {
            /* Each thread uses different built-ins */
            switch (tid % 3) {
                case 0:
                    __builtin_memset(buffers[tid], tid, 1024);
                    break;
                case 1:
                    if (tid > 0) {
                        __builtin_memcpy(buffers[tid], buffers[tid-1], 512);
                    }
                    break;
                case 2:
                    __builtin_memmove(buffers[tid], buffers[(tid+1)%num_threads], 256);
                    break;
            }
        }
        
        #pragma omp barrier
        
        /* Cross-thread memory operation */
        if (tid == 0) {
            for (int i = 1; i < num_threads; i++) {
                __builtin_memcpy(buffers[0] + i*256, buffers[i], 256);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_threads; i++) {
        free(buffers[i]);
    }
}

/* Main execution flow */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Basic built-in usage */
    printf("Phase 1: Basic built-in operations\n");
    {
        char src[128], dest[128];
        
        /* Force all three built-ins */
        __builtin_memset(src, 0x55, sizeof(src));
        __builtin_memcpy(dest, src, sizeof(src));
        __builtin_memmove(src, dest, sizeof(src)/2);
    }
    
    /* Phase 2: Recursive AST operations */
    printf("Phase 2: Recursive AST operations\n");
    ASTNode *root = create_ast(3, 1);
    
    if (root) {
        /* Complex memory operations between nodes */
        if (root->left && root->right) {
            __builtin_memcpy(root->data, root->left->data, 16);
            __builtin_memmove(root->right->data, root->data, 16);
        }
        
        /* TODO: Add recursive free function */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    /* Phase 3: Control flow with goto */
    printf("Phase 3: Control flow edge cases\n");
    complex_memory_operations();
    
    /* Phase 4: OpenMP parallel operations */
    printf("Phase 4: OpenMP parallel operations\n");
    parallel_memory_dispatch();
    
    /* Phase 5: Volatile variable usage */
    printf("Phase 5: Volatile variable operations\n");
    {
        char *dynamic_buf = malloc(volatile_len * 2);
        if (dynamic_buf) {
            /* Use volatile length */
            __builtin_memset(dynamic_buf, 0xFF, volatile_len);
            
            /* Create alias for memmove */
            char *alias = dynamic_buf + volatile_len/2;
            __builtin_memmove(alias, dynamic_buf, volatile_len/2);
            
            free(dynamic_buf);
        }
    }
    
    /* Final verification hash */
    printf("Phase 6: Result verification\n");
    {
        unsigned char hash = 0;
        char verify_buf[256];
        
        __builtin_memset(verify_buf, 0, sizeof(verify_buf));
        
        /* Mix of all operations */
        for (int i = 0; i < sizeof(verify_buf); i += 32) {
            __builtin_memset(verify_buf + i, i, 32);
        }
        
        __builtin_memcpy(verify_buf + 128, verify_buf, 128);
        __builtin_memmove(verify_buf, verify_buf + 64, 192);
        
        /* Simple hash calculation */
        for (int i = 0; i < sizeof(verify_buf); i++) {
            hash ^= verify_buf[i];
        }
        
        printf("Verification hash: 0x%02X\n", hash);
        printf("=== Test completed successfully ===\n");
    }
    
    return 0;
}
