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
    
    /* Copy data using __builtin_memcpy with goto for flow control */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    
    /* Jump label for goto testing */
    copy_start:
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Conditional goto to test flow sensitivity */
    if (depth > 3) {
        goto skip_adjust;
    }
    
    /* Another memory operation after goto */
    __builtin_memset(node->data + copy_len, '.', 5);
    
    skip_adjust:
    node->size = copy_len;
    
    /* Recursive creation with different memory patterns */
    char left_data[64];
    __builtin_memcpy(left_data, base_data, copy_len);
    __builtin_memcpy(left_data + copy_len, "_L", 3);
    
    char right_data[64];
    __builtin_memset(right_data, 0, sizeof(right_data));
    __builtin_memcpy(right_data, base_data, copy_len);
    __builtin_memcpy(right_data + copy_len, "_R", 3);
    
    node->left = create_ast(depth - 1, left_data);
    node->right = create_ast(depth - 1, right_data);
    
    return node;
}

/* Function with complex memory movement patterns */
static void transform_ast(ASTNode* src, ASTNode* dest) {
    if (!src || !dest) return;
    
    /* Use __builtin_memmove for overlapping regions */
    volatile char buffer[128];
    size_t move_size = src->size < sizeof(buffer) ? src->size : sizeof(buffer);
    
    /* Copy to buffer */
    __builtin_memcpy(buffer, src->data, move_size);
    
    /* Move with potential overlap */
    __builtin_memmove(dest->data, buffer, move_size);
    
    /* Transform data in place */
    for (size_t i = 0; i < move_size && i < sizeof(dest->data); i++) {
        dest->data[i] = src->data[i] ^ 0x20; /* Toggle case */
    }
    
    /* Recursive transformation */
    if (src->left && dest->left) {
        transform_ast(src->left, dest->left);
    }
    if (src->right && dest->right) {
        transform_ast(src->right, dest->right);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    const int num_threads = 4;
    char* buffers[num_threads];
    size_t sizes[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread allocates and operates on memory */
        sizes[tid] = g_mem_size / (tid + 1);
        buffers[tid] = (char*)malloc(sizes[tid]);
        
        if (buffers[tid]) {
            /* Pattern initialization */
            __builtin_memset(buffers[tid], tid + 'A', sizes[tid]);
            
            /* Circular shift pattern between threads */
            int next_tid = (tid + 1) % num_threads;
            #pragma omp barrier
            
            if (next_tid < num_threads && buffers[next_tid]) {
                size_t copy_size = sizes[tid] < sizes[next_tid] ? 
                                  sizes[tid] : sizes[next_tid];
                __builtin_memcpy(buffers[next_tid], buffers[tid], copy_size);
            }
        }
        
        #pragma omp barrier
        
        /* Cleanup */
        if (buffers[tid]) {
            free(buffers[tid]);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Basic built-in usage */
    char buffer1[512];
    char buffer2[512];
    
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 100, buffer1, 200);
    
    /* Phase 2: Recursive AST operations */
    ASTNode* ast1 = create_ast(4, "RootNode");
    ASTNode* ast2 = create_ast(4, "TargetNode");
    
    if (ast1 && ast2) {
        transform_ast(ast1, ast2);
        
        /* Verify transformation */
        size_t hash = 0;
        for (size_t i = 0; i < sizeof(ast2->data); i++) {
            hash = (hash * 31) + ast2->data[i];
        }
        printf("AST hash: %zu\n", hash);
    }
    
    /* Phase 3: OpenMP parallel operations */
    parallel_memory_ops();
    
    /* Phase 4: Volatile-controlled operations */
    volatile size_t dynamic_size = g_mem_size;
    char* dyn_buf1 = (char*)malloc(dynamic_size);
    char* dyn_buf2 = (char*)malloc(dynamic_size);
    
    if (dyn_buf1 && dyn_buf2) {
        /* Force compiler to keep these calls */
        __builtin_memset(dyn_buf1, 0xCC, dynamic_size);
        __builtin_memcpy(dyn_buf2, dyn_buf1, dynamic_size);
        
        /* Overlapping move */
        size_t overlap = dynamic_size / 2;
        __builtin_memmove(dyn_buf1 + overlap, dyn_buf1, overlap);
        
        free(dyn_buf1);
        free(dyn_buf2);
    }
    
    /* Phase 5: Cleanup */
    /* Helper function to free AST */
    void free_ast(ASTNode* node) {
        if (!node) return;
        free_ast(node->left);
        free_ast(node->right);
        free(node);
    }
    
    free_ast(ast1);
    free_ast(ast2);
    
    printf("Test completed successfully\n");
    return 0;
}
