/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array for parser */
static const char* tokens[] = {"memcpy", "memset", "memmove", "data", "node", "end"};
static volatile int token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    /* Force initialization of ASAN structures */
    char local_buf[256];
    __builtin_memset(local_buf, 0xAA, sizeof(local_buf));
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_globals(void) {
    /* Final memory operation to test cleanup paths */
    volatile char final_buf[128];
    __builtin_memset((void*)final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser using memory built-ins */
static ASTNode* parse_expression(int depth) {
    if (depth <= 0 || token_idx >= 6) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->size = sizeof(ASTNode);
    
    /* Copy token data using memcpy */
    const char* current_token = tokens[token_idx++];
    size_t token_len = strlen(current_token);
    if (token_len > 63) token_len = 63;
    
    __builtin_memcpy(node->data, current_token, token_len);
    node->data[token_len] = '\0';
    
    /* Control flow with goto to test edge cases */
    if (depth > 1) {
        int use_left = (depth % 2 == 0);
        
        if (use_left) {
            goto parse_left;
        } else {
            goto parse_right;
        }
        
    parse_left:
        node->left = parse_expression(depth - 1);
        goto skip_right;
        
    parse_right:
        node->right = parse_expression(depth - 1);
        goto skip_left;
        
    skip_right:
        /* memmove between node fields */
        if (node->left) {
            __builtin_memmove(&node->size, &node->left->size, 
                            sizeof(node->size));
        }
        return node;
        
    skip_left:
        return node;
    }
    
    return node;
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char src_buf[512];
        char dst_buf[512];
        
        /* Initialize with memset */
        __builtin_memset(src_buf, thread_id, sizeof(src_buf));
        
        /* Copy with memcpy */
        __builtin_memcpy(dst_buf, src_buf, sizeof(src_buf));
        
        /* Move with memmove (overlapping regions) */
        __builtin_memmove(src_buf + 256, src_buf, 256);
        
        /* Complex control flow with goto */
        if (thread_id % 2 == 0) {
            goto even_thread;
        } else {
            goto odd_thread;
        }
        
    even_thread:
        /* Additional memory operation for even threads */
        __builtin_memset(dst_buf + 128, 0xEE, 64);
        goto thread_done;
        
    odd_thread:
        /* Different operation for odd threads */
        __builtin_memcpy(dst_buf + 64, src_buf + 192, 128);
        goto thread_done;
        
    thread_done:
        /* Verify the operations */
        volatile int check = 0;
        for (size_t i = 0; i < sizeof(dst_buf); i++) {
            check += dst_buf[i];
        }
        (void)check; /* Prevent unused variable warning */
    }
}

/* Multi-stage memory dispatcher */
static size_t dispatch_memory_operations(ASTNode* root) {
    size_t total_hash = 0;
    
    if (!root) return 0;
    
    /* Stage 1: Process current node */
    char temp_buf[128];
    volatile size_t op_size = g_mem_size % 128;
    
    /* Use all three built-ins in sequence */
    __builtin_memset(temp_buf, 0x55, op_size);
    __builtin_memcpy(root->data, temp_buf, 
                    (op_size < 64) ? op_size : 63);
    __builtin_memmove(temp_buf + 64, temp_buf, 64);
    
    /* Calculate hash from node data */
    for (int i = 0; i < 64; i++) {
        total_hash += root->data[i] * (i + 1);
    }
    
    /* Stage 2: Recursive processing */
    if (root->left) {
        /* Jump label for control flow testing */
        process_left:
        total_hash += dispatch_memory_operations(root->left);
        
        /* Copy between sibling nodes */
        if (root->right) {
            __builtin_memcpy(root->left->data + 32, 
                           root->right->data, 32);
        }
    }
    
    if (root->right) {
        /* Another goto for flow complexity */
        if (total_hash % 2 == 0) {
            goto process_right;
        }
        
        process_right:
        total_hash += dispatch_memory_operations(root->right);
    }
    
    return total_hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Recursive AST construction */
    ASTNode* ast_root = parse_expression(4);
    
    /* Phase 2: Parallel memory operations */
    parallel_memory_operations();
    
    /* Phase 3: Multi-stage dispatch */
    size_t final_hash = dispatch_memory_operations(ast_root);
    
    /* Phase 4: Cleanup with memory operations */
    if (ast_root) {
        /* Use memmove for node data shuffling */
        char cleanup_buf[256];
        __builtin_memcpy(cleanup_buf, ast_root->data, 64);
        __builtin_memset(ast_root->data, 0, 64);
        __builtin_memmove(ast_root->data, cleanup_buf + 32, 32);
        
        /* Free allocated memory */
        free(ast_root);
    }
    
    printf("Final hash: %zu\n", final_hash);
    printf("Test completed successfully.\n");
    
    return 0;
}
