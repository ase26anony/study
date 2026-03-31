/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_switch = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force early builtin usage before main */
    char buf1[32], buf2[32];
    __builtin_memset(buf1, 0xAA, sizeof(buf1));
    __builtin_memcpy(buf2, buf1, sizeof(buf1));
    __builtin_memmove(buf1, buf2, sizeof(buf1));
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    /* Final builtin calls */
    char final_buf[16];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile control */
    size_t len = (volatile_len / (depth + 1)) % 32;
    __builtin_memset(node->data, depth, len);
    
    node->type = depth;
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 2);
    
    /* Copy between nodes if both exist */
    if (node->left && node->right) {
        size_t copy_len = (volatile_len / (depth + 2)) % 32;
        __builtin_memcpy(node->right->data, node->left->data, copy_len);
    }
    
    return node;
}

/* Function with goto edge cases */
static void goto_mem_operations(char* dest, char* src) {
    int use_memmove = volatile_switch & 1;
    
    if (use_memmove) {
        goto use_memmove_block;
    } else {
        goto use_memcpy_block;
    }
    
use_memmove_block:
    {
        char temp[64];
        __builtin_memcpy(temp, src, 32);
        __builtin_memmove(dest, temp, 32);
        goto after_ops;
    }
    
use_memcpy_block:
    __builtin_memcpy(dest, src, 32);
    /* Fall through */
    
after_ops:
    /* Additional memset after goto */
    __builtin_memset(dest + 32, 0xCC, 16);
}

/* OpenMP parallel section with memory operations */
static void parallel_mem_ops(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Thread-specific memory operations */
                char thread_buf[48];
                
                /* Mix of builtins */
                __builtin_memset(thread_buf, tid, 16);
                __builtin_memcpy(thread_buf + 16, nodes[i]->data, 16);
                
                /* Conditional memmove */
                if (tid % 2 == 0) {
                    __builtin_memmove(nodes[i]->data, thread_buf, 16);
                }
                
                /* Volatile-controlled length */
                size_t len = (volatile_len + tid) % 32;
                __builtin_memset(nodes[i]->data + 16, i, len);
            }
        }
        
        /* Barrier with additional builtin */
        #pragma omp barrier
        
        #pragma omp single
        {
            char single_buf[32];
            __builtin_memset(single_buf, 0xEE, sizeof(single_buf));
        }
    }
}

/* Main execution flow */
int main(void) {
    const int NUM_NODES = 8;
    ASTNode* nodes[NUM_NODES];
    
    /* Initialize AST structures */
    for (int i = 0; i < NUM_NODES; i++) {
        nodes[i] = create_ast(4 + (i % 3));
    }
    
    /* Test goto patterns */
    char src_buf[128], dest_buf[128];
    __builtin_memset(src_buf, 0x11, sizeof(src_buf));
    
    for (int i = 0; i < 4; i++) {
        volatile_switch = i;
        goto_mem_operations(dest_buf + i * 32, src_buf + i * 16);
    }
    
    /* Parallel memory operations */
    parallel_mem_ops(nodes, NUM_NODES);
    
    /* Complex memory chain */
    char chain_buf[4][64];
    for (int i = 0; i < 4; i++) {
        __builtin_memset(chain_buf[i], i * 0x20, sizeof(chain_buf[i]));
    }
    
    /* Chain of memory operations */
    __builtin_memcpy(chain_buf[1], chain_buf[0], 32);
    __builtin_memmove(chain_buf[2], chain_buf[1], 48);
    __builtin_memcpy(chain_buf[3], chain_buf[2], 32);
    __builtin_memset(chain_buf[0], 0x99, 16);
    
    /* Compute verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < NUM_NODES; i++) {
        if (nodes[i]) {
            for (size_t j = 0; j < sizeof(nodes[i]->data); j++) {
                hash = (hash * 31) + nodes[i]->data[j];
            }
            free(nodes[i]);
        }
    }
    
    /* Additional builtins in cleanup path */
    char final_verify[64];
    __builtin_memset(final_verify, 0, sizeof(final_verify));
    __builtin_memcpy(final_verify, &hash, sizeof(hash));
    
    printf("Verification hash: %lu\n", hash);
    printf("Buffer[0][0] = 0x%02X\n", (unsigned char)chain_buf[0][0]);
    
    return 0;
}
