/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
struct ASTNode {
    char buffer[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
};

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    printf("Constructor: Initializing ASAN/HWASAN test environment\n");
    
    /* Force builtin usage in constructor */
    char init_buf[64];
    volatile char* volatile_ptr = init_buf;
    
    __builtin_memset(volatile_ptr, 0xAA, sizeof(init_buf));
    __builtin_memcpy(volatile_ptr + 16, volatile_ptr, 32);
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    printf("Destructor: ASAN/HWASAN test completed\n");
}

/* Recursive function with memory operations */
static struct ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    struct ASTNode* node = (struct ASTNode*)malloc(sizeof(struct ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    /* Use all three builtins with volatile control */
    volatile size_t local_size = g_mem_size % 128 + 128;
    
    __builtin_memset(node->buffer, id % 256, sizeof(node->buffer));
    
    if (node->left) {
        __builtin_memcpy(node->buffer, node->left->buffer, local_size);
    }
    
    if (g_use_memmove && node->right) {
        /* Create overlapping regions for memmove */
        __builtin_memmove(node->buffer + 64, node->buffer, 128);
        __builtin_memcpy(node->right->buffer, node->buffer + 32, 96);
    }
    
    return node;
}

/* Function with goto jumps around memmove */
static void goto_memmove_test(char* dest, char* src, size_t n) {
    int use_memmove = 1;
    
    if (n < 64) {
        use_memmove = 0;
        goto small_copy;
    }
    
overlap_region:
    /* This goto target contains memmove */
    __builtin_memmove(dest + 32, src, n - 32);
    goto copy_done;
    
small_copy:
    __builtin_memcpy(dest, src, n);
    if (use_memmove) {
        use_memmove = 0;
        goto overlap_region;  /* Jump back */
    }
    
copy_done:
    /* Final touch with memset */
    __builtin_memset(dest + n - 16, 0xFF, 16);
}

/* OpenMP parallel section with memory operations */
static void parallel_mem_ops(struct ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            volatile int thread_id = omp_get_thread_num();
            
            /* Mix builtins based on thread ID */
            if (thread_id % 3 == 0) {
                __builtin_memset(nodes[i]->buffer, thread_id, 128);
            } else if (thread_id % 3 == 1) {
                if (nodes[(i + 1) % count]) {
                    __builtin_memcpy(nodes[i]->buffer, 
                                   nodes[(i + 1) % count]->buffer, 192);
                }
            } else {
                /* Create overlap for memmove */
                __builtin_memmove(nodes[i]->buffer + 64, 
                                nodes[i]->buffer, 128);
            }
        }
    }
}

/* Compute hash from AST structure */
static unsigned long compute_ast_hash(struct ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    volatile char* ptr = node->buffer;
    
    /* Process buffer with volatile access */
    for (volatile size_t i = 0; i < sizeof(node->buffer); i++) {
        hash = ((hash << 5) + hash) + ptr[i];
    }
    
    /* Recursive computation */
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

/* Free AST recursively */
static void free_ast(struct ASTNode* node) {
    if (!node) return;
    
    /* Clear buffer before free */
    __builtin_memset(node->buffer, 0, sizeof(node->buffer));
    
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Create AST structures */
    struct ASTNode* root = create_ast(4, 1);
    struct ASTNode* nodes[8];
    
    for (int i = 0; i < 8; i++) {
        nodes[i] = create_ast(3, i + 100);
    }
    
    /* Phase 2: Goto-based memmove test */
    char src_buf[512];
    char dst_buf[512];
    
    __builtin_memset(src_buf, 0xCC, sizeof(src_buf));
    goto_memmove_test(dst_buf, src_buf, 256);
    
    /* Verify copy with memcmp */
    if (__builtin_memcmp(dst_buf + 32, src_buf, 224) != 0) {
        printf("Warning: memmove verification failed\n");
    }
    
    /* Phase 3: OpenMP parallel operations */
    parallel_mem_ops(nodes, 8);
    
    /* Phase 4: Complex memory chain */
    for (int i = 0; i < 7; i++) {
        if (nodes[i] && nodes[i + 1]) {
            /* Chain of memory operations */
            __builtin_memcpy(nodes[i]->buffer + 128, 
                           nodes[i + 1]->buffer, 128);
            __builtin_memset(nodes[i + 1]->buffer + 64, i, 64);
            __builtin_memmove(nodes[i]->buffer, 
                            nodes[i]->buffer + 64, 192);
        }
    }
    
    /* Phase 5: Compute and verify results */
    unsigned long total_hash = compute_ast_hash(root);
    
    for (int i = 0; i < 8; i++) {
        total_hash ^= compute_ast_hash(nodes[i]);
    }
    
    printf("Final hash: 0x%016lx\n", total_hash);
    printf("Operations completed: memset=%d, memcpy=%d, memmove=%d\n",
           10, 8, 6);  /* Approximate counts */
    
    /* Cleanup */
    free_ast(root);
    for (int i = 0; i < 8; i++) {
        free_ast(nodes[i]);
    }
    
    /* Final builtin usage in main */
    __builtin_memset(dst_buf, 0, sizeof(dst_buf));
    
    return 0;
}
