/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 96;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Global token array */
static char g_token_pool[1024];

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force early initialization of memory functions */
    volatile char buffer[16];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer, "constructor", 11);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    volatile char cleanup_buf[32];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile lengths */
    __builtin_memset(node, 0, sizeof(ASTNode));
    __builtin_memcpy(node->data, base_data, 
                     g_memcpy_len < 32 ? g_memcpy_len : 31);
    node->data[31] = '\0';
    
    node->type = depth;
    node->left = create_ast(depth - 1, base_data);
    node->right = create_ast(depth - 1, base_data);
    
    /* Copy between nodes if both children exist */
    if (node->left && node->right) {
        __builtin_memmove(node->left->data, node->right->data, 16);
    }
    
    return node;
}

/* Function with goto jumps around memmove */
static void goto_memmove_test(char* dest, char* src, size_t len) {
    int use_memmove = 1;
    
    if (len > 100) goto skip_op;
    
    /* Jump into memory operation block */
    goto do_operation;
    
skip_op:
    use_memmove = 0;
    return;
    
do_operation:
    if (use_memmove) {
        /* This should trigger the builtin redirection */
        __builtin_memmove(dest, src, len);
    }
    
    /* Jump out */
    goto finish;
    
finish:
    return;
}

/* Parallel memory dispatch */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[256];
        char shared_buf[256];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, tid, sizeof(local_buf));
        
        #pragma omp barrier
        
        if (tid == 0) {
            __builtin_memcpy(shared_buf, local_buf, g_memcpy_len);
        }
        
        #pragma omp barrier
        
        __builtin_memmove(local_buf, shared_buf, g_memmove_len);
    }
}

/* Main execution flow */
int main(void) {
    /* Initialize token array with volatile control */
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)(i % 256);
    }
    
    /* Create recursive structure */
    ASTNode* root = create_ast(4, "AST_Base_Data");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Test goto with memmove */
    char src_buf[256], dst_buf[256];
    for (int i = 0; i < 256; i++) {
        src_buf[i] = (char)i;
    }
    
    goto_memmove_test(dst_buf, src_buf, g_memmove_len);
    
    /* Execute parallel operations */
    parallel_memory_ops();
    
    /* Additional builtin calls in varied contexts */
    char stack_buf1[512], stack_buf2[512];
    
    /* Chain of memory operations */
    __builtin_memset(stack_buf1, 0x5A, g_memset_len);
    __builtin_memcpy(stack_buf2, stack_buf1, g_memcpy_len);
    __builtin_memmove(stack_buf1 + 128, stack_buf2 + 64, g_memmove_len);
    
    /* Compute verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(stack_buf1); i++) {
        hash = (hash * 31) + (unsigned char)stack_buf1[i];
    }
    
    /* Traverse AST for additional verification */
    unsigned long ast_hash = 0;
    ASTNode* nodes[16];
    int node_count = 0;
    
    nodes[node_count++] = root;
    for (int i = 0; i < node_count && i < 16; i++) {
        ASTNode* node = nodes[i];
        for (int j = 0; j < 32 && node->data[j]; j++) {
            ast_hash = (ast_hash * 17) + (unsigned char)node->data[j];
        }
        if (node->left) nodes[node_count++] = node->left;
        if (node->right) nodes[node_count++] = node->right;
    }
    
    printf("Memory hash: %lu\n", hash);
    printf("AST hash: %lu\n", ast_hash);
    printf("Verification: %s\n", 
           (hash != 0 && ast_hash != 0) ? "PASS" : "FAIL");
    
    /* Cleanup */
    /* Recursive free omitted for brevity - would need implementation */
    
    return 0;
}
