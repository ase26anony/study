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
    printf("Destructor: Cleaned up resources\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy for node initialization */
    __builtin_memcpy(node->data, base_data, 32);
    node->id = depth;
    
    /* Create children with goto for flow control */
    if (depth > 1) {
        char child_data[32];
        __builtin_memset(child_data, 'B' + depth, 32);
        
        /* Jump label for goto */
        create_left:
        node->left = create_ast(depth - 1, child_data);
        
        /* Another goto jump */
        if (depth > 2) goto create_right;
        
        /* Use builtin memmove with goto */
        char temp[32];
        __builtin_memmove(temp, node->data, 32);
        __builtin_memcpy(node->data, temp, 32);
        
        create_right:
        node->right = create_ast(depth - 2, child_data);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void complex_memory_ops(void) {
    char buffer1[128];
    char buffer2[128];
    char buffer3[128];
    
    /* Initialize with volatile-controlled length */
    int len = volatile_len;
    
    /* First memset */
    __builtin_memset(buffer1, 0xCC, len);
    
    /* Jump into memory operation block */
    goto mem_block_1;
    
    skip_memset:
    /* Copy between buffers */
    __builtin_memcpy(buffer2, buffer1, len);
    
    /* Another goto jump */
    goto mem_block_2;
    
    mem_block_1:
    /* This memset should be reached via goto */
    __builtin_memset(buffer1 + 32, 0xDD, len - 32);
    goto skip_memset;
    
    mem_block_2:
    /* Memmove with overlap */
    __builtin_memmove(buffer3, buffer2, len);
    
    /* Copy back with overlap */
    __builtin_memmove(buffer2 + 16, buffer2, len - 16);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    int i;
    char parallel_buf[4][256];
    
    #pragma omp parallel for private(i)
    for (i = 0; i < 4; i++) {
        /* Each thread uses builtins */
        __builtin_memset(parallel_buf[i], 'A' + i, 256);
        
        /* Copy between thread buffers */
        if (i > 0) {
            __builtin_memcpy(parallel_buf[i], parallel_buf[i-1], 128);
        }
        
        /* Memmove within thread buffer */
        __builtin_memmove(parallel_buf[i] + 64, parallel_buf[i], 128);
    }
}

/* Calculate hash from AST */
static unsigned long ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    int i;
    
    /* Hash node data */
    for (i = 0; i < 32 && node->data[i]; i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    /* Recursive hash calculation */
    hash += ast_hash(node->left);
    hash += ast_hash(node->right);
    
    return hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, 32);
    free(node);
}

int main(void) {
    ASTNode* root = NULL;
    unsigned long final_hash = 0;
    
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Stage 1: Initialize with volatile operations */
    {
        volatile char* dest = (volatile char*)volatile_dest;
        volatile char* src = (volatile char*)volatile_src;
        
        /* Force builtin calls with volatile */
        __builtin_memset((void*)dest, 0x11, volatile_len);
        __builtin_memcpy((void*)src, (void*)dest, volatile_len);
        __builtin_memmove((void*)(dest + 32), (void*)dest, volatile_len - 32);
    }
    
    /* Stage 2: Create recursive AST */
    printf("Creating recursive AST structure...\n");
    root = create_ast(5, "BaseASTNodeDataForTesting");
    
    /* Stage 3: Complex memory operations with goto */
    printf("Performing complex memory operations...\n");
    complex_memory_ops();
    
    /* Stage 4: OpenMP parallel operations */
    printf("Running OpenMP parallel memory operations...\n");
    #ifdef _OPENMP
    parallel_memory_ops();
    #endif
    
    /* Stage 5: Process tokens and AST */
    printf("Processing token array...\n");
    for (token_index = 0; token_index < 256; token_index += 32) {
        /* Copy tokens with builtins */
        __builtin_memcpy(token_array + token_index, 
                        "TokenDataForASANTesting", 32);
        
        /* Move tokens around */
        __builtin_memmove(token_array + token_index + 16,
                         token_array + token_index, 16);
    }
    
    /* Calculate verification hash */
    final_hash = ast_hash(root);
    
    /* Additional builtin calls for coverage */
    {
        char final_buffer[512];
        __builtin_memset(final_buffer, 0, sizeof(final_buffer));
        __builtin_memcpy(final_buffer, token_array, 256);
        __builtin_memmove(final_buffer + 256, final_buffer, 256);
        
        /* Mix in AST hash */
        __builtin_memcpy(final_buffer + 384, &final_hash, sizeof(final_hash));
    }
    
    printf("Final hash: 0x%08lx\n", final_hash);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free_ast(root);
    
    return 0;
}
