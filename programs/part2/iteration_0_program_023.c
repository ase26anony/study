/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    uint32_t hash;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force early initialization with builtins */
    char buffer[32];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(&g_init_flag, buffer, sizeof(g_init_flag));
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    volatile char cleanup_buf[16];
    __builtin_memset((void*)cleanup_buf, 0, sizeof(cleanup_buf));
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_tree(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Create pattern in data */
    for (int i = 0; i < 63; i++) {
        node->data[i] = (char)(depth + i);
    }
    node->data[63] = '\0';
    
    /* Recursive creation */
    node->left = create_tree(depth - 1);
    node->right = create_tree(depth - 1);
    
    return node;
}

/* Complex memory operation with goto flow control */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int use_memmove = 1;
    
    /* Jump into memory operation block */
    if (use_memmove) goto do_memmove;
    
    normal_path:
        __builtin_memcpy(dst->data, src->data, 64);
        return;
    
    do_memmove:
        /* Overlapping memory regions */
        char temp[128];
        __builtin_memcpy(temp, src->data, 64);
        
        /* Jump out and back in */
        if (dst->hash > 100) goto skip_memmove;
        
        __builtin_memmove(dst->data, temp + 32, 32);
        goto normal_path;
        
    skip_memmove:
        __builtin_memset(dst->data, 0xFF, 64);
}

/* Compute hash with volatile control */
static uint32_t compute_node_hash(ASTNode* node) {
    if (!node) return 0;
    
    uint32_t hash = 5381;
    volatile size_t len = 64;
    
    for (volatile size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + (uint32_t)node->data[i];
    }
    
    /* Conditional builtin usage */
    if (hash & 1) {
        char temp[64];
        __builtin_memcpy(temp, node->data, 64);
        __builtin_memset(node->data + 32, hash & 0xFF, 32);
        __builtin_memcpy(node->data, temp, 64);
    }
    
    return hash;
}

/* OpenMP parallel section */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Mixed builtin usage in parallel region */
                if (tid % 3 == 0) {
                    __builtin_memset(nodes[i]->data, tid, 64);
                } else if (tid % 3 == 1) {
                    char pattern[64];
                    __builtin_memset(pattern, 0xCC, 64);
                    __builtin_memcpy(nodes[i]->data, pattern, 64);
                } else {
                    /* Create overlapping region for memmove */
                    char buffer[96];
                    __builtin_memcpy(buffer, nodes[i]->data, 64);
                    __builtin_memmove(nodes[i]->data, buffer + 16, 64);
                }
                
                nodes[i]->hash = compute_node_hash(nodes[i]);
            }
        }
    }
}

/* Multi-stage initialization */
static void initialize_system(ASTNode*** node_array, int* count) {
    volatile int depth = 3;
    *count = (1 << depth) - 1;  /* Full binary tree nodes */
    
    *node_array = (ASTNode**)malloc(*count * sizeof(ASTNode*));
    if (!*node_array) return;
    
    /* Build tree and flatten to array */
    ASTNode* root = create_tree(depth);
    
    /* Manual tree traversal with memory ops */
    ASTNode* stack[32];
    int stack_ptr = 0;
    int idx = 0;
    ASTNode* current = root;
    
    while (current || stack_ptr > 0) {
        while (current) {
            stack[stack_ptr++] = current;
            current = current->left;
        }
        
        current = stack[--stack_ptr];
        (*node_array)[idx++] = current;
        
        /* Process node with goto flow */
        if (idx > 1) {
            process_with_goto((*node_array)[idx-2], current);
        }
        
        current = current->right;
    }
    
    /* Cleanup root (nodes are in array now) */
    free(root);
}

int main(void) {
    ASTNode** nodes = NULL;
    int node_count = 0;
    
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Stage 1: Initialization with constructor */
    if (!g_init_flag) {
        char init_buf[16];
        __builtin_memset(init_buf, 0x5A, sizeof(init_buf));
    }
    
    /* Stage 2: Complex data structure setup */
    initialize_system(&nodes, &node_count);
    
    /* Stage 3: Parallel memory operations */
    parallel_memory_ops(nodes, node_count);
    
    /* Stage 4: Verification and result computation */
    uint64_t total_hash = 0;
    volatile int verify_mode = 1;
    
    for (int i = 0; i < node_count; i++) {
        if (nodes[i]) {
            /* Conditional builtin usage in verification */
            if (verify_mode && (i % 4 == 0)) {
                char verify_buf[64];
                __builtin_memcpy(verify_buf, nodes[i]->data, 64);
                __builtin_memset(nodes[i]->data + 16, i & 0xFF, 32);
                __builtin_memcpy(nodes[i]->data, verify_buf, 64);
            }
            
            total_hash += nodes[i]->hash;
            free(nodes[i]);
        }
    }
    
    free(nodes);
    
    /* Final builtin usage */
    volatile char final_buf[32];
    __builtin_memset((void*)final_buf, total_hash & 0xFF, sizeof(final_buf));
    __builtin_memcpy((void*)&total_hash, final_buf, sizeof(uint32_t));
    
    printf("Test completed. Total hash: %llu\n", 
           (unsigned long long)total_hash);
    
    return (total_hash != 0) ? 0 : 1;
}
