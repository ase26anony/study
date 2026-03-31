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
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force builtin initialization in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_late(void) {
    volatile char final_check[8];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile-controlled size */
    size_t copy_size = g_mem_size % 64;
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, base_data, copy_size);
    
    node->size = copy_size;
    
    /* Create children with different data */
    char left_data[64];
    char right_data[64];
    __builtin_memset(left_data, 'L', sizeof(left_data));
    __builtin_memset(right_data, 'R', sizeof(right_data));
    
    node->left = create_ast(depth - 1, left_data);
    node->right = create_ast(depth - 1, right_data);
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int use_copy = 1;
    
    /* Jump into memory operation block */
    if (src->size > 32) {
        goto do_memmove;
    } else {
        goto skip_memmove;
    }
    
do_memmove:
    {
        volatile size_t move_size = src->size;
        if (g_use_memmove) {
            /* This should trigger BUILT_IN_MEMMOVE redirection */
            __builtin_memmove(dst->data, src->data, move_size);
        }
        goto after_move;
    }
    
skip_memmove:
    __builtin_memset(dst->data, 0, sizeof(dst->data));
    
after_move:
    /* Jump back for second operation */
    if (dst->size > 16) {
        goto do_second_op;
    }
    return;
    
do_second_op:
    __builtin_memcpy(src->data + 8, dst->data, 8);
}

/* OpenMP parallel memory operations */
static void parallel_mem_operations(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Mix different builtins in parallel regions */
                volatile char temp[128];
                
                __builtin_memset(temp, tid, sizeof(temp));
                __builtin_memcpy(nodes[i]->data, temp, nodes[i]->size);
                
                if (i % 3 == 0) {
                    __builtin_memmove(temp + 64, temp, 64);
                }
            }
        }
        
        /* Barrier with memory operation */
        #pragma omp barrier
        
        #pragma omp single
        {
            volatile char sync_buf[16];
            __builtin_memset(sync_buf, 0xCC, sizeof(sync_buf));
        }
    }
}

/* Main execution flow */
int main(void) {
    const int NUM_NODES = 10;
    const int AST_DEPTH = 3;
    ASTNode* nodes[NUM_NODES];
    
    /* Initialize nodes with different patterns */
    for (int i = 0; i < NUM_NODES; i++) {
        char pattern[64];
        __builtin_memset(pattern, 'A' + (i % 26), sizeof(pattern));
        nodes[i] = create_ast(AST_DEPTH, pattern);
    }
    
    /* Process with goto flow control */
    for (int i = 0; i < NUM_NODES - 1; i++) {
        process_with_goto(nodes[i], nodes[i + 1]);
    }
    
    /* Execute parallel operations */
    parallel_mem_operations(nodes, NUM_NODES);
    
    /* Verify results with checksum */
    unsigned long long checksum = 0;
    for (int i = 0; i < NUM_NODES; i++) {
        if (nodes[i]) {
            for (size_t j = 0; j < nodes[i]->size; j++) {
                checksum += (unsigned char)nodes[i]->data[j];
            }
            
            /* Recursive cleanup */
            free(nodes[i]);
        }
    }
    
    printf("Checksum: %llu\n", checksum);
    printf("Built-in redirection test completed.\n");
    
    return 0;
}
