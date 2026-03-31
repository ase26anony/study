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
    int id;
} ASTNode;

/* Global token array for initialization */
static char g_token_array[1024];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Initialize token array with pattern */
    for (int i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)((i * 31) & 0xFF);
    }
    
    /* Force early built-in usage in constructor */
    volatile char local_buf[128];
    __builtin_memset(local_buf, 0xAA, sizeof(local_buf));
    __builtin_memcpy(&g_token_array[512], local_buf, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    /* Final memory operation in destructor */
    volatile char final_buf[32];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with built-ins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in data */
    volatile char pattern[32];
    __builtin_memset(pattern, (char)id, sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(pattern));
    
    node->id = id;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 3) {
        goto skip_left;
    }
    
    create_left = 0;
    
skip_left:
    if (create_left) {
        node->left = create_ast_node(depth - 1, id * 2);
        
        /* Copy between nodes if both exist */
        if (node->left) {
            __builtin_memmove(node->data + 32, node->left->data, 16);
        }
    } else {
        node->left = NULL;
    }
    
    /* Jump back into normal flow */
    goto create_right;
    
    /* Unreachable code that might confuse flow analysis */
    __builtin_memset(node->data, 0xDD, 8);
    
create_right:
    node->right = create_ast_node(depth - 2, id * 2 + 1);
    
    return node;
}

/* Complex memory operation with goto edge cases */
static void perform_memory_operations(void) {
    volatile char src_buf[512];
    volatile char dst_buf[512];
    volatile char temp_buf[256];
    
    /* Initialize source with pattern */
    for (size_t i = 0; i < sizeof(src_buf); i++) {
        src_buf[i] = (char)(i & 0xFF);
    }
    
    /* Test case 1: Direct built-in calls */
    __builtin_memcpy(dst_buf, src_buf, g_mem_size);
    
    /* Test case 2: Overlapping memory with memmove */
    __builtin_memmove(dst_buf + 128, dst_buf, 192);
    
    /* Test case 3: memset with volatile size */
    volatile size_t set_size = g_mem_size / 2;
    __builtin_memset(temp_buf, 0xCC, set_size);
    
    /* Goto jumping into memory operation block */
    int use_goto = 1;
    
    if (use_goto) {
        goto jump_into_memop;
    }
    
    /* This should be skipped */
    __builtin_memset(temp_buf, 0xEE, 64);
    
jump_into_memop:
    /* Complex memcpy after goto */
    __builtin_memcpy(dst_buf + 256, temp_buf, 128);
    
    /* Jump out of block */
    if (set_size > 100) {
        goto finish_ops;
    }
    
    /* More operations that might be skipped */
    __builtin_memmove(src_buf, dst_buf, 64);
    
finish_ops:
    /* Final operation after goto */
    __builtin_memset(dst_buf + 384, 0xAA, 64);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_dispatch(void) {
    volatile char shared_buf[1024];
    volatile char thread_buf[256];
    
    /* Initialize shared buffer */
    __builtin_memset(shared_buf, 0, sizeof(shared_buf));
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local memory operations */
        __builtin_memset(thread_buf, (char)thread_id, sizeof(thread_buf));
        
        /* Copy to shared buffer with offset */
        size_t offset = (thread_id * 128) % 768;
        __builtin_memcpy(shared_buf + offset, thread_buf, 128);
        
        /* Move data within shared buffer */
        if (thread_id % 2 == 0) {
            __builtin_memmove(shared_buf + offset + 64, 
                            shared_buf + offset, 64);
        }
    }
    
    /* Verify by hashing shared buffer */
    unsigned long hash = 0;
    for (int i = 0; i < 1024; i++) {
        hash = hash * 31 + (unsigned char)shared_buf[i];
    }
    printf("Shared buffer hash: %lu\n", hash);
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = create_ast_node(5, 1);
    
    if (root) {
        /* Copy between tree nodes */
        if (root->left && root->right) {
            __builtin_memcpy(root->right->data + 16, 
                           root->left->data + 8, 24);
        }
        
        /* Move within node */
        __builtin_memmove(root->data + 8, root->data, 32);
    }
    
    /* Phase 2: Complex memory operations with goto */
    perform_memory_operations();
    
    /* Phase 3: OpenMP parallel operations */
    #ifdef _OPENMP
    parallel_memory_dispatch();
    #endif
    
    /* Phase 4: Token array processing */
    volatile char result_buf[256];
    size_t process_size = g_mem_size;
    if (process_size > sizeof(result_buf)) {
        process_size = sizeof(result_buf);
    }
    
    __builtin_memcpy(result_buf, g_token_array, process_size);
    __builtin_memset(result_buf + 128, g_token_array[g_token_index], 64);
    
    /* Calculate verification sum */
    unsigned long sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += (unsigned char)result_buf[i];
    }
    
    printf("Verification sum: %lu\n", sum);
    printf("Test completed.\n");
    
    /* Cleanup */
    /* Note: In real code, you'd want to free the AST recursively */
    
    return 0;
}
