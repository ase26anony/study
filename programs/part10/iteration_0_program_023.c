/* ISO C99-compliant test program for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
struct ast_node {
    char data[64];
    struct ast_node* left;
    struct ast_node* right;
    int depth;
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    printf("ASAN test constructor initialized\n");
    /* Force initialization of memory function redirection */
    char buf[32];
    __builtin_memset(buf, 0, sizeof(buf));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("ASAN test destructor cleaning up\n");
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(*node));
    
    /* Copy data using __builtin_memcpy with volatile size */
    size_t copy_len = g_mem_size % 64;
    if (copy_len > 63) copy_len = 63;
    
    __builtin_memcpy(node->data, base_data, copy_len);
    node->data[copy_len] = '\0';
    node->depth = depth;
    
    /* Build children with goto for flow control */
    int build_left = 1;
    
    if (depth > 2) {
        goto build_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
build_children:
    /* Jump back into block with memory operations */
    node->left = build_ast(depth - 1, base_data);
    node->right = build_ast(depth - 1, base_data);
    
    /* Copy between nodes using __builtin_memmove */
    if (node->left && node->right) {
        volatile size_t move_size = 32;
        __builtin_memmove(node->right->data, node->left->data, move_size);
    }
    
done:
    return node;
}

/* Function with goto jumping around memory operations */
static void test_goto_memmove(char* dest, const char* src, size_t n) {
    int use_memmove = 1;
    
    if (n < 16) {
        goto small_copy;
    }
    
    /* Jump over this block initially */
    goto skip_large;
    
large_copy:
    /* This block contains __builtin_memmove */
    __builtin_memmove(dest, src, n);
    goto done;
    
skip_large:
    if (use_memmove) {
        goto large_copy;
    }
    
small_copy:
    /* Alternative path with __builtin_memcpy */
    __builtin_memcpy(dest, src, n > 16 ? 16 : n);
    
done:
    return;
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    const int num_threads = 4;
    char thread_buffers[4][128];
    volatile int sync_flag = 0;
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        /* Initialize buffer with __builtin_memset */
        __builtin_memset(thread_buffers[tid], tid + 'A', 128);
        
        #pragma omp barrier
        
        /* Rotate buffers using __builtin_memcpy */
        int next_tid = (tid + 1) % num_threads;
        __builtin_memcpy(thread_buffers[tid], 
                        thread_buffers[next_tid], 
                        64);
        
        /* Use volatile to prevent optimization */
        sync_flag = tid;
    }
}

/* Multi-stage initialization function */
static void complex_initialization(void) {
    /* Stage 1: Direct built-in calls */
    char stage1_buf[256];
    __builtin_memset(stage1_buf, 0xAA, sizeof(stage1_buf));
    
    /* Stage 2: Indirect via function pointer */
    void (*mem_func)(void*, int, size_t) = 
        (void (*)(void*, int, size_t))__builtin_memset;
    mem_func(stage1_buf + 128, 0xBB, 64);
    
    /* Stage 3: Inlined assembly barrier */
    asm volatile("" ::: "memory");
    
    /* Stage 4: Another built-in */
    char stage2_buf[256];
    __builtin_memcpy(stage2_buf, stage1_buf, sizeof(stage1_buf));
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Build recursive AST structure */
    struct ast_node* root = build_ast(4, "AST_Base_Data");
    
    /* Test goto flow control with memory operations */
    char src[64], dest[64];
    __builtin_memset(src, 'X', sizeof(src));
    test_goto_memmove(dest, src, sizeof(src));
    
    /* Execute OpenMP parallel section */
    #ifdef _OPENMP
    parallel_memory_ops();
    #endif
    
    /* Complex multi-stage initialization */
    complex_initialization();
    
    /* Verify operations by computing checksum */
    unsigned long checksum = 0;
    if (root) {
        /* Use all memory functions in verification */
        char verify_buf[256];
        __builtin_memset(verify_buf, 0, sizeof(verify_buf));
        __builtin_memcpy(verify_buf, root->data, 64);
        __builtin_memmove(verify_buf + 128, verify_buf, 64);
        
        for (size_t i = 0; i < sizeof(verify_buf); i++) {
            checksum += verify_buf[i];
        }
        
        /* Cleanup */
        free(root);
    }
    
    printf("Test completed. Checksum: %lu\n", checksum);
    printf("Compile with: -O2 -fsanitize=address -fopenmp\n");
    printf("Or for HWASAN: -O2 -fsanitize=kernel-hwaddress -fopenmp\n");
    
    return checksum == 0 ? 0 : 1;
}
