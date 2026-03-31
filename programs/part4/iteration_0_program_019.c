/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ast_node {
    char data[64];
    struct ast_node* left;
    struct ast_node* right;
    size_t size;
} ast_node_t;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force early initialization of ASAN runtime */
    char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
    g_init_flag = 1;
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Verify operations completed */
    printf("Destructor: Test completed successfully\n");
}

/* Recursive function with memory operations */
static ast_node_t* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ast_node_t* node = malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Use builtin memset for initialization */
    __builtin_memset(node, 0, sizeof(*node));
    
    /* Copy data with builtin memcpy */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) 
        copy_len = sizeof(node->data);
    __builtin_memcpy(node->data, base_data, copy_len);
    
    node->size = copy_len;
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        char child_data[64];
        __builtin_snprintf(child_data, sizeof(child_data), 
                          "%s-%d", base_data, depth);
        
        /* Jump label for goto testing */
        create_left:
        node->left = create_ast(depth - 1, child_data);
        
        /* Another jump point */
        create_right:
        node->right = create_ast(depth - 1, child_data);
        
        /* Use goto to create unusual control flow */
        if (depth == 3) {
            goto skip_memmove;
        }
    }
    
    skip_memmove:
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ast_node_t* src, ast_node_t* dst) {
    if (!src || !dst) return;
    
    int use_memmove = 0;
    
    /* Jump into memmove block */
    if (src->size > 0) {
        goto do_memmove;
    }
    
    normal_path:
    /* Regular memcpy */
    __builtin_memcpy(dst->data, src->data, src->size);
    return;
    
    do_memmove:
    /* Overlapping memory case requiring memmove */
    if (src == dst || (char*)src->data + src->size > (char*)dst->data) {
        use_memmove = 1;
        __builtin_memmove(dst->data, src->data, src->size);
    }
    
    /* Jump back out */
    if (!use_memmove) {
        goto normal_path;
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    const int num_blocks = 8;
    char* blocks[num_blocks];
    size_t block_size = g_mem_size / num_blocks;
    
    /* Allocate memory blocks */
    for (int i = 0; i < num_blocks; i++) {
        blocks[i] = malloc(block_size);
        if (!blocks[i]) return;
    }
    
    #pragma omp parallel for
    for (int i = 0; i < num_blocks; i++) {
        /* Each thread uses builtins */
        if (i % 3 == 0) {
            __builtin_memset(blocks[i], i, block_size);
        } else if (i % 3 == 1) {
            char pattern[16];
            __builtin_memset(pattern, 0xAA, sizeof(pattern));
            __builtin_memcpy(blocks[i], pattern, 
                           sizeof(pattern) < block_size ? sizeof(pattern) : block_size);
        } else {
            /* Self-copy with memmove */
            __builtin_memmove(blocks[i] + block_size/2, blocks[i], block_size/2);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_blocks; i++) {
        free(blocks[i]);
    }
}

/* Multi-stage initialization with varied builtin usage */
static void multi_stage_init(void) {
    /* Stage 1: Simple memset */
    char stage1[256];
    __builtin_memset(stage1, 0xCC, sizeof(stage1));
    
    /* Stage 2: Overlapping memmove */
    char stage2[512];
    __builtin_memset(stage2, 0xAA, sizeof(stage2));
    __builtin_memmove(stage2 + 128, stage2, 256);
    
    /* Stage 3: Complex memcpy chain */
    char stage3[1024];
    char temp[128];
    
    for (int i = 0; i < 8; i++) {
        __builtin_memset(temp, i, sizeof(temp));
        __builtin_memcpy(stage3 + i * 128, temp, sizeof(temp));
    }
    
    /* Verify with volatile read */
    volatile char* vptr = stage3;
    (void)vptr[0];
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Initialize complex token array */
    const char* tokens[] = {"alpha", "beta", "gamma", "delta", "epsilon"};
    char token_buffer[5][32];
    
    /* Copy tokens using builtins */
    for (int i = 0; i < 5; i++) {
        size_t len = strlen(tokens[i]) + 1;
        __builtin_memcpy(token_buffer[i], tokens[i], len);
    }
    
    /* Create recursive AST structure */
    ast_node_t* ast_root = create_ast(4, "root");
    if (!ast_root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process with goto control flow */
    ast_node_t* ast_copy = malloc(sizeof(ast_node_t));
    if (ast_copy) {
        __builtin_memset(ast_copy, 0, sizeof(*ast_copy));
        process_with_goto(ast_root, ast_copy);
        
        /* Verify copy */
        int match = __builtin_memcmp(ast_root->data, ast_copy->data, 
                                    ast_root->size) == 0;
        printf("AST copy verification: %s\n", match ? "PASS" : "FAIL");
        
        free(ast_copy);
    }
    
    /* Execute parallelized memory operations */
    parallel_memory_ops();
    
    /* Multi-stage interaction */
    multi_stage_init();
    
    /* Calculate and print verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; token_buffer[i][j]; j++) {
            hash = hash * 31 + token_buffer[i][j];
        }
    }
    
    /* Include AST data in hash */
    if (ast_root) {
        for (size_t i = 0; i < ast_root->size && i < sizeof(ast_root->data); i++) {
            hash = hash * 31 + ast_root->data[i];
        }
    }
    
    printf("Verification hash: %lu\n", hash);
    
    /* Cleanup */
    /* Recursive free implementation */
    void free_ast(ast_node_t* node) {
        if (!node) return;
        free_ast(node->left);
        free_ast(node->right);
        free(node);
    }
    free_ast(ast_root);
    
    printf("Test completed.\n");
    return 0;
}
