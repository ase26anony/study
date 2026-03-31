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
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Constructor attribute to force early initialization */
__attribute__((constructor))
static void init_asan_early(void) {
    /* Force built-in usage before main */
    char buf1[256], buf2[256];
    __builtin_memset(buf1, 0xAA, sizeof(buf1));
    __builtin_memcpy(buf2, buf1, sizeof(buf1));
    __builtin_memmove(buf1, buf2, sizeof(buf1));
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    /* Final built-in calls */
    char final_buf[64];
    __builtin_memset(final_buf, 0xFF, 64);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use built-ins with volatile lengths */
    __builtin_memset(node->data, depth, (size_t)g_memset_len % 256);
    
    node->type = depth;
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    /* Copy data between nodes if children exist */
    if (node->left && node->right) {
        __builtin_memcpy(node->right->data, node->left->data, 
                        (size_t)g_memcpy_len % 256);
    }
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(ASTNode* node1, ASTNode* node2) {
    if (!node1 || !node2) return;
    
    int state = 0;
    
    /* Jump into memory operation block */
    goto entry_point;
    
memory_ops:
    /* This block contains the critical built-in */
    __builtin_memmove(node1->data, node2->data, 
                     (size_t)g_memmove_len % 256);
    state = 1;
    goto exit_point;
    
entry_point:
    /* Jump to memory ops */
    if (node1->type > node2->type) {
        goto memory_ops;
    }
    
    /* Alternative path */
    __builtin_memset(node1->data, 0xCC, 128);
    
exit_point:
    /* Verify state */
    if (state) {
        __builtin_memcpy(node2->data, node1->data, 64);
    }
}

/* OpenMP parallel memory dispatcher */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count - 1; i++) {
            /* Each thread uses built-ins */
            volatile size_t local_len = (g_memcpy_len + tid) % 128;
            
            if (nodes[i] && nodes[i + 1]) {
                __builtin_memcpy(nodes[i]->data, nodes[i + 1]->data, 
                                local_len + 1);
                
                /* Nested memory operation */
                char tmp[256];
                __builtin_memset(tmp, tid, 64);
                __builtin_memmove(nodes[i]->data + 64, tmp, 32);
            }
        }
        
        /* Thread-private memory operations */
        char private_buf[512];
        __builtin_memset(private_buf, tid, sizeof(private_buf));
        
        #pragma omp barrier
        
        /* Collective operation */
        #pragma omp single
        {
            if (count > 0 && nodes[0]) {
                __builtin_memset(nodes[0]->data, 0xEE, 256);
            }
        }
    }
}

/* Compute hash from AST */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    
    /* Hash data using memory operations */
    char buffer[256];
    __builtin_memcpy(buffer, node->data, 256);
    
    for (int i = 0; i < 256; i++) {
        hash = ((hash << 5) + hash) + buffer[i];
    }
    
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create complex AST structure */
    ASTNode* root = create_ast(4);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create sibling nodes for memmove testing */
    ASTNode* nodes[8];
    nodes[0] = root;
    for (int i = 1; i < 8; i++) {
        nodes[i] = create_ast(3);
    }
    
    /* Test goto edge cases */
    process_with_goto(nodes[0], nodes[1]);
    
    /* OpenMP parallel operations */
    parallel_memory_ops(nodes, 8);
    
    /* Additional built-in calls in main */
    char main_buf[1024];
    __builtin_memset(main_buf, 0x55, sizeof(main_buf));
    __builtin_memcpy(main_buf + 512, main_buf, 512);
    __builtin_memmove(main_buf, main_buf + 256, 768);
    
    /* Compute and print verification result */
    unsigned long total_hash = 0;
    for (int i = 0; i < 8; i++) {
        if (nodes[i]) {
            total_hash ^= compute_ast_hash(nodes[i]);
        }
    }
    
    printf("Verification hash: 0x%08lx\n", total_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    for (int i = 1; i < 8; i++) {
        free(nodes[i]);
    }
    free(root);
    
    return 0;
}
