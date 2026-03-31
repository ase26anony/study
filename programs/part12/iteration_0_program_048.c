/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ast_node {
    struct ast_node* left;
    struct ast_node* right;
    char data[64];
    uint32_t hash;
} ast_node_t;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    g_init_flag = 1;
    printf("Constructor: Initializing ASAN test environment\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    printf("Destructor: Cleaning up ASAN test resources\n");
}

/* Recursive function with memory operations */
static ast_node_t* create_ast(int depth, const char* base) {
    if (depth <= 0) return NULL;
    
    ast_node_t* node = malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    
    /* Copy data with builtin memcpy */
    size_t len = strlen(base);
    if (len > 63) len = 63;
    __builtin_memcpy(node->data, base, len);
    node->data[len] = '\0';
    
    /* Create children recursively */
    char left_name[64], right_name[64];
    snprintf(left_name, sizeof(left_name), "%s-L%d", base, depth);
    snprintf(right_name, sizeof(right_name), "%s-R%d", base, depth);
    
    node->left = create_ast(depth - 1, left_name);
    node->right = create_ast(depth - 1, right_name);
    
    return node;
}

/* Function with goto control flow */
static void process_with_goto(ast_node_t* src, ast_node_t* dst) {
    volatile int use_goto = 1;
    
    if (use_goto) {
        goto memmove_block;
    }
    
    normal_path:
    /* Regular memory copy */
    __builtin_memcpy(dst->data, src->data, 64);
    return;
    
    memmove_block:
    {
        /* Use builtin memmove with goto entry */
        char temp[64];
        __builtin_memmove(temp, src->data, 64);
        
        if (dst->hash % 2) {
            goto normal_path;
        }
        
        __builtin_memmove(dst->data, temp, 64);
    }
}

/* Compute hash of AST */
static uint32_t compute_ast_hash(ast_node_t* node) {
    if (!node) return 0;
    
    uint32_t hash = 5381;
    char* p = node->data;
    
    /* Hash string using DJB2 algorithm */
    while (*p) {
        hash = ((hash << 5) + hash) + *p++;
    }
    
    uint32_t left_hash = compute_ast_hash(node->left);
    uint32_t right_hash = compute_ast_hash(node->right);
    
    /* Combine hashes */
    hash = ((hash << 5) + hash) ^ left_hash;
    hash = ((hash << 5) + hash) ^ right_hash;
    
    node->hash = hash;
    return hash;
}

/* Main test function with OpenMP */
static void run_asan_tests(void) {
    const size_t array_size = g_mem_size;
    char* buffer1 = malloc(array_size);
    char* buffer2 = malloc(array_size);
    
    if (!buffer1 || !buffer2) {
        free(buffer1);
        free(buffer2);
        return;
    }
    
    /* Initialize buffers with different patterns */
    for (size_t i = 0; i < array_size; i++) {
        buffer1[i] = (char)(i % 256);
        buffer2[i] = (char)((i + 128) % 256);
    }
    
    /* OpenMP parallel region with memory operations */
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                __builtin_memcpy(buffer1 + thread_id * 16, 
                               buffer2 + thread_id * 16, 16);
                break;
            case 1:
                __builtin_memset(buffer1 + thread_id * 16, 
                               thread_id, 16);
                break;
            case 2:
                __builtin_memmove(buffer1 + thread_id * 16,
                                buffer2 + thread_id * 16, 16);
                break;
        }
        
        /* Barrier to synchronize */
        #pragma omp barrier
        
        /* Verify the operations */
        #pragma omp single
        {
            uint32_t checksum = 0;
            for (size_t i = 0; i < array_size; i++) {
                checksum += (uint8_t)buffer1[i];
            }
            printf("Thread checksum: %u\n", checksum);
        }
    }
    
    free(buffer1);
    free(buffer2);
}

/* Function with complex control flow */
static void complex_memory_operations(void) {
    volatile int mode = 2;
    char data[128];
    char backup[128];
    
    /* Initialize with pattern */
    for (int i = 0; i < 128; i++) {
        data[i] = (char)(i * 3);
    }
    
    /* Multiple memory operations with different builtins */
    switch (mode) {
        case 0:
            __builtin_memcpy(backup, data, 128);
            break;
        case 1:
            __builtin_memset(data, 0xAA, 128);
            break;
        case 2:
            __builtin_memmove(data + 32, data, 64);
            __builtin_memcpy(backup, data, 128);
            __builtin_memset(data, 0, 128);
            __builtin_memmove(data, backup, 128);
            break;
    }
    
    /* Nested loops with memory operations */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int offset = (i * 10 + j) % 100;
            __builtin_memcpy(data + offset, backup + offset, 4);
        }
    }
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Test 1: Basic built-in calls */
    printf("\n=== Test 1: Basic built-in operations ===\n");
    {
        char src[100], dst[100];
        __builtin_memset(src, 'A', sizeof(src));
        __builtin_memcpy(dst, src, sizeof(src));
        __builtin_memmove(dst + 10, dst, 50);
        
        int sum = 0;
        for (size_t i = 0; i < sizeof(dst); i++) {
            sum += dst[i];
        }
        printf("Basic test sum: %d\n", sum);
    }
    
    /* Test 2: AST operations */
    printf("\n=== Test 2: AST structure operations ===\n");
    ast_node_t* ast = create_ast(3, "ROOT");
    if (ast) {
        ast_node_t* ast_copy = malloc(sizeof(ast_node_t));
        if (ast_copy) {
            /* Test goto control flow with memmove */
            process_with_goto(ast, ast_copy);
            
            /* Compute hashes */
            uint32_t hash1 = compute_ast_hash(ast);
            uint32_t hash2 = compute_ast_hash(ast_copy);
            printf("AST hash comparison: %u vs %u\n", hash1, hash2);
            
            free(ast_copy);
        }
        
        /* Free AST recursively */
        /* (In real code, implement proper recursive free) */
        free(ast);
    }
    
    /* Test 3: OpenMP parallel tests */
    printf("\n=== Test 3: OpenMP parallel operations ===\n");
    run_asan_tests();
    
    /* Test 4: Complex control flow */
    printf("\n=== Test 4: Complex control flow ===\n");
    complex_memory_operations();
    
    /* Test 5: Variable-sized operations */
    printf("\n=== Test 5: Variable-sized operations ===\n");
    {
        volatile size_t sizes[] = {16, 32, 64, 128, 256};
        char* buffers[5];
        
        for (int i = 0; i < 5; i++) {
            buffers[i] = malloc(sizes[i]);
            if (buffers[i]) {
                __builtin_memset(buffers[i], i + 1, sizes[i]);
                
                if (i > 0) {
                    size_t copy_size = sizes[i] < sizes[i-1] ? sizes[i] : sizes[i-1];
                    __builtin_memcpy(buffers[i], buffers[i-1], copy_size);
                }
                
                free(buffers[i]);
            }
        }
    }
    
    printf("\nASAN test completed successfully\n");
    return 0;
}
