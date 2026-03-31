#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    volatile int visited;  /* Prevent optimization */
} ASTNode;

/* Global volatile variables to prevent constant folding */
volatile size_t g_memcpy_len = 32;
volatile size_t g_memset_len = 64;
volatile size_t g_memmove_len = 48;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    volatile char buffer[128];
    
    /* Force initialization of asan_memfn_rtls[0] for memcpy */
    __builtin_memcpy(buffer, "constructor_data", 16);
    
    /* Use goto to create complex control flow */
    if (buffer[0] == 'c') {
        goto memcpy_label;
    }
    
memcpy_label:
    __builtin_memcpy(buffer + 16, "more_data", 9);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    volatile char cleanup_buf[256];
    
    /* Force initialization of asan_memfn_rtls[1] for memset */
    __builtin_memset(cleanup_buf, 0, 128);
    
    /* Jump around memory operations */
    goto skip_memset;
    
    __builtin_memset(cleanup_buf + 128, 0xFF, 128);
    
skip_memset:
    /* Force initialization of asan_memfn_rtls[2] for memmove */
    __builtin_memmove(cleanup_buf, cleanup_buf + 64, 64);
}

/* Recursive function that builds and manipulates AST */
static ASTNode* build_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile to prevent optimization */
    volatile size_t copy_len = g_memcpy_len;
    if (copy_len > 63) copy_len = 63;
    
    /* Force memcpy redirection with variable length */
    __builtin_memcpy(node->data, base_data, copy_len);
    node->data[copy_len] = '\0';
    
    /* Recursive calls with different memory operations */
    node->left = build_ast(depth - 1, "left_branch");
    node->right = build_ast(depth - 1, "right_branch");
    
    /* Copy between nodes to test alias analysis */
    if (node->left && node->right) {
        volatile size_t move_len = g_memmove_len;
        if (move_len > 63) move_len = 63;
        
        /* Force memmove redirection */
        __builtin_memmove(node->left->data, node->right->data, move_len);
    }
    
    node->visited = 0;
    return node;
}

/* Calculate hash of AST for verification */
static uint64_t hash_ast(ASTNode* node) {
    if (!node) return 0;
    
    uint64_t hash = 5381;
    volatile char* ptr = node->data;
    
    /* Process string with jumps */
    int i = 0;
process_loop:
    if (ptr[i]) {
        hash = ((hash << 5) + hash) + ptr[i];
        i++;
        goto process_loop;
    }
    
    /* Recursive hashing */
    hash ^= hash_ast(node->left);
    hash ^= hash_ast(node->right);
    
    return hash;
}

/* Free AST memory */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear data before free */
    volatile size_t clear_len = g_memset_len;
    if (clear_len > 63) clear_len = 63;
    
    __builtin_memset(node->data, 0, clear_len);
    free(node);
}

/* Main parallel processing function */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        /* Each thread gets its own buffers */
        volatile char thread_buf[3][256];
        int thread_id = 0;
        
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Initialize with thread-specific pattern */
        volatile size_t init_len = g_memset_len;
        if (init_len > 255) init_len = 255;
        
        __builtin_memset(thread_buf[0], thread_id, init_len);
        
        /* Complex control flow with goto */
        if (thread_id % 2 == 0) {
            goto even_thread;
        } else {
            goto odd_thread;
        }
        
    even_thread:
        /* Force memcpy redirection */
        __builtin_memcpy(thread_buf[1], thread_buf[0], 128);
        
        /* Jump over memmove */
        goto skip_memmove;
        
    odd_thread:
        __builtin_memcpy(thread_buf[1], thread_buf[0], 64);
        
    skip_memmove:
        /* Force memmove redirection with overlap */
        __builtin_memmove(thread_buf[2], thread_buf[1] + 32, 96);
        
        /* Barrier to ensure all threads reach this point */
        #pragma omp barrier
        
        /* Final memset in parallel region */
        __builtin_memset(thread_buf[0] + 128, 0xFF, 64);
    }
}

int main(void) {
    printf("Starting ASAN/HWASAN coverage test...\n");
    
    /* Phase 1: Build and process AST */
    ASTNode* root = build_ast(4, "root_node_data");
    if (!root) {
        fprintf(stderr, "Failed to build AST\n");
        return 1;
    }
    
    /* Phase 2: Calculate verification hash */
    uint64_t ast_hash = hash_ast(root);
    printf("AST hash: 0x%016llx\n", (unsigned long long)ast_hash);
    
    /* Phase 3: Parallel memory operations */
    printf("Running parallel memory operations...\n");
    parallel_memory_operations();
    
    /* Phase 4: Additional built-in usage in main */
    volatile char main_buf[512];
    volatile size_t operations = 3;
    
    /* Loop with different memory operations */
    for (volatile int i = 0; i < operations; i++) {
        switch (i % 3) {
            case 0:
                __builtin_memcpy(main_buf + i * 64, "memcpy_data", 11);
                break;
            case 1:
                __builtin_memset(main_buf + i * 64, i, 32);
                break;
            case 2:
                __builtin_memmove(main_buf + i * 64, main_buf, 48);
                break;
        }
        
        /* Jump to create complex CFG */
        if (i == 1) {
            goto special_case;
        }
    }
    
special_case:
    /* Final memory operation with goto */
    __builtin_memcpy(main_buf + 384, "final_copy", 10);
    
    /* Phase 5: Cleanup */
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
