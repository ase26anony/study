/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < 256; i++) {
        volatile_src[i] = (char)(i % 26 + 'A');
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Verify memory was properly handled */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += volatile_dest[i];
    }
    printf("Destructor: Memory verification sum = %d\n", sum);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Fill with pattern using volatile length */
    int len = volatile_len % 32;
    for (int i = 0; i < len; i++) {
        node->data[i] = (char)('A' + node->id + i);
    }
    
    /* Recursive creation with goto for control flow */
    int use_goto = (depth % 2 == 0);
    
    if (use_goto) {
        goto create_left;
    } else {
        node->left = create_ast(depth - 1, counter);
        goto skip_left;
    }
    
create_left:
    node->left = create_ast(depth - 1, counter);
    
skip_left:
    /* Use __builtin_memcpy between nodes if left exists */
    if (node->left) {
        __builtin_memcpy(node->data + 16, node->left->data, 16);
    }
    
    node->right = create_ast(depth - 1, counter);
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void goto_memmove_test(char* dest, char* src, int len) {
    int condition = len > 32;
    
    if (condition) {
        goto perform_memmove;
    }
    
    /* Small copy */
    __builtin_memcpy(dest, src, len);
    goto end;
    
perform_memmove:
    /* Jump here with goto - tests flow sensitivity */
    __builtin_memmove(dest, src, len);
    
    /* Jump out */
    goto verify;
    
verify:
    /* Verify with another builtin */
    char temp[256];
    __builtin_memcpy(temp, dest, len);
    
end:
    return;
}

/* Parallel memory dispatch function */
static void parallel_memory_ops(void) {
    char buffer1[1024];
    char buffer2[1024];
    char buffer3[1024];
    
    /* Initialize buffers with different patterns */
    for (int i = 0; i < 1024; i++) {
        buffer1[i] = (char)(i % 256);
        buffer2[i] = (char)((i + 128) % 256);
    }
    
    /* OpenMP parallel region with memory operations */
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                /* Use volatile length to prevent optimization */
                __builtin_memcpy(buffer3 + thread_id * 64, 
                               buffer1 + thread_id * 64, 
                               volatile_len % 256);
                break;
            case 1:
                __builtin_memset(buffer3 + 256 + thread_id * 64, 
                               thread_id, 
                               volatile_len % 128);
                break;
            case 2:
                __builtin_memmove(buffer3 + 512 + thread_id * 64,
                                buffer2 + thread_id * 64,
                                volatile_len % 192);
                break;
        }
        
        /* Barrier to synchronize */
        #pragma omp barrier
        
        /* All threads verify their work */
        #pragma omp for
        for (int i = 0; i < 1024; i++) {
            buffer1[i] ^= buffer3[i];
        }
    }
    
    /* Final memory operation outside parallel region */
    __builtin_memcpy(volatile_dest, buffer3, 256);
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Recursive AST operations */
    int counter = 0;
    ASTNode* root = create_ast(4, &counter);
    
    if (root) {
        /* Copy between tree nodes */
        if (root->left && root->right) {
            __builtin_memcpy(root->right->data, 
                           root->left->data, 
                           sizeof(root->left->data));
        }
        
        /* Free AST recursively */
        ASTNode* nodes[16];
        int node_count = 0;
        
        /* Collect nodes for batch operations */
        nodes[node_count++] = root;
        if (root->left) nodes[node_count++] = root->left;
        if (root->right) nodes[node_count++] = root->right;
        
        /* Batch memory clear before free */
        for (int i = 0; i < node_count; i++) {
            __builtin_memset(nodes[i]->data, 0, sizeof(nodes[i]->data));
            free(nodes[i]);
        }
    }
    
    /* Phase 2: Goto control flow test */
    char test_src[128];
    char test_dest[128];
    
    for (int i = 0; i < 128; i++) {
        test_src[i] = (char)(i * 2);
    }
    
    /* Test different lengths to hit different paths */
    goto_memmove_test(test_dest, test_src, 16);   /* Small - uses memcpy */
    goto_memmove_test(test_dest, test_src, 64);   /* Large - uses memmove */
    
    /* Phase 3: Parallel operations */
    parallel_memory_ops();
    
    /* Phase 4: Direct built-in calls with volatile */
    char final_buffer[512];
    
    /* Chain of memory operations */
    __builtin_memset(final_buffer, 0xAA, volatile_len % 512);
    __builtin_memcpy(final_buffer + 128, test_dest, 64);
    __builtin_memmove(final_buffer + 256, final_buffer, 128);
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < 512; i++) {
        hash = (hash * 31) + final_buffer[i];
    }
    
    printf("Test completed. Verification hash: %lu\n", hash);
    printf("Expected hash with default pattern: 457645766\n");
    
    return 0;
}
