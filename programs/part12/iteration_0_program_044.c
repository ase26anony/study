/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 96;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    volatile char final_check[64];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use all three builtins in varied contexts */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    
    /* Fill data with pattern using memcpy */
    char pattern[32];
    __builtin_memset(pattern, 'A' + depth, sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(pattern));
    
    /* Create children recursively */
    node->left = create_ast(depth - 1);
    node->left = create_ast(depth - 1);
    
    /* Copy between nodes if both exist */
    if (node->left && node->right) {
        __builtin_memcpy(node->right->data, node->left->data, 
                        sizeof(node->left->data));
    }
    
    return node;
}

/* Function with goto jumps around memmove */
static void goto_memmove_test(char* dest, char* src, size_t len) {
    int use_memmove = 1;
    
    if (len > 100) {
        goto skip_memmove;
    }
    
    /* Jump target with memmove */
memmove_block:
    __builtin_memmove(dest, src, len);
    goto after_memmove;
    
skip_memmove:
    if (use_memmove) {
        use_memmove = 0;
        goto memmove_block;
    }
    
after_memmove:
    /* Additional memcpy after goto */
    __builtin_memcpy(dest + len/2, src, len/2);
}

/* Main test function with OpenMP parallelization */
static void parallel_memory_ops(void) {
    const size_t buffer_size = 1024;
    char* buffers[8];
    
    #pragma omp parallel for
    for (int i = 0; i < 8; i++) {
        buffers[i] = (char*)malloc(buffer_size);
        if (buffers[i]) {
            /* Use volatile lengths to prevent folding */
            size_t len = (i % 3 == 0) ? g_memcpy_len :
                        (i % 3 == 1) ? g_memset_len : g_memmove_len;
            
            /* Mix different builtins based on thread ID */
            if (i % 3 == 0) {
                __builtin_memset(buffers[i], i, len);
            } else if (i % 3 == 1) {
                char pattern[128];
                __builtin_memset(pattern, 'X', sizeof(pattern));
                __builtin_memcpy(buffers[i], pattern, len);
            } else {
                /* Create overlapping regions for memmove */
                __builtin_memset(buffers[i], 0, buffer_size);
                __builtin_memset(buffers[i] + 256, 0xFF, 512);
                __builtin_memmove(buffers[i] + 128, buffers[i] + 256, len);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        if (buffers[i]) free(buffers[i]);
    }
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = create_ast(4);
    if (root) {
        /* Copy entire AST structure */
        ASTNode root_copy;
        __builtin_memcpy(&root_copy, root, sizeof(ASTNode));
        
        /* Recursive cleanup would go here */
        free(root);
    }
    
    /* Phase 2: Goto-based memmove test */
    char src[256], dest[256];
    __builtin_memset(src, 0xCC, sizeof(src));
    goto_memmove_test(dest, src, g_memmove_len);
    
    /* Phase 3: OpenMP parallel operations */
    parallel_memory_ops();
    
    /* Phase 4: Direct builtin calls with volatile control */
    volatile char final_buffer[512];
    volatile size_t final_len = 256;
    
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer + 128, final_buffer, final_len);
    __builtin_memmove(final_buffer + 64, final_buffer + 128, final_len/2);
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        hash = hash * 31 + final_buffer[i];
    }
    
    printf("Test completed. Hash: %lu\n", hash);
    printf("Compile with: -fsanitize=address or -fsanitize=kernel-hwaddress\n");
    
    return 0;
}
