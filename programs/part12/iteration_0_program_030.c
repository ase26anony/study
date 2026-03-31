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
static void init_asan_environment(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (i % 26) + 'a';
    }
    printf("[Constructor] Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("[Destructor] ASAN test completed\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->left = NULL;
    node->right = NULL;
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy pattern into node using __builtin_memcpy */
    char pattern[64];
    for (int i = 0; i < 63; i++) {
        pattern[i] = 'A' + (id + i) % 26;
    }
    pattern[63] = '\0';
    
    __builtin_memcpy(node->data, pattern, 
                    volatile_flag ? 63 : 64); /* Volatile control */
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int left_id = id * 2;
        int right_id = id * 2 + 1;
        
        /* Jump label for goto testing */
        create_left:
        node->left = create_ast_node(depth - 1, left_id);
        
        /* Use __builtin_memmove between nodes if they exist */
        if (node->left && depth > 2) {
            ASTNode temp;
            __builtin_memcpy(&temp, node->left, sizeof(ASTNode));
            __builtin_memmove(node->left->data, node->data, 32);
            __builtin_memcpy(node->data, temp.data, 32);
        }
        
        if (depth % 2 == 0) {
            /* Skip right creation in some cases */
            goto skip_right;
        }
        
        node->right = create_ast_node(depth - 1, right_id);
        skip_right:;
    }
    
    return node;
}

/* Parallel memory dispatch logic */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char src_buf[128];
        char dst_buf[128];
        
        /* Initialize with __builtin_memset */
        __builtin_memset(src_buf, thread_id + '0', sizeof(src_buf));
        
        /* Copy with __builtin_memcpy */
        __builtin_memcpy(dst_buf, src_buf, 
                        volatile_len > 128 ? 128 : volatile_len);
        
        /* Move data around with __builtin_memmove */
        if (thread_id % 2 == 0) {
            __builtin_memmove(src_buf + 32, src_buf, 64);
        }
        
        /* Complex pattern with goto */
        if (thread_id == 0) {
            goto parallel_cleanup;
        }
        
        /* Additional memory operations */
        for (int i = 0; i < 4; i++) {
            __builtin_memset(dst_buf + i * 16, 'X', 16);
        }
        
        parallel_cleanup:
        /* Final memcpy with volatile length */
        int copy_len = volatile_len % 128;
        if (copy_len > 0) {
            __builtin_memcpy(src_buf, dst_buf, copy_len);
        }
    }
}

/* Function with goto jumping into memory operation block */
static void goto_memory_test(void) {
    char buffer_a[256];
    char buffer_b[256];
    
    /* Initialize buffers */
    for (int i = 0; i < 256; i++) {
        buffer_a[i] = i % 256;
        buffer_b[i] = 255 - (i % 256);
    }
    
    int use_memmove = volatile_flag;
    
    if (use_memmove) {
        goto perform_memmove;
    }
    
    /* Regular memcpy path */
    __builtin_memcpy(buffer_a, buffer_b, 128);
    goto test_complete;
    
    perform_memmove:
    /* This tests the goto into memmove block */
    __builtin_memmove(buffer_a + 64, buffer_a, 128);
    
    test_complete:
    /* Verify by copying back */
    __builtin_memcpy(buffer_b, buffer_a, 64);
}

/* Calculate hash from AST tree */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* data = node->data;
    
    /* Simple hash computation */
    for (int i = 0; i < 64 && data[i]; i++) {
        hash = ((hash << 5) + hash) + data[i];
    }
    
    unsigned long left_hash = compute_ast_hash(node->left);
    unsigned long right_hash = compute_ast_hash(node->right);
    
    return hash ^ (left_hash << 1) ^ (right_hash >> 1);
}

int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Create and populate AST */
    printf("Phase 1: Creating AST structure...\n");
    ASTNode* root = create_ast_node(4, 1);
    
    /* Phase 2: Parallel memory operations */
    printf("Phase 2: Parallel memory dispatch...\n");
    parallel_memory_operations();
    
    /* Phase 3: Goto memory test */
    printf("Phase 3: Goto memory operation test...\n");
    goto_memory_test();
    
    /* Phase 4: Token pool operations */
    printf("Phase 4: Token pool manipulations...\n");
    char temp_buffer[512];
    
    /* Multiple built-in calls with volatile control */
    for (int i = 0; i < 8; i++) {
        int offset = (i * 128) % 4096;
        int length = (volatile_len + i * 16) % 256;
        
        __builtin_memcpy(temp_buffer, token_pool + offset, length);
        __builtin_memset(token_pool + offset, 'Z', length);
        __builtin_memmove(token_pool + offset + 64, token_pool + offset, length);
    }
    
    /* Phase 5: Compute and verify result */
    printf("Phase 5: Computing verification hash...\n");
    unsigned long final_hash = compute_ast_hash(root);
    
    /* Include token pool in hash */
    for (int i = 0; i < 1024; i += 64) {
        final_hash ^= *(unsigned long*)(token_pool + i);
    }
    
    printf("Final verification hash: 0x%016lx\n", final_hash);
    
    /* Cleanup */
    /* Note: In real ASAN, memory would be freed here */
    printf("Test completed successfully\n");
    
    return 0;
}
