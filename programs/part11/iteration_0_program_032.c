/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static const char* tokens[] = {"memcpy", "memset", "memmove", "test", "data"};
static const size_t num_tokens = sizeof(tokens)/sizeof(tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_asan_hooks(void) {
    volatile char buffer[32];
    /* Force initialization of memcpy redirection */
    __builtin_memcpy((void*)buffer, tokens[0], 6);
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_asan(void) {
    volatile char cleanup_buf[16];
    __builtin_memset((void*)cleanup_buf, 0, sizeof(cleanup_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_node(const char* src, size_t len) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy with volatile length */
    volatile size_t copy_len = len > 63 ? 63 : len;
    __builtin_memcpy(node->data, src, copy_len);
    node->data[copy_len] = '\0';
    node->size = copy_len;
    node->left = node->right = NULL;
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ASTNode* dest, ASTNode* src) {
    int use_memmove = 1;
    
    if (dest == NULL || src == NULL) {
        goto cleanup;
    }
    
    /* Jump into block with memmove */
    if (use_memmove) {
        goto do_memmove;
    }
    
    /* This should never execute */
    __builtin_memset(dest->data, 0, 64);
    return;
    
do_memmove:
    /* Force memmove redirection */
    volatile size_t move_size = src->size > dest->size ? dest->size : src->size;
    __builtin_memmove(dest->data, src->data, move_size);
    
    /* Jump out */
    goto after_move;
    
after_move:
    dest->size = move_size;
    return;
    
cleanup:
    if (dest) __builtin_memset(dest->data, 0, 64);
}

/* OpenMP parallel section with memory operations */
static size_t parallel_memory_ops(ASTNode** nodes, size_t count) {
    size_t total_hash = 0;
    
    #pragma omp parallel reduction(+:total_hash)
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (size_t i = 0; i < count; i++) {
            if (nodes[i] == NULL) continue;
            
            /* Mix of memory operations */
            char temp[64];
            volatile size_t op_size = g_mem_size % 64;
            
            /* Use all three builtins */
            __builtin_memcpy(temp, nodes[i]->data, op_size);
            
            if (tid % 3 == 0) {
                __builtin_memset(nodes[i]->data + op_size/2, 'X', op_size/4);
            }
            
            /* Circular shift using memmove */
            __builtin_memmove(nodes[i]->data + 1, nodes[i]->data, op_size - 1);
            
            /* Compute simple hash */
            for (size_t j = 0; j < op_size && j < 64; j++) {
                total_hash += (size_t)nodes[i]->data[j] * (i + 1);
            }
        }
    }
    
    return total_hash;
}

/* Main execution flow */
int main(void) {
    const size_t node_count = 8;
    ASTNode* nodes[node_count];
    size_t final_hash = 0;
    
    /* Initialize nodes with token data */
    for (size_t i = 0; i < node_count; i++) {
        const char* token = tokens[i % num_tokens];
        nodes[i] = create_node(token, strlen(token));
        
        /* Additional memset on every 3rd node */
        if (i % 3 == 0 && nodes[i]) {
            volatile int fill_char = i + 'A';
            __builtin_memset(nodes[i]->data, fill_char, 16);
        }
    }
    
    /* Test goto flow with memmove */
    if (node_count >= 2 && nodes[0] && nodes[1]) {
        process_with_goto(nodes[0], nodes[1]);
    }
    
    /* Complex memcpy between structures */
    for (size_t i = 2; i + 1 < node_count; i++) {
        if (nodes[i] && nodes[i+1]) {
            volatile size_t cpy_len = nodes[i]->size;
            if (cpy_len > nodes[i+1]->size) cpy_len = nodes[i+1]->size;
            __builtin_memcpy(nodes[i+1]->data, nodes[i]->data, cpy_len);
        }
    }
    
    /* Execute parallel memory operations */
    final_hash = parallel_memory_ops(nodes, node_count);
    
    /* Final verification memset */
    volatile char verify_buf[32];
    __builtin_memset(verify_buf, 0xAA, sizeof(verify_buf));
    
    /* Use memmove on verification buffer */
    __builtin_memmove(verify_buf + 16, verify_buf, 16);
    
    /* Print result */
    printf("Final hash: %zu\n", final_hash);
    printf("Verification: 0x%02X\n", (unsigned char)verify_buf[0]);
    
    /* Cleanup */
    for (size_t i = 0; i < node_count; i++) {
        if (nodes[i]) {
            free(nodes[i]);
        }
    }
    
    return final_hash != 0 ? 0 : 1;
}
