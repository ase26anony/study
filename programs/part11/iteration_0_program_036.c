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

/* Global token array */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "test"
};
static const int g_token_count = 6;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_sanitizer_hook(void) {
    volatile char buffer[32];
    /* Force initialization of memcpy redirection */
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, "constructor", 11);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_sanitizer_hook(void) {
    volatile char cleanup_buf[16];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Copy token data using memcpy */
    const char* token = g_tokens[id % g_token_count];
    size_t len = __builtin_strlen(token);
    if (len > sizeof(node->data) - 1)
        len = sizeof(node->data) - 1;
    
    __builtin_memcpy(node->data, token, len);
    node->data[len] = '\0';
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    if (depth > 3) {
        create_left = 0;
        goto skip_left;
    }
    
    node->left = create_ast_node(depth - 1, id * 2);
    
skip_left:
    node->right = create_ast_node(depth - 2, id * 2 + 1);
    
    return node;
}

/* Complex memory operation with goto jumps */
static void perform_memory_operations(volatile char* dest, volatile char* src) {
    volatile char temp[128];
    volatile int use_memmove = 0;
    
    /* First memcpy */
    __builtin_memcpy((void*)dest, (void*)src, g_mem_size);
    
    if (g_use_hwasan) {
        use_memmove = 1;
        goto use_memmove_block;
    }
    
    /* Regular memset */
    __builtin_memset((void*)temp, 0xAA, sizeof(temp));
    
    /* Jump into memmove block */
    if (dest[0] > 0) {
        goto memmove_entry;
    }
    
use_memmove_block:
    /* This tests the memmove redirection */
    __builtin_memmove((void*)dest, (void*)temp, 64);
    goto after_memmove;
    
memmove_entry:
    /* Alternative memmove path */
    __builtin_memmove((void*)temp, (void*)dest, 32);
    
after_memmove:
    /* Final memcpy with overlapping regions */
    __builtin_memcpy((void*)dest, (void*)temp, 48);
}

/* Parallel memory dispatch */
static unsigned long parallel_memory_dispatch(void) {
    unsigned long hash = 0;
    volatile char buffers[4][256];
    
    #pragma omp parallel reduction(+:hash)
    {
        int tid = omp_get_thread_num();
        volatile char* buf = buffers[tid];
        
        /* Initialize buffer with memset */
        __builtin_memset((void*)buf, tid, g_mem_size);
        
        /* Copy between buffers using memcpy */
        int src_tid = (tid + 1) % 4;
        __builtin_memcpy((void*)buf, (void*)buffers[src_tid], 128);
        
        /* Compute simple hash */
        for (int i = 0; i < 64; i++) {
            hash += buf[i];
        }
        
        /* Thread-specific memmove */
        if (tid % 2 == 0) {
            __builtin_memmove((void*)(buf + 64), (void*)buf, 64);
        }
    }
    
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = create_ast_node(5, 1);
    
    if (root) {
        /* Copy between AST nodes */
        ASTNode temp_node;
        __builtin_memcpy(&temp_node, root, sizeof(ASTNode));
        __builtin_memcpy(root->left, &temp_node, sizeof(ASTNode));
        
        /* Cleanup */
        free(root->right);
        free(root->left);
        free(root);
    }
    
    /* Phase 2: Volatile memory operations */
    volatile char src[512];
    volatile char dest[512];
    
    /* Initialize source with pattern */
    for (size_t i = 0; i < sizeof(src); i++) {
        src[i] = (char)(i % 256);
    }
    
    /* Perform complex memory operations with goto */
    perform_memory_operations(dest, src);
    
    /* Phase 3: OpenMP parallel operations */
    unsigned long result_hash = parallel_memory_dispatch();
    
    /* Phase 4: Direct built-in calls in varied contexts */
    volatile int* int_array = (volatile int*)malloc(100 * sizeof(int));
    if (int_array) {
        /* memset to zero */
        __builtin_memset((void*)int_array, 0, 100 * sizeof(int));
        
        /* memcpy from source */
        int src_ints[100];
        for (int i = 0; i < 100; i++) src_ints[i] = i;
        __builtin_memcpy((void*)int_array, src_ints, 100 * sizeof(int));
        
        /* memmove with overlap */
        __builtin_memmove((void*)(int_array + 10), int_array, 90 * sizeof(int));
        
        free((void*)int_array);
    }
    
    /* Verify operations by computing checksum */
    unsigned long checksum = 0;
    for (size_t i = 0; i < 256; i++) {
        checksum += dest[i];
    }
    checksum += result_hash;
    
    printf("Test completed. Checksum: %lu\n", checksum);
    printf("If compiled with -fsanitize=address or -fsanitize=kernel-hwaddress,\n");
    printf("the ASAN/HWASAN built-in redirection logic should be triggered.\n");
    
    return 0;
}
