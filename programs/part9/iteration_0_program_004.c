/* asan_coverage.c - Comprehensive test for ASAN memory built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char g_token_array[4096];
static volatile size_t g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token array with pattern */
    for (size_t i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)((i * 31) & 0xFF);
    }
    printf("Constructor: Initialized token array\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(size_t depth, volatile size_t* counter) {
    if (depth == 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[64];
    for (int i = 0; i < 64; i++) {
        pattern[i] = (char)((depth + i) & 0xFF);
    }
    __builtin_memcpy(node->data, pattern, 64);
    
    node->size = depth * 64;
    (*counter)++;
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_goto = (depth % 2 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, counter);
        node->right = NULL;
        return node;
        
    create_children:
        node->left = create_ast(depth - 1, counter);
        node->right = create_ast(depth - 1, counter);
        
        /* Use __builtin_memmove between children */
        if (node->left && node->right) {
            __builtin_memmove(node->left->data + 32, 
                            node->right->data, 32);
        }
    }
    
    return node;
}

/* Complex memory operation with goto edge cases */
static void memory_operations_with_goto(volatile char* dest, 
                                       volatile char* src, 
                                       size_t size) {
    volatile int stage = 0;
    
    /* Multiple goto labels to test flow sensitivity */
    stage_0:
    if (stage == 0) {
        /* First memcpy */
        __builtin_memcpy((void*)dest, (void*)src, size / 4);
        stage = 1;
        goto stage_1;
    }
    
    stage_1:
    if (stage == 1) {
        /* Jump into memmove block */
        volatile char* temp = dest + size / 2;
        goto do_memmove;
    }
    
    stage_2:
    if (stage == 2) {
        /* Final memset */
        __builtin_memset((void*)(dest + size * 3 / 4), 0xFF, size / 4);
        return;
    }
    
    do_memmove:
    /* This block contains __builtin_memmove with goto in/out */
    {
        volatile char buffer[128];
        __builtin_memcpy(buffer, src + size / 4, 128);
        __builtin_memmove((void*)temp, buffer, 128);
        stage = 2;
        goto stage_2;
    }
}

/* OpenMP parallel memory dispatch */
static void parallel_memory_dispatch(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        size_t block_size = g_mem_size / omp_get_num_threads();
        
        /* Each thread works on its own buffer */
        char* local_buf = (char*)malloc(block_size);
        if (!local_buf) return;
        
        /* Use all three builtins in parallel region */
        __builtin_memset(local_buf, thread_id, block_size);
        
        /* Copy from global token array */
        size_t offset = (thread_id * block_size) % sizeof(g_token_array);
        __builtin_memcpy(local_buf, g_token_array + offset, 
                        block_size > sizeof(g_token_array) - offset ? 
                        sizeof(g_token_array) - offset : block_size);
        
        /* Move data within buffer */
        if (block_size > 64) {
            __builtin_memmove(local_buf + 32, local_buf, block_size - 32);
        }
        
        free(local_buf);
    }
}

/* Main test execution */
int main(void) {
    volatile size_t node_counter = 0;
    size_t total_hash = 0;
    
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Test 1: Recursive AST creation with memory operations */
    printf("Creating recursive AST structure...\n");
    ASTNode* root = create_ast(4, &node_counter);
    printf("Created %zu AST nodes\n", node_counter);
    
    /* Test 2: Memory operations with goto edge cases */
    printf("Testing memory operations with goto...\n");
    char buffer1[512];
    char buffer2[512];
    
    /* Initialize buffers */
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        buffer1[i] = (char)(i & 0xFF);
        buffer2[i] = (char)((i + 128) & 0xFF);
    }
    
    memory_operations_with_goto((volatile char*)buffer1, 
                               (volatile char*)buffer2, 
                               sizeof(buffer1));
    
    /* Test 3: OpenMP parallel memory operations */
    printf("Testing OpenMP parallel memory dispatch...\n");
    #ifdef _OPENMP
    parallel_memory_dispatch();
    printf("OpenMP parallel section completed\n");
    #else
    printf("OpenMP not available, skipping parallel test\n");
    #endif
    
    /* Test 4: Direct built-in calls with volatile control */
    printf("Testing direct built-in calls...\n");
    volatile char direct_buf[256];
    volatile char src_buf[256];
    
    /* Force all three builtins to be called */
    __builtin_memset(direct_buf, 0xAA, sizeof(direct_buf));
    __builtin_memcpy((void*)src_buf, (void*)direct_buf, sizeof(src_buf));
    __builtin_memmove(direct_buf + 128, direct_buf, 128);
    
    /* Calculate verification hash */
    for (size_t i = 0; i < sizeof(direct_buf); i++) {
        total_hash += direct_buf[i];
    }
    
    /* Cleanup AST */
    /* Note: In real ASAN, this would detect leaks if we don't free */
    /* For coverage testing, we intentionally leak to test ASAN leak detection */
    /* root would need to be freed with proper tree traversal in production */
    
    printf("Total hash: %zu\n", total_hash);
    printf("Test completed successfully\n");
    
    return 0;
}
