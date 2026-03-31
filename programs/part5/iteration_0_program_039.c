/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char global_tokens[4096];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(global_tokens, 0xAA, sizeof(global_tokens));
    printf("Constructor: Initialized global tokens\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use builtin memcpy in destructor */
    char temp[64];
    __builtin_memcpy(temp, global_tokens, 64);
    printf("Destructor: Cleaned up %zu bytes\n", sizeof(temp));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* data, size_t depth) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy for node initialization */
    size_t copy_len = strlen(data) + 1;
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    
    __builtin_memcpy(node->data, data, copy_len);
    node->size = copy_len;
    
    if (depth > 0) {
        char left_data[128], right_data[128];
        snprintf(left_data, sizeof(left_data), "L%d_%s", (int)depth, data);
        snprintf(right_data, sizeof(right_data), "R%d_%s", (int)depth, data);
        
        node->left = create_ast_node(left_data, depth - 1);
        node->right = create_ast_node(right_data, depth - 1);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with goto and memory operations */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int state = 0;
    
    /* Jump into block with memmove */
    if (volatile_flag) goto process_block;
    
    normal_path:
    __builtin_memset(dst->data, 0, sizeof(dst->data));
    return;
    
    process_block:
    /* Use builtin memmove with overlap */
    if (src && dst) {
        __builtin_memmove(dst->data, src->data, 
                         src->size < sizeof(dst->data) ? src->size : sizeof(dst->data));
    }
    
    /* Jump out of block */
    if (state++ < 2) goto normal_path;
}

/* Parallel memory dispatch */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        char buffer1[256];
        char buffer2[256];
        size_t local_len = volatile_len;
        
        /* Each thread uses builtins */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            /* Pattern 1: memset */
            __builtin_memset(buffer1, i, local_len);
            
            /* Pattern 2: memcpy */
            __builtin_memcpy(buffer2, buffer1, local_len);
            
            /* Pattern 3: memmove with overlap */
            if (i % 3 == 0) {
                __builtin_memmove(buffer1 + 32, buffer1, local_len - 32);
            }
            
            /* Store result in global array */
            #pragma omp critical
            {
                size_t offset = (token_index * 64) % sizeof(global_tokens);
                __builtin_memcpy(global_tokens + offset, buffer2, 64);
                token_index++;
            }
        }
    }
}

/* Complex initialization with varied memory operations */
static void initialize_complex_buffer(char* buf, size_t size) {
    char pattern[128];
    
    /* Multiple builtin calls in sequence */
    __builtin_memset(pattern, 0xCC, sizeof(pattern));
    
    for (size_t i = 0; i < size; i += 64) {
        size_t chunk = (size - i) < 64 ? (size - i) : 64;
        
        if (i % 128 == 0) {
            /* Use memcpy */
            __builtin_memcpy(buf + i, pattern, chunk);
        } else {
            /* Use memmove for overlapping regions */
            if (i >= 64) {
                __builtin_memmove(buf + i, buf + i - 64, chunk);
            } else {
                __builtin_memset(buf + i, i & 0xFF, chunk);
            }
        }
    }
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Initialize complex structures */
    ASTNode* tree1 = create_ast_node("ROOT", 3);
    ASTNode* tree2 = create_ast_node("COPY", 3);
    
    if (!tree1 || !tree2) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Phase 2: Process with goto (tests flow sensitivity) */
    process_with_goto(tree1, tree2);
    
    /* Phase 3: Complex buffer initialization */
    char complex_buf[2048];
    initialize_complex_buffer(complex_buf, sizeof(complex_buf));
    
    /* Phase 4: Parallel operations */
    parallel_memory_ops();
    
    /* Phase 5: Verify results */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(global_tokens); i++) {
        hash = (hash * 31) + (unsigned char)global_tokens[i];
    }
    
    /* Use builtins in verification */
    char verify_buf[256];
    __builtin_memcpy(verify_buf, &hash, sizeof(hash));
    __builtin_memset(verify_buf + sizeof(hash), 0, sizeof(verify_buf) - sizeof(hash));
    
    printf("Result hash: 0x%08lx\n", hash);
    printf("Token operations: %d\n", token_index);
    
    /* Cleanup */
    free(tree1);
    free(tree2);
    
    return 0;
}
