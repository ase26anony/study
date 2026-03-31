/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int g_volatile_size = 64;
static volatile char g_volatile_char = 'A';

/* Recursive AST-like structure */
typedef struct ast_node {
    int type;
    char data[32];
    struct ast_node* left;
    struct ast_node* right;
    struct ast_node* parent;
} ast_node_t;

/* Global token array for parser */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove", "data", "test", "asan", "hwasan"
};
static const int g_num_tokens = 7;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    volatile char buffer[128];
    
    /* Force initialization of asan_memfn_rtls[0] for memcpy */
    __builtin_memcpy((void*)buffer, (void*)g_tokens[0], 7);
    
    printf("Constructor: Initialized ASAN test environment\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    volatile int cleanup_buf[16];
    
    /* Force initialization of asan_memfn_rtls[1] for memset */
    __builtin_memset((void*)cleanup_buf, 0, sizeof(cleanup_buf));
    
    printf("Destructor: Cleaned up ASAN test environment\n");
}

/* Recursive parser using AST nodes */
static ast_node_t* parse_expression(int depth, int token_idx) {
    if (depth <= 0 || token_idx >= g_num_tokens) {
        return NULL;
    }
    
    ast_node_t* node = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    node->type = depth;
    
    /* Copy token data with memcpy */
    size_t len = strlen(g_tokens[token_idx]);
    if (len > 31) len = 31;
    __builtin_memcpy(node->data, g_tokens[token_idx], len);
    node->data[len] = '\0';
    
    /* Recursive construction */
    node->left = parse_expression(depth - 1, (token_idx + 1) % g_num_tokens);
    node->right = parse_expression(depth - 2, (token_idx + 2) % g_num_tokens);
    
    /* Set parent pointers */
    if (node->left) node->left->parent = node;
    if (node->right) node->right->parent = node;
    
    return node;
}

/* Function with goto edge cases */
static void test_goto_memmove(ast_node_t* src, ast_node_t* dst) {
    volatile int use_goto = 1;
    
    if (use_goto) {
        goto copy_block;
    }
    
    /* This block should be skipped by goto */
    {
        char temp[32];
        __builtin_memset(temp, 0, sizeof(temp));
    }
    
copy_block:
    /* Force initialization of asan_memfn_rtls[2] for memmove */
    if (src && dst) {
        __builtin_memmove(dst->data, src->data, 32);
    }
    
    /* Jump out of the block */
    goto after_copy;
    
    /* Unreachable code that still contains memory ops */
    {
        char unused[16];
        __builtin_memset(unused, 0, sizeof(unused));
    }
    
after_copy:
    return;
}

/* Calculate hash of AST tree */
static unsigned long calculate_tree_hash(const ast_node_t* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    const char* p = node->data;
    
    /* Simple hash calculation */
    while (*p) {
        hash = ((hash << 5) + hash) + *p++;
    }
    
    hash ^= calculate_tree_hash(node->left);
    hash ^= calculate_tree_hash(node->right);
    hash ^= node->type;
    
    return hash;
}

/* Free AST tree */
static void free_tree(ast_node_t* node) {
    if (!node) return;
    
    free_tree(node->left);
    free_tree(node->right);
    
    /* Clear node data before free */
    volatile char* data = (volatile char*)node->data;
    for (int i = 0; i < 32; i++) {
        data[i] = '\0';
    }
    
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create AST trees */
    ast_node_t* tree1 = parse_expression(4, 0);
    ast_node_t* tree2 = parse_expression(3, 1);
    
    if (!tree1 || !tree2) {
        fprintf(stderr, "Failed to create AST trees\n");
        return 1;
    }
    
    /* Test goto with memmove */
    test_goto_memmove(tree1, tree2);
    
    /* OpenMP parallel section */
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs memory operations */
        volatile char thread_buffer[256];
        volatile int buffer_size = g_volatile_size;
        
        /* Force memcpy in parallel context */
        __builtin_memcpy((void*)thread_buffer, 
                        (void*)&g_volatile_char, 
                        sizeof(g_volatile_char));
        
        /* Force memset in parallel context */
        __builtin_memset((void*)(thread_buffer + 128), 
                        thread_id, 
                        buffer_size / 2);
        
        /* Force memmove in parallel context */
        if (thread_id % 2 == 0) {
            __builtin_memmove((void*)(thread_buffer + 64),
                             (void*)thread_buffer,
                             32);
        }
        
        #pragma omp barrier
        
        /* Verify operations */
        #pragma omp master
        {
            printf("OpenMP: %d threads completed memory operations\n",
                   #ifdef _OPENMP
                   omp_get_num_threads()
                   #else
                   1
                   #endif
                  );
        }
    }
    
    /* Additional memory operations with volatile sizes */
    volatile int dynamic_size = g_volatile_size + 16;
    char* dynamic_buf = (char*)malloc(dynamic_size);
    
    if (dynamic_buf) {
        /* Chain of memory operations */
        __builtin_memset(dynamic_buf, 0xFF, dynamic_size);
        __builtin_memcpy(dynamic_buf + 16, tree1->data, 16);
        __builtin_memmove(dynamic_buf + 32, dynamic_buf + 16, 16);
        
        /* Verify with checksum */
        unsigned char checksum = 0;
        for (int i = 0; i < dynamic_size; i++) {
            checksum ^= dynamic_buf[i];
        }
        printf("Dynamic buffer checksum: 0x%02X\n", checksum);
        
        free(dynamic_buf);
    }
    
    /* Calculate and print results */
    unsigned long hash1 = calculate_tree_hash(tree1);
    unsigned long hash2 = calculate_tree_hash(tree2);
    
    printf("Tree 1 hash: 0x%08lX\n", hash1);
    printf("Tree 2 hash: 0x%08lX\n", hash2);
    printf("Combined hash: 0x%08lX\n", hash1 ^ hash2);
    
    /* Cleanup */
    free_tree(tree1);
    free_tree(tree2);
    
    printf("Test completed successfully\n");
    return 0;
}
