/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int g_volatile_len = 64;
static volatile char g_volatile_char = 'A';

/* Recursive AST-like structure */
typedef struct ast_node {
    char data[256];
    struct ast_node* left;
    struct ast_node* right;
    int id;
} ast_node_t;

/* Global token array */
static char g_token_array[4096];
static int g_token_index = 0;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token array with pattern */
    for (int i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (i % 26) + 'A';
    }
    printf("Constructor: Token array initialized\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_destructor(void) {
    printf("Destructor: Program cleanup completed\n");
}

/* Recursive parser with memory operations */
static ast_node_t* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ast_node_t* node = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    node->id = id;
    
    /* Fill data with pattern using volatile length */
    int len = g_volatile_len % 256;
    for (int i = 0; i < len; i++) {
        node->data[i] = g_volatile_char + (i % 10);
    }
    node->data[len] = '\0';
    
    /* Recursive creation with goto for flow control */
    int use_goto = (id % 3 == 0);
    
    if (use_goto) {
        goto create_children;
    }
    
    node->left = create_ast(depth - 1, id * 2);
    
create_children:
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    if (use_goto) {
        node->left = create_ast(depth - 1, id * 2);
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ast_node_t* src, ast_node_t* dst) {
    int condition = src->id % 4;
    
    if (condition == 0) {
        goto copy_block;
    } else if (condition == 1) {
        /* Direct copy */
        __builtin_memcpy(dst->data, src->data, g_volatile_len % 256);
        return;
    } else if (condition == 2) {
        /* Move operation */
        __builtin_memmove(dst->data, src->data, g_volatile_len % 256);
        return;
    }
    
copy_block:
    /* This block is jumped into */
    __builtin_memmove(dst->data + 10, src->data, 
                     (g_volatile_len % 200) + 1);
    
    /* Jump out to another operation */
    if (dst->id % 2 == 0) {
        goto finalize;
    }
    
    /* Additional memset */
    __builtin_memset(dst->data + 100, g_volatile_char, 50);
    
finalize:
    /* Final memory operation */
    __builtin_memcpy(dst->data + 150, src->data + 50, 100);
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
        char local_buf1[512];
        char local_buf2[512];
        
        /* Initialize with builtins */
        __builtin_memset(local_buf1, thread_id + '0', sizeof(local_buf1));
        __builtin_memset(local_buf2, 0, sizeof(local_buf2));
        
        /* Copy between buffers with volatile length */
        int copy_len = (g_volatile_len + thread_id) % 256;
        __builtin_memcpy(local_buf2, local_buf1, copy_len);
        
        /* Move operation within same buffer */
        __builtin_memmove(local_buf1 + 100, local_buf1, 100);
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Update global token array */
        #pragma omp critical
        {
            int offset = (thread_id * 64) % sizeof(g_token_array);
            __builtin_memcpy(g_token_array + offset, local_buf1, 64);
            g_token_index += copy_len;
        }
    }
}

/* Complex memory pattern generator */
static void generate_memory_patterns(ast_node_t* root) {
    if (!root) return;
    
    /* Array of memory operations to test different paths */
    void* buffers[4];
    for (int i = 0; i < 4; i++) {
        buffers[i] = malloc(1024);
        if (buffers[i]) {
            /* Use all three builtins in sequence */
            __builtin_memset(buffers[i], i + 'A', 1024);
            
            if (i > 0) {
                __builtin_memcpy(buffers[i], buffers[i-1], 512);
            }
            
            /* Overlapping move */
            __builtin_memmove((char*)buffers[i] + 256, 
                             buffers[i], 256);
        }
    }
    
    /* Process AST with goto flow control */
    if (root->left && root->right) {
        process_with_goto(root->left, root->right);
        process_with_goto(root->right, root->left);
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        if (buffers[i]) free(buffers[i]);
    }
    
    /* Recursive processing */
    generate_memory_patterns(root->left);
    generate_memory_patterns(root->right);
}

/* Calculate verification hash */
static unsigned long calculate_hash(void) {
    unsigned long hash = 5381;
    
    for (size_t i = 0; i < sizeof(g_token_array); i++) {
        hash = ((hash << 5) + hash) + g_token_array[i];
    }
    
    hash += g_token_index;
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Create recursive AST */
    ast_node_t* root = create_ast(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Generate memory patterns */
    generate_memory_patterns(root);
    
    /* Phase 3: Parallel operations */
    parallel_memory_operations();
    
    /* Phase 4: Additional builtin calls in main */
    char main_buf1[1024];
    char main_buf2[1024];
    
    /* Test all three builtins in main context */
    __builtin_memset(main_buf1, 'M', sizeof(main_buf1));
    __builtin_memcpy(main_buf2, main_buf1, sizeof(main_buf1));
    __builtin_memmove(main_buf1 + 500, main_buf1, 500);
    
    /* Copy from AST data */
    if (root->left) {
        __builtin_memcpy(main_buf1, root->left->data, 256);
    }
    
    /* Phase 5: Calculate and print verification result */
    unsigned long final_hash = calculate_hash();
    printf("Verification hash: 0x%08lx\n", final_hash);
    printf("Token index: %d\n", g_token_index);
    
    /* Cleanup */
    /* Note: Proper AST cleanup omitted for brevity */
    
    return 0;
}
