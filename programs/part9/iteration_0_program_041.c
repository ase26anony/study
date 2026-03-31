/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
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

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_asan_early(void) {
    char buffer[128];
    volatile char* dest = buffer;
    volatile char* src = "Constructor init";
    
    /* Force __builtin_memcpy in constructor */
    __builtin_memcpy((void*)dest, (void*)src, 16);
    
    /* Force __builtin_memset in constructor */
    __builtin_memset(buffer + 16, 0xAA, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_asan_late(void) {
    char cleanup_buf[256];
    
    /* Force __builtin_memmove in destructor */
    const char* msg = "Cleanup complete";
    __builtin_memcpy(cleanup_buf, msg, strlen(msg) + 1);
    __builtin_memmove(cleanup_buf + 8, cleanup_buf, 16);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with __builtin_memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[32];
    for (int i = 0; i < 32; i++) {
        pattern[i] = (char)((id + i) & 0xFF);
    }
    __builtin_memcpy(node->data, pattern, 32);
    
    /* Recursive creation with goto for control flow */
    int create_right = 0;
    
    if (volatile_flag) {
        create_right = 1;
        goto create_right_branch;
    }
    
create_left_branch:
    node->left = create_ast(depth - 1, id * 2);
    if (!create_right) goto skip_right;
    
create_right_branch:
    node->right = create_ast(depth - 1, id * 2 + 1);
    
skip_right:
    return node;
}

/* Function with goto jumping into memory operation block */
static void goto_memmove_test(char* dest, char* src, size_t len) {
    int use_memmove = volatile_flag;
    
    if (!use_memmove) {
        goto direct_copy;
    }
    
    /* Jump into this block */
    goto perform_memmove;
    
perform_memmove:
    /* Force __builtin_memmove with goto entry */
    __builtin_memmove(dest, src, len);
    goto after_copy;
    
direct_copy:
    __builtin_memcpy(dest, src, len);
    
after_copy:
    /* Verify with __builtin_memset */
    __builtin_memset(dest + len - 8, 0xCC, 8);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    const int num_workers = 4;
    char buffers[num_workers][256];
    int results[num_workers];
    
    #pragma omp parallel num_threads(num_workers)
    {
        int tid = 0;
        #ifdef _OPENMP
        tid = omp_get_thread_num();
        #endif
        
        /* Each thread uses different builtins */
        switch (tid % 3) {
            case 0:
                __builtin_memset(buffers[tid], tid, volatile_len);
                break;
            case 1:
                __builtin_memcpy(buffers[tid], buffers[(tid + 1) % num_workers], 
                                volatile_len);
                break;
            case 2:
                __builtin_memmove(buffers[tid], buffers[(tid + 2) % num_workers],
                                 volatile_len);
                break;
        }
        
        /* Compute checksum */
        int sum = 0;
        for (int i = 0; i < volatile_len; i++) {
            sum += buffers[tid][i];
        }
        results[tid] = sum;
    }
    
    /* Verify parallel results */
    int total = 0;
    for (int i = 0; i < num_workers; i++) {
        total += results[i];
    }
    printf("Parallel checksum: %d\n", total);
}

/* Multi-stage initialization with varied memory operations */
static void multi_stage_init(void) {
    /* Stage 1: Direct builtin calls */
    char stage1_buf[512];
    __builtin_memset(stage1_buf, 0x11, 128);
    
    /* Stage 2: Nested calls */
    {
        char temp[256];
        __builtin_memcpy(temp, stage1_buf, 128);
        __builtin_memmove(stage1_buf + 128, temp, 128);
    }
    
    /* Stage 3: Conditional execution */
    for (int i = 0; i < 4; i++) {
        if (i & 1) {
            __builtin_memset(stage1_buf + i * 64, i, 64);
        } else {
            __builtin_memcpy(stage1_buf + i * 64, 
                           stage1_buf + ((i + 1) * 64) % 256, 
                           64);
        }
    }
    
    /* Stage 4: Function pointer-like behavior */
    void (*mem_ops[3])(void*, const void*, size_t) = {
        (void(*)(void*, const void*, size_t))__builtin_memcpy,
        (void(*)(void*, const void*, size_t))__builtin_memset,
        (void(*)(void*, const void*, size_t))__builtin_memmove
    };
    
    char final_buf[256];
    for (int i = 0; i < 3; i++) {
        mem_ops[i](final_buf, stage1_buf + i * 64, 64);
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN builtin redirection test...\n");
    
    /* 1. Create recursive AST structure */
    ASTNode* root = create_ast(3, 1);
    
    /* 2. Perform AST memory operations */
    if (root && root->left && root->right) {
        /* Copy between AST nodes */
        __builtin_memcpy(root->left->data, root->data, 128);
        __builtin_memmove(root->right->data, root->left->data, 128);
        
        /* Clear with memset */
        __builtin_memset(root->data + 192, 0, 64);
    }
    
    /* 3. Goto-based memory move test */
    char src_data[256], dest_data[256];
    for (int i = 0; i < 256; i++) {
        src_data[i] = (char)(i & 0xFF);
    }
    
    goto_memmove_test(dest_data, src_data, volatile_len);
    
    /* 4. OpenMP parallel operations */
    #ifdef _OPENMP
    parallel_memory_ops();
    #endif
    
    /* 5. Multi-stage initialization */
    multi_stage_init();
    
    /* 6. Final verification with all three builtins */
    char verify_buf[1024];
    __builtin_memset(verify_buf, 0, sizeof(verify_buf));
    __builtin_memcpy(verify_buf + 256, dest_data, 256);
    __builtin_memmove(verify_buf + 512, verify_buf + 256, 256);
    
    /* Compute final hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(verify_buf); i++) {
        hash = (hash * 31) + verify_buf[i];
    }
    
    printf("Final hash: %lu\n", hash);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    /* Recursive free would be here in real implementation */
    
    return 0;
}
