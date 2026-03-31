/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char *data;
    size_t len;
    struct ASTNode *left;
    struct ASTNode *right;
    unsigned char redzone[32]; /* Simulate redzone space */
} ASTNode;

/* Constructor function (runs before main) */
static void __attribute__((constructor)) init_asan_early(void) {
    /* Force early initialization of ASAN runtime */
    volatile char buffer[64];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
}

/* Destructor function (runs after main) */
static void __attribute__((destructor)) cleanup_asan_late(void) {
    /* Final memory operation to ensure coverage */
    volatile char final_buf[16];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char *base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->len = g_mem_size % 128 + 64;
    node->data = malloc(node->len);
    
    /* Use builtin memset to initialize */
    __builtin_memset(node->data, depth, node->len);
    
    /* Copy base pattern using memcpy */
    size_t copy_len = strlen(base_data) < node->len ? strlen(base_data) : node->len;
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Create children recursively */
    node->left = create_ast(depth - 1, base_data);
    node->right = create_ast(depth - 1, base_data);
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ASTNode *src, ASTNode *dst) {
    if (!src || !dst || src->len != dst->len) return;
    
    int use_memmove = g_use_memmove;
    
    if (use_memmove) {
        goto use_memmove_block;
    } else {
        goto use_memcpy_block;
    }
    
use_memmove_block:
    /* This tests flow sensitivity of asan_memfn_rtls logic */
    __builtin_memmove(dst->data, src->data, src->len);
    goto after_copy;
    
use_memcpy_block:
    __builtin_memcpy(dst->data, src->data, src->len);
    goto after_copy;
    
after_copy:
    /* Verify copy by modifying source */
    __builtin_memset(src->data + src->len/2, 0xCC, src->len/4);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode **nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        #pragma omp for
        for (int i = 0; i < count - 1; i++) {
            volatile size_t op_size = g_mem_size % 100 + 50;
            
            /* Mix different builtins in parallel regions */
            if (tid % 3 == 0) {
                __builtin_memset(nodes[i]->data, tid, op_size);
            } else if (tid % 3 == 1) {
                __builtin_memcpy(nodes[i]->data, nodes[i+1]->data, 
                               op_size < nodes[i]->len ? op_size : nodes[i]->len);
            } else {
                /* Force memmove with overlapping regions */
                size_t move_len = nodes[i]->len / 2;
                __builtin_memmove(nodes[i]->data + move_len/2, 
                                nodes[i]->data, move_len);
            }
        }
    }
}

/* Complex token processing */
static unsigned long process_tokens(const char *tokens[], int token_count) {
    unsigned long hash = 0xDEADBEEF;
    char buffer[256];
    
    for (int i = 0; i < token_count; i++) {
        size_t token_len = strlen(tokens[i]);
        
        /* Clear buffer with memset */
        __builtin_memset(buffer, 0, sizeof(buffer));
        
        /* Copy token with memcpy */
        __builtin_memcpy(buffer, tokens[i], 
                        token_len < sizeof(buffer) ? token_len : sizeof(buffer)-1);
        
        /* Move data around with memmove */
        if (token_len > 16) {
            __builtin_memmove(buffer + 8, buffer, 16);
        }
        
        /* Compute simple hash */
        for (size_t j = 0; j < token_len && j < sizeof(buffer); j++) {
            hash = (hash << 5) + hash + buffer[j];
        }
    }
    
    return hash;
}

int main(void) {
    const char *tokens[] = {
        "memcpy", "memset", "memmove", "asan", "hwasan",
        "redzone", "instrumentation", "coverage", "builtin"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Initialize complex data structures */
    ASTNode *root = create_ast(3, "AST_BASE_DATA");
    ASTNode *copy = create_ast(3, "COPY_BASE_DATA");
    
    if (!root || !copy) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Process tokens with memory builtins */
    unsigned long token_hash = process_tokens(tokens, token_count);
    printf("Token hash: 0x%lx\n", token_hash);
    
    /* Test goto-based control flow */
    process_with_goto(root, copy);
    
    /* Create array for parallel operations */
    ASTNode *node_array[6];
    for (int i = 0; i < 6; i++) {
        node_array[i] = create_ast(2, "PARALLEL_NODE");
    }
    
    /* Execute OpenMP parallel memory operations */
    parallel_memory_ops(node_array, 6);
    
    /* Verify results with final memory operation */
    char verify_buf[128];
    __builtin_memset(verify_buf, 0xAA, sizeof(verify_buf));
    __builtin_memcpy(verify_buf + 32, verify_buf, 64);
    __builtin_memmove(verify_buf, verify_buf + 16, 48);
    
    /* Compute final checksum */
    unsigned long final_sum = token_hash;
    for (int i = 0; i < 128; i++) {
        final_sum += verify_buf[i];
    }
    
    printf("Final checksum: 0x%lx\n", final_sum);
    
    /* Cleanup */
    /* Note: In real ASAN, memory would be freed automatically */
    
    return 0;
}
