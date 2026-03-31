/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ast_node {
    struct ast_node* left;
    struct ast_node* right;
    char data[64];
    uint32_t hash;
} ast_node_t;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force early initialization of memory functions */
    char buffer[32];
    volatile char* volatile_ptr = buffer;
    
    /* Use all three builtins in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 8);
    __builtin_memmove(buffer + 8, buffer, 8);
    
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Additional builtin usage in destructor */
    char final_buf[16];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive tree manipulation with memory operations */
static ast_node_t* create_tree(int depth) {
    if (depth <= 0) return NULL;
    
    ast_node_t* node = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    
    /* Fill data with pattern */
    for (int i = 0; i < 63; i++) {
        node->data[i] = (char)('A' + (i % 26));
    }
    node->data[63] = '\0';
    
    /* Recursive creation */
    node->left = create_tree(depth - 1);
    node->right = create_tree(depth - 1);
    
    return node;
}

/* Complex memory operation with goto flow control */
static void process_with_goto(ast_node_t* dest, ast_node_t* src) {
    volatile int use_goto = 1;
    
    if (use_goto) {
        goto copy_block;
    }
    
    /* This block will be jumped into */
copy_block:
    /* Force memmove with goto into block */
    __builtin_memmove(dest->data, src->data, 64);
    
    if (use_goto) {
        goto hash_block;
    }
    
hash_block:
    /* Compute hash using memory operations */
    uint32_t hash = 0;
    for (int i = 0; i < 64; i += 4) {
        uint32_t chunk;
        __builtin_memcpy(&chunk, &src->data[i], 4);
        hash ^= chunk;
    }
    dest->hash = hash;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ast_node_t** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            /* Each thread uses builtins */
            if (nodes[i]) {
                /* Mix of all three builtins */
                __builtin_memset(nodes[i]->data + 32, tid, 16);
                
                if (i > 0) {
                    __builtin_memcpy(nodes[i]->data, 
                                   nodes[i-1]->data, 32);
                }
                
                /* Self-overlapping memmove */
                __builtin_memmove(nodes[i]->data + 16,
                                nodes[i]->data, 32);
            }
        }
    }
}

/* Multi-stage processing with varied memory operations */
static uint64_t process_tree(ast_node_t* root) {
    if (!root) return 0;
    
    uint64_t total = 0;
    volatile size_t local_size = g_mem_size % 128;
    
    /* Stage 1: Initial processing */
    char temp_buf[128];
    __builtin_memset(temp_buf, 0, sizeof(temp_buf));
    __builtin_memcpy(temp_buf, root->data, 64);
    
    /* Stage 2: Overlapping operations */
    __builtin_memmove(temp_buf + 32, temp_buf, 64);
    
    /* Stage 3: Variable-sized operations */
    for (int i = 0; i < 64; i += 8) {
        __builtin_memcpy(root->data + i, temp_buf + i, 8);
    }
    
    /* Recursive processing */
    total += process_tree(root->left);
    total += process_tree(root->right);
    
    /* Final hash accumulation */
    for (int i = 0; i < 64; i++) {
        total += (uint64_t)root->data[i] * root->hash;
    }
    
    return total;
}

/* Main test driver */
int main(void) {
    printf("ASAN Built-in Redirection Test\n");
    
    /* Phase 1: Tree creation and initialization */
    ast_node_t* tree = create_tree(3);
    if (!tree) {
        fprintf(stderr, "Failed to create tree\n");
        return 1;
    }
    
    /* Phase 2: Goto-based flow control */
    ast_node_t* copy = create_tree(1);
    if (copy) {
        process_with_goto(copy, tree);
    }
    
    /* Phase 3: Array of nodes for parallel processing */
    ast_node_t* nodes[8];
    nodes[0] = tree;
    for (int i = 1; i < 8; i++) {
        nodes[i] = create_tree(2);
    }
    
    /* Phase 4: OpenMP parallel operations */
    #ifdef _OPENMP
    parallel_memory_ops(nodes, 8);
    #endif
    
    /* Phase 5: Complex recursive processing */
    uint64_t result = process_tree(tree);
    
    /* Phase 6: Additional builtin usage in main */
    char final_buffer[256];
    volatile char* vol_dest = final_buffer;
    volatile char* vol_src = (char*)tree;
    
    __builtin_memset(vol_dest, 0, sizeof(final_buffer));
    __builtin_memcpy(vol_dest, vol_src, sizeof(ast_node_t));
    __builtin_memmove(vol_dest + 128, vol_dest, 128);
    
    /* Verification output */
    printf("Processing complete. Result hash: %llu\n", 
           (unsigned long long)result);
    printf("Init flag: %d\n", g_init_flag);
    
    /* Cleanup */
    free(copy);
    for (int i = 1; i < 8; i++) {
        free(nodes[i]);
    }
    
    /* Recursive free function */
    void free_tree(ast_node_t* node) {
        if (!node) return;
        free_tree(node->left);
        free_tree(node->right);
        free(node);
    }
    free_tree(tree);
    
    return 0;
}
