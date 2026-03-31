/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char *volatile_dest = NULL;
static volatile const char *volatile_src = NULL;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode *left;
    struct ASTNode *right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    
    /* Force built-in usage in constructor */
    char buf1[128], buf2[128];
    __builtin_memset(buf1, 0xAA, sizeof(buf1));
    __builtin_memcpy(buf2, buf1, sizeof(buf1));
    __builtin_memmove(buf1 + 32, buf1, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with built-ins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with pattern */
    char pattern[32];
    __builtin_memset(pattern, 'A' + (id % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(node->data));
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
skip_left:
    node->right = create_ast(depth - 1, id * 2 + 1);
    return node;
    
create_children:
    if (create_left) {
        node->left = create_ast(depth - 1, id * 2);
        create_left = 0;
        goto skip_left;
    }
}

/* Function with goto jumping into memory block */
static void test_goto_memmove(void) {
    char buffer1[256], buffer2[256];
    int use_memmove = 1;
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 0xCC, sizeof(buffer1));
    __builtin_memset(buffer2, 0xDD, sizeof(buffer2));
    
    if (use_memmove) {
        goto do_memmove;
    }
    
    /* Normal path */
    __builtin_memcpy(buffer2, buffer1, 128);
    return;
    
do_memmove:
    /* Jump into memmove block */
    __builtin_memmove(buffer2 + 64, buffer1, 128);
    
    /* Jump out */
    goto after_memmove;
    
after_memmove:
    /* Verify with another built-in */
    __builtin_memset(buffer1, 0xEE, 64);
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    const int num_arrays = 8;
    char *arrays[num_arrays];
    
    /* Allocate and initialize arrays */
    for (int i = 0; i < num_arrays; i++) {
        arrays[i] = malloc(volatile_len * 2);
        if (arrays[i]) {
            __builtin_memset(arrays[i], i * 16, volatile_len * 2);
        }
    }
    
    /* OpenMP parallel region */
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                if (arrays[thread_id]) {
                    __builtin_memcpy(arrays[thread_id] + 32, 
                                   arrays[(thread_id + 1) % num_arrays], 
                                   volatile_len);
                }
                break;
            case 1:
                if (arrays[thread_id]) {
                    __builtin_memset(arrays[thread_id] + 16, 
                                   thread_id, 
                                   volatile_len / 2);
                }
                break;
            case 2:
                if (arrays[thread_id] && arrays[(thread_id + 2) % num_arrays]) {
                    __builtin_memmove(arrays[thread_id],
                                    arrays[(thread_id + 2) % num_arrays] + 16,
                                    volatile_len);
                }
                break;
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_arrays; i++) {
        free(arrays[i]);
    }
}

/* Complex token processing */
static unsigned long process_tokens(const char *tokens[], int count) {
    unsigned long hash = 0xDEADBEEF;
    char buffer[512];
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        
        /* Use volatile-controlled length */
        size_t copy_len = len;
        if (volatile_len > 0 && copy_len > (size_t)volatile_len) {
            copy_len = volatile_len;
        }
        
        /* Chain memory operations */
        __builtin_memset(buffer, 0, sizeof(buffer));
        __builtin_memcpy(buffer, tokens[i], copy_len);
        
        /* Overlap with memmove */
        if (i > 0 && copy_len > 16) {
            __builtin_memmove(buffer + 8, buffer, copy_len - 8);
        }
        
        /* Compute simple hash */
        for (size_t j = 0; j < copy_len && j < sizeof(buffer); j++) {
            hash = (hash * 31) + buffer[j];
        }
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* 1. Test recursive AST structure */
    ASTNode *root = create_ast(4, 1);
    if (root) {
        /* Copy between AST nodes */
        ASTNode *copy = malloc(sizeof(ASTNode));
        if (copy) {
            __builtin_memcpy(copy, root, sizeof(ASTNode));
            
            /* Move within node */
            __builtin_memmove(copy->data + 8, copy->data, 16);
            
            free(copy);
        }
        
        /* TODO: Proper AST cleanup needed */
        free(root);
    }
    
    /* 2. Test goto flow control */
    test_goto_memmove();
    
    /* 3. Test with volatile variables */
    char dest[256];
    const char src[] = "Test source data for built-in memory functions";
    
    volatile_dest = dest;
    volatile_src = src;
    
    __builtin_memcpy((char*)volatile_dest, (const char*)volatile_src, 
                    volatile_len < sizeof(dest) ? volatile_len : sizeof(dest));
    __builtin_memset((char*)volatile_dest + 32, 0x55, volatile_len / 2);
    __builtin_memmove((char*)volatile_dest + 64, volatile_dest, 32);
    
    /* 4. Test parallel operations */
    parallel_memory_ops();
    
    /* 5. Test token processing */
    const char *tokens[] = {
        "memcpy", "memset", "memmove", "asan", "hwasan",
        "builtin", "redirection", "coverage", "test", "gcc"
    };
    
    unsigned long result = process_tokens(tokens, 
                                        sizeof(tokens)/sizeof(tokens[0]));
    
    printf("Test completed. Hash result: 0x%08lX\n", result);
    printf("Expected: All built-in memory functions redirected via asan.cc\n");
    
    return 0;
}
