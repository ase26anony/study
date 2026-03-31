/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 16;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char global_tokens[256];

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(global_tokens, 'A', sizeof(global_tokens));
    
    /* Force redirection initialization early */
    char local_buf[32];
    __builtin_memcpy(local_buf, "CONSTRUCTOR_INIT", 16);
    __builtin_memset(local_buf + 16, 0, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use all three builtins in destructor */
    char cleanup_buf[64];
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
    __builtin_memcpy(cleanup_buf, "CLEANUP", 7);
    __builtin_memmove(cleanup_buf + 7, cleanup_buf, 7);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Use volatile-controlled length */
    int len = volatile_len;
    if (len > 63) len = 63;
    
    /* Initialize node data with builtins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    char temp[32];
    __builtin_memset(temp, '0' + (depth % 10), 31);
    temp[31] = '\0';
    
    /* Copy with goto for flow control */
    int i = 0;
copy_loop:
    if (i < len) {
        node->data[i] = temp[i % 31];
        i++;
        goto copy_loop;
    }
    
    /* Create children */
    node->left = create_ast(depth - 1, counter);
    node->right = create_ast(depth - 1, counter);
    
    return node;
}

/* Function with goto jumping into memmove block */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int use_memmove = volatile_flag;
    
    if (use_memmove) {
        goto do_memmove;
    } else {
        /* Regular path */
        __builtin_memcpy(dst->data, src->data, sizeof(dst->data));
        return;
    }
    
do_memmove:
    /* Jump into memmove block */
    __builtin_memmove(dst->data, src->data, sizeof(dst->data));
    
    /* Jump out */
    goto finish;
    
    /* Unreachable code that might confuse flow analysis */
    __builtin_memset(dst->data, 0, sizeof(dst->data));
    
finish:
    return;
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        /* Each thread gets its own buffers */
        char thread_buf1[128];
        char thread_buf2[128];
        char thread_buf3[128];
        
        /* Initialize with different patterns */
        #pragma omp for
        for (int i = 0; i < 128; i++) {
            thread_buf1[i] = (char)(i % 256);
        }
        
        /* Use all three builtins in parallel region */
        __builtin_memset(thread_buf2, 0xAA, sizeof(thread_buf2));
        __builtin_memcpy(thread_buf3, thread_buf1, sizeof(thread_buf1));
        
        /* Overlapping copy with memmove */
        __builtin_memmove(thread_buf1 + 32, thread_buf1, 64);
        
        /* Verify with volatile access */
        volatile char check = thread_buf1[0];
        (void)check;
    }
}

/* Complex initialization with multiple stages */
static void initialize_system(void) {
    /* Stage 1: Direct builtin calls */
    char stage1[256];
    __builtin_memset(stage1, 0, sizeof(stage1));
    
    /* Stage 2: Indirect through function pointer */
    void (*mem_ops[3])(void*, const void*, size_t) = {
        (void(*)(void*, const void*, size_t))__builtin_memcpy,
        (void(*)(void*, const void*, size_t))__builtin_memset,
        (void(*)(void*, const void*, size_t))__builtin_memmove
    };
    
    char stage2[128];
    char stage2_src[128] = "SOURCE_DATA_FOR_COPY_OPERATIONS_TEST";
    
    for (int i = 0; i < 3; i++) {
        mem_ops[i](stage2, stage2_src, 64);
    }
    
    /* Stage 3: Nested memory operations */
    char nested[3][64];
    for (int i = 0; i < 3; i++) {
        __builtin_memset(nested[i], i + 'A', 64);
    }
    
    /* Copy between nested arrays */
    __builtin_memcpy(nested[1], nested[0], 32);
    __builtin_memmove(nested[2] + 16, nested[1], 32);
}

/* Main execution flow */
int main(void) {
    int counter = 0;
    unsigned long hash = 0;
    
    /* Initialize complex token array */
    for (int i = 0; i < (int)sizeof(global_tokens); i++) {
        global_tokens[i] = (char)((i * 13) % 256);
    }
    
    /* Create recursive AST */
    ASTNode* root = create_ast(4, &counter);
    
    if (root && root->left && root->right) {
        /* Process with goto jumps */
        process_with_goto(root, root->left);
        process_with_goto(root->left, root->right);
        
        /* Calculate hash from AST data */
        for (int i = 0; i < 64; i++) {
            hash = (hash * 31) + (unsigned long)root->data[i];
            if (root->left) hash = (hash * 31) + (unsigned long)root->left->data[i];
            if (root->right) hash = (hash * 31) + (unsigned long)root->right->data[i];
        }
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Complex initialization */
    initialize_system();
    
    /* Final builtin calls with volatile lengths */
    char final_buf[512];
    int final_len = volatile_len * 2;
    if (final_len > 511) final_len = 511;
    
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf, global_tokens, final_len);
    __builtin_memmove(final_buf + 256, final_buf, final_len);
    
    /* Add final buffer to hash */
    for (int i = 0; i < final_len; i++) {
        hash = (hash * 31) + (unsigned long)final_buf[i];
    }
    
    /* Print verification result */
    printf("Verification hash: %lu\n", hash);
    printf("AST nodes created: %d\n", counter);
    
    /* Cleanup */
    /* Note: In real code, you'd want to properly free the AST */
    
    return 0;
}
