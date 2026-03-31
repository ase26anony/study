/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 256 + (rand() % 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    printf("Destructor: ASAN test completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with volatile size */
    size_t copy_size = g_mem_size % 64;
    __builtin_memcpy(node->data, base_data, copy_size);
    node->size = copy_size;
    
    /* Create children with goto-based control flow */
    if (depth > 1) {
        int use_left = (depth % 2 == 0);
        
        if (use_left) {
            goto create_left;
        } else {
            goto create_right;
        }
        
    create_left:
        node->left = create_ast(depth - 1, "left_branch");
        if (depth > 2) goto create_right;
        else goto finish;
        
    create_right:
        node->right = create_ast(depth - 1, "right_branch");
        goto finish;
        
    finish:
        ; /* Empty statement for label */
    }
    
    return node;
}

/* Function with __builtin_memmove and goto jumps */
static void rearrange_ast_data(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int do_copy = 1;
    
    if (do_copy) {
        goto copy_operation;
    } else {
        goto skip_copy;
    }
    
copy_operation:
    /* Use __builtin_memmove for overlapping regions */
    if (src->data + 10 < dst->data + 50) {
        __builtin_memmove(dst->data + 20, src->data + 10, 30);
    }
    
    /* Jump out of the block */
    goto after_copy;
    
skip_copy:
    printf("Skipping copy operation\n");
    
after_copy:
    /* Nested goto for control flow complexity */
    if (src->left && dst->right) {
        goto recursive_call;
    }
    
    return;
    
recursive_call:
    rearrange_ast_data(src->left, dst->right);
}

/* OpenMP parallel memory operations */
static void parallel_mem_operations(void) {
    const int num_threads = 4;
    char* buffers[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread allocates and operates on memory */
        buffers[tid] = (char*)malloc(g_mem_size);
        if (buffers[tid]) {
            /* Pattern initialization using __builtin_memset */
            __builtin_memset(buffers[tid], tid + 'A', g_mem_size);
            
            /* Inter-thread copying using __builtin_memcpy */
            if (tid > 0) {
                size_t copy_len = (g_mem_size / 2) + (tid * 8);
                __builtin_memcpy(buffers[tid], buffers[tid-1], copy_len);
            }
            
            /* Use __builtin_memmove for overlapping within buffer */
            if (g_mem_size > 100) {
                __builtin_memmove(buffers[tid] + 50, buffers[tid] + 25, 25);
            }
        }
        
        #pragma omp barrier
        
        /* Verify and checksum */
        unsigned long checksum = 0;
        for (size_t i = 0; i < g_mem_size && i < 128; i++) {
            checksum += (unsigned char)buffers[tid][i];
        }
        printf("Thread %d checksum: %lu\n", tid, checksum);
        
        free(buffers[tid]);
    }
}

/* Multi-stage initialization with different memory built-ins */
static void multi_stage_memory_test(void) {
    /* Stage 1: Direct built-in calls */
    char stage1_buf[512];
    __builtin_memset(stage1_buf, 0xAA, sizeof(stage1_buf));
    
    /* Stage 2: Volatile-controlled operations */
    volatile size_t stage2_size = g_mem_size * 2;
    char* stage2_buf = (char*)malloc(stage2_size);
    
    if (stage2_buf) {
        __builtin_memcpy(stage2_buf, stage1_buf, 
                        stage2_size < sizeof(stage1_buf) ? stage2_size : sizeof(stage1_buf));
        
        /* Overlapping move */
        __builtin_memmove(stage2_buf + 100, stage2_buf + 50, 150);
        
        free(stage2_buf);
    }
    
    /* Stage 3: Recursive structure operations */
    ASTNode* tree1 = create_ast(4, "tree1_root");
    ASTNode* tree2 = create_ast(3, "tree2_root");
    
    if (tree1 && tree2) {
        rearrange_ast_data(tree1, tree2);
        
        /* Free recursively */
        free(tree1);
        free(tree2);
    }
}

int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Force initialization of asan_memfn_rtls cache */
    char init_buf[128];
    __builtin_memset(init_buf, 0, sizeof(init_buf));
    __builtin_memcpy(init_buf, "initial_data", 12);
    __builtin_memmove(init_buf + 20, init_buf + 10, 10);
    
    /* Execute multi-stage test */
    multi_stage_memory_test();
    
    /* Execute OpenMP parallel operations */
    parallel_mem_operations();
    
    /* Final verification with all three built-ins */
    char final_buf[1024];
    volatile size_t final_size = g_mem_size % 1024;
    
    __builtin_memset(final_buf, 0xFF, final_size);
    __builtin_memcpy(final_buf + 100, "verification_string", 19);
    __builtin_memmove(final_buf + 200, final_buf + 100, 50);
    
    /* Calculate and print result */
    unsigned long final_hash = 0;
    for (size_t i = 0; i < final_size && i < 256; i++) {
        final_hash = final_hash * 31 + (unsigned char)final_buf[i];
    }
    
    printf("Final hash: %lu\n", final_hash);
    printf("Test completed successfully\n");
    
    return 0;
}
