/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_array[1024];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(token_array, 0xAA, sizeof(token_array));
    printf("Constructor: Initialized token array\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use builtin memcpy in destructor */
    char temp[64];
    __builtin_memcpy(temp, token_array, 64);
    printf("Destructor: Cleaned up %d bytes\n", 64);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset for node initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with pattern using builtin memcpy */
    char pattern[32];
    __builtin_memset(pattern, id % 256, sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(node->data));
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        node->right = create_ast(depth - 1, id * 2 + 1);
        
        if (use_goto) {
            create_children:
            /* This goto tests flow sensitivity */
            node->left = create_ast(depth - 1, id * 2);
            node->right = create_ast(depth - 1, id * 2 + 1);
        }
    }
    
    return node;
}

/* Copy between AST nodes with builtin memmove */
static void copy_ast_data(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use builtin memmove for overlapping/non-overlapping cases */
    __builtin_memmove(dest->data, src->data, sizeof(dest->data));
    
    /* Recursive copy */
    if (src->left && dest->left) {
        copy_ast_data(dest->left, src->left);
    }
    if (src->right && dest->right) {
        copy_ast_data(dest->right, src->right);
    }
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_ops(void) {
    int i;
    char buffer1[128];
    char buffer2[128];
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 0xCC, sizeof(buffer1));
    __builtin_memset(buffer2, 0xDD, sizeof(buffer2));
    
    #pragma omp parallel for private(i)
    for (i = 0; i < 8; i++) {
        char local_buf[64];
        int offset = i * 8;
        
        /* Test all three builtins in parallel region */
        __builtin_memset(local_buf, i, sizeof(local_buf));
        __builtin_memcpy(buffer1 + offset, local_buf, 8);
        __builtin_memmove(buffer2 + offset, buffer1 + offset, 8);
        
        /* Use volatile variables */
        if (volatile_len > offset) {
            __builtin_memcpy((void*)volatile_dest, buffer2, volatile_len % 64);
        }
    }
}

/* Complex memory dispatch with goto patterns */
static void memory_dispatch_logic(void) {
    char workspace[256];
    char* ptr1 = workspace;
    char* ptr2 = workspace + 128;
    
    /* Initialize with builtin memset */
    __builtin_memset(workspace, 0, sizeof(workspace));
    
    /* Goto into block with memmove */
    int choice = token_array[0] % 3;
    
    if (choice == 0) {
        goto use_memcpy;
    } else if (choice == 1) {
        goto use_memset;
    } else {
        goto use_memmove;
    }
    
use_memcpy:
    __builtin_memcpy(ptr1, token_array, 64);
    goto after_ops;
    
use_memset:
    __builtin_memset(ptr2, 0xFF, 64);
    goto after_ops;
    
use_memmove:
    /* Overlapping memmove */
    __builtin_memmove(ptr1 + 32, ptr1, 96);
    /* Non-overlapping memmove */
    __builtin_memmove(ptr2, ptr1, 64);
    
after_ops:
    /* Final builtin operation */
    __builtin_memcpy((void*)volatile_dest, workspace, 128);
}

/* Calculate hash of AST */
static unsigned long ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    int i;
    
    /* Hash node data */
    for (i = 0; i < sizeof(node->data); i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    /* Recursive hash */
    hash += ast_hash(node->left);
    hash += ast_hash(node->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Initialize and populate token array */
    for (int i = 0; i < sizeof(token_array); i++) {
        token_array[i] = (char)(i % 256);
    }
    
    /* Use builtins on token array */
    __builtin_memset(token_array + 512, 0x55, 256);
    __builtin_memcpy(token_array + 768, token_array, 256);
    __builtin_memmove(token_array + 256, token_array + 512, 128);
    
    /* Phase 2: Create recursive AST structures */
    ASTNode* ast1 = create_ast(4, 1);
    ASTNode* ast2 = create_ast(3, 100);
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 3: Copy between AST nodes */
    copy_ast_data(ast2, ast1);
    
    /* Phase 4: Parallel memory operations */
    parallel_memory_ops();
    
    /* Phase 5: Memory dispatch with goto */
    memory_dispatch_logic();
    
    /* Phase 6: Additional builtin calls with volatile control */
    int dynamic_len = volatile_len;
    if (dynamic_len > 0) {
        __builtin_memset((void*)volatile_src, 0xAA, dynamic_len % 128);
        __builtin_memcpy((void*)volatile_dest, volatile_src, dynamic_len % 128);
        __builtin_memmove((void*)(volatile_dest + 64), volatile_dest, 64);
    }
    
    /* Calculate and print verification result */
    unsigned long hash1 = ast_hash(ast1);
    unsigned long hash2 = ast_hash(ast2);
    unsigned long final_hash = hash1 ^ hash2;
    
    /* Incorporate volatile data */
    for (int i = 0; i < 64; i++) {
        final_hash ^= (unsigned long)volatile_dest[i] << (i % 56);
    }
    
    printf("Verification hash: 0x%016lx\n", final_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real code, would need to free AST nodes recursively */
    
    return 0;
}
