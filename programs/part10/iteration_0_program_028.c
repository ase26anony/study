/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
struct ASTNode {
    char data[128];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
};

/* Global token array */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "instrument"
};
#define TOKEN_COUNT (sizeof(tokens)/sizeof(tokens[0]))

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_sanitizer_hook(void) {
    volatile char buffer[64];
    /* Force builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, tokens[0], strlen(tokens[0]));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_sanitizer_hook(void) {
    volatile int cleanup_buf[32];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive function with memory operations */
static struct ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    struct ASTNode* node = malloc(sizeof(struct ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(*node));
    node->id = id;
    
    /* Copy token data using builtin */
    const char* token = tokens[id % TOKEN_COUNT];
    size_t len = strlen(token);
    if (len > sizeof(node->data) - 1) len = sizeof(node->data) - 1;
    __builtin_memcpy(node->data, token, len);
    node->data[len] = '\0';
    
    /* Create children */
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    /* Copy between nodes if both exist */
    if (node->left && node->right) {
        size_t copy_size = sizeof(node->left->data);
        if (g_use_memmove) {
            __builtin_memmove(node->left->data + 32, 
                            node->right->data, 
                            copy_size > 32 ? 32 : copy_size);
        } else {
            __builtin_memcpy(node->left->data + 32,
                           node->right->data,
                           copy_size > 32 ? 32 : copy_size);
        }
    }
    
    return node;
}

/* Function with goto control flow */
static void process_with_goto(struct ASTNode* node) {
    if (!node) return;
    
    volatile char temp[256];
    int state = 0;
    
    /* Jump into memory operation block */
    goto entry_point;
    
memory_operation:
    /* This block tests flow sensitivity */
    __builtin_memmove(temp, node->data, strlen(node->data) + 1);
    state = 1;
    goto exit_point;
    
entry_point:
    /* Conditional goto */
    if (node->id % 3 == 0) {
        goto memory_operation;
    } else {
        __builtin_memset(temp, node->id, sizeof(temp));
    }
    
exit_point:
    /* Copy back using builtin */
    __builtin_memcpy(node->data + 64, temp, 64);
}

/* Parallel processing function */
static void parallel_memory_ops(struct ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                volatile char thread_buf[512];
                size_t op_size = g_mem_size;
                
                /* Mix of memory operations */
                __builtin_memset(thread_buf, tid, op_size);
                
                if (i % 2 == 0) {
                    __builtin_memcpy(thread_buf + 128, 
                                   nodes[i]->data, 
                                   strlen(nodes[i]->data));
                } else {
                    __builtin_memmove(thread_buf + 256,
                                    thread_buf,
                                    128);
                }
                
                /* Copy back to node */
                __builtin_memcpy(nodes[i]->data + 96,
                               thread_buf,
                               64);
            }
        }
    }
}

/* Calculate hash from AST */
static unsigned long compute_ast_hash(struct ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    /* Process string with volatile to prevent optimization */
    volatile char c;
    while ((c = *ptr++)) {
        hash = ((hash << 5) + hash) + c;
    }
    
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Create AST structure */
    struct ASTNode* root = create_ast(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process with goto flow */
    process_with_goto(root);
    
    /* Create array of nodes for parallel processing */
    struct ASTNode* node_array[8];
    node_array[0] = root;
    for (int i = 1; i < 8; i++) {
        node_array[i] = create_ast(3, i + 100);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(node_array, 8);
    
    /* Additional direct builtin calls */
    volatile char final_buffer[1024];
    size_t final_size = g_mem_size * 2;
    
    __builtin_memset(final_buffer, 0xCC, final_size);
    __builtin_memcpy(final_buffer + 512, root->data, strlen(root->data));
    __builtin_memmove(final_buffer, final_buffer + 256, 256);
    
    /* Compute and print verification hash */
    unsigned long total_hash = compute_ast_hash(root);
    
    /* Add buffer hash */
    for (size_t i = 0; i < 256; i++) {
        total_hash = ((total_hash << 3) + total_hash) + final_buffer[i];
    }
    
    printf("Verification hash: %lu\n", total_hash);
    printf("Test completed\n");
    
    /* Cleanup */
    /* Note: In real usage, would need proper AST freeing */
    
    return 0;
}
