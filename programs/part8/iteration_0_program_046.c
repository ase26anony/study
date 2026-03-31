/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    uint32_t hash;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force early initialization of ASAN runtime */
    volatile char buffer[16];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_late(void) {
    /* Final memory operation to ensure all paths are taken */
    volatile char final_buf[8];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Create pattern in data using memcpy */
    char pattern[64];
    for (int i = 0; i < 64; i++) {
        pattern[i] = (char)(depth + i);
    }
    __builtin_memcpy(node->data, pattern, 64);
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_left = depth % 2;
        
        if (use_left) {
            node->left = create_ast(depth - 1);
            goto skip_right;
        }
        
        node->right = create_ast(depth - 1);
        
        skip_right:
        /* Copy between nodes if both exist */
        if (node->left && node->right) {
            if (g_use_memmove) {
                /* Use memmove for overlapping regions */
                __builtin_memmove(node->left->data + 16, 
                                 node->left->data, 
                                 32);
            }
        }
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void goto_mem_operations(void* dest, void* src, size_t n) {
    int condition = 1;
    
    if (condition) {
        goto do_memcpy;
    }
    
    /* This label is jumped into */
    do_memcpy:
    __builtin_memcpy(dest, src, n);
    
    /* Jump out to another operation */
    goto do_memset;
    
    do_memset:
    __builtin_memset(src, 0xAA, n / 2);
    return;
}

/* Calculate hash of AST */
static uint32_t hash_ast(ASTNode* node) {
    if (!node) return 0;
    
    uint32_t hash = 0;
    
    /* Hash data using byte-by-byte operations */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        hash = (hash << 5) - hash + node->data[i];
    }
    
    /* Recursive hash */
    hash ^= hash_ast(node->left);
    hash ^= hash_ast(node->right);
    
    node->hash = hash;
    return hash;
}

/* Parallel memory operations */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char src_buf[128];
        char dst_buf[128];
        
        /* Initialize with builtin memset */
        __builtin_memset(src_buf, thread_id, sizeof(src_buf));
        
        /* Copy with builtin memcpy */
        __builtin_memcpy(dst_buf, src_buf, sizeof(src_buf));
        
        /* Move with builtin memmove (overlapping) */
        __builtin_memmove(src_buf + 32, src_buf, 64);
        
        /* Verify copy */
        for (size_t i = 0; i < sizeof(src_buf); i++) {
            dst_buf[i] ^= src_buf[i];
        }
    }
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Phase 1: Basic builtin operations */
    char buffer1[256];
    char buffer2[256];
    
    __builtin_memset(buffer1, 0xCC, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    
    /* Use volatile size to prevent constant folding */
    size_t copy_size = g_mem_size;
    if (copy_size > sizeof(buffer1)) copy_size = sizeof(buffer1);
    
    __builtin_memmove(buffer1 + 128, buffer1, copy_size / 2);
    
    /* Phase 2: Goto control flow */
    goto_mem_operations(buffer1, buffer2, 64);
    
    /* Phase 3: Recursive AST operations */
    ASTNode* root = create_ast(4);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 4: Parallel operations */
    parallel_mem_ops();
    
    /* Phase 5: Complex memory pattern */
    uint32_t final_hash = hash_ast(root);
    
    /* Additional memory operations in loop */
    for (int i = 0; i < 3; i++) {
        char temp[100];
        __builtin_memset(temp, i, sizeof(temp));
        __builtin_memcpy(buffer1 + i * 50, temp, 50);
        
        if (i == 1) {
            __builtin_memmove(temp, temp + 25, 25);
        }
    }
    
    /* Print result to prevent optimization */
    printf("AST hash: 0x%08X\n", final_hash);
    printf("Buffer1[0] = 0x%02X, Buffer2[0] = 0x%02X\n", 
           (unsigned char)buffer1[0], 
           (unsigned char)buffer2[0]);
    
    /* Cleanup */
    /* Note: In real ASAN, memory is automatically tracked */
    
    return 0;
}
