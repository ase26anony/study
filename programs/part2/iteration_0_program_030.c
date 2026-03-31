#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
volatile int g_use_hwasan = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN globals\n");
    g_mem_size = 128;  /* Different size to stress different paths */
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_globals(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int value) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = value;
    node->size = g_mem_size;  /* volatile access */
    
    /* Create children with goto for control flow */
    if (depth > 1) {
        int use_goto = (value % 3 == 0);
        
        if (use_goto) {
            goto create_left;
        }
        
        node->left = create_ast(depth - 1, value * 2);
        
    create_left:
        node->right = create_ast(depth - 1, value * 3);
        
        /* Copy data between nodes using __builtin_memcpy */
        if (node->left && node->right) {
            size_t copy_size = sizeof(ASTNode) - offsetof(ASTNode, value);
            __builtin_memcpy(&node->right->value, 
                           &node->left->value, 
                           copy_size);
        }
    }
    
    return node;
}

/* Function with complex goto patterns around memmove */
static void process_with_gotos(ASTNode* src, ASTNode* dst, int iterations) {
    int i = 0;
    
start_loop:
    if (i >= iterations) goto end_processing;
    
    /* Jump into block with memmove */
    if (i % 2 == 0) {
        goto do_memmove;
    }
    
    /* Normal path */
    src->value += i;
    i++;
    goto start_loop;
    
do_memmove:
    {
        /* Use __builtin_memmove with overlapping regions */
        char buffer[256];
        volatile size_t move_size = g_mem_size / 2;
        
        /* Initialize buffer */
        __builtin_memset(buffer, i, sizeof(buffer));
        
        /* Move overlapping regions */
        __builtin_memmove(buffer + 32, buffer, move_size);
        
        /* Copy back to node */
        __builtin_memcpy(&dst->value, buffer + 32, sizeof(dst->value));
    }
    i++;
    goto start_loop;
    
end_processing:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Each thread uses builtins */
            volatile size_t local_size = g_mem_size + i;
            
            /* Create temporary buffer */
            char* temp = (char*)malloc(local_size);
            if (temp) {
                /* Use all three builtins */
                __builtin_memset(temp, 0xAA, local_size);
                
                /* Copy to/from node */
                size_t copy_len = (local_size < sizeof(ASTNode)) ? 
                                 local_size : sizeof(ASTNode);
                __builtin_memcpy(temp, nodes[i], copy_len);
                
                /* Move data around */
                __builtin_memmove(temp + 16, temp, copy_len - 16);
                
                /* Copy back */
                __builtin_memcpy(nodes[i], temp + 16, copy_len - 16);
                
                free(temp);
            }
        }
    }
}

/* Function to compute hash of AST */
static uint32_t compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    uint32_t hash = 5381;
    char* ptr = (char*)node;
    
    /* Hash node contents using byte-by-byte access */
    for (size_t i = 0; i < sizeof(ASTNode); i++) {
        hash = ((hash << 5) + hash) + ptr[i];  /* hash * 33 + c */
    }
    
    /* Recursive hash computation */
    hash ^= compute_ast_hash(node->left);
    hash ^= compute_ast_hash(node->right);
    
    return hash;
}

int main(void) {
    const int NUM_NODES = 8;
    ASTNode* nodes[NUM_NODES];
    uint32_t final_hash = 0;
    
    printf("Starting ASAN coverage test\n");
    
    /* Phase 1: Create AST structure */
    ASTNode* root = create_ast(4, 42);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Process with goto patterns */
    process_with_gotos(root, root, 5);
    
    /* Phase 3: Create array of nodes for parallel processing */
    for (int i = 0; i < NUM_NODES; i++) {
        nodes[i] = create_ast(3, i * 10);
    }
    
    /* Phase 4: Parallel memory operations */
    parallel_memory_ops(nodes, NUM_NODES);
    
    /* Phase 5: Additional builtin usage in main */
    {
        volatile char buffer1[256];
        volatile char buffer2[256];
        volatile size_t op_size = g_mem_size;
        
        /* Ensure all three builtins are called from main */
        __builtin_memset((void*)buffer1, 0xCC, sizeof(buffer1));
        __builtin_memcpy((void*)buffer2, (void*)buffer1, op_size);
        __builtin_memmove((void*)buffer1 + 64, (void*)buffer1, op_size);
    }
    
    /* Phase 6: Compute and verify result */
    final_hash = compute_ast_hash(root);
    for (int i = 0; i < NUM_NODES; i++) {
        final_hash ^= compute_ast_hash(nodes[i]);
    }
    
    printf("Final hash: 0x%08X\n", final_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* ... cleanup code would go here ... */
    
    return 0;
}
