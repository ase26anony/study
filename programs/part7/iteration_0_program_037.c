/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char g_token_array[4096];
static volatile size_t g_token_pos = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Force initialization of memory functions */
    volatile char buffer[128];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(g_token_array, buffer, 64);
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[64];
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* src, size_t depth) {
    if (depth > 5) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy with volatile size */
    volatile size_t copy_size = (g_mem_size < 256) ? g_mem_size : 256;
    __builtin_memcpy(node->data, src, copy_size);
    node->size = copy_size;
    
    /* Recursive creation with goto for flow control */
    if (depth < 3) {
        node->left = create_ast_node(src + 1, depth + 1);
        
        /* Jump label for goto */
        create_right:
        node->right = create_ast_node(src + 2, depth + 1);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with goto jumping into memmove block */
static void process_with_goto(ASTNode* node1, ASTNode* node2) {
    if (!node1 || !node2) return;
    
    volatile int use_memmove = 1;
    
    if (use_memmove) {
        goto perform_memmove;
    }
    
    /* This block will be jumped into */
    perform_memmove:
    {
        volatile char temp[256];
        size_t move_size = (node1->size < node2->size) ? node1->size : node2->size;
        
        /* Force builtin memmove with volatile parameters */
        __builtin_memmove(temp, node1->data, move_size);
        __builtin_memmove(node2->data, temp, move_size);
        
        /* Jump out of the block */
        goto after_memmove;
    }
    
    after_memmove:
    /* Additional memory operation */
    if (node1->size > 0) {
        __builtin_memset(node1->data + node1->size/2, 0xCC, 16);
    }
}

/* Parallel memory dispatch using OpenMP */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        /* Each thread gets its own buffers */
        char thread_buf1[256];
        char thread_buf2[256];
        volatile int thread_id = omp_get_thread_num();
        
        /* Initialize with builtin memset */
        __builtin_memset(thread_buf1, thread_id, sizeof(thread_buf1));
        
        /* Copy between buffers */
        __builtin_memcpy(thread_buf2, thread_buf1, sizeof(thread_buf1));
        
        /* Move data around */
        __builtin_memmove(thread_buf1 + 128, thread_buf2, 128);
        
        #pragma omp critical
        {
            /* Copy to global array */
            size_t offset = (g_token_pos * 64) % sizeof(g_token_array);
            __builtin_memcpy(g_token_array + offset, thread_buf1, 64);
            g_token_pos++;
        }
    }
}

/* Complex initialization with varied memory operations */
static void initialize_token_array(void) {
    volatile char pattern[] = "ASAN_TEST_PATTERN_0123456789_ABCDEF";
    volatile size_t pattern_len = sizeof(pattern) - 1;
    
    /* Fill array using different builtins */
    for (size_t i = 0; i < sizeof(g_token_array); i += 128) {
        if (i % 256 == 0) {
            __builtin_memset(g_token_array + i, 0x00, 64);
            __builtin_memcpy(g_token_array + i + 64, pattern, 
                           (pattern_len < 64) ? pattern_len : 64);
        } else {
            __builtin_memmove(g_token_array + i, pattern, 
                            (pattern_len < 128) ? pattern_len : 128);
        }
    }
}

/* Compute verification hash */
static unsigned long compute_verification_hash(void) {
    unsigned long hash = 0;
    volatile char* ptr = g_token_array;
    
    for (size_t i = 0; i < sizeof(g_token_array); i++) {
        hash = (hash * 31) + ptr[i];
        
        /* Occasional memory operation during hash computation */
        if (i % 512 == 0) {
            volatile char temp[32];
            __builtin_memcpy(temp, ptr + i, 32);
            __builtin_memset(ptr + i, hash & 0xFF, 1);
        }
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Stage 1: Initialize token array */
    initialize_token_array();
    
    /* Stage 2: Create recursive AST structures */
    ASTNode* root1 = create_ast_node(g_token_array, 0);
    ASTNode* root2 = create_ast_node(g_token_array + 128, 0);
    
    /* Stage 3: Process with goto flow control */
    process_with_goto(root1, root2);
    
    /* Stage 4: Parallel memory operations */
    parallel_memory_operations();
    
    /* Stage 5: Additional builtin calls in different contexts */
    {
        volatile char final_buf[512];
        volatile size_t final_size = g_mem_size % 512;
        
        /* Chain of memory operations */
        __builtin_memset(final_buf, 0x55, final_size);
        __builtin_memcpy(final_buf + 128, g_token_array, 256);
        __builtin_memmove(g_token_array, final_buf, 128);
        
        /* Nested memory operations */
        for (int i = 0; i < 4; i++) {
            volatile char nested_buf[64];
            __builtin_memcpy(nested_buf, final_buf + i * 64, 64);
            __builtin_memset(final_buf + i * 64, i, 64);
            __builtin_memmove(final_buf + 256 + i * 64, nested_buf, 64);
        }
    }
    
    /* Stage 6: Compute and print verification result */
    unsigned long verification_hash = compute_verification_hash();
    printf("Verification hash: 0x%016lx\n", verification_hash);
    
    /* Cleanup */
    free(root1);
    free(root2);
    
    printf("Test completed successfully.\n");
    return 0;
}
