/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)(i % 256);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    /* Clear sensitive data */
    __builtin_memset(token_pool, 0, sizeof(token_pool));
    printf("Destructor: Token pool cleared\n");
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with pattern using memcpy */
    char pattern[256];
    for (int i = 0; i < 256; i++) {
        pattern[i] = (char)((id + i) % 256);
    }
    __builtin_memcpy(node->data, pattern, 256);
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        node->right = create_ast(depth - 1, id * 2 + 1);
        
        if (use_goto) {
            create_children:
            /* Jump back into normal flow */
            node->left = create_ast(depth - 1, id * 2);
            node->right = create_ast(depth - 1, id * 2 + 1);
        }
    }
    
    return node;
}

/* Complex memory operation with goto edge cases */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int stage = 0;
    
    stage_0:
    if (volatile_flag) {
        /* Copy data between nodes using memmove (handles overlap) */
        __builtin_memmove(dst->data, src->data, volatile_len);
        stage = 1;
        goto stage_1;
    }
    
    stage_1:
    {
        /* Create overlapping regions for memmove test */
        char temp[512];
        __builtin_memcpy(temp, src->data, 256);
        
        /* Overlapping copy */
        __builtin_memmove(src->data + 128, src->data, 128);
        
        /* Restore */
        __builtin_memcpy(src->data, temp, 256);
        stage = 2;
        
        if (volatile_len > 32) {
            goto stage_2;
        }
    }
    
    stage_2:
    /* Fill with pattern using memset */
    __builtin_memset(dst->data + 128, 0xAA, volatile_len % 128);
    stage = 3;
    
    stage_3:
    /* Final copy back */
    __builtin_memcpy(dst->data, src->data, 256);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[1024];
        char shared_buf[1024];
        
        /* Initialize with builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        /* Copy to shared with memcpy */
        __builtin_memcpy(shared_buf, local_buf, volatile_len);
        
        /* Move within shared buffer */
        __builtin_memmove(shared_buf + 512, shared_buf, 512);
        
        #pragma omp barrier
        
        /* Verify with memset pattern */
        __builtin_memset(local_buf, 0xCC, sizeof(local_buf));
    }
}

/* Multi-stage processing with different memory functions */
static unsigned long process_tokens(void) {
    unsigned long hash = 0;
    char buffer[2048];
    char* current = token_pool;
    
    /* Stage 1: Direct copies */
    for (int i = 0; i < 4; i++) {
        __builtin_memcpy(buffer + i * 512, current + i * 1024, 512);
    }
    
    /* Stage 2: Overlapping moves */
    __builtin_memmove(buffer + 256, buffer, 768);
    
    /* Stage 3: Pattern initialization */
    __builtin_memset(buffer + 1024, 0xFE, 512);
    
    /* Stage 4: Final consolidation */
    __builtin_memcpy(buffer + 1536, buffer, 512);
    
    /* Compute simple hash */
    for (size_t i = 0; i < sizeof(buffer); i++) {
        hash = (hash * 31) + (unsigned char)buffer[i];
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: AST operations */
    ASTNode* root = create_ast(4, 1);
    ASTNode* copy = create_ast(4, 1000);
    
    if (root && copy) {
        /* Test goto flow with memory operations */
        process_with_goto(root, copy);
        
        /* Verify copy */
        int diff_count = 0;
        for (int i = 0; i < 256; i++) {
            if (root->data[i] != copy->data[i]) diff_count++;
        }
        printf("AST diff count: %d\n", diff_count);
    }
    
    /* Phase 2: OpenMP parallel operations */
    printf("Running OpenMP memory operations\n");
    parallel_memory_ops();
    
    /* Phase 3: Token processing */
    printf("Processing token pool\n");
    unsigned long final_hash = process_tokens();
    printf("Final hash: %lu\n", final_hash);
    
    /* Phase 4: Direct builtin calls with volatile control */
    char test_buf1[1024];
    char test_buf2[1024];
    
    volatile int dynamic_size = volatile_len * 2;
    
    __builtin_memset(test_buf1, 0xAB, dynamic_size);
    __builtin_memcpy(test_buf2, test_buf1, dynamic_size);
    __builtin_memmove(test_buf1 + 256, test_buf1, 256);
    
    /* Cleanup */
    if (root) free(root);
    if (copy) free(copy);
    
    printf("Test completed successfully\n");
    return 0;
}
