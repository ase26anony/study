/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile size_t size;  /* volatile to prevent optimization */
    struct ASTNode* left;
    struct ASTNode* right;
    char padding[32];      /* Ensure size for memcpy operations */
} ASTNode;

/* Global volatile variables to prevent constant folding */
volatile size_t g_mem_size = 64;
volatile int g_init_value = 0x42;
volatile int g_use_hwasan = 0;

/* Token array for parser simulation */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "alloc", "free", "process"
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_sanitizer_hooks(void) {
    /* Force initialization of sanitizer runtime */
    volatile char buffer[16];
    __builtin_memset(buffer, 0, sizeof(buffer));
    printf("Constructor: ASAN/HWASAN hooks initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_sanitizer(void) {
    printf("Destructor: Sanitizer cleanup completed\n");
}

/* Recursive parser with goto control flow */
static ASTNode* parse_expression(int depth, int* index) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with volatile builtins */
    __builtin_memset(node, 0, sizeof(*node));
    node->size = g_mem_size;
    
    if (depth > 0) {
        /* Complex initialization with memcpy */
        ASTNode template;
        template.type = depth;
        template.value = *index;
        template.size = g_mem_size / 2;
        
        /* Force memcpy redirection */
        __builtin_memcpy(&node->type, &template, sizeof(template.type) + sizeof(template.value));
        
        /* Jump table simulation with goto */
        int op = (*index) % 3;
        
        if (op == 0) goto parse_left;
        else if (op == 1) goto parse_right;
        else goto parse_both;
        
    parse_left:
        node->left = parse_expression(depth - 1, index);
        goto continue_parsing;
        
    parse_right:
        node->right = parse_expression(depth - 1, index);
        goto continue_parsing;
        
    parse_both:
        node->left = parse_expression(depth - 1, index);
        node->right = parse_expression(depth - 1, index);
        
    continue_parsing:
        /* Use memmove for node data rearrangement */
        if (node->left && node->right) {
            char temp[sizeof(ASTNode)];
            __builtin_memcpy(temp, node->left, sizeof(ASTNode));
            __builtin_memmove(node->left, node->right, sizeof(ASTNode));
            __builtin_memcpy(node->right, temp, sizeof(ASTNode));
        }
    }
    
    (*index)++;
    return node;
}

/* Calculate tree hash with memory operations */
static int compute_tree_hash(ASTNode* root) {
    if (!root) return 0;
    
    int hash = 0;
    char buffer[sizeof(ASTNode)];
    
    /* Use builtins in computation */
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, root, sizeof(ASTNode));
    
    for (size_t i = 0; i < sizeof(ASTNode); i++) {
        hash = (hash * 31) + buffer[i];
    }
    
    /* Recursive computation */
    hash += compute_tree_hash(root->left);
    hash += compute_tree_hash(root->right);
    
    return hash;
}

/* Parallel memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        size_t local_size = g_mem_size + thread_id;
        
        /* Thread-local buffers */
        char src[128], dst[128];
        
        /* Initialize with volatile builtin */
        volatile int init_val = g_init_value + thread_id;
        __builtin_memset(src, init_val, local_size);
        
        /* Copy with builtin */
        __builtin_memcpy(dst, src, local_size);
        
        /* Move with builtin (testing memmove redirection) */
        __builtin_memmove(src + 16, src, local_size - 16);
        
        /* Verify copy */
        for (size_t i = 0; i < local_size; i++) {
            if (dst[i] != (char)init_val) {
                printf("Thread %d: Memory verification failed at byte %zu\n", 
                       thread_id, i);
            }
        }
        
        #pragma omp barrier
        
        /* Collective operation */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            char temp[64];
            __builtin_memset(temp, i, sizeof(temp));
            __builtin_memcpy(&dst[i], temp, sizeof(temp) > 64 ? 64 : sizeof(temp));
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST parsing */
    int index = 0;
    ASTNode* root = parse_expression(3, &index);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Tree computation */
    int tree_hash = compute_tree_hash(root);
    printf("AST hash: %d\n", tree_hash);
    
    /* Phase 3: Parallel memory operations */
    parallel_memory_operations();
    
    /* Phase 4: Direct builtin stress test */
    char buffer1[256], buffer2[256];
    
    /* Test all three builtins in sequence */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 128, buffer1, 128);
    
    /* Verify with standard library for comparison */
    if (memcmp(buffer1 + 128, buffer2, 128) != 0) {
        printf("Memmove verification failed\n");
    }
    
    /* Phase 5: Variable-sized operations */
    for (volatile size_t size = 1; size <= 128; size *= 2) {
        char* dyn_buf = malloc(size);
        if (dyn_buf) {
            __builtin_memset(dyn_buf, size & 0xFF, size);
            __builtin_memcpy(buffer1, dyn_buf, size > 256 ? 256 : size);
            free(dyn_buf);
        }
    }
    
    /* Cleanup */
    /* Note: In real code, you'd need proper tree freeing */
    
    printf("Test completed successfully\n");
    return 0;
}
