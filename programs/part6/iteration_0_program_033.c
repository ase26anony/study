/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "test"
};
static const int g_token_count = sizeof(g_tokens) / sizeof(g_tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    volatile char buffer[128];
    
    /* Force built-in initialization in constructor context */
    __builtin_memset((void*)buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy((void*)(buffer + 32), "constructor_init", 16);
    
    g_init_flag = 1;
    printf("[Constructor] ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[64];
    __builtin_memset((void*)cleanup_buf, 0xFF, sizeof(cleanup_buf));
    printf("[Destructor] Cleanup completed\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use all three built-ins in node initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    char temp[256];
    __builtin_memcpy(temp, g_tokens[id % g_token_count], 32);
    __builtin_memcpy(node->data, temp, 256);
    
    node->id = id;
    
    /* Recursive creation with goto for flow complexity */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    node->left = create_ast_node(depth - 1, id * 2);
    node->right = create_ast_node(depth - 1, id * 2 + 1);
    
    /* Copy data between nodes using memmove */
    if (node->left && node->right) {
        __builtin_memmove(node->right->data + 128, 
                         node->left->data, 
                         128);
    }
    
done:
    return node;
}

/* Complex memory operation with goto jumps */
static void perform_memory_operations(void) {
    volatile char src_buffer[512];
    volatile char dst_buffer[512];
    volatile char temp_buffer[512];
    
    /* Initialize with volatile size */
    size_t op_size = g_mem_size;
    
    /* Pattern 1: Direct built-in calls */
    __builtin_memset((void*)src_buffer, 0xCC, op_size);
    __builtin_memcpy((void*)dst_buffer, (void*)src_buffer, op_size);
    
    /* Pattern 2: Goto into memory block */
    int use_memmove = 1;
    
    if (use_memmove) {
        goto do_memmove;
    }
    
    __builtin_memset((void*)temp_buffer, 0, op_size);
    goto skip_memmove;
    
do_memmove:
    /* This tests flow sensitivity */
    __builtin_memmove((void*)temp_buffer, 
                     (void*)src_buffer, 
                     op_size / 2);
    
skip_memmove:
    /* Pattern 3: Overlapping regions */
    __builtin_memmove((void*)(src_buffer + 64), 
                     (void*)src_buffer, 
                     op_size - 64);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_dispatch(void) {
    int i;
    volatile char parallel_buffers[8][256];
    
    #pragma omp parallel for private(i)
    for (i = 0; i < 8; i++) {
        /* Each thread uses built-ins */
        __builtin_memset(parallel_buffers[i], i, 256);
        
        if (i % 2 == 0) {
            __builtin_memcpy(parallel_buffers[i] + 128, 
                           parallel_buffers[(i + 1) % 8], 
                           128);
        } else {
            __builtin_memmove(parallel_buffers[i], 
                            parallel_buffers[i] + 64, 
                            192);
        }
    }
}

/* Calculate hash from AST */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    /* Simple DJB2 hash */
    while (*ptr) {
        hash = ((hash << 5) + hash) + *ptr++;
    }
    
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Verify constructor ran */
    if (!g_init_flag) {
        printf("Warning: Constructor not executed\n");
    }
    
    /* Phase 1: Create recursive structure */
    printf("Creating AST structure...\n");
    ASTNode* root = create_ast_node(4, 1);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Perform complex memory operations */
    printf("Performing memory operations...\n");
    perform_memory_operations();
    
    /* Phase 3: Parallel execution */
    printf("Executing parallel memory dispatch...\n");
    parallel_memory_dispatch();
    
    /* Phase 4: Verify results */
    printf("Computing verification hash...\n");
    unsigned long final_hash = compute_ast_hash(root);
    
    /* Additional built-in calls in main */
    volatile char final_buffer[1024];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer + 512, root->data, 256);
    __builtin_memmove(final_buffer, final_buffer + 256, 768);
    
    printf("Final hash: 0x%08lx\n", final_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real code, would need to free AST recursively */
    
    return 0;
}
