/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 96;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 8);
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile int check[4];
    __builtin_memset(check, 0xFF, sizeof(check));
}

/* Function with goto jumping into memory operation block */
static void goto_memmove_test(char* dest, const char* src, size_t len) {
    int use_memmove = 1;
    
    if (len > 0) {
        goto perform_op;
    } else {
        return;
    }
    
perform_op:
    /* Jump into this block */
    if (use_memmove) {
        __builtin_memmove(dest, src, len);
        goto after_op;
    }
    
after_op:
    /* Verify the move */
    for (size_t i = 0; i < len; i++) {
        if (dest[i] != src[i]) {
            __builtin_trap();
        }
    }
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_ast_node(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    node->value = depth * 7;
    
    /* Fill data with pattern */
    for (int i = 0; i < 256; i++) {
        node->data[i] = (char)((i + depth) & 0xFF);
    }
    
    /* Recursive creation */
    node->left = create_ast_node(depth - 1);
    node->right = create_ast_node(depth - 1);
    
    return node;
}

/* Copy AST node data using builtin */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use volatile to prevent optimization */
    volatile size_t copy_len = sizeof(dest->data);
    __builtin_memcpy(dest->data, src->data, copy_len);
    
    /* Also copy metadata */
    dest->type = src->type;
    dest->value = src->value;
}

/* OpenMP parallel memory operations */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char buffer1[256];
        char buffer2[256];
        
        /* Each thread uses builtins */
        __builtin_memset(buffer1, thread_id, sizeof(buffer1));
        __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
        
        /* Conditional memmove with goto */
        if (thread_id % 2 == 0) {
            goto_memmove_test(buffer1 + 64, buffer2 + 32, 128);
        }
        
        #pragma omp barrier
        
        /* Verify in parallel */
        #pragma omp for
        for (int i = 0; i < 256; i++) {
            if (buffer1[i] != buffer2[i]) {
                #pragma omp critical
                {
                    fprintf(stderr, "Mismatch at %d\n", i);
                }
            }
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Phase 1: Direct builtin calls with volatile lengths */
    char src[512], dst[512];
    
    __builtin_memset(src, 0xCC, sizeof(src));
    __builtin_memcpy(dst, src, g_memcpy_len);
    __builtin_memmove(dst + 64, dst, g_memmove_len);
    __builtin_memset(dst + 128, 0xDD, g_memset_len);
    
    /* Phase 2: Recursive AST operations */
    ASTNode* tree1 = create_ast_node(3);
    ASTNode* tree2 = create_ast_node(3);
    
    if (tree1 && tree2) {
        copy_ast_data(tree2, tree1);
        
        /* Verify copy */
        for (int i = 0; i < 256; i++) {
            if (tree1->data[i] != tree2->data[i]) {
                printf("AST copy mismatch at %d\n", i);
                break;
            }
        }
    }
    
    /* Phase 3: OpenMP parallel section */
    parallel_mem_ops();
    
    /* Phase 4: Complex goto patterns */
    char buf1[256], buf2[256];
    for (int i = 0; i < 256; i++) {
        buf1[i] = (char)i;
    }
    
    int mode = 1;
    if (mode) goto do_memmove;
    
do_memmove:
    __builtin_memmove(buf2, buf1, 128);
    goto after_all;
    
after_all:
    /* Final verification */
    unsigned long hash = 0;
    for (int i = 0; i < 256; i++) {
        hash = hash * 31 + (unsigned char)buf2[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    
    /* Cleanup */
    free(tree1);
    free(tree2);
    
    return (hash != 0) ? 0 : 1;
}
