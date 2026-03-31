/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char *data;
    size_t size;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force early initialization of ASAN runtime */
    char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
    printf("[Constructor] Initialized ASAN buffer\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    printf("[Destructor] ASAN cleanup complete\n");
}

/* Function with goto jumps around memmove */
static void goto_memmove_test(char *dest, char *src, size_t n) {
    int use_memmove = 0;
    
    if (volatile_flag > 0) {
        use_memmove = 1;
        goto do_copy;
    }
    
    skip_copy:
    __builtin_memset(dest, 0, n);
    return;
    
    do_copy:
    /* This memmove should be intercepted by ASAN */
    __builtin_memmove(dest, src, n);
    
    if (use_memmove) {
        goto skip_copy;
    }
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_node(size_t size) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->size = size;
    node->data = malloc(size);
    node->left = node->right = NULL;
    
    /* Initialize with pattern */
    for (size_t i = 0; i < size; i++) {
        node->data[i] = (char)(i % 256);
    }
    
    return node;
}

static void copy_node_data(ASTNode *dest, ASTNode *src) {
    if (!dest || !src) return;
    
    size_t copy_size = dest->size < src->size ? dest->size : src->size;
    
    /* Force memcpy interception */
    __builtin_memcpy(dest->data, src->data, copy_size);
    
    /* Recursive copy */
    if (src->left && dest->left) {
        copy_node_data(dest->left, src->left);
    }
    if (src->right && dest->right) {
        copy_node_data(dest->right, src->right);
    }
}

static void free_tree(ASTNode *node) {
    if (!node) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node->data);
    free(node);
}

/* OpenMP parallel memory operations */
static void parallel_mem_ops(void) {
    const size_t block_size = 1024;
    char *blocks[4];
    
    #pragma omp parallel for
    for (int i = 0; i < 4; i++) {
        blocks[i] = malloc(block_size);
        if (blocks[i]) {
            /* Each thread uses builtins - should trigger ASAN redirection */
            __builtin_memset(blocks[i], i, block_size);
            
            if (i > 0) {
                __builtin_memcpy(blocks[i], blocks[i-1], block_size / 2);
            }
        }
    }
    
    /* Cross-thread memmove in critical section */
    #pragma omp parallel
    {
        #pragma omp critical
        {
            if (blocks[0] && blocks[2]) {
                __builtin_memmove(blocks[0], blocks[2], block_size / 4);
            }
        }
    }
    
    for (int i = 0; i < 4; i++) {
        free(blocks[i]);
    }
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Test 1: Direct builtin calls with volatile lengths */
    char src[128], dest[128];
    size_t len = volatile_len;
    
    __builtin_memset(src, 0xAA, sizeof(src));
    __builtin_memcpy(dest, src, len);
    __builtin_memmove(dest + 10, dest, len - 10);
    
    /* Test 2: Goto flow control with memmove */
    goto_memmove_test(dest, src, len);
    
    /* Test 3: Recursive AST operations */
    ASTNode *tree1 = create_node(256);
    ASTNode *tree2 = create_node(256);
    
    if (tree1 && tree2) {
        tree1->left = create_node(128);
        tree1->right = create_node(128);
        tree2->left = create_node(128);
        tree2->right = create_node(128);
        
        copy_node_data(tree2, tree1);
        
        /* Verify copy */
        int match = 1;
        for (size_t i = 0; i < tree1->size && i < tree2->size; i++) {
            if (tree1->data[i] != tree2->data[i]) {
                match = 0;
                break;
            }
        }
        printf("AST copy verification: %s\n", match ? "PASS" : "FAIL");
        
        free_tree(tree1);
        free_tree(tree2);
    }
    
    /* Test 4: OpenMP parallel operations */
    parallel_mem_ops();
    
    /* Test 5: Mixed builtin usage in loops */
    char buffer[3][64];
    for (int i = 0; i < 3; i++) {
        __builtin_memset(buffer[i], i * 10, sizeof(buffer[i]));
    }
    
    for (int i = 1; i < 3; i++) {
        __builtin_memcpy(buffer[i], buffer[i-1], sizeof(buffer[i]) / 2);
        __builtin_memmove(buffer[i] + 16, buffer[i], 32);
    }
    
    /* Compute verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(dest); i++) {
        hash = (hash * 31) + (unsigned char)dest[i];
    }
    
    printf("Final hash: 0x%08lx\n", hash);
    printf("=== Test Complete ===\n");
    
    return 0;
}
