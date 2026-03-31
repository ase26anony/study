/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
char global_tokens[1024];
int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(global_tokens, 'A', sizeof(global_tokens));
    printf("Constructor: Initialized global tokens\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use builtin memcpy in destructor */
    char temp[1024];
    __builtin_memcpy(temp, global_tokens, sizeof(global_tokens));
    printf("Destructor: Copied %zu bytes\n", sizeof(global_tokens));
}

/* Recursive parser with memory operations */
ASTNode* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtin memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in data */
    char pattern[32];
    __builtin_memset(pattern, '0' + depth, 32);
    __builtin_memcpy(node->data, pattern, 32);
    
    node->id = depth;
    node->left = create_ast(depth + 1, max_depth);
    node->right = create_ast(depth + 1, max_depth);
    
    /* Copy between child nodes if they exist */
    if (node->left && node->right) {
        size_t copy_len = volatile_len % 128;
        __builtin_memcpy(node->right->data, node->left->data, copy_len);
    }
    
    return node;
}

/* Function with goto jumps around memmove */
void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int use_memmove = 0;
    
    /* Jump into block containing memmove */
    if (volatile_flag) goto memmove_block;
    
    normal_path:
        __builtin_memcpy(dst->data, src->data, 64);
        return;
    
    memmove_block:
        use_memmove = 1;
        /* Jump out and back in */
        if (dst->id % 2) goto after_memmove;
        
        /* This memmove should trigger redirection */
        __builtin_memmove(dst->data + 32, dst->data, 32);
        
        after_memmove:
        if (use_memmove) {
            /* Jump back to use memmove again */
            goto memmove_block_2;
        }
    
    memmove_block_2:
        __builtin_memmove(src->data, dst->data, 48);
        goto normal_path;
}

/* Parallel memory operations */
void parallel_memory_ops(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Mix different builtins */
                switch (i % 3) {
                    case 0:
                        __builtin_memset(nodes[i]->data + tid * 8, tid, 8);
                        break;
                    case 1:
                        if (i + 1 < count && nodes[i + 1]) {
                            __builtin_memcpy(nodes[i + 1]->data, nodes[i]->data, 32);
                        }
                        break;
                    case 2:
                        __builtin_memmove(nodes[i]->data + 16, nodes[i]->data, 16);
                        break;
                }
            }
        }
        
        /* Barrier to ensure all threads see modifications */
        #pragma omp barrier
        
        #pragma omp single
        {
            /* Master thread consolidates with memcpy */
            for (int i = 1; i < count; i++) {
                if (nodes[0] && nodes[i]) {
                    size_t len = (volatile_len + tid) % 64;
                    __builtin_memcpy(nodes[0]->data, nodes[i]->data, len);
                }
            }
        }
    }
}

/* Compute hash from AST */
unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    /* Process data with volatile length */
    size_t len = volatile_len % sizeof(node->data);
    for (size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + ptr[i];
    }
    
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Create AST structure */
    ASTNode* root = create_ast(0, 4);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    ASTNode* nodes[8];
    nodes[0] = root;
    for (int i = 1; i < 8; i++) {
        nodes[i] = create_ast(1, 3);
    }
    
    /* Test goto flow with memmove */
    printf("Testing goto flow control...\n");
    process_with_goto(nodes[0], nodes[1]);
    
    /* Test parallel memory operations */
    printf("Testing parallel memory operations...\n");
    parallel_memory_ops(nodes, 8);
    
    /* Additional builtin calls in main */
    char buffer1[256], buffer2[256];
    
    /* Force all three builtins to be used */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 128, buffer1, 128);
    
    /* Use volatile to control lengths */
    size_t dyn_len = volatile_len;
    if (dyn_len > sizeof(buffer1)) dyn_len = sizeof(buffer1);
    
    __builtin_memset(buffer2, 0xBB, dyn_len);
    __builtin_memcpy(global_tokens + 512, buffer2, dyn_len);
    
    /* Compute and print result */
    unsigned long total_hash = 0;
    for (int i = 0; i < 8; i++) {
        if (nodes[i]) {
            total_hash ^= compute_ast_hash(nodes[i]);
        }
    }
    
    printf("Result hash: 0x%016lx\n", total_hash);
    printf("Token array first byte: 0x%02x\n", (unsigned char)global_tokens[0]);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        /* Recursive free - simplified for example */
        free(nodes[i]);
    }
    
    printf("Test completed successfully\n");
    return 0;
}
