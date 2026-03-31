/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array for parser simulation */
static const char* tokens[] = {"memcpy", "memset", "memmove", "data", "node"};
static const int token_count = 5;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_hooks(void) {
    volatile char buffer[32];
    /* Force initialization of memcpy redirection */
    __builtin_memcpy((void*)buffer, "constructor_init", 16);
    printf("[Constructor] Initialized ASAN hooks\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan(void) {
    volatile char cleanup_buf[16];
    __builtin_memset((void*)cleanup_buf, 0, sizeof(cleanup_buf));
    printf("[Destructor] Cleaned up ASAN buffers\n");
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth, int* token_idx) {
    if (depth <= 0 || *token_idx >= token_count) {
        return NULL;
    }
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = depth * 100 + (*token_idx);
    
    /* Copy token into node data using memcpy */
    const char* current_token = tokens[*token_idx];
    size_t token_len = strlen(current_token) + 1;
    if (token_len > sizeof(node->data)) token_len = sizeof(node->data);
    __builtin_memcpy(node->data, current_token, token_len);
    
    (*token_idx)++;
    
    /* Recursive parsing with goto for flow control */
    int use_goto = (depth % 2 == 0);
    
    if (use_goto) {
        goto parse_left;
    }
    
    node->left = parse_expression(depth - 1, token_idx);
    
parse_left:
    if (use_goto) {
        node->left = parse_expression(depth - 1, token_idx);
    }
    
    /* Jump back for right subtree */
    if (depth > 2) {
        goto parse_right;
    }
    
    node->right = parse_expression(depth - 2, token_idx);
    return node;

parse_right:
    node->right = parse_expression(depth - 2, token_idx);
    return node;
}

/* Memory operation dispatcher with OpenMP */
static void dispatch_memory_operations(ASTNode* nodes[], int count) {
    volatile char src_buffer[512];
    volatile char dst_buffer[512];
    
    /* Initialize source with pattern */
    for (size_t i = 0; i < sizeof(src_buffer); i++) {
        src_buffer[i] = (char)(i % 256);
    }
    
    #pragma omp parallel num_threads(4)
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < count; i++) {
            if (nodes[i] == NULL) continue;
            
            size_t op_size = g_mem_size + thread_id * 16;
            if (op_size > sizeof(dst_buffer)) op_size = sizeof(dst_buffer);
            
            /* Use all three builtins based on conditions */
            if (i % 3 == 0) {
                /* memcpy path */
                __builtin_memcpy((void*)dst_buffer, (void*)src_buffer, op_size);
                
                /* Copy to AST node */
                size_t copy_len = (op_size < sizeof(nodes[i]->data)) ? 
                                 op_size : sizeof(nodes[i]->data);
                __builtin_memcpy(nodes[i]->data, dst_buffer, copy_len);
            }
            else if (i % 3 == 1) {
                /* memset path */
                char fill_char = (char)(thread_id + 65); /* 'A' + thread_id */
                __builtin_memset((void*)dst_buffer, fill_char, op_size);
                
                /* Also memset node data */
                __builtin_memset(nodes[i]->data, fill_char, 
                               sizeof(nodes[i]->data) / 2);
            }
            else {
                /* memmove path with overlapping regions */
                volatile char overlap_buf[256];
                __builtin_memcpy((void*)overlap_buf, (void*)src_buffer, 128);
                
                if (g_use_memmove) {
                    /* Create overlapping copy */
                    __builtin_memmove((void*)(overlap_buf + 64), 
                                     (void*)overlap_buf, 128);
                    
                    /* Copy result to node */
                    __builtin_memcpy(nodes[i]->data, overlap_buf + 64, 64);
                }
            }
        }
        
        /* Thread-local memory operation */
        volatile char thread_buf[128];
        __builtin_memset((void*)thread_buf, thread_id, sizeof(thread_buf));
        
        #pragma omp barrier
        
        /* Inter-thread memory copy */
        if (thread_id == 0) {
            for (int t = 1; t < omp_get_num_threads(); t++) {
                #pragma omp critical
                {
                    __builtin_memcpy((void*)(dst_buffer + t * 32), 
                                   (void*)thread_buf, 32);
                }
            }
        }
    }
}

/* Calculate hash of AST structure */
static unsigned long calculate_ast_hash(ASTNode* node, int depth) {
    if (node == NULL || depth <= 0) {
        return 5381;
    }
    
    unsigned long hash = 5381;
    
    /* Process node data */
    for (size_t i = 0; i < sizeof(node->data) && node->data[i] != '\0'; i++) {
        hash = ((hash << 5) + hash) + (unsigned long)node->data[i];
    }
    
    hash = ((hash << 5) + hash) + (unsigned long)node->id;
    
    /* Recursive hash calculation */
    unsigned long left_hash = calculate_ast_hash(node->left, depth - 1);
    unsigned long right_hash = calculate_ast_hash(node->right, depth - 1);
    
    /* Combine hashes */
    hash = ((hash << 5) + hash) + left_hash;
    hash = ((hash << 5) + hash) + right_hash;
    
    return hash;
}

/* Free AST memory */
static void free_ast(ASTNode* node) {
    if (node == NULL) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear node data before free */
    volatile char* data_ptr = node->data;
    __builtin_memset((void*)data_ptr, 0, sizeof(node->data));
    
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Parse expressions to create AST */
    ASTNode* ast_nodes[10];
    int token_idx = 0;
    
    for (int i = 0; i < 10; i++) {
        ast_nodes[i] = parse_expression(3 + (i % 3), &token_idx);
        if (token_idx >= token_count) token_idx = 0;
    }
    
    /* Dispatch memory operations with OpenMP */
    dispatch_memory_operations(ast_nodes, 10);
    
    /* Calculate verification hash */
    unsigned long total_hash = 0;
    for (int i = 0; i < 10; i++) {
        if (ast_nodes[i]) {
            unsigned long node_hash = calculate_ast_hash(ast_nodes[i], 3);
            total_hash ^= node_hash;
            
            /* Additional memory operation on hash result */
            volatile char hash_buffer[sizeof(unsigned long)];
            __builtin_memcpy((void*)hash_buffer, &node_hash, sizeof(node_hash));
            
            /* Use memmove on the buffer */
            __builtin_memmove((void*)(hash_buffer + 1), hash_buffer, 
                            sizeof(unsigned long) - 1);
        }
    }
    
    printf("Result hash: 0x%08lx\n", total_hash);
    
    /* Test overlapping memory regions */
    volatile char overlap_test[256];
    for (int i = 0; i < 256; i++) {
        overlap_test[i] = (char)i;
    }
    
    /* Force multiple memmove calls with different overlaps */
    __builtin_memmove((void*)(overlap_test + 32), overlap_test, 128);
    __builtin_memmove(overlap_test, (void*)(overlap_test + 64), 128);
    __builtin_memmove((void*)(overlap_test + 16), overlap_test, 240);
    
    /* Cleanup */
    for (int i = 0; i < 10; i++) {
        free_ast(ast_nodes[i]);
    }
    
    printf("Test completed successfully\n");
    return 0;
}
