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
    char *data;
    size_t size;
    struct ASTNode *left;
    struct ASTNode *right;
    int id;
};

/* Global token array */
static char g_token_array[4096];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Force initialization of ASAN runtime early */
    volatile char buffer[64];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    
    /* Initialize token array with pattern */
    for (int i = 0; i < (int)sizeof(g_token_array); i++) {
        g_token_array[i] = (char)(i % 256);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    volatile char verify[32];
    __builtin_memset(verify, 0xFF, sizeof(verify));
}

/* Recursive function with memory operations */
static struct ASTNode* create_ast(int depth, size_t base_size) {
    if (depth <= 0) return NULL;
    
    struct ASTNode *node = malloc(sizeof(struct ASTNode));
    if (!node) return NULL;
    
    node->size = base_size * depth;
    node->data = malloc(node->size);
    node->id = depth;
    
    if (!node->data) {
        free(node);
        return NULL;
    }
    
    /* Use __builtin_memset with volatile size */
    volatile size_t fill_size = node->size;
    __builtin_memset(node->data, depth, fill_size);
    
    /* Create children recursively */
    node->left = create_ast(depth - 1, base_size);
    node->right = create_ast(depth - 1, base_size);
    
    return node;
}

/* Function with goto and memory operations */
static void process_with_goto(struct ASTNode *src, struct ASTNode *dst) {
    if (!src || !dst || src->size != dst->size) return;
    
    int use_copy = 1;
    
    /* Jump into memory operation block */
    if (use_copy) goto do_copy;
    
    skip_copy:
        return;
    
    do_copy: {
        /* Force __builtin_memcpy redirection */
        volatile size_t copy_size = src->size;
        __builtin_memcpy(dst->data, src->data, copy_size);
        
        /* Jump out */
        goto skip_copy;
    }
}

/* Parallel memory dispatch */
static void parallel_memory_ops(struct ASTNode **nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i] && nodes[i]->data) {
                /* Mixed memory operations */
                volatile size_t op_size = g_mem_size;
                
                if (g_use_memmove && i % 3 == 0) {
                    /* Create overlapping regions for memmove */
                    char *mid = nodes[i]->data + (nodes[i]->size / 2);
                    __builtin_memmove(mid, nodes[i]->data, op_size / 2);
                } else if (i % 3 == 1) {
                    /* Use memset */
                    __builtin_memset(nodes[i]->data, tid, op_size);
                } else {
                    /* Use memcpy between token array and node */
                    size_t offset = (tid * 64) % sizeof(g_token_array);
                    __builtin_memcpy(nodes[i]->data, 
                                   g_token_array + offset, 
                                   op_size);
                }
            }
        }
    }
}

/* Compute verification hash */
static unsigned long compute_hash(struct ASTNode *node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    for (size_t i = 0; i < node->size && i < 1024; i++) {
        hash = ((hash << 5) + hash) + (unsigned char)node->data[i];
    }
    
    hash ^= compute_hash(node->left);
    hash ^= compute_hash(node->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create AST structure */
    struct ASTNode *root = create_ast(4, 128);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create sibling node for memcpy/memmove tests */
    struct ASTNode *sibling = create_ast(4, 128);
    if (!sibling) {
        fprintf(stderr, "Failed to create sibling node\n");
        return 1;
    }
    
    /* Test goto with memcpy */
    process_with_goto(root, sibling);
    
    /* Create node array for parallel processing */
    struct ASTNode *nodes[8];
    nodes[0] = root;
    nodes[1] = sibling;
    for (int i = 2; i < 8; i++) {
        nodes[i] = create_ast(3, 64);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(nodes, 8);
    
    /* Additional built-in calls in main flow */
    volatile char temp_buffer[512];
    volatile size_t temp_size = sizeof(temp_buffer);
    
    /* Ensure all three builtins are called */
    __builtin_memset(temp_buffer, 0xCC, temp_size);
    __builtin_memcpy(temp_buffer + 128, g_token_array, 256);
    
    if (g_use_memmove) {
        __builtin_memmove(temp_buffer, temp_buffer + 64, 128);
    }
    
    /* Compute verification result */
    unsigned long final_hash = 0;
    for (int i = 0; i < 8; i++) {
        if (nodes[i]) {
            final_hash ^= compute_hash(nodes[i]);
        }
    }
    
    /* Mix in temp buffer */
    for (size_t i = 0; i < sizeof(temp_buffer); i += 64) {
        final_hash ^= (unsigned long)temp_buffer[i];
    }
    
    printf("Verification hash: 0x%08lx\n", final_hash);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        if (nodes[i]) {
            free(nodes[i]->data);
            free(nodes[i]);
        }
    }
    
    printf("Test completed successfully\n");
    return 0;
}
