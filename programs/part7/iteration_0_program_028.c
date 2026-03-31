/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char *data;
    size_t len;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_asan_early(void) {
    volatile char buffer[64];
    /* Force __builtin_memset redirection early */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    printf("[constructor] Early memset performed\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_asan_late(void) {
    volatile char buffer[32];
    __builtin_memset(buffer, 0xFF, sizeof(buffer));
    printf("[destructor] Late cleanup memset\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char *base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->len = g_mem_size % 128 + 64;
    node->data = malloc(node->len);
    
    /* __builtin_memset with volatile size */
    volatile size_t fill_size = node->len;
    __builtin_memset(node->data, depth, fill_size);
    
    /* Copy base pattern using __builtin_memcpy */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > node->len) copy_len = node->len;
    __builtin_memcpy(node->data, base_data, copy_len);
    
    node->left = create_ast(depth - 1, base_data);
    node->right = create_ast(depth - 1, base_data);
    
    return node;
}

/* Function with goto jumps around __builtin_memmove */
static void memmove_with_goto(char *dest, char *src, size_t n) {
    int use_memmove = g_use_memmove;
    
    if (use_memmove) goto do_memmove;
    
    /* This block should be skipped by goto */
    __builtin_memcpy(dest, src, n);
    return;
    
do_memmove:
    /* Target of goto - tests flow sensitivity */
    __builtin_memmove(dest, src, n);
    
    /* Jump back out */
    if (n > 100) goto finish;
    
    /* Another memmove in different basic block */
    __builtin_memmove(dest + 10, src + 10, n - 20);
    
finish:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_mem_operations(void) {
    const int num_threads = 4;
    char *buffers[num_threads];
    size_t sizes[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        sizes[tid] = (tid + 1) * 64;
        buffers[tid] = malloc(sizes[tid]);
        
        /* Each thread uses different builtins */
        if (tid % 3 == 0) {
            __builtin_memset(buffers[tid], tid, sizes[tid]);
        } else if (tid % 3 == 1) {
            if (tid > 0) {
                __builtin_memcpy(buffers[tid], buffers[tid-1], 
                               sizes[tid] < sizes[tid-1] ? sizes[tid] : sizes[tid-1]);
            }
        } else {
            char temp[256];
            __builtin_memset(temp, 0xCC, sizeof(temp));
            __builtin_memmove(buffers[tid], temp, 
                            sizes[tid] < sizeof(temp) ? sizes[tid] : sizeof(temp));
        }
        
        #pragma omp barrier
        
        /* Cross-thread memory operation */
        if (tid == 0) {
            for (int i = 1; i < num_threads; i++) {
                __builtin_memmove(buffers[i], buffers[0], 
                                sizes[i] < sizes[0] ? sizes[i] : sizes[0]);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_threads; i++) {
        free(buffers[i]);
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Basic built-in calls */
    char buffer1[512];
    char buffer2[512];
    volatile size_t op_size = g_mem_size;
    
    __builtin_memset(buffer1, 0x11, op_size);
    __builtin_memcpy(buffer2, buffer1, op_size);
    
    /* Force memmove redirection */
    if (g_use_memmove) {
        __builtin_memmove(buffer1 + 100, buffer1, 200);
    }
    
    /* Phase 2: Recursive AST operations */
    ASTNode *root = create_ast(3, "AST_Base_Data");
    if (root && root->left && root->right) {
        /* Copy between AST nodes */
        size_t copy_len = root->left->len < root->right->len ? 
                         root->left->len : root->right->len;
        __builtin_memcpy(root->right->data, root->left->data, copy_len);
        
        /* Move within same node */
        __builtin_memmove(root->data + 10, root->data, root->len - 10);
    }
    
    /* Phase 3: Goto-controlled memmove */
    char src[300], dest[300];
    for (size_t i = 0; i < sizeof(src); i++) {
        src[i] = (char)(i % 256);
    }
    
    memmove_with_goto(dest, src, sizeof(src));
    
    /* Phase 4: OpenMP parallel operations */
    parallel_mem_operations();
    
    /* Phase 5: Mixed operations in loops */
    for (int i = 0; i < 10; i++) {
        char temp[100];
        volatile int use_memset = i % 2;
        
        if (use_memset) {
            __builtin_memset(temp, i, sizeof(temp));
        } else {
            __builtin_memcpy(temp, buffer1 + i * 10, sizeof(temp));
        }
        
        if (i == 5) {
            __builtin_memmove(temp + 20, temp, 50);
        }
    }
    
    /* Verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < op_size && i < sizeof(buffer1); i++) {
        hash = hash * 31 + buffer1[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup AST */
    /* ... cleanup code would go here ... */
    
    return 0;
}
