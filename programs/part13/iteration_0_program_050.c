/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
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

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 128;  /* Force non-constant size */
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_asan_test(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Function with goto jumps around memmove */
static void test_goto_memmove(char* dest, char* src, size_t n) {
    int use_copy = 1;
    
    if (n == 0) goto skip_copy;
    
    /* Jump into memory operation block */
    goto do_copy;
    
copy_block:
    /* This forces flow-sensitive analysis */
    __builtin_memmove(dest, src, n);
    goto after_copy;
    
do_copy:
    if (use_copy) goto copy_block;
    
skip_copy:
    /* Alternative path */
    __builtin_memset(dest, 0, n);
    
after_copy:
    /* Jump out of scope */
    return;
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_tree(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Initialize node data with memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in data */
    char pattern[32];
    __builtin_memset(pattern, 'A' + (depth % 26), 31);
    pattern[31] = '\0';
    
    /* Copy pattern into node data */
    __builtin_memcpy(node->data, pattern, 31);
    
    /* Recursive creation */
    node->left = create_tree(depth - 1, counter);
    node->right = create_tree(depth - 1, counter);
    
    /* Copy between nodes if both children exist */
    if (node->left && node->right) {
        __builtin_memcpy(node->left->data + 32, node->right->data, 32);
    }
    
    return node;
}

/* Complex memory dispatch with OpenMP */
static void parallel_memory_ops(void) {
    const int NUM_BUFFERS = 8;
    char* buffers[NUM_BUFFERS];
    size_t sizes[NUM_BUFFERS];
    
    /* Initialize buffers with varying sizes */
    for (int i = 0; i < NUM_BUFFERS; i++) {
        sizes[i] = (g_mem_size * (i + 1)) % 256 + 64;
        buffers[i] = (char*)malloc(sizes[i]);
        if (!buffers[i]) continue;
        
        /* Use volatile to prevent folding */
        volatile int fill_char = '0' + (i % 10);
        __builtin_memset(buffers[i], fill_char, sizes[i]);
    }
    
    /* OpenMP parallel region */
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        /* Each thread performs different memory operations */
        #pragma omp for
        for (int i = 0; i < NUM_BUFFERS; i++) {
            if (buffers[i]) {
                /* Mix of memory operations */
                if (i % 3 == 0) {
                    __builtin_memcpy(buffers[(i + 1) % NUM_BUFFERS], 
                                    buffers[i], 
                                    sizes[i] < sizes[(i + 1) % NUM_BUFFERS] ? 
                                    sizes[i] : sizes[(i + 1) % NUM_BUFFERS]);
                } else if (i % 3 == 1) {
                    __builtin_memset(buffers[i], 'X', sizes[i] / 2);
                } else {
                    /* Create overlapping regions for memmove */
                    if (i > 0 && buffers[i - 1]) {
                        size_t move_size = sizes[i] < sizes[i - 1] ? 
                                          sizes[i] : sizes[i - 1];
                        __builtin_memmove(buffers[i], buffers[i - 1], move_size);
                    }
                }
            }
        }
        
        /* Thread-private memory operations */
        char private_buf[128];
        __builtin_memset(private_buf, tid + 'A', sizeof(private_buf));
        
        #pragma omp barrier
        
        /* Cross-thread memory operations after barrier */
        #pragma omp single
        {
            for (int i = 0; i < NUM_BUFFERS - 1; i++) {
                if (buffers[i] && buffers[i + 1]) {
                    __builtin_memcpy(buffers[i], buffers[i + 1], 
                                    sizes[i] < sizes[i + 1] ? 
                                    sizes[i] : sizes[i + 1]);
                }
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < NUM_BUFFERS; i++) {
        free(buffers[i]);
    }
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Basic built-in calls */
    printf("Phase 1: Testing basic built-ins\n");
    {
        char src[256], dst[256];
        volatile size_t copy_size = g_mem_size;
        
        __builtin_memset(src, 'S', sizeof(src));
        __builtin_memcpy(dst, src, copy_size);
        __builtin_memmove(dst + 32, dst, copy_size / 2);
        
        test_goto_memmove(dst + 64, src + 64, copy_size / 4);
    }
    
    /* Phase 2: Recursive AST operations */
    printf("Phase 2: Testing recursive AST operations\n");
    {
        int counter = 0;
        ASTNode* root = create_tree(4, &counter);
        
        if (root) {
            /* Perform memory operations between tree nodes */
            ASTNode* stack[16];
            int top = 0;
            stack[top++] = root;
            
            while (top > 0) {
                ASTNode* current = stack[--top];
                
                if (current->left && current->right) {
                    /* Cross-copy between siblings */
                    __builtin_memcpy(current->left->data + 64, 
                                    current->right->data + 64, 64);
                    __builtin_memmove(current->right->data, 
                                     current->left->data, 128);
                }
                
                if (current->right) stack[top++] = current->right;
                if (current->left) stack[top++] = current->left;
            }
            
            /* TODO: Add proper tree freeing */
        }
    }
    
    /* Phase 3: Parallel memory operations */
    printf("Phase 3: Testing parallel memory operations\n");
    parallel_memory_ops();
    
    /* Phase 4: Variable-sized operations */
    printf("Phase 4: Testing variable-sized operations\n");
    {
        /* Dynamically sized operations */
        for (int i = 0; i < 10; i++) {
            size_t size = (g_mem_size * (i + 1)) % 512;
            char* buf1 = (char*)malloc(size);
            char* buf2 = (char*)malloc(size);
            
            if (buf1 && buf2) {
                __builtin_memset(buf1, i + 'a', size);
                __builtin_memcpy(buf2, buf1, size);
                
                /* Overlapping move */
                if (size > 32) {
                    __builtin_memmove(buf1 + 16, buf1, size - 32);
                }
            }
            
            free(buf1);
            free(buf2);
        }
    }
    
    /* Verification hash */
    printf("Phase 5: Computing verification hash\n");
    {
        unsigned long hash = 0;
        char verify_buf[1024];
        
        __builtin_memset(verify_buf, 0, sizeof(verify_buf));
        
        /* Fill with pattern */
        for (int i = 0; i < (int)sizeof(verify_buf); i++) {
            verify_buf[i] = (i * 31) % 256;
        }
        
        /* Multiple memory operations */
        __builtin_memcpy(verify_buf + 256, verify_buf, 256);
        __builtin_memmove(verify_buf + 512, verify_buf + 256, 256);
        __builtin_memset(verify_buf + 768, 0xFF, 256);
        
        /* Compute simple hash */
        for (int i = 0; i < (int)sizeof(verify_buf); i++) {
            hash = (hash * 31) + verify_buf[i];
        }
        
        printf("Verification hash: %lu\n", hash);
        printf("Test completed successfully\n");
    }
    
    return 0;
}
