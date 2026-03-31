/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_trigger = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor attribute to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[128];
    /* Force __builtin_memset initialization */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    printf("Constructor: Initialized ASAN early\n");
}

/* Destructor for cleanup coordination */
__attribute__((destructor))
static void cleanup_asan(void) {
    printf("Destructor: ASAN cleanup complete\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset for initialization */
    __builtin_memset(node->data, 0, sizeof(node->data));
    node->size = sizeof(node->data);
    
    /* Fill with pattern using __builtin_memcpy */
    char pattern[32];
    __builtin_memset(pattern, 'A' + (depth % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(pattern));
    
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    return node;
}

/* Function with goto jumps around memmove */
static void goto_memmove_test(char* dest, char* src, size_t len) {
    int use_memmove = 0;
    
    /* Jump into block with __builtin_memmove */
    if (volatile_trigger) goto do_memmove;
    
    normal_path:
    __builtin_memcpy(dest, src, len);
    return;
    
    do_memmove:
    use_memmove = 1;
    /* This tests flow-sensitive RTL handling */
    __builtin_memmove(dest, src, len);
    
    if (use_memmove) goto normal_path;
}

/* OpenMP parallel memory operations */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[256];
        char shared_buf[256];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, tid, sizeof(local_buf));
        
        #pragma omp barrier
        
        /* Critical section with memcpy */
        #pragma omp critical
        {
            __builtin_memcpy(shared_buf, local_buf, 
                           volatile_len % sizeof(local_buf));
        }
        
        /* Use memmove within parallel region */
        if (tid % 2 == 0) {
            __builtin_memmove(local_buf + 32, local_buf, 64);
        }
    }
}

/* Multi-stage processing with different builtins */
static uint64_t process_ast(ASTNode* root) {
    if (!root) return 0;
    
    uint64_t hash = 0;
    char temp[256];
    
    /* Stage 1: Copy node data */
    __builtin_memcpy(temp, root->data, root->size);
    
    /* Stage 2: Modify with memset */
    __builtin_memset(temp + 16, 0xCC, 32);
    
    /* Stage 3: Overlap with memmove */
    __builtin_memmove(temp + 8, temp, 128);
    
    /* Compute simple hash */
    for (size_t i = 0; i < sizeof(temp); i++) {
        hash = (hash * 31) + (uint8_t)temp[i];
    }
    
    /* Recursive processing */
    hash += process_ast(root->left);
    hash += process_ast(root->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Initialize test buffers */
    char src[512], dest[512];
    size_t len = volatile_len;
    
    /* Force all three builtins in main */
    __builtin_memset(src, 0x55, sizeof(src));
    __builtin_memcpy(dest, src, len);
    
    /* Test goto with memmove */
    goto_memmove_test(dest + 64, src, 128);
    
    /* Create recursive structure */
    ASTNode* ast = create_ast(4);
    
    /* Parallel memory operations */
    parallel_mem_ops();
    
    /* Process AST with memory operations */
    uint64_t result = process_ast(ast);
    
    /* Final verification with all builtins */
    char final_buf[1024];
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf, dest, 256);
    __builtin_memmove(final_buf + 512, final_buf, 256);
    
    printf("Result hash: %llu\n", (unsigned long long)result);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(ast);
    
    return 0;
}
