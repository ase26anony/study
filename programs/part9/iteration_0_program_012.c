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
    int id;
} ASTNode;

/* Global token array */
static char global_tokens[1024];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(global_tokens, 'A', sizeof(global_tokens));
    printf("Constructor: Initialized global tokens\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use builtin memcpy in destructor */
    char temp[64];
    __builtin_memcpy(temp, global_tokens, 64);
    printf("Destructor: Cleaned up %d bytes\n", 64);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy for node initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = depth;
    
    /* Copy data with builtin memcpy */
    size_t copy_len = volatile_len % 256;
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        char child_data[256];
        __builtin_memcpy(child_data, base_data, 128);
        
        /* Goto label for flow control */
        create_left:
        node->left = create_ast(depth - 1, child_data);
        
        /* Modify data between children */
        __builtin_memset(child_data + 64, 'B', 32);
        
        /* Another goto for right child */
        if (volatile_flag) goto create_right;
        
        create_right:
        node->right = create_ast(depth - 1, child_data);
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void process_with_goto(ASTNode* node1, ASTNode* node2) {
    char buffer[512];
    int use_memmove = 0;
    
    /* First memory operation */
    __builtin_memcpy(buffer, node1->data, 128);
    
    /* Goto into memory operation block */
    if (node1->id % 2 == 0) goto memmove_block;
    
    /* Regular memset */
    __builtin_memset(buffer + 128, 'X', 64);
    goto after_memmove;
    
memmove_block:
    /* This tests memmove with overlapping regions */
    __builtin_memmove(buffer + 64, buffer, 128);
    use_memmove = 1;
    
after_memmove:
    /* Copy result back */
    if (use_memmove) {
        __builtin_memcpy(node2->data, buffer + 64, 128);
    } else {
        __builtin_memcpy(node2->data, buffer, 128);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        char local_buf[256];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, i, sizeof(local_buf));
        
        if (nodes[i]) {
            /* Copy to node with volatile length */
            size_t len = (volatile_len + i) % 256;
            __builtin_memcpy(nodes[i]->data, local_buf, len);
            
            /* Move data around within node */
            if (len > 128) {
                __builtin_memmove(nodes[i]->data + 64, 
                                 nodes[i]->data, 128);
            }
        }
    }
}

/* Compute verification hash */
static unsigned long compute_hash(const ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    const char* p = node->data;
    
    /* Simple DJB2 hash */
    for (int i = 0; i < 256 && *p; i++) {
        hash = ((hash << 5) + hash) + *p++;
    }
    
    hash += compute_hash(node->left);
    hash += compute_hash(node->right);
    
    return hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Create AST structures */
    ASTNode* root1 = create_ast(4, "BaseDataForAST1");
    ASTNode* root2 = create_ast(3, "BaseDataForAST2");
    
    if (!root1 || !root2) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Process with goto flow control */
    process_with_goto(root1, root2);
    
    /* Create array for parallel processing */
    ASTNode* node_array[8];
    node_array[0] = root1;
    node_array[1] = root2;
    
    for (int i = 2; i < 8; i++) {
        char base[32];
        __builtin_memset(base, '0' + i, sizeof(base));
        node_array[i] = create_ast(2, base);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(node_array, 8);
    
    /* Additional memory operations in main */
    char main_buffer[1024];
    
    /* Test all three builtins */
    __builtin_memset(main_buffer, 0, sizeof(main_buffer));
    __builtin_memcpy(main_buffer, root1->data, 256);
    
    /* Overlapping memmove */
    __builtin_memmove(main_buffer + 128, main_buffer, 384);
    
    /* Copy back with different size */
    size_t final_len = volatile_len % 512;
    __builtin_memcpy(root2->data, main_buffer + 64, final_len);
    
    /* Compute verification result */
    unsigned long hash1 = compute_hash(root1);
    unsigned long hash2 = compute_hash(root2);
    unsigned long total_hash = hash1 + hash2;
    
    printf("Hash results: root1=0x%lx, root2=0x%lx, total=0x%lx\n",
           hash1, hash2, total_hash);
    
    /* Cleanup */
    for (int i = 2; i < 8; i++) {
        if (node_array[i]) free_ast(node_array[i]);
    }
    
    free_ast(root1);
    free_ast(root2);
    
    /* Final memory operation */
    __builtin_memset(global_tokens, 0, sizeof(global_tokens));
    
    printf("Test completed successfully\n");
    return 0;
}
