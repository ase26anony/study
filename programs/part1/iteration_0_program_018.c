/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_token_pool[4096];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)(i % 256);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    /* Clear sensitive data */
    __builtin_memset(g_token_pool, 0, sizeof(g_token_pool));
    printf("Destructor: Token pool cleared\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int* node_id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*node_id)++;
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy pattern from global pool using __builtin_memcpy */
    size_t copy_size = sizeof(node->data) < g_mem_size ? 
                      sizeof(node->data) : (size_t)g_mem_size;
    __builtin_memcpy(node->data, &g_token_pool[g_token_index], copy_size);
    g_token_index = (g_token_index + 32) % sizeof(g_token_pool);
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, node_id);
        node->right = NULL;
        
        create_children:
        /* Jump target for goto */
        node->right = create_ast(depth - 2, node_id);
        
        if (!use_goto) {
            node->left = create_ast(depth - 1, node_id);
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void process_with_goto(ASTNode* node, int level) {
    if (!node) return;
    
    char buffer[128];
    volatile int do_copy = 1;
    
    /* Label for goto into memory operation block */
    if (level % 2 == 0) {
        goto memory_block;
    }
    
    /* Normal path */
    __builtin_memset(buffer, 0xA5, sizeof(buffer));
    
    memory_block:
    /* Jump target - contains __builtin_memmove */
    if (do_copy && node->left) {
        /* Use volatile to control operation */
        volatile size_t move_size = sizeof(node->data);
        
        if (g_use_memmove) {
            /* This should trigger the memmove redirection */
            __builtin_memmove(node->right->data, node->left->data, move_size);
        } else {
            __builtin_memcpy(node->right->data, node->left->data, move_size);
        }
        
        /* Jump out of the block */
        if (level > 3) goto after_memory;
    }
    
    /* Process children */
    process_with_goto(node->left, level + 1);
    
    after_memory:
    process_with_goto(node->right, level + 1);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode* root) {
    int sum = 0;
    
    #pragma omp parallel reduction(+:sum)
    {
        int thread_id = omp_get_thread_num();
        char local_buf[256];
        
        /* Each thread performs memory operations */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            /* Initialize with pattern */
            __builtin_memset(local_buf, thread_id, sizeof(local_buf));
            
            /* Copy to/from global pool */
            size_t offset = (thread_id * 64 + i * 8) % sizeof(g_token_pool);
            __builtin_memcpy(&local_buf[128], &g_token_pool[offset], 64);
            
            /* Conditional memmove */
            if (i % 7 == 0) {
                __builtin_memmove(&local_buf[64], &local_buf[128], 32);
            }
            
            /* Compute simple hash */
            for (int j = 0; j < 64; j++) {
                sum += local_buf[j];
            }
        }
        
        /* Thread-specific AST processing */
        if (thread_id == 0 && root) {
            char temp[64];
            __builtin_memcpy(temp, root->data, sizeof(temp));
            __builtin_memset(root->data, 0, sizeof(root->data));
            __builtin_memcpy(root->data, temp, sizeof(temp));
        }
    }
    
    printf("Parallel sum: %d\n", sum);
}

/* Multi-stage initialization */
static void initialize_subsystem(void) {
    static volatile int initialized = 0;
    
    if (!initialized) {
        /* Force multiple memory builtin calls */
        char init_buf[512];
        
        __builtin_memset(init_buf, 0xFF, sizeof(init_buf));
        __builtin_memcpy(&init_buf[256], g_token_pool, 256);
        __builtin_memmove(&init_buf[128], &init_buf[256], 128);
        
        initialized = 1;
    }
}

/* Main execution flow */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Stage 1: Subsystem initialization */
    initialize_subsystem();
    
    /* Stage 2: Create recursive AST */
    int node_id = 0;
    ASTNode* root = create_ast(5, &node_id);
    printf("Created AST with %d nodes\n", node_id);
    
    /* Stage 3: Process with goto flow control */
    process_with_goto(root, 0);
    
    /* Stage 4: Parallel memory operations */
    parallel_memory_ops(root);
    
    /* Stage 5: Additional builtin stress test */
    {
        char final_buf[1024];
        volatile size_t sizes[] = {16, 32, 64, 128, 256};
        
        for (int i = 0; i < 5; i++) {
            size_t s = sizes[i];
            
            /* Mix all three builtins */
            __builtin_memset(final_buf, i, s);
            __builtin_memcpy(&final_buf[s], final_buf, s/2);
            __builtin_memmove(final_buf, &final_buf[s/2], s/2);
            
            /* Verify by computing checksum */
            int checksum = 0;
            for (size_t j = 0; j < s; j++) {
                checksum += final_buf[j];
            }
            printf("Buffer %d checksum: %d\n", i, checksum);
        }
    }
    
    /* Cleanup */
    /* Note: In real code, you'd want to free the AST recursively */
    
    printf("=== Test completed ===\n");
    return 0;
}
