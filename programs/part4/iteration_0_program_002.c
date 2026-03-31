/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 256 + (rand() % 768);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    printf("Destructor: ASAN test completed\n");
}

/* Function with goto jumps around memmove */
static void test_goto_memmove(char* dest, const char* src, size_t n) {
    int use_builtin = 1;
    
    if (n == 0) goto skip_copy;
    
    /* Jump into memory operation block */
    goto do_copy;
    
copy_block:
    /* Force builtin memmove with goto control flow */
    __builtin_memmove(dest, src, n);
    goto after_copy;
    
do_copy:
    if (use_builtin) goto copy_block;
    
    /* Alternative path */
    for (size_t i = 0; i < n; i++) dest[i] = src[i];
    
after_copy:
    dest[n-1] = '\0';
    
skip_copy:
    return;
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    node->left = create_ast(depth - 1, counter);
    node->right = create_ast(depth - 1, counter);
    
    /* Initialize node data with builtin memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in data */
    snprintf(node->data, sizeof(node->data), 
             "AST_Node_%d_Depth_%d", node->id, depth);
    
    return node;
}

/* Copy AST data between nodes using builtin memcpy */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use volatile to prevent optimization */
    volatile size_t copy_size = sizeof(dest->data);
    
    /* Force builtin memcpy with volatile size */
    __builtin_memcpy(dest->data, src->data, copy_size);
    
    /* Recursive copy */
    if (src->left && dest->left) {
        copy_ast_data(dest->left, src->left);
    }
    if (src->right && dest->right) {
        copy_ast_data(dest->right, src->right);
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
        
        /* Each thread uses different size */
        sizes[tid] = g_mem_size / (tid + 1) + 64;
        buffers[tid] = (char*)malloc(sizes[tid]);
        
        if (buffers[tid]) {
            /* Initialize with builtin memset */
            __builtin_memset(buffers[tid], tid + 'A', sizes[tid]);
            
            /* Copy between buffers with builtin memcpy */
            if (tid > 0) {
                size_t copy_len = sizes[tid] < sizes[tid-1] ? 
                                 sizes[tid] : sizes[tid-1];
                __builtin_memcpy(buffers[tid], buffers[tid-1], copy_len);
            }
            
            /* Use builtin memmove for overlapping regions */
            if (sizes[tid] > 128) {
                __builtin_memmove(buffers[tid] + 64, buffers[tid], 64);
            }
        }
        
        #pragma omp barrier
        
        /* Verify and free */
        if (buffers[tid]) {
            free(buffers[tid]);
        }
    }
}

/* Complex token processing with memory operations */
static unsigned long process_tokens(const char** tokens, int count) {
    unsigned long hash = 5381;
    char buffer[256];
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        size_t copy_len = len < sizeof(buffer) ? len : sizeof(buffer) - 1;
        
        /* Clear buffer with builtin memset */
        __builtin_memset(buffer, 0, sizeof(buffer));
        
        /* Copy token with builtin memcpy */
        __builtin_memcpy(buffer, tokens[i], copy_len);
        
        /* Process with goto jumps */
        int j = 0;
    process_loop:
        if (j < copy_len) {
            hash = ((hash << 5) + hash) + buffer[j];
            j++;
            goto process_loop;
        }
        
        /* Test memmove with goto */
        if (i > 0 && copy_len > 16) {
            test_goto_memmove(buffer + 8, buffer, 8);
        }
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: AST operations */
    int counter = 1;
    ASTNode* ast1 = create_ast(3, &counter);
    ASTNode* ast2 = create_ast(3, &counter);
    
    if (ast1 && ast2) {
        copy_ast_data(ast2, ast1);
        printf("AST copy completed: Node1='%s', Node2='%s'\n", 
               ast1->data, ast2->data);
    }
    
    /* Phase 2: Token processing */
    const char* tokens[] = {
        "memcpy_test", "memset_data", "memmove_buffer",
        "asan_instrumentation", "hwasan_kernel", "builtin_redirect"
    };
    
    unsigned long token_hash = process_tokens(tokens, 
                        sizeof(tokens)/sizeof(tokens[0]));
    printf("Token hash: %lu\n", token_hash);
    
    /* Phase 3: OpenMP parallel operations */
    printf("Starting parallel memory operations\n");
    parallel_memory_ops();
    
    /* Phase 4: Direct builtin calls with volatile control */
    char final_buffer[512];
    volatile size_t final_size = 256;
    
    __builtin_memset(final_buffer, 0xAA, final_size);
    __builtin_memcpy(final_buffer + 128, final_buffer, 64);
    __builtin_memmove(final_buffer, final_buffer + 64, 128);
    
    /* Verify final buffer */
    unsigned long sum = 0;
    for (size_t i = 0; i < final_size; i++) {
        sum += (unsigned char)final_buffer[i];
    }
    printf("Final buffer checksum: %lu\n", sum);
    
    /* Cleanup */
    // Note: In real code, would need recursive free for AST nodes
    
    printf("ASAN test completed successfully\n");
    return 0;
}
