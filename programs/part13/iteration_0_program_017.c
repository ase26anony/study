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
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    char buffer[128];
    volatile char* dest = buffer;
    volatile char* src = "Constructor initialization";
    
    /* Force __builtin_memcpy in constructor */
    __builtin_memcpy((void*)dest, (void*)src, 27);
    
    /* Also use memset */
    __builtin_memset(buffer + 27, 0, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_late(void) {
    char cleanup_buf[256];
    
    /* Use all three builtins in destructor */
    __builtin_memset(cleanup_buf, 0xFF, 256);
    __builtin_memcpy(cleanup_buf + 128, cleanup_buf, 128);
    __builtin_memmove(cleanup_buf, cleanup_buf + 64, 192);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile-controlled length */
    size_t copy_len = (size_t)volatile_len;
    if (copy_len > 255) copy_len = 255;
    
    /* Force __builtin_memcpy with variable length */
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, base_data, copy_len);
    
    node->size = copy_len;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        char child_data[256];
        __builtin_memcpy(child_data, node->data, copy_len);
        
        /* Goto block for flow sensitivity testing */
        if (volatile_flag) {
            goto create_left;
        }
        
        node->left = NULL;
        node->right = NULL;
        return node;
        
    create_left:
        node->left = create_ast(depth - 1, child_data);
        
        /* Another goto for memmove testing */
        if (depth > 2) {
            goto create_right_with_move;
        }
        
        node->right = create_ast(depth - 1, child_data);
        return node;
        
    create_right_with_move:
        /* Use __builtin_memmove with overlapping regions */
        char temp[512];
        __builtin_memcpy(temp, child_data, copy_len);
        __builtin_memmove(temp + 128, temp, copy_len);
        __builtin_memcpy(child_data, temp + 128, copy_len);
        
        node->right = create_ast(depth - 1, child_data);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with complex goto patterns around memmove */
static void test_goto_memmove(void) {
    char buffer1[1024];
    char buffer2[1024];
    volatile int use_goto = volatile_flag;
    
    __builtin_memset(buffer1, 'A', 512);
    __builtin_memset(buffer2, 'B', 512);
    
    if (use_goto) {
        goto perform_memmove;
    }
    
    /* Normal path */
    __builtin_memcpy(buffer1 + 256, buffer2, 256);
    return;
    
perform_memmove:
    /* Goto target with __builtin_memmove */
    __builtin_memmove(buffer1 + 128, buffer1, 384);
    
    /* Jump back out */
    if (use_goto > 1) {
        goto finish;
    }
    
    /* More operations */
    __builtin_memcpy(buffer2, buffer1, 384);
    
finish:
    /* Final memmove with overlap */
    __builtin_memmove(buffer1, buffer1 + 64, 448);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    const int num_workers = 4;
    char shared_buffers[4][1024];
    volatile long results[4] = {0};
    
    #pragma omp parallel num_threads(num_workers)
    {
        int tid = 0;
        #ifdef _OPENMP
        tid = omp_get_thread_num();
        #endif
        
        /* Each thread uses different builtins */
        switch (tid % 3) {
            case 0:
                __builtin_memset(shared_buffers[tid], tid + '0', 1024);
                break;
            case 1:
                if (tid > 0) {
                    __builtin_memcpy(shared_buffers[tid], 
                                   shared_buffers[tid-1], 
                                   512);
                }
                break;
            case 2:
                __builtin_memmove(shared_buffers[tid] + 256,
                                shared_buffers[tid],
                                768);
                break;
        }
        
        /* Compute hash to prevent optimization */
        for (int i = 0; i < 1024; i++) {
            results[tid] += shared_buffers[tid][i];
        }
    }
    
    /* Verify by printing sum (prevents dead code elimination) */
    long total = 0;
    for (int i = 0; i < 4; i++) {
        total += results[i];
    }
    printf("Parallel hash sum: %ld\n", total);
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Basic builtin calls */
    char buffer1[4096];
    char buffer2[4096];
    
    __builtin_memset(buffer1, 0xAA, 4096);
    __builtin_memcpy(buffer2, buffer1, 4096);
    __builtin_memmove(buffer1 + 2048, buffer1, 2048);
    
    /* Phase 2: Recursive AST with memory ops */
    ASTNode* root = create_ast(4, "Base AST node data for testing");
    
    if (root) {
        /* Copy between AST nodes */
        if (root->left && root->right) {
            size_t copy_size = root->left->size;
            if (copy_size > root->right->size) {
                copy_size = root->right->size;
            }
            
            __builtin_memcpy(root->right->data, 
                           root->left->data, 
                           copy_size);
            
            /* Overlapping move within same node */
            __builtin_memmove(root->left->data + 32,
                            root->left->data,
                            copy_size - 32);
        }
        
        /* Free AST recursively */
        /* ... (omitted for brevity) */
        free(root);
    }
    
    /* Phase 3: Goto flow testing */
    test_goto_memmove();
    
    /* Phase 4: OpenMP parallel section */
    #ifdef _OPENMP
    parallel_memory_ops();
    #endif
    
    /* Phase 5: Volatile-controlled operations */
    volatile char* dyn_buf = (volatile char*)malloc(8192);
    if (dyn_buf) {
        for (int i = 0; i < 10; i++) {
            size_t len = (size_t)(volatile_len * (i + 1));
            if (len > 8192) len = 8192;
            
            /* Alternate between builtins */
            switch (i % 3) {
                case 0:
                    __builtin_memset((void*)dyn_buf, i, len);
                    break;
                case 1:
                    __builtin_memcpy((void*)(dyn_buf + 4096),
                                   (void*)dyn_buf,
                                   len / 2);
                    break;
                case 2:
                    __builtin_memmove((void*)dyn_buf,
                                    (void*)(dyn_buf + 2048),
                                    len / 4);
                    break;
            }
        }
        free((void*)dyn_buf);
    }
    
    /* Final verification hash */
    unsigned long final_hash = 0;
    for (int i = 0; i < 4096; i++) {
        final_hash += buffer1[i] + buffer2[i];
    }
    
    printf("Test completed. Final hash: %lu\n", final_hash);
    printf("If compiled with ASAN/HWASAN, builtins should be redirected.\n");
    
    return 0;
}
