/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 256;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 192;

/* Recursive AST-like structure */
struct ast_node {
    int type;
    char data[64];
    struct ast_node *left;
    struct ast_node *right;
};

/* Global token array */
static char token_pool[4096];
volatile int token_index = 0;

/* Constructor/destructor for initialization coordination */
__attribute__((constructor)) static void init_tokens(void) {
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)(i % 256);
    }
}

__attribute__((destructor)) static void cleanup(void) {
    /* Verify operations by computing simple hash */
    unsigned long hash = 0;
    for (int i = 0; i < sizeof(token_pool); i++) {
        hash = (hash * 31 + token_pool[i]) & 0xFFFF;
    }
    printf("Final token hash: 0x%04lx\n", hash);
}

/* Recursive parser with memory operations */
static struct ast_node* parse_expression(int depth) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(struct ast_node));
    
    /* Fill data with tokens using __builtin_memcpy */
    size_t copy_len = g_memcpy_len % 64;
    __builtin_memcpy(node->data, &token_pool[token_index], copy_len);
    token_index = (token_index + 64) % sizeof(token_pool);
    
    /* Recursive construction */
    node->left = parse_expression(depth - 1);
    node->right = parse_expression(depth - 1);
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(struct ast_node* src, struct ast_node* dst) {
    int state = 0;
    
    /* Jump into memory operation block */
    goto entry_point;
    
memory_ops:
    /* This block contains __builtin_memmove with goto control flow */
    if (src && dst) {
        size_t move_len = g_memmove_len % 64;
        __builtin_memmove(dst->data, src->data, move_len);
    }
    state = 1;
    goto exit_point;
    
entry_point:
    /* Conditional jump to memory ops */
    if (src != NULL) {
        goto memory_ops;
    }
    
exit_point:
    /* Jump out of block */
    if (state == 0) {
        goto final;
    }
    
    /* Additional memory operation after goto */
    if (dst) {
        __builtin_memset(dst->data + 32, 0xFF, 16);
    }
    
final:
    return;
}

/* OpenMP parallel memory dispatch */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[256];
        char src_buf[256];
        
        /* Initialize source buffer */
        for (int i = 0; i < 256; i++) {
            src_buf[i] = (char)((i + tid) % 256);
        }
        
        /* Force built-in calls in parallel region */
        #pragma omp barrier
        
        /* Use all three built-ins */
        __builtin_memcpy(local_buf, src_buf, g_memcpy_len % 256);
        __builtin_memset(local_buf + 128, tid, g_memset_len % 128);
        
        /* Self-overlapping memmove */
        __builtin_memmove(local_buf + 64, local_buf, g_memmove_len % 192);
        
        #pragma omp critical
        {
            /* Copy results back to global pool */
            __builtin_memcpy(&token_pool[tid * 64], local_buf, 64);
        }
    }
}

/* Multi-stage interaction function */
static void complex_memory_chains(void) {
    struct ast_node* nodes[4];
    
    /* Create AST nodes */
    for (int i = 0; i < 4; i++) {
        nodes[i] = parse_expression(3);
    }
    
    /* Chain memory operations between nodes */
    for (int i = 0; i < 3; i++) {
        /* Use volatile lengths to prevent folding */
        volatile size_t len = g_memcpy_len % 64;
        __builtin_memcpy(nodes[i+1]->data, nodes[i]->data, len);
        
        /* Goto-based processing */
        process_with_goto(nodes[i], nodes[i+1]);
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        if (nodes[i]) {
            free(nodes[i]);
        }
    }
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Stage 1: Initialize and parse */
    struct ast_node* root = parse_expression(4);
    
    /* Stage 2: Parallel memory operations */
    parallel_memory_ops();
    
    /* Stage 3: Complex memory chains */
    complex_memory_chains();
    
    /* Stage 4: Final memory operations with root node */
    if (root) {
        char verify_buf[256];
        __builtin_memset(verify_buf, 0, sizeof(verify_buf));
        __builtin_memcpy(verify_buf, root->data, 64);
        __builtin_memmove(verify_buf + 128, verify_buf, 64);
        
        /* Compute verification sum */
        unsigned long sum = 0;
        for (int i = 0; i < 256; i++) {
            sum += verify_buf[i];
        }
        printf("Verification sum: %lu\n", sum);
        
        free(root);
    }
    
    printf("Test completed\n");
    return 0;
}
