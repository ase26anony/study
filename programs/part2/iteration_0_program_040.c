#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Complex token structure for AST-like operations */
typedef struct TokenNode {
    volatile int type;
    volatile char* value;
    struct TokenNode* left;
    struct TokenNode* right;
    unsigned char padding[64];  /* Ensure size for memcpy operations */
} TokenNode;

/* Global volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_init_flag = 0;

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_constructor(void) {
    volatile char buffer[128];
    /* Force __builtin_memset in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    g_init_flag = 1;
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_destructor(void) {
    volatile int cleanup[32];
    __builtin_memset(cleanup, 0xFF, sizeof(cleanup));
}

/* Recursive AST builder with memory operations */
static TokenNode* build_ast(int depth, const char* base) {
    if (depth <= 0) return NULL;
    
    TokenNode* node = (TokenNode*)malloc(sizeof(TokenNode));
    if (!node) return NULL;
    
    /* Initialize with __builtin_memset */
    __builtin_memset(node, 0, sizeof(TokenNode));
    
    node->type = depth;
    node->value = (char*)malloc(32);
    if (node->value) {
        /* Use __builtin_memcpy for string setup */
        __builtin_memcpy(node->value, base, strlen(base) + 1);
    }
    
    /* Recursive construction with goto for control flow */
    if (depth > 1) {
        int use_left = (depth % 2 == 0);
        
        if (use_left) {
            goto build_left;
        } else {
            goto build_right;
        }
        
    build_left:
        node->left = build_ast(depth - 1, "left");
        goto skip_right;
        
    build_right:
        node->right = build_ast(depth - 1, "right");
        goto skip_left;
        
    skip_right:
        node->right = build_ast(depth - 2, "skip");
        goto done;
        
    skip_left:
        node->left = build_ast(depth - 2, "skip");
        goto done;
    }
    
done:
    return node;
}

/* Complex memory operation with goto jumps */
static void process_with_goto(TokenNode* src, TokenNode* dst) {
    if (!src || !dst) return;
    
    volatile int stage = 0;
    
    /* Jump into memory operation block */
    if (src->type > 0) {
        goto start_copy;
    } else {
        goto skip_ops;
    }
    
start_copy:
    {
        /* This block tests flow sensitivity */
        volatile char temp[128];
        __builtin_memset(temp, src->type, sizeof(temp));
        
        /* Use __builtin_memmove with goto out */
        if (dst->value) {
            goto do_memmove;
        }
        goto finish_copy;
        
    do_memmove:
        __builtin_memmove(dst->value, temp, 32);
        goto finish_copy;
    }
    
finish_copy:
    stage = 1;
    
skip_ops:
    /* Additional operation after skip */
    if (stage == 0) {
        volatile int dummy[16];
        __builtin_memset(dummy, 0xCC, sizeof(dummy));
    }
}

/* OpenMP parallel memory dispatcher */
static void parallel_memory_ops(TokenNode** nodes, int count) {
    volatile long checksum = 0;
    
    #pragma omp parallel reduction(+:checksum)
    {
        int tid = omp_get_thread_num();
        volatile char thread_buf[256];
        
        /* Each thread performs memory operations */
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Mix of memory builtins */
                __builtin_memset(thread_buf, tid + i, sizeof(thread_buf));
                
                if (nodes[i]->value) {
                    __builtin_memcpy(nodes[i]->value, thread_buf, 32);
                }
                
                /* Conditional memmove with volatile control */
                volatile int do_move = (i % 3 == 0);
                if (do_move && i > 0 && nodes[i-1]) {
                    char temp[64];
                    __builtin_memcpy(temp, nodes[i]->value, 32);
                    __builtin_memmove(nodes[i-1]->value, temp, 32);
                }
                
                checksum += nodes[i]->type;
            }
        }
        
        /* Thread-local memory operation */
        volatile char local_copy[128];
        __builtin_memset(local_copy, 0xDD, sizeof(local_copy));
        __builtin_memcpy(local_copy + 64, local_copy, 64);
    }
    
    /* Store checksum in global volatile */
    g_mem_size = checksum;
}

/* Main execution with complex flow */
int main(void) {
    const int NODES = 8;
    TokenNode* nodes[NODES];
    volatile int result = 0;
    
    /* Phase 1: Build AST structures */
    for (int i = 0; i < NODES; i++) {
        char base[16];
        snprintf(base, sizeof(base), "node%d", i);
        nodes[i] = build_ast(3 + (i % 3), base);
    }
    
    /* Phase 2: Goto-based processing */
    for (int i = 0; i < NODES - 1; i++) {
        process_with_goto(nodes[i], nodes[i + 1]);
    }
    
    /* Phase 3: OpenMP parallel operations */
    parallel_memory_ops(nodes, NODES);
    
    /* Phase 4: Final verification with mixed builtins */
    volatile char final_buf[512];
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    
    for (int i = 0; i < NODES; i++) {
        if (nodes[i] && nodes[i]->value) {
            __builtin_memcpy(final_buf + i * 32, nodes[i]->value, 32);
            result += nodes[i]->type;
        }
    }
    
    /* Use __builtin_memmove for overlapping regions */
    __builtin_memmove(final_buf + 128, final_buf, 256);
    
    /* Calculate and print verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(final_buf); i++) {
        hash = hash * 31 + final_buf[i];
    }
    
    printf("Result: %d, Hash: %lu, Global: %zu\n", 
           result, hash, (size_t)g_mem_size);
    
    /* Cleanup */
    for (int i = 0; i < NODES; i++) {
        if (nodes[i]) {
            free(nodes[i]->value);
            free(nodes[i]);
        }
    }
    
    return 0;
}
