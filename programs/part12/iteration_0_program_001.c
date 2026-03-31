/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force builtin initialization in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 8, buffer, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_late(void) {
    volatile char final_check[16];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* create_tree(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Initialize node data with builtins */
    __builtin_memset(node->data, node->id, sizeof(node->data));
    
    /* Create pattern in first half */
    char pattern[32];
    __builtin_memset(pattern, 0xCC, sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(pattern));
    
    node->left = create_tree(depth - 1, counter);
    node->right = create_tree(depth - 1, counter);
    
    /* Copy between child nodes if both exist */
    if (node->left && node->right) {
        __builtin_memcpy(node->left->data + 16, node->right->data, 16);
        if (g_use_memmove) {
            __builtin_memmove(node->right->data + 8, node->right->data, 24);
        }
    }
    
    return node;
}

/* Function with goto jumps around memory operations */
static void goto_mem_operations(char* dest, char* src, size_t n) {
    int use_memcpy = 1;
    
    if (n > 128) goto use_memset;
    
    __builtin_memcpy(dest, src, n);
    goto cleanup;
    
use_memset:
    __builtin_memset(dest, 0xDD, n);
    if (dest[0] > 100) {
        goto use_memmove;
    }
    goto cleanup;
    
use_memmove:
    /* Overlapping regions to force memmove */
    __builtin_memmove(dest + n/2, dest, n/2);
    
cleanup:
    /* Final operation after goto */
    __builtin_memset(dest + n - 8, 0xEE, 8);
}

/* OpenMP parallel section with memory operations */
static void parallel_mem_operations(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[256];
        char shared_pattern[64];
        
        /* Initialize pattern */
        #pragma omp master
        {
            __builtin_memset(shared_pattern, tid, sizeof(shared_pattern));
        }
        #pragma omp barrier
        
        /* Each thread copies pattern */
        __builtin_memcpy(local_buf, shared_pattern, sizeof(shared_pattern));
        
        /* Thread-specific memory operations */
        if (tid % 2 == 0) {
            __builtin_memset(local_buf + 128, tid, 64);
        } else {
            __builtin_memmove(local_buf + 32, local_buf, 96);
        }
        
        #pragma omp barrier
        
        /* Verify copy */
        __builtin_memcpy(shared_pattern, local_buf + 64, 32);
    }
}

/* Main execution flow */
int main(void) {
    /* Initialize counters */
    int node_counter = 0;
    size_t mem_size = g_mem_size;
    
    /* Phase 1: Tree operations */
    printf("Creating AST tree...\n");
    ASTNode* root = create_tree(4, &node_counter);
    
    /* Phase 2: Goto-based memory operations */
    printf("Performing goto-based memory operations...\n");
    char buffer1[512];
    char buffer2[512];
    
    __builtin_memset(buffer1, 0x11, sizeof(buffer1));
    __builtin_memset(buffer2, 0x22, sizeof(buffer2));
    
    for (int i = 0; i < 3; i++) {
        goto_mem_operations(buffer1 + i * 64, buffer2, mem_size / 2);
    }
    
    /* Phase 3: OpenMP parallel operations */
    printf("Running OpenMP memory operations...\n");
    parallel_mem_operations();
    
    /* Phase 4: Complex memory chain */
    printf("Executing memory chain...\n");
    char* dynamic_buf = malloc(mem_size * 2);
    if (dynamic_buf) {
        __builtin_memset(dynamic_buf, 0x33, mem_size * 2);
        
        /* Chain of memory operations */
        __builtin_memcpy(dynamic_buf + 128, buffer1, 128);
        __builtin_memmove(dynamic_buf, dynamic_buf + 64, 192);
        __builtin_memset(dynamic_buf + 256, 0x44, 128);
        
        /* Verify with final memcpy */
        __builtin_memcpy(buffer2, dynamic_buf + 32, 64);
        
        free(dynamic_buf);
    }
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        hash = (hash * 31) + buffer1[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Total nodes created: %d\n", node_counter);
    
    /* Cleanup tree */
    /* ... tree cleanup code would go here ... */
    
    return 0;
}
