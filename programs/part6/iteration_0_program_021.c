#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    volatile int depth;
} ASTNode;

/* Global volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    volatile char buffer[128];
    /* Force __builtin_memset in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    
    /* Test all three builtins in constructor context */
    volatile char src[64];
    __builtin_memset(src, 0xBB, 64);
    __builtin_memcpy(buffer + 32, src, 32);
    
    if (g_use_memmove) {
        __builtin_memmove(buffer, buffer + 16, 48);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    volatile char cleanup_buf[256];
    __builtin_memset(cleanup_buf, 0xFF, 256);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data with __builtin_memcpy */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    __builtin_memcpy(node->data, base_data, copy_len);
    
    node->depth = depth;
    
    /* Create children with modified data */
    char child_data[64];
    __builtin_memset(child_data, 0, sizeof(child_data));
    __builtin_memcpy(child_data, base_data, copy_len);
    child_data[copy_len - 1] = 'L';
    
    node->left = create_ast(depth - 1, child_data);
    
    __builtin_memset(child_data + copy_len - 1, 'R', 1);
    node->right = create_ast(depth - 1, child_data);
    
    return node;
}

/* Function with goto and memory operations */
static void process_with_goto(ASTNode* node1, ASTNode* node2) {
    volatile int use_memmove = g_use_memmove;
    
    if (!node1 || !node2) goto cleanup;
    
    /* Jump into memory operation block */
    goto mem_operation;
    
mem_operation:
    if (use_memmove) {
        /* Use __builtin_memmove with overlapping regions */
        __builtin_memmove(node1->data + 16, node1->data, 32);
        goto after_move;
    }
    
after_move:
    /* Copy between nodes */
    __builtin_memcpy(node2->data, node1->data, sizeof(node1->data));
    
    /* Jump out and back in */
    if (node1->depth > 2) {
        goto recursive_call;
    }
    
    goto cleanup;
    
recursive_call:
    /* Process children */
    if (node1->left && node2->left) {
        __builtin_memcpy(node2->left->data, node1->left->data, 32);
    }
    
cleanup:
    return;
}

/* Calculate hash of AST */
static uint32_t hash_ast(const ASTNode* node) {
    if (!node) return 0;
    
    uint32_t hash = 5381;
    volatile const char* p = node->data;
    
    /* Process string with volatile pointer */
    while (*p) {
        hash = ((hash << 5) + hash) + *p++;
    }
    
    /* Recursively hash children */
    uint32_t left_hash = hash_ast(node->left);
    uint32_t right_hash = hash_ast(node->right);
    
    /* Mix hashes */
    __builtin_memset(&hash, hash ^ left_hash, sizeof(hash));
    hash ^= right_hash;
    
    return hash;
}

/* Free AST */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear data before free */
    volatile char* data = node->data;
    __builtin_memset(data, 0, sizeof(node->data));
    
    free(node);
}

int main(void) {
    volatile size_t local_size = g_mem_size;
    uint32_t total_hash = 0;
    
    /* Create complex token array */
    volatile char tokens[4][128];
    
    /* Initialize tokens with different patterns */
    for (int i = 0; i < 4; i++) {
        __builtin_memset(tokens[i], i * 0x40, 128);
    }
    
    /* OpenMP parallel section with memory operations */
    #pragma omp parallel reduction(+:total_hash)
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread creates its own AST */
        char base_data[64];
        __builtin_memset(base_data, 'A' + thread_id, 63);
        base_data[63] = '\0';
        
        ASTNode* root = create_ast(3, base_data);
        
        if (root) {
            /* Process with goto flow */
            process_with_goto(root, root);
            
            /* Calculate hash */
            uint32_t hash = hash_ast(root);
            
            /* Thread-specific memory operations */
            volatile char thread_buf[256];
            __builtin_memset(thread_buf, thread_id, 256);
            
            /* Copy between buffers */
            __builtin_memcpy(thread_buf + 128, thread_buf, 128);
            
            /* Conditional memmove */
            if (thread_id % 2 == 0) {
                __builtin_memmove(thread_buf, thread_buf + 64, 192);
            }
            
            total_hash += hash;
            
            /* Clean up */
            free_ast(root);
        }
        
        /* Additional memory operations in parallel region */
        volatile char parallel_buf[512];
        size_t op_size = local_size * (thread_id + 1);
        
        if (op_size > sizeof(parallel_buf)) op_size = sizeof(parallel_buf);
        
        __builtin_memset(parallel_buf, 0xCC, op_size);
        __builtin_memcpy(parallel_buf + op_size/2, parallel_buf, op_size/2);
    }
    
    /* Final memory operations in main */
    volatile char final_buf[1024];
    
    /* Test all three builtins in sequence */
    __builtin_memset(final_buf, 0x11, 512);
    __builtin_memcpy(final_buf + 512, final_buf, 256);
    __builtin_memmove(final_buf + 256, final_buf, 512);
    
    /* Process tokens with memory operations */
    for (int i = 0; i < 3; i++) {
        __builtin_memcpy(tokens[i+1], tokens[i], 64);
    }
    
    /* Final __builtin_memmove with overlap */
    __builtin_memmove(tokens[0], tokens[0] + 32, 96);
    
    printf("Total hash: %u\n", total_hash);
    
    return 0;
}
