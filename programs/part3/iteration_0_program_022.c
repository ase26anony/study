/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ast_node {
    int type;
    char data[32];
    struct ast_node *left;
    struct ast_node *right;
} ast_node_t;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < 256; i++) {
        volatile_src[i] = (char)(i % 26 + 'A');
    }
    
    /* Use builtins in constructor */
    __builtin_memset(volatile_dest, 0, sizeof(volatile_dest));
    __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Final builtin usage */
    __builtin_memset(volatile_dest, 0xFF, 16);
}

/* Recursive function with memory operations */
static ast_node_t* create_ast(int depth, int index) {
    if (depth <= 0) return NULL;
    
    ast_node_t* node = malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Initialize node data with builtins */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    node->type = index;
    
    /* Copy data between volatile and node */
    __builtin_memcpy(node->data, (void*)volatile_src + index * 8, 16);
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    if (index % 3 == 0) {
        goto skip_left;
    }
    
    node->left = create_ast(depth - 1, index * 2);
    create_left = 0;
    
skip_left:
    if (create_left) {
        /* Jump back in with memmove */
        char temp[32];
        __builtin_memmove(temp, node->data, 16);
        __builtin_memmove(node->data + 8, temp, 8);
    }
    
    node->right = create_ast(depth - 1, index * 2 + 1);
    
    return node;
}

/* Complex memory operation with goto jumps */
static void complex_memory_ops(void) {
    char buffer1[128];
    char buffer2[128];
    char buffer3[128];
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 'A', sizeof(buffer1));
    __builtin_memset(buffer2, 'B', sizeof(buffer2));
    
    /* Goto jumping into memory operation block */
    int use_memmove = 1;
    if (volatile_len > 32) {
        goto do_memmove;
    }
    
    /* Regular memcpy path */
    __builtin_memcpy(buffer3, buffer1, volatile_len);
    goto after_memmove;
    
do_memmove:
    /* Jump target with memmove */
    __builtin_memmove(buffer3, buffer1, volatile_len);
    
    /* Jump out and back in */
    if (volatile_len < 100) {
        goto copy_again;
    }
    
after_memmove:
    /* Another memcpy after goto */
    __builtin_memcpy(buffer2 + 32, buffer3 + 16, 16);
    return;
    
copy_again:
    /* Jump back to copy more */
    __builtin_memcpy(buffer2, buffer3, 8);
    goto after_memmove;
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    int i;
    char local_buf[1024];
    
    /* Initialize with memset */
    __builtin_memset(local_buf, 0, sizeof(local_buf));
    
    #pragma omp parallel private(i)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf + tid * 64, tid, 64);
        
        #pragma omp for
        for (i = 0; i < 16; i++) {
            char thread_buf[64];
            
            /* Mix of builtins in parallel region */
            __builtin_memcpy(thread_buf, local_buf + i * 64, 32);
            __builtin_memset(thread_buf + 32, i, 16);
            __builtin_memmove(local_buf + i * 64 + 16, thread_buf, 32);
        }
    }
    
    /* Final builtin after parallel region */
    __builtin_memcpy((void*)volatile_dest, local_buf, 256);
}

/* Multi-stage initialization */
static void initialize_system(void) {
    /* Stage 1: Direct builtin calls */
    char init_buf[512];
    __builtin_memset(init_buf, 0xCC, sizeof(init_buf));
    
    /* Stage 2: Volatile length builtin */
    __builtin_memcpy(init_buf + 128, (void*)volatile_src, volatile_len);
    
    /* Stage 3: Nested builtin calls */
    char temp[64];
    __builtin_memset(temp, 0xAA, sizeof(temp));
    __builtin_memmove(init_buf + 256, temp, sizeof(temp));
    __builtin_memcpy(temp, init_buf + 128, 32);
}

/* Main execution flow */
int main(void) {
    ast_node_t *root = NULL;
    unsigned long hash = 0;
    
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Force initialization of asan_memfn_rtls cache */
    initialize_system();
    
    /* Create recursive structure with memory ops */
    root = create_ast(4, 1);
    
    /* Execute complex memory operations with goto */
    complex_memory_ops();
    
    /* Run parallel memory operations */
    parallel_memory_ops();
    
    /* Verify operations by computing hash */
    for (int i = 0; i < 256; i++) {
        hash += (unsigned char)volatile_dest[i];
        hash = (hash << 3) | (hash >> 29); /* Simple rotation */
    }
    
    /* Additional builtin calls in main */
    char final_buf[128];
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf, (void*)volatile_dest, 64);
    __builtin_memmove(final_buf + 64, final_buf, 32);
    
    printf("Result hash: 0x%08lx\n", hash);
    printf("Test completed.\n");
    
    /* Cleanup */
    free(root);
    
    return 0;
}
