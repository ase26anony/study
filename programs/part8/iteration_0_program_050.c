/* asan_coverage.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)((i * 31) & 0xFF);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Clear sensitive data */
    __builtin_memset(token_pool, 0, sizeof(token_pool));
    printf("Destructor: Token pool cleared\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = depth;
    
    /* Copy data using __builtin_memcpy with volatile length */
    int copy_len = (volatile_len % 128) + 128;
    if (copy_len > 255) copy_len = 255;
    
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Create children recursively */
    char child_data[256];
    __builtin_memcpy(child_data, base_data, 256);
    child_data[0] = (char)(child_data[0] + depth);
    
    node->left = create_ast(depth - 1, child_data);
    node->right = create_ast(depth - 1, child_data);
    
    return node;
}

/* Function with goto jumps around memory operations */
static void goto_memmove_test(char* dest, const char* src, size_t len) {
    int use_memmove = volatile_flag & 1;
    
    if (use_memmove) {
        goto use_memmove_block;
    } else {
        goto use_memcpy_block;
    }
    
use_memmove_block:
    {
        char temp[512];
        /* Jump into block containing __builtin_memmove */
        __builtin_memmove(temp, src, len);
        __builtin_memmove(dest, temp, len);
        goto after_operation;
    }
    
use_memcpy_block:
    {
        /* Different path with __builtin_memcpy */
        __builtin_memcpy(dest, src, len);
        goto after_operation;
    }
    
after_operation:
    /* Verify with volatile read */
    if (dest[0] != src[0]) {
        volatile_flag = 0;
    }
}

/* Parallel memory dispatch with OpenMP */
static void parallel_memory_ops(void) {
    const int num_blocks = 16;
    char blocks[num_blocks][256];
    long long hash_sum = 0;
    
    #pragma omp parallel reduction(+:hash_sum)
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        #pragma omp for
        for (int i = 0; i < num_blocks; i++) {
            /* Initialize block with pattern */
            __builtin_memset(blocks[i], (char)(i + thread_id), sizeof(blocks[i]));
            
            /* Copy between blocks with offset */
            int src_idx = (i + 1) % num_blocks;
            __builtin_memcpy(blocks[i] + 128, blocks[src_idx], 128);
            
            /* Move data within block */
            __builtin_memmove(blocks[i], blocks[i] + 64, 64);
            
            /* Compute simple hash */
            for (int j = 0; j < 64; j++) {
                hash_sum += (long long)blocks[i][j];
            }
        }
    }
    
    printf("Parallel hash sum: %lld\n", hash_sum);
}

/* Complex initialization with multiple built-in calls */
static void initialize_complex_buffer(char* buffer, size_t size) {
    /* Phase 1: Clear with memset */
    __builtin_memset(buffer, 0xAA, size / 2);
    
    /* Phase 2: Copy pattern */
    char pattern[128];
    for (int i = 0; i < sizeof(pattern); i++) {
        pattern[i] = (char)(i * 7);
    }
    
    for (size_t offset = 0; offset < size; offset += sizeof(pattern)) {
        size_t remaining = size - offset;
        size_t copy_size = (remaining > sizeof(pattern)) ? sizeof(pattern) : remaining;
        __builtin_memcpy(buffer + offset, pattern, copy_size);
    }
    
    /* Phase 3: Move overlapping regions */
    size_t move_size = size / 4;
    __builtin_memmove(buffer + move_size, buffer, move_size * 3);
    
    /* Phase 4: Final clear of first quarter */
    __builtin_memset(buffer, 0, move_size);
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Test 1: Basic built-in calls */
    printf("\nTest 1: Basic memory built-ins\n");
    char buffer1[1024];
    char buffer2[1024];
    
    __builtin_memset(buffer1, 0x42, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 256, buffer1, 512);
    
    /* Test 2: Recursive AST with memory operations */
    printf("\nTest 2: Recursive AST operations\n");
    ASTNode* root = create_ast(4, token_pool);
    
    if (root) {
        /* Copy between AST nodes */
        if (root->left && root->right) {
            __builtin_memcpy(root->right->data, root->left->data, 256);
            __builtin_memmove(root->left->data + 128, root->left->data, 128);
        }
        
        /* Free AST recursively */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    /* Test 3: Goto flow control */
    printf("\nTest 3: Goto-controlled memory operations\n");
    char src_data[512];
    char dst_data[512];
    
    for (int i = 0; i < sizeof(src_data); i++) {
        src_data[i] = (char)(i * 13);
    }
    
    goto_memmove_test(dst_data, src_data, sizeof(src_data));
    
    /* Test 4: OpenMP parallel operations */
    printf("\nTest 4: Parallel memory dispatch\n");
    parallel_memory_ops();
    
    /* Test 5: Complex buffer initialization */
    printf("\nTest 5: Complex buffer initialization\n");
    char complex_buffer[2048];
    initialize_complex_buffer(complex_buffer, sizeof(complex_buffer));
    
    /* Verify results with checksum */
    unsigned int checksum = 0;
    for (size_t i = 0; i < sizeof(complex_buffer); i++) {
        checksum += (unsigned char)complex_buffer[i];
    }
    
    printf("\nFinal checksum: 0x%08X\n", checksum);
    printf("=== Test completed ===\n");
    
    return 0;
}
