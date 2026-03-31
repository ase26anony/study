/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 16;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* AST-like recursive structure */
typedef struct ASTNode {
    int type;
    char data[32];
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Global token array */
static const char *tokens[] = {
    "memcpy", "memset", "memmove", "test", "data", "asan", "hwasan"
};
static const int token_count = sizeof(tokens) / sizeof(tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_test(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < 256; i++) {
        volatile_src[i] = (char)(i % 256);
    }
    printf("Constructor: Initialized ASAN test environment\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Recursive AST creation */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = depth;
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy token into node data using __builtin_memcpy */
    const char *token = tokens[depth % token_count];
    size_t len = strlen(token);
    if (len > sizeof(node->data) - 1) len = sizeof(node->data) - 1;
    __builtin_memcpy(node->data, token, len);
    node->data[len] = '\0';
    
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    return node;
}

/* Recursive AST traversal with memory operations */
static int traverse_ast(ASTNode *node, char *buffer, int offset) {
    if (!node) return offset;
    
    int new_offset = offset;
    
    /* Copy node data to buffer using __builtin_memcpy */
    if (node->data[0]) {
        size_t len = strlen(node->data);
        __builtin_memcpy(buffer + new_offset, node->data, len);
        new_offset += len;
        buffer[new_offset++] = ' ';
    }
    
    /* Process children */
    new_offset = traverse_ast(node->left, buffer, new_offset);
    new_offset = traverse_ast(node->right, buffer, new_offset);
    
    return new_offset;
}

/* Function with goto for flow control testing */
static void test_goto_memmove(void) {
    char buffer1[64];
    char buffer2[64];
    int use_memmove = 0;
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 'A', sizeof(buffer1));
    __builtin_memset(buffer2, 'B', sizeof(buffer2));
    
    goto label1;
    
skip_memmove:
    /* This should be skipped on first pass */
    return;
    
label1:
    if (use_memmove) {
        /* Jump back to use memmove */
        goto perform_memmove;
    }
    
    /* First pass: use memcpy */
    __builtin_memcpy(buffer2, buffer1, 32);
    use_memmove = 1;
    goto label1;  /* Jump back to test different path */
    
perform_memmove:
    /* Second pass: use memmove with overlapping regions */
    __builtin_memmove(buffer1 + 16, buffer1, 32);
    goto skip_memmove;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    const int array_size = 1024;
    char *src_array = (char*)malloc(array_size);
    char *dest_array = (char*)malloc(array_size);
    
    if (!src_array || !dest_array) {
        free(src_array);
        free(dest_array);
        return;
    }
    
    /* Initialize source array */
    for (int i = 0; i < array_size; i++) {
        src_array[i] = (char)(i % 256);
    }
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        int chunk_size = array_size / 4;
        int start = thread_id * chunk_size;
        
        if (thread_id % 3 == 0) {
            /* Use __builtin_memcpy */
            __builtin_memcpy(dest_array + start, src_array + start, chunk_size);
        } else if (thread_id % 3 == 1) {
            /* Use __builtin_memset */
            __builtin_memset(dest_array + start, thread_id, chunk_size);
        } else {
            /* Use __builtin_memmove with overlap */
            int overlap_start = start - (chunk_size / 2);
            if (overlap_start < 0) overlap_start = 0;
            __builtin_memmove(dest_array + overlap_start, 
                            src_array + overlap_start, 
                            chunk_size);
        }
    }
    
    /* Verify with volatile operations */
    volatile char verify;
    for (int i = 0; i < array_size; i += 64) {
        verify = dest_array[i];
    }
    
    free(src_array);
    free(dest_array);
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Test 1: Basic built-in calls with volatile */
    {
        int len = volatile_len;
        __builtin_memset(volatile_dest, 0x42, len);
        __builtin_memcpy((char*)volatile_dest + 8, volatile_src, len);
        __builtin_memmove((char*)volatile_dest + 4, volatile_dest, len);
    }
    
    /* Test 2: AST operations */
    ASTNode *root = create_ast(4);
    if (root) {
        char ast_buffer[1024];
        int filled = traverse_ast(root, ast_buffer, 0);
        ast_buffer[filled] = '\0';
        
        /* Use built-ins on AST data */
        char copy_buffer[1024];
        __builtin_memcpy(copy_buffer, ast_buffer, filled);
        __builtin_memset(ast_buffer + filled / 2, '*', 32);
        __builtin_memmove(ast_buffer, ast_buffer + 16, filled - 16);
        
        /* Free AST recursively */
        /* (In real code, you'd want a proper free function) */
        free(root);
    }
    
    /* Test 3: Goto flow control */
    test_goto_memmove();
    
    /* Test 4: OpenMP parallel operations */
    parallel_memory_ops();
    
    /* Test 5: Variable length built-in calls */
    for (int i = 8; i <= 128; i *= 2) {
        char buf1[256], buf2[256];
        __builtin_memset(buf1, i, i);
        __builtin_memcpy(buf2, buf1, i);
        __builtin_memmove(buf1 + i/2, buf1, i/2);
    }
    
    /* Final verification */
    volatile int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += volatile_dest[i];
    }
    
    printf("Test completed. Checksum: %d\n", checksum);
    return 0;
}
