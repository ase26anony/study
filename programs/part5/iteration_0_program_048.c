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
    size_t size;
} ASTNode;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force initialization of ASAN runtime before main */
    volatile char buffer[32];
    __builtin_memset(buffer, 0xA5, sizeof(buffer));
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    /* Final memory operation in destructor */
    volatile char final_buf[16];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for initialization */
    __builtin_memcpy(node->data, base_data, strlen(base_data) + 1);
    node->size = strlen(base_data) + 1;
    
    /* Create children with different data */
    char left_data[256];
    char right_data[256];
    __builtin_memset(left_data, 'L', sizeof(left_data));
    __builtin_memset(right_data, 'R', sizeof(right_data));
    left_data[255] = right_data[255] = '\0';
    
    node->left = create_ast(depth - 1, left_data);
    node->right = create_ast(depth - 1, right_data);
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int use_copy = 1;
    
    /* Jump into memory operation block */
    if (use_memmove) goto do_memmove;
    
copy_only:
    __builtin_memcpy(dst->data, src->data, src->size);
    goto done;
    
do_memmove:
    /* This should trigger BUILT_IN_MEMMOVE case */
    __builtin_memmove(dst->data, src->data, src->size);
    
    /* Jump out to different operation */
    if (use_copy) {
        use_copy = 0;
        goto copy_only;
    }
    
done:
    /* Additional memset to ensure all builtins are covered */
    __builtin_memset(dst->data + src->size - 1, 0, 1);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Mix of memory operations in parallel region */
                char temp[256];
                
                /* Use volatile length to prevent optimization */
                size_t len = volatile_len % 256;
                
                if (tid % 3 == 0) {
                    __builtin_memcpy(temp, nodes[i]->data, len);
                } else if (tid % 3 == 1) {
                    __builtin_memset(nodes[i]->data, tid, len);
                } else {
                    /* Create overlapping regions for memmove */
                    if (len > 128) {
                        __builtin_memmove(nodes[i]->data, 
                                         nodes[i]->data + 64, 
                                         len - 64);
                    }
                }
            }
        }
    }
}

/* Main execution flow */
int main(void) {
    const int NUM_NODES = 8;
    const int AST_DEPTH = 3;
    
    /* Initialize token array */
    char tokens[][32] = {
        "TOKEN_ALPHA", "TOKEN_BETA", "TOKEN_GAMMA",
        "TOKEN_DELTA", "TOKEN_EPSILON", "TOKEN_ZETA"
    };
    
    /* Create AST nodes */
    ASTNode* nodes[NUM_NODES];
    for (int i = 0; i < NUM_NODES; i++) {
        nodes[i] = create_ast(AST_DEPTH, tokens[i % 6]);
    }
    
    /* Process with goto jumps */
    for (int i = 0; i < NUM_NODES - 1; i++) {
        process_with_goto(nodes[i], nodes[i + 1]);
    }
    
    /* Parallel memory operations */
    parallel_memory_ops(nodes, NUM_NODES);
    
    /* Compute verification hash */
    unsigned long long hash = 0;
    for (int i = 0; i < NUM_NODES; i++) {
        if (nodes[i]) {
            for (size_t j = 0; j < nodes[i]->size && j < 256; j++) {
                hash = (hash * 31 + nodes[i]->data[j]) % 1000000007;
            }
            
            /* Recursive cleanup */
            free(nodes[i]->left);
            free(nodes[i]->right);
            free(nodes[i]);
        }
    }
    
    printf("Verification hash: %llu\n", hash);
    return 0;
}
