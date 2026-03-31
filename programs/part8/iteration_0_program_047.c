/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_selector = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    printf("Constructor: Initializing ASAN/HWASAN environment\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    printf("Destructor: ASAN/HWASAN cleanup complete\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Fill data with pattern using builtin memcpy */
    char pattern[256];
    for (int i = 0; i < 256; i++) {
        pattern[i] = (char)(i % 256);
    }
    __builtin_memcpy(node->data, pattern, 256);
    
    node->size = 256;
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    return node;
}

/* Function with goto jumps around memmove */
static void goto_memmove_test(char* dest, char* src, size_t n) {
    int use_memmove = 0;
    
    /* Jump into block containing builtin memmove */
    if (volatile_selector > 0) {
        goto memmove_block;
    }
    
normal_path:
    __builtin_memcpy(dest, src, n);
    return;
    
memmove_block:
    /* Force overlapping memory regions */
    char* mid = src + n/2;
    __builtin_memmove(mid, src, n/2);
    
    if (volatile_selector < 0) {
        goto normal_path;
    }
}

/* OpenMP parallel memory operations */
static void parallel_mem_ops(void) {
    const int num_threads = 4;
    char* buffers[num_threads];
    size_t sizes[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        sizes[tid] = (tid + 1) * volatile_len;
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
                                       sizes[tid] < sizes[tid-1] ? sizes[tid] : sizes[tid-1]);
                    }
                    break;
                case 2:
                    __builtin_memmove(buffers[tid] + sizes[tid]/2, 
                                    buffers[tid], sizes[tid]/2);
                    break;
            }
        }
        
        #pragma omp barrier
        
        /* Verify operations */
        if (buffers[tid]) {
            unsigned long hash = 0;
            for (size_t i = 0; i < sizes[tid] && i < 100; i++) {
                hash = hash * 31 + buffers[tid][i];
            }
            #pragma omp critical
            printf("Thread %d hash: %lu\n", tid, hash);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_threads; i++) {
        if (buffers[i]) free(buffers[i]);
    }
}

/* Multi-stage initialization with memory builtins */
static void init_stage(char* buffer, size_t size) {
    /* Stage 1: Clear with memset */
    __builtin_memset(buffer, 0, size);
    
    /* Stage 2: Fill pattern with memcpy */
    char pattern[128];
    for (int i = 0; i < 128; i++) pattern[i] = (char)(i ^ 0x55);
    __builtin_memcpy(buffer, pattern, size < 128 ? size : 128);
    
    /* Stage 3: Shift data with memmove */
    if (size > 64) {
        __builtin_memmove(buffer + 32, buffer, size - 32);
    }
}

int main(void) {
    printf("=== ASAN/HWASAN Built-in Redirection Test ===\n");
    
    /* Test 1: Basic builtin usage with volatile control */
    char src[256], dest[256];
    for (int i = 0; i < 256; i++) src[i] = (char)i;
    
    __builtin_memcpy(dest, src, volatile_len);
    __builtin_memset(dest + 128, 0xAA, volatile_len / 2);
    __builtin_memmove(dest + 64, dest, 128);
    
    /* Test 2: Goto flow control */
    goto_memmove_test(dest + 100, src + 50, 75);
    
    /* Test 3: Recursive AST operations */
    ASTNode* root = create_ast(3);
    if (root && root->left && root->right) {
        /* Copy between AST nodes */
        __builtin_memcpy(root->left->data, root->right->data, 
                        root->left->size < root->right->size ? 
                        root->left->size : root->right->size);
        
        /* Move within node */
        __builtin_memmove(root->data + 50, root->data, 150);
    }
    
    /* Test 4: OpenMP parallel section */
    parallel_mem_ops();
    
    /* Test 5: Multi-stage buffer */
    char stage_buffer[512];
    init_stage(stage_buffer, sizeof(stage_buffer));
    
    /* Verification hash */
    unsigned long final_hash = 0;
    for (int i = 0; i < 256; i++) {
        final_hash = final_hash * 31 + dest[i];
    }
    if (root) {
        for (int i = 0; i < 100; i++) {
            final_hash = final_hash * 31 + root->data[i];
        }
    }
    for (int i = 0; i < 100; i++) {
        final_hash = final_hash * 31 + stage_buffer[i];
    }
    
    printf("Final verification hash: %lu\n", final_hash);
    printf("=== Test Complete ===\n");
    
    /* Cleanup */
    /* Note: AST cleanup omitted for brevity - would need recursive free */
    
    return (final_hash != 0) ? 0 : 1;
}
