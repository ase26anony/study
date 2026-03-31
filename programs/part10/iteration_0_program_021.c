/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 1024;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char g_token_pool[4096];
static volatile size_t g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token pool with pattern */
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i * 13) & 0xFF);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive function using built-in memory operations */
static ASTNode* create_ast(size_t depth, volatile size_t* counter) {
    if (depth == 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[256];
    for (int i = 0; i < 256; i++) {
        pattern[i] = (char)((*counter + i) & 0xFF);
    }
    __builtin_memcpy(node->data, pattern, 256);
    
    /* Set size using volatile */
    node->size = g_mem_size + depth;
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        volatile int use_goto = (depth % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, counter);
        node->right = NULL;
        
        create_children:
        (*counter)++;
        node->right = create_ast(depth - 2, counter);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with goto jumping into memmove block */
static void test_goto_memmove(volatile char* dest, volatile char* src, size_t n) {
    int condition = (n > 100);
    
    if (condition) {
        goto perform_copy;
    }
    
    /* This block should be jumped into */
    perform_copy:
    {
        char buffer[512];
        /* Use __builtin_memmove with overlap */
        __builtin_memmove(buffer, (void*)src, n);
        __builtin_memmove((void*)dest, buffer, n);
    }
    
    /* Jump out */
    if (n > 200) {
        goto cleanup;
    }
    
    /* Additional memory operation */
    __builtin_memset((void*)dest + n/2, 0xAA, n/4);
    
    cleanup:
    return;
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_ops(void) {
    const size_t num_blocks = 16;
    char* blocks[num_blocks];
    
    #pragma omp parallel for
    for (size_t i = 0; i < num_blocks; i++) {
        blocks[i] = (char*)malloc(g_mem_size);
        if (blocks[i]) {
            /* Use all three built-ins in parallel */
            __builtin_memset(blocks[i], (int)(i & 0xFF), g_mem_size);
            
            if (i > 0) {
                __builtin_memcpy(blocks[i], blocks[i-1], g_mem_size / 2);
            }
            
            /* Create overlap for memmove */
            if (i > 1) {
                __builtin_memmove(blocks[i] + g_mem_size/4, 
                                 blocks[i] + g_mem_size/8, 
                                 g_mem_size/8);
            }
        }
    }
    
    /* Cleanup */
    #pragma omp parallel for
    for (size_t i = 0; i < num_blocks; i++) {
        if (blocks[i]) free(blocks[i]);
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    volatile size_t counter = 0;
    size_t total_hash = 0;
    
    /* Test 1: Recursive AST creation with memory operations */
    printf("Test 1: Creating recursive AST structure\n");
    ASTNode* root = create_ast(5, &counter);
    
    if (root) {
        /* Copy between AST nodes */
        ASTNode* copy = (ASTNode*)malloc(sizeof(ASTNode));
        if (copy) {
            __builtin_memcpy(copy, root, sizeof(ASTNode));
            
            /* Calculate hash from copied data */
            for (size_t i = 0; i < sizeof(copy->data); i++) {
                total_hash += (size_t)copy->data[i];
            }
            
            free(copy);
        }
        
        /* Free AST recursively */
        free(root);
    }
    
    /* Test 2: Goto with memmove */
    printf("Test 2: Testing goto with memmove\n");
    char src_buffer[1024];
    char dest_buffer[1024];
    
    /* Initialize with pattern */
    for (size_t i = 0; i < sizeof(src_buffer); i++) {
        src_buffer[i] = (char)(i & 0xFF);
    }
    
    test_goto_memmove((volatile char*)dest_buffer, 
                     (volatile char*)src_buffer, 
                     g_mem_size);
    
    /* Verify copy */
    for (size_t i = 0; i < 100; i++) {
        total_hash += (size_t)dest_buffer[i];
    }
    
    /* Test 3: OpenMP parallel operations */
    printf("Test 3: Running parallel memory operations\n");
    parallel_memory_ops();
    
    /* Test 4: Direct built-in calls with volatile control */
    printf("Test 4: Direct built-in calls\n");
    volatile char* dynamic_buf = (volatile char*)malloc(2048);
    if (dynamic_buf) {
        /* Chain of memory operations */
        __builtin_memset((void*)dynamic_buf, 0xCC, 512);
        __builtin_memcpy((void*)(dynamic_buf + 512), (void*)dynamic_buf, 256);
        __builtin_memmove((void*)(dynamic_buf + 256), (void*)dynamic_buf, 512);
        
        /* Add to hash */
        for (size_t i = 0; i < 128; i++) {
            total_hash += (size_t)dynamic_buf[i];
        }
        
        free((void*)dynamic_buf);
    }
    
    /* Test 5: Token pool operations */
    printf("Test 5: Token pool memory operations\n");
    char token_copy[1024];
    
    /* Use all three built-ins on token pool */
    __builtin_memcpy(token_copy, g_token_pool, 512);
    __builtin_memset(token_copy + 512, 0xDD, 256);
    __builtin_memmove(g_token_pool + 256, token_copy, 512);
    
    /* Final hash calculation */
    for (size_t i = 0; i < 256; i++) {
        total_hash += (size_t)g_token_pool[i];
    }
    
    printf("Test completed. Final hash: %zu\n", total_hash);
    printf("Expected to trigger ASAN built-in redirection for:\n");
    printf("  - __builtin_memcpy\n");
    printf("  - __builtin_memset\n");
    printf("  - __builtin_memmove\n");
    
    return 0;
}
