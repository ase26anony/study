/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 256;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 192;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force early builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    volatile char final_check[16];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    node->value = (*counter)++;
    
    /* Fill data with pattern */
    __builtin_memset(node->data, 'A' + depth, sizeof(node->data) - 1);
    node->data[sizeof(node->data) - 1] = '\0';
    
    /* Recursive creation */
    node->left = create_ast(depth - 1, counter);
    node->right = create_ast(depth - 1, counter);
    
    /* Copy data between nodes if both children exist */
    if (node->left && node->right) {
        __builtin_memcpy(node->left->data, node->right->data, 
                        sizeof(node->data) / 2);
    }
    
    return node;
}

/* Function with goto control flow */
static void goto_memmove_test(char* dest, char* src, size_t len) {
    int use_memmove = 1;
    
    if (len > 100) {
        goto do_operation;
    }
    
    /* Jump over this block */
    if (use_memmove) {
        goto skip_direct;
    }
    
    __builtin_memcpy(dest, src, len);
    return;
    
skip_direct:
    /* Fall through */
    
do_operation:
    /* This label is jumped into */
    if (dest > src && dest < src + len) {
        /* Overlapping regions require memmove */
        __builtin_memmove(dest, src, len);
    } else {
        __builtin_memcpy(dest, src, len);
    }
    
    /* Jump out of block */
    goto finish;
    
    /* Unreachable code that might confuse flow analysis */
    __builtin_memset(dest, 0, len);
    
finish:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    const int num_threads = 4;
    char* buffers[num_threads];
    size_t sizes[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        sizes[tid] = (tid + 1) * 64;
        buffers[tid] = (char*)malloc(sizes[tid]);
        
        if (buffers[tid]) {
            /* Each thread uses different builtins */
            switch (tid % 3) {
                case 0:
                    __builtin_memset(buffers[tid], tid, sizes[tid]);
                    break;
                case 1:
                    if (tid > 0) {
                        __builtin_memcpy(buffers[tid], buffers[tid-1], 
                                        sizes[tid] < sizes[tid-1] ? 
                                        sizes[tid] : sizes[tid-1]);
                    }
                    break;
                case 2:
                    __builtin_memmove(buffers[tid] + 10, buffers[tid], 
                                     sizes[tid] - 10);
                    break;
            }
        }
        
        #pragma omp barrier
        
        /* Cross-thread memory operation */
        if (tid == 0) {
            for (int i = 1; i < num_threads; i++) {
                if (buffers[i]) {
                    size_t copy_len = sizes[0] < sizes[i] ? sizes[0] : sizes[i];
                    __builtin_memcpy(buffers[0], buffers[i], copy_len);
                }
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_threads; i++) {
        free(buffers[i]);
    }
}

/* Multi-stage initialization */
static void complex_initialization(void) {
    volatile char stage1[256];
    volatile char stage2[256];
    volatile char stage3[256];
    
    /* Stage 1: memset pattern */
    __builtin_memset(stage1, 0x11, sizeof(stage1));
    
    /* Stage 2: memcpy with volatile lengths */
    size_t len1 = g_memcpy_len % sizeof(stage1);
    __builtin_memcpy(stage2, stage1, len1);
    
    /* Stage 3: memmove with overlap */
    size_t len2 = g_memmove_len % (sizeof(stage3) / 2);
    __builtin_memcpy(stage3, stage2, sizeof(stage3));
    __builtin_memmove(stage3 + 128, stage3, len2);
    
    /* Final memset with computed length */
    size_t len3 = g_memset_len % sizeof(stage3);
    __builtin_memset(stage3, 0x33, len3);
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Initialize counters */
    int counter = 0;
    
    /* Create recursive structure */
    ASTNode* root = create_ast(4, &counter);
    printf("Created AST with %d nodes\n", counter);
    
    /* Test goto control flow */
    char src[256], dest[256];
    for (int i = 0; i < sizeof(src); i++) {
        src[i] = (char)(i % 256);
    }
    
    goto_memmove_test(dest, src, sizeof(dest));
    
    /* Verify the copy */
    int valid = 1;
    for (size_t i = 0; i < sizeof(dest); i++) {
        if (dest[i] != src[i]) {
            valid = 0;
            break;
        }
    }
    printf("Goto memmove test: %s\n", valid ? "PASS" : "FAIL");
    
    /* Complex initialization */
    complex_initialization();
    
    /* Parallel operations */
    parallel_memory_ops();
    
    /* Compute hash from AST */
    unsigned long hash = 0;
    ASTNode* stack[100];
    int top = 0;
    
    if (root) stack[top++] = root;
    
    while (top > 0) {
        ASTNode* node = stack[--top];
        hash = hash * 31 + node->value;
        
        /* Process node data with builtins */
        char temp[64];
        __builtin_memcpy(temp, node->data, sizeof(temp));
        for (int i = 0; i < 64; i++) {
            hash = hash * 17 + temp[i];
        }
        
        if (node->right) stack[top++] = node->right;
        if (node->left) stack[top++] = node->left;
        
        /* Cleanup node */
        free(node);
    }
    
    printf("Final hash: %lu\n", hash);
    printf("Test completed\n");
    
    return 0;
}
