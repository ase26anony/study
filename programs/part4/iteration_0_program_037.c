#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* AST-like recursive structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile int flags;
    struct ASTNode *left;
    struct ASTNode *right;
    char padding[32];  /* Ensure size for memcpy operations */
} ASTNode;

/* Global volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 96;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    volatile char buffer[256];
    
    /* Force builtin calls in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 128, buffer, 64);
    
    printf("Constructor: Initialized ASAN buffers\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    volatile int cleanup_buf[32];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
    printf("Destructor: Cleaned up\n");
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast_node(int depth, int value) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    node->value = value;
    node->flags = depth * 2;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int branch = value % 3;
        
        if (branch == 0) {
            goto create_left;
        } else if (branch == 1) {
            node->left = create_ast_node(depth - 1, value + 1);
            goto create_right;
        } else {
            node->right = create_ast_node(depth - 1, value - 1);
            goto finish;
        }
        
    create_left:
        node->left = create_ast_node(depth - 1, value * 2);
        
    create_right:
        node->right = create_ast_node(depth - 1, value / 2);
        
    finish:
        /* Copy node data using builtin */
        if (node->left && node->right) {
            volatile size_t copy_len = sizeof(ASTNode) - offsetof(ASTNode, padding);
            __builtin_memcpy(&node->padding, &node->left->padding, copy_len);
        }
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void complex_memory_operations(void) {
    volatile char src_buffer[512];
    volatile char dst_buffer[512];
    volatile char temp_buffer[512];
    
    /* Initialize with builtin */
    __builtin_memset(src_buffer, 0xCC, sizeof(src_buffer));
    
    /* Goto-based flow control around memmove */
    int stage = 0;
    
stage1:
    if (stage++ > 3) goto final;
    
    /* Force memmove with goto jumps */
    __builtin_memmove(dst_buffer, src_buffer, g_memmove_len);
    
    if (stage % 2 == 0) {
        goto stage2;
    } else {
        goto stage3;
    }
    
stage2:
    /* Cross-copy operations */
    __builtin_memcpy(temp_buffer, dst_buffer, g_memcpy_len);
    goto stage1;
    
stage3:
    /* Nested memory operations */
    __builtin_memset(dst_buffer + 64, 0xDD, g_memset_len);
    goto stage1;
    
final:
    /* Final consolidation */
    __builtin_memmove(src_buffer, temp_buffer, 128);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_dispatch(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        volatile char local_buf[256];
        volatile char shared_buf[256];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        #pragma omp for
        for (int i = 0; i < 4; i++) {
            volatile char iteration_buf[128];
            __builtin_memset(iteration_buf, i, sizeof(iteration_buf));
            __builtin_memcpy(shared_buf + i * 32, iteration_buf, 32);
        }
        
        #pragma omp single
        {
            /* Master thread consolidates */
            __builtin_memmove(local_buf, shared_buf, 128);
        }
    }
}

/* Main execution with diversified patterns */
int main(void) {
    printf("Starting ASAN coverage test...\n");
    
    /* Phase 1: AST operations */
    ASTNode* root = create_ast_node(4, 42);
    if (root) {
        /* Copy between AST nodes */
        if (root->left && root->right) {
            volatile size_t node_copy_size = sizeof(ASTNode) / 2;
            __builtin_memcpy(root->right, root->left, node_copy_size);
        }
        
        /* Recursive traversal with memory ops */
        ASTNode* current = root;
        int depth = 0;
        while (current && depth < 3) {
            volatile char node_buf[64];
            __builtin_memcpy(node_buf, current, sizeof(node_buf));
            current = current->left;
            depth++;
        }
        
        free(root);
    }
    
    /* Phase 2: Complex memory patterns */
    complex_memory_operations();
    
    /* Phase 3: Parallel execution */
    parallel_memory_dispatch();
    
    /* Phase 4: Final verification */
    volatile int final_check[1024];
    uint64_t hash = 0;
    
    /* Use all three builtins in sequence */
    __builtin_memset(final_check, 0x11, sizeof(final_check));
    __builtin_memcpy(final_check + 512, final_check, 512 * sizeof(int));
    __builtin_memmove(final_check + 256, final_check + 768, 256 * sizeof(int));
    
    /* Calculate hash to prevent optimization */
    for (size_t i = 0; i < sizeof(final_check)/sizeof(final_check[0]); i++) {
        hash = (hash * 31) + final_check[i];
    }
    
    printf("Final hash: 0x%016llX\n", (unsigned long long)hash);
    printf("ASAN coverage test completed.\n");
    
    return 0;
}
