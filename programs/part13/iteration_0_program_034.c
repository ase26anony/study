/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _OPENMP
#include <omp.h>
#endif

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char *data;
    size_t size;
    struct ASTNode *left;
    struct ASTNode *right;
    uint32_t checksum;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Force early initialization of ASAN runtime */
    volatile char buffer[64];
    __builtin_memset(buffer, 0, sizeof(buffer));
    printf("[Constructor] Initialized ASAN runtime\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    printf("[Destructor] ASAN cleanup complete\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char *base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->size = g_mem_size % 128 + 128; /* Non-constant size */
    node->data = malloc(node->size);
    
    /* Use __builtin_memset to initialize */
    __builtin_memset(node->data, 0, node->size);
    
    /* Copy base data using __builtin_memcpy */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > node->size) copy_len = node->size;
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Calculate checksum */
    node->checksum = 0;
    for (size_t i = 0; i < copy_len; i++) {
        node->checksum = (node->checksum * 31) + node->data[i];
    }
    
    /* Recursive creation with goto for control flow */
    int use_left = 1;
    
    if (depth > 2) {
        use_left = 0;
        goto skip_left;
    }
    
    node->left = create_ast(depth - 1, "left_branch");
    
skip_left:
    if (use_left) {
        /* This path won't be taken due to goto, testing edge cases */
        volatile char temp[32];
        __builtin_memset(temp, 0xFF, sizeof(temp));
    }
    
    node->right = create_ast(depth - 1, "right_branch");
    
    return node;
}

/* Function with goto jumping around memmove */
static void process_with_goto(ASTNode *src, ASTNode *dst) {
    if (!src || !dst || src->size != dst->size) return;
    
    int stage = 0;
    
    if (g_use_memmove) {
        stage = 1;
        goto perform_memmove;
    }
    
    /* Normal path */
    __builtin_memcpy(dst->data, src->data, src->size);
    goto finish;
    
perform_memmove:
    /* Use __builtin_memmove with potential overlap */
    size_t half = src->size / 2;
    __builtin_memmove(dst->data, src->data + half, half);
    __builtin_memmove(dst->data + half, src->data, half);
    
    stage = 2;
    
finish:
    /* Update checksum */
    dst->checksum = src->checksum ^ stage;
}

/* Parallel processing with OpenMP */
static uint64_t parallel_memory_operations(ASTNode **nodes, int count) {
    uint64_t total_checksum = 0;
    
    #pragma omp parallel reduction(+:total_checksum)
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Each thread uses memory builtins */
                volatile char local_buf[256];
                
                /* Use all three builtins */
                __builtin_memset(local_buf, thread_id, sizeof(local_buf));
                __builtin_memcpy(local_buf + 64, nodes[i]->data, 
                               nodes[i]->size > 192 ? 192 : nodes[i]->size);
                __builtin_memmove(local_buf + 128, local_buf + 32, 64);
                
                /* Update node with thread-specific pattern */
                __builtin_memset(nodes[i]->data + 16, thread_id, 32);
                total_checksum += nodes[i]->checksum + thread_id;
            }
        }
    }
    
    return total_checksum;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create AST structure */
    ASTNode *root = create_ast(4, "root_node");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    const int node_count = 8;
    ASTNode *nodes[node_count];
    
    for (int i = 0; i < node_count; i++) {
        char name[32];
        snprintf(name, sizeof(name), "node_%d", i);
        nodes[i] = create_ast(3, name);
    }
    
    /* Test goto with memmove */
    if (node_count >= 2) {
        process_with_goto(nodes[0], nodes[1]);
    }
    
    /* Perform parallel memory operations */
    uint64_t result = parallel_memory_operations(nodes, node_count);
    
    /* Additional memory operations in main */
    volatile char main_buffer[512];
    volatile int use_memmove = g_use_memmove;
    
    __builtin_memset(main_buffer, 0xAA, sizeof(main_buffer));
    
    if (use_memmove) {
        __builtin_memmove(main_buffer + 256, main_buffer, 256);
    }
    
    __builtin_memcpy(main_buffer + 384, root->data, 
                    root->size > 128 ? 128 : root->size);
    
    /* Calculate final verification hash */
    uint64_t final_hash = result;
    for (size_t i = 0; i < sizeof(main_buffer); i += 64) {
        final_hash = (final_hash * 6364136223846793005ULL) + main_buffer[i];
    }
    
    printf("Result: 0x%016llX\n", (unsigned long long)final_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* ... cleanup code would go here ... */
    
    return 0;
}
