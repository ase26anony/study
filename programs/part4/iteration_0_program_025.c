/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
struct ASTNode {
    char data[128];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
};

/* Global token array */
static char g_tokens[][16] = {
    "token1", "token2", "token3", "token4",
    "token5", "token6", "token7", "token8"
};

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_early(void) {
    volatile char buffer[64];
    /* Force __builtin_memset initialization */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    
    /* Copy between global tokens */
    __builtin_memcpy(g_tokens[7], g_tokens[0], 8);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_late(void) {
    volatile char final_buf[32];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static struct ASTNode* build_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    struct ASTNode* node = malloc(sizeof(*node));
    if (!node) return NULL;
    
    /* Initialize with __builtin_memset */
    __builtin_memset(node, 0, sizeof(*node));
    node->id = id;
    
    /* Fill data with pattern using memcpy */
    char pattern[32];
    __builtin_memset(pattern, 'A' + (id % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(pattern));
    
    /* Build children */
    node->left = build_ast(depth - 1, id * 2);
    node->right = build_ast(depth - 1, id * 2 + 1);
    
    /* Copy data between nodes if children exist */
    if (node->left && node->right) {
        __builtin_memcpy(node->right->data, node->left->data, 64);
    }
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(struct ASTNode* node) {
    if (!node) return;
    
    volatile char temp[128];
    int use_memmove = g_use_memmove;
    
    /* Jump into memory operation block */
    goto entry_point;
    
memmove_block:
    /* This tests flow sensitivity */
    __builtin_memmove(temp, node->data, 64);
    goto after_block;
    
entry_point:
    /* Regular memcpy first */
    __builtin_memcpy(temp, node->data, 64);
    
    if (use_memmove) {
        goto memmove_block;
    }
    
after_block:
    /* Process temp buffer */
    __builtin_memset(temp + 32, 0xCC, 32);
}

/* OpenMP parallel section */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        volatile char local_buf[256];
        size_t size = g_mem_size;
        
        /* Each thread uses different builtins */
        switch (tid % 3) {
            case 0:
                __builtin_memset(local_buf, tid, size % 256);
                break;
            case 1:
                __builtin_memcpy(local_buf, g_tokens[tid % 8], 16);
                break;
            case 2:
                __builtin_memmove(local_buf + 32, local_buf, 64);
                break;
        }
        
        /* Barrier to ensure all threads reach this */
        #pragma omp barrier
        
        /* Collective operation */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            volatile char iter_buf[64];
            __builtin_memset(iter_buf, i, sizeof(iter_buf));
        }
    }
}

/* Calculate hash from AST */
static unsigned long compute_ast_hash(struct ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* p = node->data;
    
    /* Process data */
    for (int i = 0; i < 64 && *p; i++) {
        hash = ((hash << 5) + hash) + *p++;
    }
    
    /* Recursive hash combination */
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

/* Free AST memory */
static void free_ast(struct ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Build recursive structure */
    struct ASTNode* root = build_ast(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to build AST\n");
        return 1;
    }
    
    /* Process with goto edge cases */
    process_with_goto(root);
    
    /* Toggle volatile to test different paths */
    g_use_memmove = 0;
    process_with_goto(root->left);
    g_use_memmove = 1;
    
    /* Execute OpenMP parallel section */
    parallel_memory_ops();
    
    /* Additional memory operations in main */
    volatile char main_buf[512];
    size_t size = g_mem_size;
    
    /* Chain of builtin calls */
    __builtin_memset(main_buf, 0x11, size);
    __builtin_memcpy(main_buf + 128, g_tokens[3], 32);
    __builtin_memmove(main_buf + 256, main_buf + 128, 64);
    __builtin_memset(main_buf + 384, 0x22, 64);
    
    /* Compute and print verification result */
    unsigned long hash = compute_ast_hash(root);
    printf("AST hash: %lu\n", hash);
    
    /* Final memory operation */
    __builtin_memcpy(main_buf, &hash, sizeof(hash));
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
