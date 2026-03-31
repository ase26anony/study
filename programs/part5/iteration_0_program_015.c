/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char* g_tokens[] = {"token1", "token2", "token3", "token4"};
static const int g_num_tokens = 4;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[128];
    /* Force builtin initialization in constructor context */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 32, "constructor_init", 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    volatile char final_buf[64];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = depth;
    
    /* Copy token data using memcpy */
    int token_idx = depth % g_num_tokens;
    __builtin_memcpy(node->data, g_tokens[token_idx], 
                    strlen(g_tokens[token_idx]) + 1);
    
    /* Recursive creation */
    node->left = create_ast(depth + 1, max_depth);
    node->right = create_ast(depth + 2, max_depth);
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    volatile int condition = 1;
    
    if (condition) goto copy_block;
    
    /* This block should be jumped into */
copy_block:
    {
        volatile char temp[256];
        /* Use memmove with overlapping regions */
        __builtin_memmove(temp, src->data, g_mem_size);
        __builtin_memmove(dst->data, temp, g_mem_size);
    }
    
    if (g_use_memmove) goto end_processing;
    
    /* Unreachable in normal flow but present for coverage */
    __builtin_memset(dst->data, 0xCC, g_mem_size);
    
end_processing:
    return;
}

/* Parallel memory dispatch */
static unsigned long parallel_memory_ops(ASTNode** nodes, int count) {
    unsigned long hash_sum = 0;
    
    #pragma omp parallel reduction(+:hash_sum)
    {
        int tid = omp_get_thread_num();
        volatile size_t local_size = g_mem_size + tid;
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            volatile char thread_buf[512];
            
            /* Mix of memory operations */
            __builtin_memset(thread_buf, tid, local_size);
            
            if (nodes[i]) {
                __builtin_memcpy(thread_buf + 128, nodes[i]->data, 
                                strlen(nodes[i]->data));
                
                /* Conditional memmove */
                if (i % 2 == 0) {
                    __builtin_memmove(nodes[i]->data, thread_buf, 64);
                }
            }
            
            /* Compute simple hash */
            for (size_t j = 0; j < local_size && j < sizeof(thread_buf); j++) {
                hash_sum += (unsigned char)thread_buf[j];
            }
        }
    }
    
    return hash_sum;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Create AST structure */
    ASTNode* root = create_ast(0, 4);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create destination nodes */
    ASTNode* dest_nodes[8];
    for (int i = 0; i < 8; i++) {
        dest_nodes[i] = (ASTNode*)malloc(sizeof(ASTNode));
        if (dest_nodes[i]) {
            __builtin_memset(dest_nodes[i], 0, sizeof(ASTNode));
        }
    }
    
    /* Test goto flow with memmove */
    if (root && dest_nodes[0]) {
        process_with_goto(root, dest_nodes[0]);
    }
    
    /* Prepare node array for parallel processing */
    ASTNode* node_array[16];
    int node_count = 0;
    
    /* Fill array with existing nodes */
    node_array[node_count++] = root;
    for (int i = 0; i < 8 && dest_nodes[i]; i++) {
        node_array[node_count++] = dest_nodes[i];
    }
    
    /* Add some NULLs for edge cases */
    for (int i = node_count; i < 16; i++) {
        node_array[i] = NULL;
    }
    
    /* Execute parallel memory operations */
    unsigned long result = parallel_memory_ops(node_array, 16);
    printf("Memory operations hash sum: %lu\n", result);
    
    /* Additional explicit builtin calls in main */
    volatile char final_buffer[1024];
    __builtin_memset(final_buffer, 0x5A, sizeof(final_buffer));
    
    /* Overlapping memmove test */
    __builtin_memmove(final_buffer + 256, final_buffer, 512);
    
    /* Final memcpy */
    char verification[64];
    __builtin_memcpy(verification, final_buffer + 128, 63);
    verification[63] = '\0';
    
    /* Cleanup */
    free_ast(root);
    for (int i = 0; i < 8; i++) {
        free(dest_nodes[i]);
    }
    
    printf("Test completed successfully\n");
    return 0;
}
