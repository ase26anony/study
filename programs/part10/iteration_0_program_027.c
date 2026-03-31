/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    uint32_t hash;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force early initialization of ASAN runtime */
    char buf[32];
    __builtin_memset(buf, 0, sizeof(buf));
    printf("Constructor: Early ASAN init\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan(void) {
    printf("Destructor: ASAN cleanup\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile size */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Fill data with pattern */
    for (int i = 0; i < 63; i++) {
        node->data[i] = 'A' + (depth + i) % 26;
    }
    node->data[63] = '\0';
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_left = depth % 2;
        
        if (use_left) {
            node->left = create_ast(depth - 1);
            goto skip_right;
        }
        
        node->right = create_ast(depth - 1);
        skip_right:
        
        /* Copy between nodes if both exist */
        if (node->left && node->right) {
            /* Force memcpy with goto jumping into block */
            if (g_use_memmove) {
                goto do_memmove;
            }
            
            __builtin_memcpy(node->left->data, node->right->data, 
                           g_mem_size % 64);
            goto after_copy;
            
            do_memmove:
            /* This tests the BUILT_IN_MEMMOVE case */
            __builtin_memmove(node->left->data, node->right->data, 
                            g_mem_size % 64);
            after_copy:;
        }
    }
    
    /* Compute hash using memory operations */
    node->hash = 0;
    for (int i = 0; i < 64; i++) {
        node->hash = (node->hash * 31) + node->data[i % 63];
    }
    
    return node;
}

/* Function with OpenMP parallel section */
static void parallel_memory_ops(ASTNode* nodes[], int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Create local buffer with memset */
            char local_buf[128];
            __builtin_memset(local_buf, i, sizeof(local_buf));
            
            /* Copy from AST node to local buffer */
            __builtin_memcpy(local_buf, nodes[i]->data, 
                           g_mem_size % sizeof(local_buf));
            
            /* Process with memmove if needed */
            if (i % 3 == 0) {
                __builtin_memmove(local_buf + 32, local_buf, 64);
            }
            
            /* Update node hash */
            uint32_t new_hash = 0;
            for (int j = 0; j < 64; j++) {
                new_hash = (new_hash * 17) + local_buf[j];
            }
            nodes[i]->hash ^= new_hash;
        }
    }
}

/* Main test driver */
int main(void) {
    const int NUM_NODES = 8;
    ASTNode* nodes[NUM_NODES];
    uint32_t total_hash = 0;
    
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Create AST nodes recursively */
    for (int i = 0; i < NUM_NODES; i++) {
        nodes[i] = create_ast(3 + (i % 3));
    }
    
    /* Perform parallel memory operations */
    parallel_memory_ops(nodes, NUM_NODES);
    
    /* Additional sequential memory operations */
    char* dynamic_buf = (char*)malloc(512);
    if (dynamic_buf) {
        /* Test all three builtins in sequence */
        __builtin_memset(dynamic_buf, 0xAA, 512);
        
        /* Copy from first node */
        if (nodes[0]) {
            __builtin_memcpy(dynamic_buf, nodes[0]->data, 64);
        }
        
        /* Overlapping memmove */
        __builtin_memmove(dynamic_buf + 128, dynamic_buf, 256);
        
        free(dynamic_buf);
    }
    
    /* Compute final hash */
    for (int i = 0; i < NUM_NODES; i++) {
        if (nodes[i]) {
            total_hash += nodes[i]->hash;
            
            /* Cleanup with memset before free */
            __builtin_memset(nodes[i], 0, sizeof(ASTNode));
            free(nodes[i]);
        }
    }
    
    printf("Total hash: 0x%08X\n", total_hash);
    printf("Test completed successfully\n");
    
    return total_hash != 0 ? 0 : 1;
}
