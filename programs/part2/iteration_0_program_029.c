/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 1024;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char g_token_pool[4096];
static volatile size_t g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Initialize token pool with pattern */
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i * 31) & 0xFF);
    }
    
    /* Force early initialization of memory functions */
    char temp[64];
    __builtin_memset(temp, 0xAA, sizeof(temp));
    __builtin_memcpy(temp + 32, temp, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Verify memory operations completed */
    volatile char verify = g_token_pool[0];
    (void)verify; /* Suppress unused warning */
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* src, size_t depth) {
    if (depth > 3) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy with volatile size */
    volatile size_t copy_size = (depth * 64) % 256;
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, src, copy_size);
    
    /* Create children with goto for flow control */
    if (depth < 3) {
        goto create_children;
        
        create_children:
        node->left = create_ast_node(src + 64, depth + 1);
        
        /* Jump back for right child */
        if (node->left) {
            char buffer[128];
            volatile size_t move_size = 96;
            
            /* Use goto to jump into memmove block */
            if (move_size > 64) goto do_memmove;
            
            do_memmove:
            __builtin_memmove(buffer, node->left->data, move_size);
            __builtin_memcpy(buffer + 32, node->data, 32);
        }
        
        node->right = create_ast_node(src + 128, depth + 1);
    } else {
        node->left = node->right = NULL;
    }
    
    node->size = copy_size;
    return node;
}

/* Complex memory operation with OpenMP */
static void parallel_memory_operations(void) {
    const size_t chunk_size = 512;
    char* buffers[8];
    
    /* Allocate buffers */
    for (int i = 0; i < 8; i++) {
        buffers[i] = (char*)malloc(chunk_size);
        if (!buffers[i]) return;
        
        /* Initialize with pattern */
        __builtin_memset(buffers[i], i * 16, chunk_size);
    }
    
    /* OpenMP parallel region */
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread performs memory operations */
        #pragma omp for
        for (int i = 0; i < 8; i++) {
            volatile size_t op_size = (thread_id * 64 + i * 32) % chunk_size;
            
            /* Mix of memory operations */
            if (i % 3 == 0) {
                __builtin_memcpy(buffers[i] + 128, buffers[(i + 1) % 8], op_size);
            } else if (i % 3 == 1) {
                __builtin_memset(buffers[i] + 256, thread_id, op_size);
            } else {
                /* Use goto for memmove with flow control */
                if (op_size > 128) goto large_move;
                continue;
                
                large_move:
                __builtin_memmove(buffers[i], buffers[i] + 64, op_size);
            }
        }
        
        /* Barrier to synchronize */
        #pragma omp barrier
        
        /* Verify operations */
        #pragma omp single
        {
            volatile char check = buffers[0][0];
            (void)check;
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        free(buffers[i]);
    }
}

/* Multi-stage memory test */
static size_t execute_memory_test_suite(void) {
    size_t hash = 0;
    ASTNode* root = NULL;
    
    /* Stage 1: Basic builtin operations */
    {
        char stage1_buf[1024];
        volatile size_t stage1_size = g_mem_size % 512;
        
        __builtin_memset(stage1_buf, 0xCC, sizeof(stage1_buf));
        __builtin_memcpy(stage1_buf + 256, g_token_pool, stage1_size);
        __builtin_memmove(stage1_buf, stage1_buf + 128, 256);
        
        for (size_t i = 0; i < 256; i++) {
            hash ^= (size_t)stage1_buf[i] << ((i % 8) * 8);
        }
    }
    
    /* Stage 2: Recursive AST operations */
    root = create_ast_node(g_token_pool, 0);
    if (root) {
        /* Traverse AST and accumulate */
        ASTNode* stack[16];
        int top = 0;
        stack[top++] = root;
        
        while (top > 0) {
            ASTNode* current = stack[--top];
            
            /* Process node data */
            for (size_t i = 0; i < current->size && i < 64; i++) {
                hash += (size_t)current->data[i];
            }
            
            /* Push children */
            if (current->right) stack[top++] = current->right;
            if (current->left) stack[top++] = current->left;
            
            /* Memory operation between nodes */
            if (current->left && current->right) {
                volatile size_t inter_size = (current->size + 32) % 128;
                __builtin_memcpy(current->data + 128, 
                               current->left->data, 
                               inter_size);
                __builtin_memmove(current->right->data,
                                current->data,
                                inter_size);
            }
        }
        
        /* Cleanup AST */
        /* ... (recursive free implementation would go here) */
        free(root);
    }
    
    /* Stage 3: Parallel operations */
    parallel_memory_operations();
    
    return hash;
}

/* Main execution flow */
int main(void) {
    size_t final_hash = 0;
    
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Force initialization of all three builtins early */
    char init_buf[128];
    __builtin_memset(init_buf, 0, sizeof(init_buf));
    __builtin_memcpy(init_buf + 32, init_buf, 64);
    __builtin_memmove(init_buf, init_buf + 16, 48);
    
    /* Execute comprehensive test suite */
    final_hash = execute_memory_test_suite();
    
    /* Additional edge case: overlapping memory with goto */
    {
        char edge_buf[256];
        volatile int use_memmove = 1;
        
        __builtin_memset(edge_buf, 0xAA, sizeof(edge_buf));
        
        if (use_memmove) goto perform_overlap;
        
        perform_overlap:
        __builtin_memmove(edge_buf + 64, edge_buf, 128);
        
        /* Mix with memcpy */
        __builtin_memcpy(edge_buf + 192, edge_buf + 32, 64);
    }
    
    printf("Test completed. Final hash: %zu\n", final_hash);
    printf("Token pool[0] = %d\n", (int)g_token_pool[0]);
    
    return 0;
}
