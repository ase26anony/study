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
struct ASTNode {
    int type;
    char *data;
    size_t data_len;
    struct ASTNode *left;
    struct ASTNode *right;
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[64];
    /* Force early builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    volatile char final_buf[32];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static struct ASTNode* create_ast(int depth, const char *base_data) {
    if (depth <= 0) return NULL;
    
    struct ASTNode *node = malloc(sizeof(struct ASTNode));
    if (!node) return NULL;
    
    node->type = depth;
    node->data_len = g_memcpy_len / (depth + 1);
    node->data = malloc(node->data_len);
    
    if (node->data) {
        /* Use all three builtins with volatile lengths */
        __builtin_memset(node->data, depth, node->data_len);
        
        if (base_data) {
            size_t copy_len = node->data_len < 64 ? node->data_len : 64;
            __builtin_memcpy(node->data, base_data, copy_len);
        }
        
        /* Create overlapping copy within same buffer */
        if (node->data_len > 32) {
            __builtin_memmove(node->data + 16, node->data, 32);
        }
    }
    
    node->left = create_ast(depth - 1, node->data);
    node->right = create_ast(depth - 2, node->data);
    
    return node;
}

/* Function with goto jumps around memmove */
static void goto_memmove_test(char *dest, char *src, size_t len) {
    int use_memmove = 1;
    
    if (len > 100) {
        goto skip_memmove;
    }
    
    /* This block contains the builtin */
    {
        char temp[256];
        __builtin_memcpy(temp, src, len < 256 ? len : 256);
        __builtin_memmove(dest, temp, len);
    }
    
    goto after_memmove;
    
skip_memmove:
    __builtin_memset(dest, 0, len);
    
after_memmove:
    /* Jump back into scope with memmove */
    if (use_memmove) {
        goto do_final_memmove;
    }
    
    return;
    
do_final_memmove:
    __builtin_memmove(dest + len/2, dest, len/2);
}

/* Parallel memory operations */
static void parallel_mem_ops(void) {
    const int num_threads = 4;
    char *buffers[num_threads];
    size_t sizes[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        sizes[tid] = (tid + 1) * 64;
        buffers[tid] = malloc(sizes[tid]);
        
        if (buffers[tid]) {
            /* Each thread uses different builtins */
            switch (tid % 3) {
                case 0:
                    __builtin_memset(buffers[tid], tid, sizes[tid]);
                    break;
                case 1:
                    if (tid > 0) {
                        __builtin_memcpy(buffers[tid], buffers[tid-1], 
                                       sizes[tid] < sizes[tid-1] ? sizes[tid] : sizes[tid-1]);
                    }
                    break;
                case 2:
                    __builtin_memmove(buffers[tid] + 16, buffers[tid], sizes[tid] - 16);
                    break;
            }
        }
        
        #pragma omp barrier
        
        /* Cross-thread memory operations */
        if (tid == 0) {
            for (int i = 1; i < num_threads; i++) {
                if (buffers[i]) {
                    __builtin_memcpy(buffers[0] + i*16, buffers[i], 16);
                }
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_threads; i++) {
        free(buffers[i]);
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN builtin redirection test\n");
    
    /* Phase 1: Basic builtin calls with volatile lengths */
    char buffer1[512];
    char buffer2[512];
    
    __builtin_memset(buffer1, 0x11, g_memset_len);
    __builtin_memcpy(buffer2, buffer1, g_memcpy_len);
    __builtin_memmove(buffer1 + 64, buffer1, g_memmove_len);
    
    /* Phase 2: Recursive AST with memory operations */
    struct ASTNode *root = create_ast(4, "base_data_string");
    
    /* Phase 3: Goto flow control test */
    goto_memmove_test(buffer1, buffer2, 150);
    goto_memmove_test(buffer1, buffer2, 50);
    
    /* Phase 4: OpenMP parallel section */
    parallel_mem_ops();
    
    /* Phase 5: Complex nested operations */
    volatile int iterations = 3;
    for (volatile int i = 0; i < iterations; i++) {
        char temp[256];
        size_t len = (i + 1) * 32;
        
        __builtin_memset(temp, i, len);
        __builtin_memcpy(buffer1 + i*64, temp, len);
        
        if (i % 2 == 0) {
            __builtin_memmove(temp + 32, temp, len - 32);
        }
    }
    
    /* Verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        hash = (hash * 31) + buffer1[i];
    }
    
    printf("Final hash: %lu\n", hash);
    printf("Test completed\n");
    
    /* Cleanup */
    /* AST cleanup would go here */
    
    return 0;
}
