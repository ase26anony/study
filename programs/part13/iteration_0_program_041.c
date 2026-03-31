/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 96;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_sanitizer_hook(void) {
    volatile char buffer[32];
    /* Force builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 8);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_sanitizer_hook(void) {
    volatile char final_check[16];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    
    /* Fill data with pattern */
    for (int i = 0; i < 255; i++) {
        node->data[i] = (char)((depth + i) % 256);
    }
    node->data[255] = '\0';
    
    /* Recursive creation */
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 2);
    
    return node;
}

/* Function with goto jumps around memmove */
static void goto_memmove_test(char* dest, char* src, size_t len) {
    int use_memmove = 1;
    
    if (len > 100) goto skip_op;
    
    /* Jump into block with memmove */
    goto do_memmove;
    
skip_op:
    use_memmove = 0;
    return;
    
do_memmove:
    /* This should trigger the builtin redirection */
    __builtin_memmove(dest, src, len);
    
    /* Jump out */
    if (use_memmove) {
        goto after_memmove;
    }
    
after_memmove:
    return;
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buf[512];
        char shared_buf[512];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        #pragma omp for
        for (int i = 0; i < 4; i++) {
            size_t len = g_memcpy_len + i * 16;
            __builtin_memcpy(shared_buf + i * 128, local_buf, len);
        }
        
        #pragma omp single
        {
            /* Master thread does memmove */
            __builtin_memmove(shared_buf + 256, shared_buf, 128);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Phase 1: Basic builtin calls */
    char buffer1[1024];
    char buffer2[1024];
    
    __builtin_memset(buffer1, 0xCC, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, g_memcpy_len);
    __builtin_memmove(buffer1 + 256, buffer1, g_memmove_len);
    
    /* Phase 2: AST operations */
    ASTNode* root = create_ast(4);
    if (root) {
        /* Copy between AST nodes */
        if (root->left && root->right) {
            __builtin_memcpy(root->right->data, root->left->data, 128);
        }
        
        /* Goto test with memmove */
        goto_memmove_test(root->data, buffer1, 64);
    }
    
    /* Phase 3: OpenMP parallel section */
    parallel_memory_ops();
    
    /* Phase 4: Complex memory pattern */
    volatile char* dynamic_buf = (char*)malloc(2048);
    if (dynamic_buf) {
        for (int i = 0; i < 4; i++) {
            size_t len = g_memset_len + i * 32;
            __builtin_memset(dynamic_buf + i * 512, i * 0x11, len);
        }
        
        /* Overlapping copy */
        __builtin_memcpy(dynamic_buf + 512, dynamic_buf, 768);
        
        free((void*)dynamic_buf);
    }
    
    /* Verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        hash = (hash * 31) + buffer1[i];
    }
    
    printf("Test completed. Hash: 0x%08lx\n", hash);
    
    /* Cleanup */
    /* Note: Proper AST cleanup omitted for brevity */
    
    return 0;
}
