/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int g_volatile_size = 64;
static volatile char g_volatile_char = 'A';

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    volatile char buffer[16];
    /* Force __builtin_memset in constructor */
    __builtin_memset(buffer, g_volatile_char, sizeof(buffer));
    printf("Constructor: Initialized buffer with %c\n", buffer[0]);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    volatile int cleanup_buf[8];
    /* Force __builtin_memcpy in destructor */
    int src[] = {1, 2, 3, 4, 5, 6, 7, 8};
    __builtin_memcpy(cleanup_buf, src, sizeof(src));
    printf("Destructor: Cleanup completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    node->left = NULL;
    node->right = NULL;
    
    /* Initialize node data with __builtin_memset */
    __builtin_memset(node->data, '0' + (depth % 10), sizeof(node->data));
    
    /* Create children recursively */
    node->left = create_ast(depth - 1, counter);
    node->right = create_ast(depth - 1, counter);
    
    /* Copy data between nodes if children exist */
    if (node->left && node->right) {
        /* Use __builtin_memcpy with goto for flow control */
        goto copy_block;
        
        copy_block:
        __builtin_memcpy(node->right->data, node->left->data, 
                        sizeof(node->data) / 2);
        /* Jump out of the block */
        goto after_copy;
        
        after_copy:;
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void test_goto_memmove(void) {
    volatile char src[128], dst[128];
    int use_memmove = 1;
    
    /* Initialize source with pattern */
    for (int i = 0; i < sizeof(src); i++) {
        src[i] = (char)(i % 256);
    }
    
    if (use_memmove) {
        goto perform_memmove;
    }
    
    /* This label is jumped into */
    perform_memmove:
    /* Force __builtin_memmove with overlapping regions */
    __builtin_memmove(dst, src, g_volatile_size);
    
    /* Jump out to different code path */
    goto after_operation;
    
    after_operation:
    /* Verify the copy */
    int sum = 0;
    for (int i = 0; i < g_volatile_size; i++) {
        sum += dst[i];
    }
    printf("Goto test sum: %d\n", sum);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    volatile int results[4] = {0};
    
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        volatile char thread_buffer[256];
        volatile char thread_src[256];
        
        /* Initialize thread-specific source */
        for (int i = 0; i < sizeof(thread_src); i++) {
            thread_src[i] = (char)((tid * 64 + i) % 256);
        }
        
        /* Force __builtin_memcpy in parallel region */
        __builtin_memcpy(thread_buffer, thread_src, 
                        g_volatile_size + tid * 16);
        
        /* Compute thread result */
        int local_sum = 0;
        for (int i = 0; i < g_volatile_size; i++) {
            local_sum += thread_buffer[i];
        }
        results[tid] = local_sum;
    }
    
    /* Combine results */
    int total = 0;
    for (int i = 0; i < 4; i++) {
        total += results[i];
    }
    printf("Parallel total: %d\n", total);
}

/* Complex token parsing with memory operations */
static int parse_tokens(const char** tokens, int count) {
    volatile char parse_buffer[512];
    int buffer_pos = 0;
    
    for (int i = 0; i < count; i++) {
        const char* token = tokens[i];
        size_t token_len = strlen(token);
        
        /* Use __builtin_memcpy for token copying */
        if (buffer_pos + token_len < sizeof(parse_buffer)) {
            __builtin_memcpy(&parse_buffer[buffer_pos], 
                           token, token_len);
            buffer_pos += token_len;
            
            /* Add separator */
            if (i < count - 1) {
                parse_buffer[buffer_pos++] = '|';
            }
        }
    }
    
    /* Compute hash of parsed buffer */
    int hash = 0;
    for (int i = 0; i < buffer_pos; i++) {
        hash = (hash * 31 + parse_buffer[i]) & 0x7FFFFFFF;
    }
    return hash;
}

/* Main test driver */
int main(void) {
    printf("=== ASAN/HWASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Basic built-in calls */
    volatile char buffer1[256], buffer2[256];
    
    /* Force all three built-ins */
    __builtin_memset(buffer1, g_volatile_char, g_volatile_size);
    __builtin_memcpy(buffer2, buffer1, g_volatile_size);
    __builtin_memmove(buffer1, buffer2, g_volatile_size / 2);
    
    /* Phase 2: Recursive AST operations */
    int counter = 0;
    ASTNode* root = create_ast(3, &counter);
    
    if (root) {
        /* Copy between tree nodes */
        if (root->left && root->right) {
            __builtin_memcpy(root->data, root->left->data, 16);
            __builtin_memcpy(root->right->data, root->data, 16);
        }
        
        /* Cleanup */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    /* Phase 3: Goto flow control test */
    test_goto_memmove();
    
    /* Phase 4: OpenMP parallel operations */
    parallel_memory_ops();
    
    /* Phase 5: Token parsing */
    const char* tokens[] = {
        "memcpy", "memset", "memmove", 
        "asan", "hwasan", "coverage", "test"
    };
    int token_hash = parse_tokens(tokens, 
                                 sizeof(tokens)/sizeof(tokens[0]));
    printf("Token hash: %d\n", token_hash);
    
    /* Phase 6: Variable-sized operations */
    for (int i = 1; i <= 4; i++) {
        volatile int size = g_volatile_size * i;
        volatile char dyn_buf1[size], dyn_buf2[size];
        
        __builtin_memset(dyn_buf1, i, size);
        __builtin_memcpy(dyn_buf2, dyn_buf1, size);
        
        /* Partial overlap memmove */
        if (size > 32) {
            __builtin_memmove(dyn_buf1 + 16, dyn_buf1, size - 32);
        }
    }
    
    printf("=== Test completed successfully ===\n");
    return 0;
}
