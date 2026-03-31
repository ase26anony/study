#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile int flags;  /* volatile to prevent optimization */
    struct ASTNode *left;
    struct ASTNode *right;
    char padding[32];    /* Ensure size for memcpy operations */
} ASTNode;

/* Global token array */
volatile int token_array[256];
volatile int token_index = 0;

/* Function prototypes */
ASTNode* create_node(int type, int value);
void recursive_parser(ASTNode* node, int depth);
void parallel_memory_operations(void);
int compute_hash(ASTNode* root);
void __attribute__((constructor)) init_tokens(void);
void __attribute__((destructor)) cleanup(void);

/* Constructor - runs before main() */
void __attribute__((constructor)) init_tokens(void) {
    volatile int i;
    volatile int seed = 42;
    
    /* Initialize token array with non-foldable values */
    for (i = 0; i < 256; i++) {
        token_array[i] = (i * seed + 12345) % 1000;
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Use __builtin_memset in constructor context */
    volatile char buffer[64];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    
    /* Force initialization of asan_memfn_rtls cache */
    volatile char dest[32], src[32];
    __builtin_memcpy(dest, src, 32);
}

/* Create AST node with memory initialization */
ASTNode* create_node(int type, int value) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with __builtin_memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = type;
    node->value = value;
    node->flags = token_array[value % 256];  /* volatile access */
    node->left = NULL;
    node->right = NULL;
    
    return node;
}

/* Recursive parser with goto statements and memory operations */
void recursive_parser(ASTNode* node, int depth) {
    if (depth >= 5 || !node) return;
    
    volatile int use_goto = (depth % 2 == 0);
    
    if (use_goto) {
        /* Jump into block with memory operation */
        goto mem_block;
    } else {
        /* Normal path */
        node->value += depth;
        goto continue_parse;
    }
    
mem_block:
    {
        /* Block with __builtin_memmove */
        volatile char temp[sizeof(ASTNode)];
        __builtin_memmove(temp, (void*)node, sizeof(ASTNode));
        __builtin_memmove((void*)node, temp, sizeof(ASTNode));
        
        /* Jump out of block */
        goto continue_parse;
    }
    
continue_parse:
    /* Create children with volatile size control */
    volatile size_t child_size = sizeof(ASTNode);
    
    if (node->left == NULL) {
        node->left = create_node(depth * 2, node->value * 3);
        if (node->left) {
            /* Copy parent data to child */
            __builtin_memcpy(&node->left->padding, 
                           &node->padding, 
                           sizeof(node->padding));
        }
    }
    
    if (node->right == NULL) {
        node->right = create_node(depth * 2 + 1, node->value * 5);
    }
    
    /* Recursive calls */
    recursive_parser(node->left, depth + 1);
    recursive_parser(node->right, depth + 1);
    
    /* Another goto example */
    if (depth == 2) {
        goto extra_operation;
    }
    
    return;
    
extra_operation:
    {
        volatile char extra_buf[16];
        __builtin_memset(extra_buf, depth, sizeof(extra_buf));
        return;
    }
}

/* Parallel memory operations using OpenMP */
void parallel_memory_operations(void) {
    volatile int num_threads;
    volatile int i;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            num_threads = omp_get_num_threads();
        }
        
        #pragma omp for private(i)
        for (i = 0; i < 1000; i++) {
            volatile char buffer1[128];
            volatile char buffer2[128];
            volatile int size = (i % 64) + 1;  /* Non-foldable size */
            
            /* Use all three builtins in parallel context */
            __builtin_memset(buffer1, i & 0xFF, size);
            __builtin_memcpy(buffer2, buffer1, size);
            
            /* Conditional memmove */
            if (i % 3 == 0) {
                __builtin_memmove(buffer1 + 16, buffer1, size);
            }
            
            /* Store result in token array */
            #pragma omp atomic
            token_array[i % 256] += buffer1[0];
        }
    }
    
    /* Additional memory operation outside parallel region */
    volatile char final_buffer[256];
    volatile int copy_size = token_index % 128 + 64;
    __builtin_memcpy(final_buffer, (void*)token_array, copy_size);
    __builtin_memset(final_buffer + copy_size, 0xFF, 256 - copy_size);
}

/* Compute hash from AST */
int compute_hash(ASTNode* root) {
    if (!root) return 0;
    
    volatile int hash = root->value;
    volatile char temp[sizeof(int)];
    
    /* Use __builtin_memcpy for hash computation */
    __builtin_memcpy(temp, &hash, sizeof(int));
    
    /* Mix in child hashes */
    hash ^= compute_hash(root->left);
    hash ^= compute_hash(root->right);
    
    /* Final memory operation */
    __builtin_memset(temp, hash & 0xFF, sizeof(temp));
    
    return hash & 0x7FFFFFFF;  /* Ensure positive */
}

/* Destructor - runs after main() */
void __attribute__((destructor)) cleanup(void) {
    volatile char cleanup_buf[1024];
    
    /* Final memory operations in destructor */
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
    
    /* Use all three builtins one more time */
    volatile char src_buf[512];
    __builtin_memset(src_buf, 0xCC, sizeof(src_buf));
    __builtin_memcpy(cleanup_buf, src_buf, 512);
    __builtin_memmove(cleanup_buf + 256, cleanup_buf, 256);
}

/* Main execution flow */
int main(void) {
    ASTNode* root = NULL;
    volatile int result = 0;
    
    printf("Starting ASAN coverage test...\n");
    
    /* Create root node */
    root = create_node(1, 100);
    if (!root) {
        fprintf(stderr, "Failed to create root node\n");
        return 1;
    }
    
    /* Build recursive AST */
    recursive_parser(root, 0);
    
    /* Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Compute verification hash */
    result = compute_hash(root);
    
    /* Additional memory operations in main */
    volatile char main_buffer[64];
    volatile int op_size = (result % 32) + 16;
    
    __builtin_memset(main_buffer, 0xAA, op_size);
    __builtin_memcpy(main_buffer + op_size, main_buffer, 32 - op_size);
    __builtin_memmove(main_buffer, main_buffer + 16, 32);
    
    printf("Result hash: %d\n", result);
    printf("Token array[0]: %d\n", token_array[0]);
    
    /* Cleanup */
    free(root);
    
    return 0;
}
