/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
struct ast_node {
    char data[256];
    struct ast_node *left;
    struct ast_node *right;
    int id;
};

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[128];
    /* Force memcpy redirection early */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 64, buffer, 32);
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile int verify[16];
    __builtin_memset(verify, 0xFF, sizeof(verify));
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in data */
    for (int i = 0; i < 128; i++) {
        node->data[i] = (char)((id + i) & 0xFF);
    }
    
    /* Copy data with overlap using memmove */
    volatile int offset = 32;
    __builtin_memmove(node->data + offset, node->data, 64);
    
    node->id = id;
    node->left = build_ast(depth - 1, id * 2);
    node->right = build_ast(depth - 1, id * 2 + 1);
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(struct ast_node* src, struct ast_node* dst) {
    volatile int use_memmove = 1;
    
    if (src && dst) {
        goto copy_block;
    }
    
    skip_copy:
        return;
    
    copy_block:
        /* This tests flow-sensitivity of asan_memfn_rtls retrieval */
        if (use_memmove) {
            __builtin_memmove(dst->data, src->data, g_mem_size);
            goto skip_copy;
        }
        
    alternate_path:
        __builtin_memcpy(dst->data, src->data, g_mem_size);
}

/* Parallel processing with OpenMP */
static void parallel_memory_ops(struct ast_node** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        volatile char temp[256];
        size_t size = g_mem_size + (i % 32);
        
        /* Mix of memory operations */
        __builtin_memset(temp, i, size);
        
        if (nodes[i]) {
            __builtin_memcpy(nodes[i]->data, temp, size);
            
            /* Conditional memmove with overlap */
            if (i % 3 == 0) {
                __builtin_memmove(nodes[i]->data + 16, 
                                 nodes[i]->data, 
                                 size - 16);
            }
        }
    }
}

/* Complex token processing */
static unsigned long process_tokens(const char** tokens, int token_count) {
    unsigned long hash = 0;
    volatile char buffer[512];
    
    for (int i = 0; i < token_count; i++) {
        size_t len = strlen(tokens[i]);
        
        /* Clear buffer with memset */
        __builtin_memset(buffer, 0, sizeof(buffer));
        
        /* Copy token with memcpy */
        __builtin_memcpy(buffer, tokens[i], len);
        
        /* Move data around with memmove */
        if (len > 32) {
            __builtin_memmove(buffer + 64, buffer + 32, len - 32);
        }
        
        /* Compute simple hash */
        for (size_t j = 0; j < len && j < sizeof(buffer); j++) {
            hash = hash * 31 + buffer[j];
        }
    }
    
    return hash;
}

/* Multi-stage initialization */
static struct ast_node** init_node_array(int count) {
    struct ast_node** nodes = malloc(count * sizeof(struct ast_node*));
    if (!nodes) return NULL;
    
    for (int i = 0; i < count; i++) {
        nodes[i] = build_ast(3, i + 1);
        
        /* Additional memory operations during initialization */
        if (nodes[i] && i > 0) {
            volatile char temp[128];
            __builtin_memset(temp, 0xCC, sizeof(temp));
            __builtin_memcpy(nodes[i]->data + 64, temp, 64);
        }
    }
    
    return nodes;
}

/* Main execution flow */
int main(void) {
    /* Initialize token array */
    const char* tokens[] = {
        "asan_test_token_1",
        "builtin_redirection",
        "memcpy_memset_memmove",
        "coverage_verification",
        "openmp_parallel_section",
        "recursive_ast_structure",
        "volatile_optimization_barrier",
        "goto_flow_control_edge"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Stage 1: Token processing */
    unsigned long token_hash = process_tokens(tokens, token_count);
    printf("Token hash: %lu\n", token_hash);
    
    /* Stage 2: AST operations */
    struct ast_node* root = build_ast(4, 1);
    if (root && root->left && root->right) {
        process_with_goto(root->left, root->right);
    }
    
    /* Stage 3: Parallel processing */
    struct ast_node** node_array = init_node_array(8);
    if (node_array) {
        parallel_memory_ops(node_array, 8);
        
        /* Verify operations */
        unsigned long ast_hash = 0;
        for (int i = 0; i < 8; i++) {
            if (node_array[i]) {
                for (int j = 0; j < 64; j++) {
                    ast_hash = ast_hash * 17 + node_array[i]->data[j];
                }
            }
        }
        printf("AST hash: %lu\n", ast_hash);
        
        /* Cleanup */
        for (int i = 0; i < 8; i++) {
            free(node_array[i]);
        }
        free(node_array);
    }
    
    /* Final memory operation */
    volatile char final_buffer[1024];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer + 512, final_buffer, 256);
    __builtin_memmove(final_buffer + 256, final_buffer + 128, 384);
    
    printf("Test completed successfully.\n");
    return 0;
}
