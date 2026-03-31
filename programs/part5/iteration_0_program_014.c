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
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_global_data(void) {
    /* Force initialization of memory function cache early */
    char buffer[32];
    __builtin_memset(buffer, 0xA5, sizeof(buffer));
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_global_data(void) {
    /* Final memory operation to ensure cache is used */
    volatile char final_buf[16];
    __builtin_memset((void*)final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_tree(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Initialize node data with memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Fill with pattern using memcpy */
    char pattern[16] = "TREE_NODE_DATA";
    __builtin_memcpy(node->data, pattern, 
                    sizeof(pattern) < sizeof(node->data) ? 
                    sizeof(pattern) : sizeof(node->data));
    
    /* Create children */
    node->left = create_tree(depth - 1, counter);
    node->right = create_tree(depth - 1, counter);
    
    return node;
}

/* Complex memory operation with goto flow control */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int use_memmove = 1;
    
    /* Jump into memory operation block */
    if (src->id % 2 == 0) {
        goto do_memcpy;
    } else {
        goto do_memset;
    }
    
do_memcpy:
    {
        /* Force memcpy redirection */
        size_t copy_size = g_mem_size % sizeof(src->data);
        __builtin_memcpy(dst->data, src->data, copy_size);
        
        if (use_memmove) {
            goto do_memmove;
        }
        goto finish;
    }
    
do_memset:
    {
        /* Force memset redirection */
        __builtin_memset(dst->data, src->id & 0xFF, sizeof(dst->data));
        goto finish;
    }
    
do_memmove:
    {
        /* Force memmove redirection with overlapping regions */
        char temp[128];
        __builtin_memcpy(temp, dst->data, sizeof(dst->data));
        __builtin_memmove(dst->data + 16, dst->data, sizeof(dst->data) - 16);
        __builtin_memcpy(dst->data, temp, 16);
        /* fall through */
    }
    
finish:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Each thread uses different memory builtins */
                switch (tid % 3) {
                    case 0:
                        __builtin_memset(nodes[i]->data, tid, 
                                        sizeof(nodes[i]->data) / 2);
                        break;
                    case 1:
                        if (i > 0) {
                            __builtin_memcpy(nodes[i]->data, 
                                           nodes[i-1]->data,
                                           sizeof(nodes[i]->data));
                        }
                        break;
                    case 2:
                        if (i > 0 && i < count - 1) {
                            __builtin_memmove(nodes[i]->data,
                                            nodes[i]->data + 16,
                                            sizeof(nodes[i]->data) - 16);
                        }
                        break;
                }
            }
        }
    }
}

/* Main execution flow */
int main(void) {
    int counter = 0;
    long long hash_sum = 0;
    
    /* Create tree structure */
    ASTNode* root = create_tree(3, &counter);
    if (!root) {
        fprintf(stderr, "Failed to create tree\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    ASTNode* nodes[8];
    nodes[0] = root;
    for (int i = 1; i < 8; i++) {
        nodes[i] = create_tree(2, &counter);
    }
    
    /* Process with goto-based control flow */
    for (int i = 0; i < 4; i++) {
        if (nodes[i] && nodes[i+4]) {
            process_with_goto(nodes[i], nodes[i+4]);
        }
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(nodes, 8);
    
    /* Calculate verification hash */
    for (int i = 0; i < 8; i++) {
        if (nodes[i]) {
            for (size_t j = 0; j < sizeof(nodes[i]->data); j++) {
                hash_sum += (long long)nodes[i]->data[j] * (i + 1);
            }
            hash_sum += nodes[i]->id;
        }
    }
    
    /* Additional builtin calls in different contexts */
    volatile char final_buffer[256];
    volatile size_t dynamic_size = g_mem_size;
    
    __builtin_memset((void*)final_buffer, 0xCC, dynamic_size % 256);
    
    char* aligned_ptr = (char*)__builtin_assume_aligned(final_buffer, 16);
    __builtin_memcpy(aligned_ptr + 32, aligned_ptr, 64);
    
    /* Print verification result */
    printf("Memory operations completed. Hash sum: %lld\n", hash_sum);
    printf("Init flag: %d\n", g_init_flag);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        free(nodes[i]);
    }
    
    return 0;
}
