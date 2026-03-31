/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_globals(void) {
    /* Force initialization of ASAN runtime */
    volatile char buffer[128];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    
    /* This should trigger ASAN built-in redirection */
    volatile char dest[128];
    __builtin_memcpy(dest, buffer, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_globals(void) {
    /* Final memory operation to ensure coverage */
    volatile int final_check[16];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* pattern) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy pattern with builtin memcpy */
    size_t copy_len = strlen(pattern);
    if (copy_len > sizeof(node->data) - 1)
        copy_len = sizeof(node->data) - 1;
    
    __builtin_memcpy(node->data, pattern, copy_len);
    node->data[copy_len] = '\0';
    node->size = copy_len;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
create_children:
    node->left = create_ast(depth - 1, "LEFT");
    
    /* Jump back to avoid optimization */
    if (create_left) {
        create_left = 0;
        goto create_right;
    }
    
create_right:
    node->right = create_ast(depth - 1, "RIGHT");
    
    return node;
}

/* Function with goto jumping around memmove */
static void process_with_goto(ASTNode* src, ASTNode* dest, int mode) {
    volatile int use_memmove = 1;
    
    if (mode == 0) {
        goto normal_copy;
    } else if (mode == 1) {
        goto memmove_block;
    } else {
        goto skip_all;
    }

normal_copy:
    /* Standard memcpy */
    __builtin_memcpy(dest->data, src->data, src->size);
    goto after_ops;

memmove_block:
    /* This should trigger the memmove redirection */
    if (use_memmove) {
        __builtin_memmove(dest->data + 10, dest->data, src->size);
        use_memmove = 0;
        goto normal_copy;  /* Jump out of memmove block */
    }

skip_all:
    return;

after_ops:
    /* Verify the operation */
    volatile int check = 0;
    for (size_t i = 0; i < src->size; i++) {
        if (dest->data[i] != src->data[i]) {
            check = 1;
            break;
        }
    }
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[256];
        char local_dest[256];
        
        /* Initialize with builtin memset */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        /* Copy between buffers */
        __builtin_memcpy(local_dest, local_buf, sizeof(local_buf));
        
        /* Move data around */
        __builtin_memmove(local_buf + 128, local_buf, 128);
        
        /* Barrier to ensure all threads execute */
        #pragma omp barrier
        
        /* Final verification memset */
        __builtin_memset(local_dest, 0xCC, sizeof(local_dest));
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create AST structures */
    ASTNode* ast1 = create_ast(4, "ROOT_NODE");
    ASTNode* ast2 = create_ast(3, "COPY_TARGET");
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Test 1: Basic builtin operations */
    volatile char buffer1[512];
    volatile char buffer2[512];
    
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 256, buffer1, 256);
    
    /* Test 2: Goto flow control with memmove */
    for (int i = 0; i < 3; i++) {
        process_with_goto(ast1, ast2, i);
    }
    
    /* Test 3: OpenMP parallel section */
    #ifdef _OPENMP
    printf("Running OpenMP parallel memory operations...\n");
    parallel_memory_ops();
    #endif
    
    /* Test 4: Variable-sized operations */
    volatile size_t dynamic_size = g_mem_size % 256;
    char* dyn_buf1 = malloc(dynamic_size);
    char* dyn_buf2 = malloc(dynamic_size);
    
    if (dyn_buf1 && dyn_buf2) {
        __builtin_memset(dyn_buf1, 0xBB, dynamic_size);
        __builtin_memcpy(dyn_buf2, dyn_buf1, dynamic_size);
        __builtin_memmove(dyn_buf1 + dynamic_size/2, dyn_buf1, dynamic_size/2);
        
        free(dyn_buf1);
        free(dyn_buf2);
    }
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < ast1->size && i < sizeof(ast1->data); i++) {
        hash = (hash * 31) + (unsigned char)ast1->data[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    /* Recursive free implementation omitted for brevity */
    
    return 0;
}
