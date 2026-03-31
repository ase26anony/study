/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* __attribute__((constructor)) function */
static void __attribute__((constructor)) init_globals(void) {
    g_init_flag = 1;
    printf("Constructor: Global initialization complete\n");
}

/* __attribute__((destructor)) function */
static void __attribute__((destructor)) cleanup_globals(void) {
    printf("Destructor: Cleaning up\n");
}

/* Function with goto statements for flow control */
static void memcpy_with_goto(char* dest, const char* src, size_t n) {
    if (n == 0) goto end;
    
    /* Jump into memory operation block */
    goto copy_block;
    
copy_block:
    /* Force builtin memcpy usage */
    __builtin_memcpy(dest, src, n);
    
    /* Jump out of block */
    goto end;
    
end:
    return;
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_tree(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Create pattern in data */
    for (size_t i = 0; i < sizeof(node->data) - 1; i++) {
        node->data[i] = 'A' + (depth % 26);
    }
    node->data[sizeof(node->data) - 1] = '\0';
    node->size = sizeof(node->data);
    
    /* Recursive creation */
    node->left = create_tree(depth - 1);
    node->right = create_tree(depth - 1);
    
    return node;
}

/* Copy between tree nodes using memmove */
static void copy_tree_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use memmove for overlapping memory consideration */
    __builtin_memmove(dest->data, src->data, 
                     dest->size < src->size ? dest->size : src->size);
    
    /* Recursive copy */
    copy_tree_data(dest->left, src->left);
    copy_tree_data(dest->right, src->right);
}

/* Complex token processing */
static unsigned long process_tokens(char** tokens, size_t count) {
    unsigned long hash = 5381;
    char buffer[512];
    
    for (size_t i = 0; i < count; i++) {
        /* Clear buffer with memset */
        __builtin_memset(buffer, 0, sizeof(buffer));
        
        /* Copy token with memcpy */
        size_t len = strlen(tokens[i]);
        if (len >= sizeof(buffer)) len = sizeof(buffer) - 1;
        __builtin_memcpy(buffer, tokens[i], len);
        
        /* DJB2 hash algorithm */
        for (size_t j = 0; j < len; j++) {
            hash = ((hash << 5) + hash) + buffer[j];
        }
    }
    
    return hash;
}

/* Main test function with OpenMP parallel section */
static void parallel_memory_operations(void) {
    const size_t array_size = 1024;
    char* src_array = (char*)malloc(array_size);
    char* dest_array = (char*)malloc(array_size);
    
    if (!src_array || !dest_array) {
        free(src_array);
        free(dest_array);
        return;
    }
    
    /* Initialize source with pattern */
    for (size_t i = 0; i < array_size; i++) {
        src_array[i] = (char)(i % 256);
    }
    
    /* OpenMP parallel region */
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        size_t chunk_size = array_size / omp_get_num_threads();
        size_t start = thread_id * chunk_size;
        
        /* Each thread performs memory operations */
        __builtin_memcpy(dest_array + start, src_array + start, chunk_size);
        
        /* Verify with memcmp */
        if (__builtin_memcmp(dest_array + start, src_array + start, chunk_size) != 0) {
            #pragma omp critical
            printf("Thread %d: Memory verification failed!\n", thread_id);
        }
    }
    
    /* Final memmove for overlapping regions */
    size_t overlap_size = g_mem_size;
    if (overlap_size > array_size / 2) overlap_size = array_size / 2;
    
    __builtin_memmove(dest_array, dest_array + overlap_size, array_size - overlap_size);
    
    free(src_array);
    free(dest_array);
}

/* Function with switch-based dispatch */
static void dispatch_memory_operation(int op_type, void* dest, const void* src, size_t n) {
    switch (op_type) {
        case 0:
            __builtin_memcpy(dest, src, n);
            break;
        case 1:
            __builtin_memset(dest, 0xAA, n);
            break;
        case 2:
            __builtin_memmove(dest, src, n);
            break;
        default:
            break;
    }
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Test 1: Basic built-in calls */
    char buffer1[256];
    char buffer2[256];
    
    __builtin_memset(buffer1, 'X', sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 128, buffer1, 128);
    
    /* Test 2: Goto flow control */
    memcpy_with_goto(buffer2, buffer1, 64);
    
    /* Test 3: Recursive AST structures */
    ASTNode* tree1 = create_tree(3);
    ASTNode* tree2 = create_tree(3);
    
    if (tree1 && tree2) {
        copy_tree_data(tree2, tree1);
        
        /* Free trees */
        free(tree1->left);
        free(tree1->right);
        free(tree1);
        free(tree2->left);
        free(tree2->right);
        free(tree2);
    }
    
    /* Test 4: Token processing */
    char* tokens[] = {"test1", "test2", "test3", "asan", "hwasan", "coverage"};
    size_t token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    unsigned long hash = process_tokens(tokens, token_count);
    printf("Token hash: %lu\n", hash);
    
    /* Test 5: Parallel operations */
    parallel_memory_operations();
    
    /* Test 6: Dispatch with volatile control */
    volatile int op_type = 0;
    char test_buf[100];
    
    for (op_type = 0; op_type < 3; op_type++) {
        dispatch_memory_operation(op_type, test_buf, buffer1, 
                                 g_mem_size < sizeof(test_buf) ? g_mem_size : sizeof(test_buf));
    }
    
    /* Test 7: Variable length operations */
    volatile size_t dynamic_size = 50;
    char dyn_buf1[100];
    char dyn_buf2[100];
    
    __builtin_memset(dyn_buf1, 'Z', dynamic_size);
    __builtin_memcpy(dyn_buf2, dyn_buf1, dynamic_size);
    __builtin_memmove(dyn_buf1 + 25, dyn_buf1, dynamic_size - 25);
    
    printf("ASAN test completed successfully\n");
    return 0;
}
