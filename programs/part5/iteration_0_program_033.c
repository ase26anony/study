/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 128;
volatile size_t g_memset_len = 256;
volatile size_t g_memmove_len = 64;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    volatile char final_check[16];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile lengths */
    __builtin_memset(node, 0, sizeof(ASTNode));
    __builtin_memcpy(node->data, base_data, g_memcpy_len % 256);
    node->size = g_memcpy_len % 256;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    node->left = create_ast(depth - 1, base_data + 1);
    node->right = create_ast(depth - 2, base_data + 2);
    
done:
    return node;
}

/* Function with goto jumping around memmove */
static void complex_flow_with_memmove(char* dest, char* src, size_t len) {
    int use_memmove = 1;
    
    if (len < 10) {
        goto small_case;
    }
    
    /* Jump into block with builtin */
    goto do_memmove;
    
small_case:
    __builtin_memcpy(dest, src, len);
    goto finish;
    
do_memmove:
    /* This should trigger the memmove redirection */
    __builtin_memmove(dest, src, g_memmove_len % 128);
    
    /* Jump out */
    goto post_memmove;
    
    /* Unreachable code path */
    __builtin_memset(dest, 0, len);
    
post_memmove:
    /* Additional operation after memmove */
    __builtin_memset(dest + len/2, 0xCC, len/4);
    
finish:
    return;
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    const int num_threads = 4;
    char thread_buffers[num_threads][512];
    volatile size_t local_len = 128;
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread uses different builtins */
        switch (tid % 3) {
            case 0:
                __builtin_memset(thread_buffers[tid], tid, local_len);
                break;
            case 1:
                __builtin_memcpy(thread_buffers[tid], 
                               thread_buffers[(tid + 1) % num_threads],
                               local_len);
                break;
            case 2:
                __builtin_memmove(thread_buffers[tid],
                                thread_buffers[tid] + 64,
                                local_len / 2);
                break;
        }
        
        /* Barrier to ensure all builtins are processed */
        #pragma omp barrier
        
        /* Cross-thread memory operation */
        if (tid == 0) {
            __builtin_memcpy(thread_buffers[1],
                           thread_buffers[2],
                           local_len);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Phase 1: Basic builtin usage with volatiles */
    char buffer1[1024];
    char buffer2[1024];
    
    __builtin_memset(buffer1, 0x11, g_memset_len % 512);
    __builtin_memcpy(buffer2, buffer1, g_memcpy_len % 512);
    __builtin_memmove(buffer1 + 128, buffer1, g_memmove_len % 256);
    
    /* Phase 2: Recursive structure operations */
    ASTNode* root = create_ast(4, "TestASTData");
    if (root) {
        ASTNode temp;
        __builtin_memcpy(&temp, root, sizeof(ASTNode));
        __builtin_memmove(root->data, root->data + 32, 64);
        
        /* Cleanup */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    /* Phase 3: Complex flow with goto */
    char src[256], dest[256];
    for (int i = 0; i < 256; i++) {
        src[i] = (char)i;
    }
    
    complex_flow_with_memmove(dest, src, 128);
    
    /* Phase 4: OpenMP parallel operations */
    parallel_memory_ops();
    
    /* Phase 5: Mixed operations in loops */
    volatile int iterations = 3;
    char loop_buffers[3][256];
    
    for (volatile int i = 0; i < iterations; i++) {
        switch (i % 3) {
            case 0:
                __builtin_memset(loop_buffers[i], i * 16, 128);
                break;
            case 1:
                __builtin_memcpy(loop_buffers[(i + 1) % 3],
                               loop_buffers[i],
                               64);
                break;
            case 2:
                __builtin_memmove(loop_buffers[i],
                                loop_buffers[i] + 32,
                                96);
                break;
        }
    }
    
    /* Verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        hash = hash * 31 + buffer1[i];
    }
    for (size_t i = 0; i < sizeof(dest); i++) {
        hash = hash * 31 + dest[i];
    }
    
    printf("Test completed. Hash: %lu\n", hash);
    printf("If compiled with -fsanitize=address or -fsanitize=kernel-hwaddress,\n");
    printf("the ASAN/HWASAN builtin redirection logic should be triggered.\n");
    
    return 0;
}
