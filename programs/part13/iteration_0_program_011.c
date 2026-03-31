/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 16;
static volatile char volatile_flag = 1;

/* AST-like recursive structure */
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
    
    /* Conditional memset with goto */
    if (volatile_flag) {
        goto memset_block;
    }
    
    return;
    
memset_block:
    __builtin_memset(global_tokens + 128, 'B', 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Final cleanup with memmove */
    char temp[256];
    __builtin_memcpy(temp, global_tokens, sizeof(global_tokens));
    __builtin_memmove(global_tokens, temp + 128, 128);
}

/* Recursive AST manipulation */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtin memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in data */
    for (int i = 0; i < sizeof(node->data) - 1; i++) {
        node->data[i] = '0' + (id % 10);
    }
    
    node->id = id;
    node->left = create_ast_node(depth - 1, id * 2);
    node->right = create_ast_node(depth - 1, id * 2 + 1);
    
    return node;
}

/* Copy AST data between nodes */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use builtin memcpy with volatile length */
    int len = volatile_len;
    if (len > sizeof(dest->data)) len = sizeof(dest->data);
    
    __builtin_memcpy(dest->data, src->data, len);
    
    /* Recursive copy */
    copy_ast_data(dest->left, src->left);
    copy_ast_data(dest->right, src->right);
}

/* Complex memory operation with goto flow control */
static void complex_memory_ops(void) {
    char buffer1[256];
    char buffer2[256];
    char buffer3[256];
    
    /* Initialization */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    
    /* Goto into memmove block */
    if (volatile_flag) {
        goto memmove_block;
    }
    
    /* This should be skipped */
    __builtin_memset(buffer2, 0xBB, sizeof(buffer2));
    
memmove_block:
    /* Move data with builtin memmove */
    __builtin_memmove(buffer2, buffer1, volatile_len);
    
    /* Jump out and back in */
    if (volatile_flag > 0) {
        goto copy_block;
    }
    
copy_block:
    /* Copy with builtin memcpy */
    __builtin_memcpy(buffer3, buffer2, volatile_len * 2);
    
    /* Nested conditional with goto */
    if (volatile_len > 8) {
        goto final_memset;
    }
    
    return;
    
final_memset:
    __builtin_memset(buffer3 + 128, 0xCC, 64);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    const int num_workers = 4;
    char shared_buffers[num_workers][128];
    char private_buffers[num_workers][64];
    
    #pragma omp parallel num_threads(num_workers)
    {
        int tid = 0;
        #ifdef _OPENMP
        tid = omp_get_thread_num();
        #endif
        
        /* Thread-private initialization */
        __builtin_memset(private_buffers[tid], tid + '0', sizeof(private_buffers[tid]));
        
        /* Shared buffer operations */
        #pragma omp critical
        {
            /* Copy to shared with builtin memcpy */
            __builtin_memcpy(shared_buffers[tid], private_buffers[tid], 
                           volatile_len % sizeof(private_buffers[tid]));
            
            /* Move data between shared buffers */
            if (tid > 0) {
                __builtin_memmove(shared_buffers[tid - 1], 
                                shared_buffers[tid], 
                                32);
            }
        }
        
        /* Final memset in parallel region */
        __builtin_memset(private_buffers[tid] + 32, 0xFF, 16);
    }
}

/* Multi-stage initialization */
static void multi_stage_init(void) {
    static char stage1[512];
    static char stage2[512];
    static int initialized = 0;
    
    if (!initialized) {
        /* Stage 1: Large memset */
        __builtin_memset(stage1, 0x11, sizeof(stage1));
        
        /* Stage 2: Pattern copy */
        for (int i = 0; i < sizeof(stage2); i++) {
            stage2[i] = (i % 256);
        }
        
        /* Stage 3: Overlap move */
        __builtin_memmove(stage1 + 256, stage2, 256);
        
        /* Stage 4: Final copy to global */
        __builtin_memcpy(global_tokens, stage1, sizeof(global_tokens));
        
        initialized = 1;
    }
}

/* Compute verification hash */
static unsigned int compute_hash(const char* data, size_t len) {
    unsigned int hash = 0;
    for (size_t i = 0; i < len; i++) {
        hash = (hash * 31) + (unsigned char)data[i];
    }
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* 1. Initialize complex token array */
    multi_stage_init();
    
    /* 2. Create and manipulate recursive AST */
    ASTNode* ast1 = create_ast_node(3, 1);
    ASTNode* ast2 = create_ast_node(3, 100);
    
    if (ast1 && ast2) {
        /* Copy data between AST nodes */
        copy_ast_data(ast2, ast1);
        
        /* Additional memory operations on AST */
        __builtin_memmove(ast1->data + 16, ast2->data + 32, 32);
        __builtin_memset(ast1->data + 48, 'Z', 16);
    }
    
    /* 3. Execute complex memory operations with goto */
    complex_memory_ops();
    
    /* 4. Parallelized memory dispatch */
    #ifdef _OPENMP
    parallel_memory_ops();
    #endif
    
    /* 5. Final built-in calls in main */
    char final_buffer[1024];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, global_tokens, sizeof(global_tokens));
    __builtin_memmove(final_buffer + 512, final_buffer, 512);
    
    /* 6. Compute and print verification result */
    unsigned int hash1 = compute_hash(global_tokens, sizeof(global_tokens));
    unsigned int hash2 = compute_hash(final_buffer, sizeof(final_buffer));
    
    printf("Verification hashes:\n");
    printf("  Global tokens: 0x%08X\n", hash1);
    printf("  Final buffer:  0x%08X\n", hash2);
    printf("  Hash sum:      %u\n", hash1 + hash2);
    
    /* Cleanup */
    /* Note: In real code, you'd need to free AST nodes recursively */
    
    printf("Test completed.\n");
    return 0;
}
