/* asan_coverage.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_token_pool[4096];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token pool with pattern */
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i % 26) + 'A');
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    /* Use __builtin_memset to initialize data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy pattern from token pool */
    size_t copy_len = (id % 32) + 16;
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    
    __builtin_memcpy(node->data, &g_token_pool[g_token_index], copy_len);
    g_token_index = (g_token_index + copy_len) % sizeof(g_token_pool);
    
    return node;
}

/* Function with goto and memory operations */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int use_memmove = g_use_memmove;
    char buffer[128];
    
    /* Jump into block with memmove */
    if (use_memmove) goto use_memmove_block;
    
    /* Normal path with memcpy */
    __builtin_memcpy(buffer, src->data, sizeof(buffer));
    goto after_copy;
    
use_memmove_block:
    /* This tests flow-sensitivity */
    __builtin_memmove(buffer, src->data, sizeof(buffer));
    
after_copy:
    /* Copy to destination */
    if (dst) {
        __builtin_memcpy(dst->data, buffer, sizeof(dst->data));
    }
}

/* Parallel processing function */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf1[256];
        char local_buf2[256];
        
        /* Initialize with memset */
        __builtin_memset(local_buf1, thread_id, sizeof(local_buf1));
        
        /* Copy between buffers */
        __builtin_memcpy(local_buf2, local_buf1, sizeof(local_buf1));
        
        /* Conditional memmove */
        if (thread_id % 2 == 0) {
            __builtin_memmove(local_buf1 + 64, local_buf1, 128);
        }
        
        /* Verify copy */
        int errors = 0;
        for (size_t i = 0; i < sizeof(local_buf1); i++) {
            if (local_buf1[i] != local_buf2[i]) errors++;
        }
        
        #pragma omp critical
        {
            printf("Thread %d: Memory ops completed, errors=%d\n", 
                   thread_id, errors);
        }
    }
}

/* Complex memory operation sequence */
static void complex_memory_sequence(void) {
    /* Dynamic allocation with volatile size */
    size_t size = g_mem_size;
    char* dyn_buf1 = (char*)malloc(size);
    char* dyn_buf2 = (char*)malloc(size);
    
    if (!dyn_buf1 || !dyn_buf2) {
        free(dyn_buf1);
        free(dyn_buf2);
        return;
    }
    
    /* Initialize sequence */
    __builtin_memset(dyn_buf1, 0xAA, size);
    __builtin_memset(dyn_buf2, 0xBB, size);
    
    /* Overlapping copy with memmove */
    size_t overlap = size / 2;
    __builtin_memmove(dyn_buf1 + overlap, dyn_buf1, size - overlap);
    
    /* Cross-copy */
    __builtin_memcpy(dyn_buf2, dyn_buf1, size);
    __builtin_memcpy(dyn_buf1, dyn_buf2, size);
    
    /* Verify with checksum */
    unsigned long long sum1 = 0, sum2 = 0;
    for (size_t i = 0; i < size; i++) {
        sum1 += (unsigned char)dyn_buf1[i];
        sum2 += (unsigned char)dyn_buf2[i];
    }
    
    printf("Checksums: dyn_buf1=%llu, dyn_buf2=%llu\n", sum1, sum2);
    
    free(dyn_buf1);
    free(dyn_buf2);
}

/* Main test driver */
int main(void) {
    printf("=== ASAN/HWASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Create recursive structures */
    printf("\nPhase 1: Creating AST structures...\n");
    ASTNode* root = create_ast(4, 1);
    ASTNode* copy = create_ast(4, 100);
    
    if (!root || !copy) {
        printf("Failed to create AST structures\n");
        return 1;
    }
    
    /* Phase 2: Goto-based memory operations */
    printf("\nPhase 2: Goto-based memory operations...\n");
    process_with_goto(root, copy);
    
    /* Toggle flag and repeat */
    g_use_memmove = 0;
    process_with_goto(copy, root);
    g_use_memmove = 1;
    
    /* Phase 3: Parallel operations */
    printf("\nPhase 3: Parallel memory operations...\n");
    parallel_memory_ops();
    
    /* Phase 4: Complex sequence */
    printf("\nPhase 4: Complex memory sequence...\n");
    complex_memory_sequence();
    
    /* Phase 5: Direct built-in calls with varying sizes */
    printf("\nPhase 5: Direct built-in calls...\n");
    {
        char small_buf[32];
        char medium_buf[256];
        char large_buf[1024];
        
        /* Small memset */
        __builtin_memset(small_buf, 0xCC, sizeof(small_buf));
        
        /* Medium memcpy */
        __builtin_memcpy(medium_buf, small_buf, 
                        sizeof(small_buf) < sizeof(medium_buf) ? 
                        sizeof(small_buf) : sizeof(medium_buf));
        
        /* Large memmove with overlap */
        __builtin_memset(large_buf, 0xDD, sizeof(large_buf));
        __builtin_memmove(large_buf + 512, large_buf, 512);
        
        /* Verify final state */
        int final_check = 0;
        for (size_t i = 0; i < sizeof(small_buf); i++) {
            final_check += (unsigned char)small_buf[i];
        }
        printf("Final check value: %d\n", final_check);
    }
    
    /* Cleanup */
    /* Note: In real code, you'd need to free the AST recursively */
    
    printf("\n=== Test completed successfully ===\n");
    return 0;
}
