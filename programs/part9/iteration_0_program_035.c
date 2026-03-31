/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Force inclusion of builtins via header */
#include <builtins.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    uint32_t hash;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Early builtin usage forces initialization */
    char buffer[32];
    __builtin_memset(buffer, 0xA5, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan(void) {
    /* Final builtin usage */
    volatile char final_buf[8];
    __builtin_memset((void*)final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data with builtin memcpy */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) 
        copy_len = sizeof(node->data);
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Create children recursively */
    node->left = create_ast(depth - 1, base_data);
    node->right = create_ast(depth - 1, base_data);
    
    /* Compute hash using memory operations */
    node->hash = 0;
    for (size_t i = 0; i < sizeof(node->data); i++) {
        node->hash ^= (uint32_t)node->data[i] << ((i % 4) * 8);
    }
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int use_memmove = g_use_memmove;
    
    if (use_memmove) {
        goto use_memmove_block;
    } else {
        goto use_memcpy_block;
    }
    
use_memmove_block:
    {
        /* This block tests flow sensitivity */
        char temp[128];
        __builtin_memcpy(temp, src->data, sizeof(src->data));
        
        if (dst) {
            __builtin_memmove(dst->data, temp, sizeof(dst->data));
        }
        goto after_operation;
    }
    
use_memcpy_block:
    {
        if (dst) {
            __builtin_memcpy(dst->data, src->data, sizeof(dst->data));
        }
        /* fall through */
    }
    
after_operation:
    /* Additional builtin usage after jump */
    if (dst) {
        __builtin_memset(dst->data + 32, 0xCC, 16);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        volatile size_t local_size = g_mem_size;
        char buffer[512];
        
        /* Mix of builtins in parallel region */
        __builtin_memset(buffer, i, local_size % sizeof(buffer));
        
        if (nodes[i]) {
            __builtin_memcpy(nodes[i]->data, buffer, 
                           sizeof(nodes[i]->data) < 64 ? 
                           sizeof(nodes[i]->data) : 64);
            
            /* Conditional memmove */
            if (i > 0 && nodes[i-1]) {
                __builtin_memmove(nodes[i-1]->data + 16,
                                nodes[i]->data + 8, 32);
            }
        }
        
        /* Volatile write to prevent optimization */
        *(volatile char*)buffer = 0;
    }
}

/* Main execution flow */
int main(void) {
    const int num_nodes = 8;
    ASTNode* nodes[num_nodes];
    uint32_t total_hash = 0;
    
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Create AST structure */
    ASTNode* root = create_ast(3, "TestASTData");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Initialize node array */
    for (int i = 0; i < num_nodes; i++) {
        nodes[i] = create_ast(2, "NodeData");
    }
    
    /* Test goto flow control */
    process_with_goto(root, nodes[0]);
    
    /* Force different paths */
    g_use_memmove = 0;
    process_with_goto(nodes[0], nodes[1]);
    g_use_memmove = 1;
    
    /* OpenMP parallel section */
    #ifdef _OPENMP
    parallel_memory_ops(nodes, num_nodes);
    #endif
    
    /* Additional builtin stress tests */
    {
        char large_buffer[1024];
        volatile size_t op_size = g_mem_size;
        
        /* Chain of builtin operations */
        __builtin_memset(large_buffer, 0xAA, op_size % sizeof(large_buffer));
        __builtin_memcpy(large_buffer + 128, large_buffer, 256);
        __builtin_memmove(large_buffer + 256, large_buffer + 128, 128);
        
        /* Copy to AST nodes */
        for (int i = 0; i < num_nodes && nodes[i]; i++) {
            __builtin_memcpy(nodes[i]->data, 
                           large_buffer + (i * 64),
                           sizeof(nodes[i]->data));
        }
    }
    
    /* Compute verification hash */
    for (int i = 0; i < num_nodes && nodes[i]; i++) {
        total_hash ^= nodes[i]->hash;
        
        /* Final builtin usage in computation loop */
        if (i % 2 == 0) {
            __builtin_memset(nodes[i]->data + 48, total_hash & 0xFF, 8);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_nodes; i++) {
        if (nodes[i]) free(nodes[i]);
    }
    if (root) free(root);
    
    printf("Total hash: 0x%08X\n", total_hash);
    printf("Test completed successfully.\n");
    
    return 0;
}
