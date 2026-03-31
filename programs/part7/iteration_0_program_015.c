/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_trigger = 1;

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
    volatile char buffer[128];
    /* Force __builtin_memset initialization */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    printf("[constructor] Initialized ASAN early buffer\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    printf("[destructor] ASAN cleanup completed\n");
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
    __builtin_memcpy(node->data, pattern, sizeof(pattern) < sizeof(node->data) ? 
                    sizeof(pattern) : sizeof(node->data));
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        node->right = create_ast(depth - 1, id * 2 + 1);
        
        if (use_goto) {
            create_children:
            /* This goto tests flow-sensitivity of ASAN instrumentation */
            node->left = create_ast(depth - 1, id * 2);
            node->right = NULL;
        }
    }
    
    return node;
}

/* Function with __builtin_memmove and goto jumps */
static void manipulate_ast(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int use_memmove = volatile_trigger;
    
    if (use_memmove) {
        /* Jump into memory operation block */
        goto do_memmove;
    }
    
    /* Normal path */
    __builtin_memcpy(dst->data, src->data, volatile_len % sizeof(dst->data));
    
    do_memmove:
    /* This tests memmove redirection with goto */
    __builtin_memmove(dst->data + 32, src->data, 
                     (volatile_len / 2) % (sizeof(dst->data) - 32));
    
    /* Jump out of block */
    if (use_memmove) {
        goto after_ops;
    }
    
    after_ops:
    /* Additional memset */
    __builtin_memset(dst->data + 64, 0xCC, 32);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Each thread uses different builtins */
                switch (tid % 3) {
                    case 0:
                        __builtin_memset(nodes[i]->data, tid, 
                                        volatile_len % sizeof(nodes[i]->data));
                        break;
                    case 1:
                        if (i > 0) {
                            __builtin_memcpy(nodes[i]->data, nodes[i-1]->data,
                                           (volatile_len / 2) % sizeof(nodes[i]->data));
                        }
                        break;
                    case 2:
                        if (i > 0 && i < count - 1) {
                            __builtin_memmove(nodes[i]->data, nodes[i+1]->data,
                                            (volatile_len / 3) % sizeof(nodes[i]->data));
                        }
                        break;
                }
            }
        }
    }
}

/* Calculate hash of AST structure */
static unsigned long ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    /* Use __builtin_memcpy in hash calculation */
    char buffer[256];
    __builtin_memcpy(buffer, node->data, sizeof(buffer));
    
    for (size_t i = 0; i < sizeof(buffer) && buffer[i]; i++) {
        hash = ((hash << 5) + hash) + buffer[i];
    }
    
    hash += ast_hash(node->left);
    hash += ast_hash(node->right);
    hash += node->id;
    
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Create complex AST structure */
    ASTNode* root = create_ast(4, 1);
    ASTNode* copy = create_ast(4, 100);
    
    if (!root || !copy) {
        fprintf(stderr, "Failed to create AST structures\n");
        return 1;
    }
    
    /* Test goto and memmove logic */
    manipulate_ast(root, copy);
    
    /* Create array for parallel operations */
    ASTNode* nodes[8];
    nodes[0] = root;
    nodes[1] = copy;
    for (int i = 2; i < 8; i++) {
        nodes[i] = create_ast(3, 200 + i);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(nodes, 8);
    
    /* Additional builtin calls in main */
    char main_buffer[512];
    __builtin_memset(main_buffer, 0, sizeof(main_buffer));
    __builtin_memcpy(main_buffer + 128, root->data, 128);
    __builtin_memmove(main_buffer + 256, main_buffer + 128, 128);
    
    /* Calculate and print verification result */
    unsigned long total_hash = 0;
    for (int i = 0; i < 8; i++) {
        if (nodes[i]) {
            total_hash ^= ast_hash(nodes[i]);
        }
    }
    
    /* Mix in main_buffer hash */
    for (size_t i = 0; i < sizeof(main_buffer); i++) {
        total_hash = ((total_hash << 3) + total_hash) + main_buffer[i];
    }
    
    printf("Verification hash: 0x%08lx\n", total_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        free(nodes[i]);
    }
    
    return 0;
}
