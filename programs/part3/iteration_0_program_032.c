/* ISO C99-compliant test program for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_token_array[4096];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token array with pattern */
    for (int i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)((i * 31) & 0xFF);
    }
    printf("Constructor: Token array initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Clear sensitive data */
    __builtin_memset(g_token_array, 0, sizeof(g_token_array));
    printf("Destructor: Cleanup completed\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = depth;
    
    /* Fill data with pattern using memcpy */
    char pattern[256];
    for (int i = 0; i < 256; i++) {
        pattern[i] = (char)((depth * 17 + i * 13) & 0xFF);
    }
    __builtin_memcpy(node->data, pattern, 256);
    
    /* Recursive creation with goto for flow control */
    if (depth < max_depth - 1) {
        int create_left = 1;
        
        /* Use goto to create complex control flow */
        if (depth % 2 == 0) goto create_right;
        
        create_left:
        node->left = create_ast(depth + 1, max_depth);
        
        create_right:
        node->right = create_ast(depth + 1, max_depth);
        
        /* Jump back if needed */
        if (depth == 0 && node->right) goto copy_data;
    }
    
    copy_data:
    /* Copy between nodes if siblings exist */
    if (depth > 0 && node->left && node->right) {
        /* Use memmove for overlapping regions */
        __builtin_memmove(node->left->data + 128, 
                         node->right->data, 
                         128);
    }
    
    return node;
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    int i;
    size_t local_size = g_mem_size;
    
    #pragma omp parallel private(i) shared(local_size)
    {
        #pragma omp for
        for (i = 0; i < 8; i++) {
            /* Local buffers for each thread */
            char src[256];
            char dst[256];
            
            /* Initialize with builtin memset */
            __builtin_memset(src, i * 32, local_size);
            
            /* Copy with builtin memcpy */
            __builtin_memcpy(dst, src, local_size);
            
            /* Move with builtin memmove (potential overlap) */
            if (i % 2 == 0) {
                __builtin_memmove(dst + 64, dst, 64);
            }
            
            /* Update global token array */
            #pragma omp critical
            {
                size_t offset = (g_token_index * 32) % sizeof(g_token_array);
                __builtin_memcpy(g_token_array + offset, dst, 32);
                g_token_index++;
            }
        }
    }
}

/* Complex memory dispatch with goto */
static void memory_dispatch_with_goto(void) {
    char buffer1[512];
    char buffer2[512];
    volatile int stage = 0;
    
    /* Stage 0: Initialization */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    
    if (g_use_hwasan) goto hwasan_path;
    
    /* Normal ASAN path */
    __builtin_memcpy(buffer2, buffer1, 256);
    stage = 1;
    goto process_data;
    
hwasan_path:
    /* Alternative path */
    __builtin_memset(buffer2, 0xBB, sizeof(buffer2));
    stage = 2;
    
process_data:
    /* Process with memmove */
    if (stage == 1) {
        /* Overlapping copy */
        __builtin_memmove(buffer1 + 128, buffer1, 256);
    } else {
        /* Non-overlapping */
        __builtin_memmove(buffer2, buffer1, 128);
    }
    
    /* Jump to cleanup */
    goto cleanup;
    
    /* Unreachable code (for coverage) */
    __builtin_memset(buffer1, 0xFF, 16);
    
cleanup:
    /* Final operations */
    __builtin_memcpy(g_token_array, buffer1, 64);
    __builtin_memcpy(g_token_array + 64, buffer2, 64);
}

/* Calculate hash of token array */
static unsigned long calculate_hash(void) {
    unsigned long hash = 5381;
    volatile char* ptr = g_token_array;
    
    for (size_t i = 0; i < sizeof(g_token_array); i++) {
        hash = ((hash << 5) + hash) + ptr[i]; /* hash * 33 + c */
    }
    
    return hash;
}

/* Main execution flow */
int main(void) {
    ASTNode* root = NULL;
    unsigned long final_hash = 0;
    
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Create recursive AST */
    printf("Creating AST structure...\n");
    root = create_ast(0, 4);
    
    /* Phase 2: Parallel memory operations */
    printf("Executing parallel memory ops...\n");
    parallel_memory_ops();
    
    /* Phase 3: Dispatch with goto */
    printf("Running memory dispatch...\n");
    memory_dispatch_with_goto();
    
    /* Phase 4: Process AST */
    if (root) {
        /* Copy between tree nodes */
        if (root->left && root->right) {
            __builtin_memcpy(root->left->data, 
                           root->right->data, 
                           128);
            __builtin_memmove(root->right->data + 64,
                            root->left->data,
                            64);
        }
        
        /* Free AST */
        free(root);
    }
    
    /* Final calculation */
    final_hash = calculate_hash();
    printf("Final hash: 0x%016lx\n", final_hash);
    printf("Token index: %d\n", g_token_index);
    
    return 0;
}
