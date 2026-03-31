/* ISO C99-compliant test program for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations for attribute functions */
void __attribute__((constructor)) init_asan_test(void);
void __attribute__((destructor)) cleanup_asan_test(void);

/* Recursive AST-like structure */
struct ast_node {
    int type;
    int value;
    volatile int volatile_marker;  /* Prevent optimization */
    struct ast_node *left;
    struct ast_node *right;
    char padding[16];  /* Ensure size for memcpy operations */
};

/* Global volatile variables to prevent constant folding */
volatile size_t volatile_len = 64;
volatile int volatile_switch = 1;

/* Token array for parser simulation */
static unsigned char token_buffer[256];
static volatile int token_index = 0;

/* Initialize token buffer with pattern */
static void init_tokens(void) {
    for (int i = 0; i < 256; i++) {
        token_buffer[i] = (unsigned char)(i ^ 0x55);
    }
}

/* Recursive parser with memory operations */
static struct ast_node* parse_expression(int depth) {
    if (depth <= 0) {
        struct ast_node *leaf = malloc(sizeof(struct ast_node));
        if (!leaf) return NULL;
        
        /* Use memset to initialize */
        __builtin_memset(leaf, 0, sizeof(struct ast_node));
        leaf->type = 1;
        leaf->value = token_buffer[token_index++ % 256];
        leaf->volatile_marker = volatile_switch;
        return leaf;
    }
    
    struct ast_node *node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(struct ast_node));
    node->type = 2;
    node->volatile_marker = volatile_switch;
    
    /* Recursive parsing with goto for flow control */
    int use_goto = (depth % 3 == 0);
    
    if (use_goto) {
        goto parse_left;
    }
    
    node->left = parse_expression(depth - 1);
    
parse_left:
    if (use_goto) {
        node->left = parse_expression(depth - 1);
    }
    
    node->right = parse_expression(depth - 2);
    
    /* Copy data between nodes using memcpy */
    if (node->left && node->right) {
        size_t copy_len = volatile_len % sizeof(struct ast_node);
        __builtin_memcpy(&node->padding[0], 
                        &node->left->padding[0], 
                        copy_len);
        
        /* Use memmove for overlapping regions */
        if (copy_len > 8) {
            __builtin_memmove(&node->padding[8], 
                            &node->padding[0], 
                            copy_len - 8);
        }
    }
    
    return node;
}

/* Free AST recursively */
static void free_ast(struct ast_node *node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

/* Compute hash of AST */
static unsigned long compute_ast_hash(struct ast_node *node) {
    if (!node) return 0;
    
    unsigned long hash = node->type * 31 + node->value;
    hash = hash * 31 + compute_ast_hash(node->left);
    hash = hash * 31 + compute_ast_hash(node->right);
    
    /* Access padding to ensure it's used */
    for (int i = 0; i < 16; i++) {
        hash = hash * 17 + node->padding[i];
    }
    
    return hash;
}

/* Constructor - runs before main */
void __attribute__((constructor)) init_asan_test(void) {
    printf("ASAN Test Constructor: Initializing...\n");
    init_tokens();
    
    /* Early built-in usage in constructor */
    char init_buf[32];
    __builtin_memset(init_buf, 0xAA, sizeof(init_buf));
    __builtin_memcpy(&init_buf[16], &init_buf[0], 16);
    __builtin_memmove(&init_buf[8], &init_buf[0], 24);
}

/* Destructor - runs after main */
void __attribute__((destructor)) cleanup_asan_test(void) {
    printf("ASAN Test Destructor: Cleaning up...\n");
}

/* Main test function with OpenMP parallel section */
int main(void) {
    struct ast_node *ast = NULL;
    unsigned long final_hash = 0;
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create complex AST */
    ast = parse_expression(5);
    if (!ast) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* OpenMP parallel section with memory operations */
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers for memory operations */
        char local_buf[128];
        char src_buf[128];
        
        /* Initialize source buffer with pattern */
        for (int i = 0; i < 128; i++) {
            src_buf[i] = (char)((i + thread_id) & 0xFF);
        }
        
        /* Force all three built-ins in parallel context */
        #pragma omp for
        for (int i = 0; i < 10; i++) {
            /* Use volatile to prevent optimization */
            volatile size_t len = volatile_len % 64 + 16;
            
            /* Test memcpy */
            __builtin_memcpy(&local_buf[i * 8], &src_buf[i * 8], len);
            
            /* Test memset */
            __builtin_memset(&local_buf[64 + i * 4], i, len / 2);
            
            /* Test memmove with potential overlap */
            if (i > 0) {
                __builtin_memmove(&local_buf[i * 4], &local_buf[(i-1) * 4], len);
            }
        }
        
        /* Compute partial hash from local buffer */
        unsigned long thread_hash = 0;
        for (int i = 0; i < 128; i++) {
            thread_hash = thread_hash * 31 + local_buf[i];
        }
        
        #pragma omp atomic
        final_hash ^= thread_hash;
    }
    
    /* Additional built-in usage outside parallel region */
    char final_buf[256];
    volatile size_t final_len = volatile_len % 128 + 64;
    
    /* Chain of memory operations to stress redirection */
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(&final_buf[64], token_buffer, 128);
    __builtin_memmove(&final_buf[32], &final_buf[64], 96);
    __builtin_memset(&final_buf[160], 0xFF, final_len);
    __builtin_memcpy(&final_buf[192], &final_buf[32], 64);
    
    /* Compute AST hash and combine with buffer hash */
    unsigned long ast_hash = compute_ast_hash(ast);
    final_hash ^= ast_hash;
    
    /* Add buffer hash */
    for (int i = 0; i < 256; i++) {
        final_hash = final_hash * 17 + final_buf[i];
    }
    
    printf("Test completed. Final hash: 0x%016lx\n", final_hash);
    
    /* Cleanup */
    free_ast(ast);
    
    return 0;
}
