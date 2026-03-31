/* ISO C99-compliant test program for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
struct ast_node {
    int type;
    int value;
    struct ast_node *left;
    struct ast_node *right;
    char padding[32];  /* Ensure size for memcpy operations */
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    /* Force initialization of memory function redirection */
    char buffer[64];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive function with memory operations */
static struct ast_node* create_ast(int depth, int value) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Use builtin memset to initialize node */
    __builtin_memset(node, 0, sizeof(struct ast_node));
    
    node->type = depth;
    node->value = value;
    
    /* Create children with goto for flow control */
    if (depth > 1) {
        int child_val = value * 2;
        
        /* Jump label for goto */
        create_left:
        node->left = create_ast(depth - 1, child_val);
        
        /* Use goto to jump around */
        if (node->left && (depth % 2 == 0)) {
            goto skip_right;
        }
        
        node->right = create_ast(depth - 1, child_val + 1);
        goto done;
        
        skip_right:
        node->right = NULL;
        
        done:
        /* Copy data between nodes using builtin memcpy */
        if (node->left && node->right) {
            __builtin_memcpy(&node->left->padding[0], 
                           &node->right->padding[0], 
                           16);
        }
    }
    
    return node;
}

/* Function with complex memory operations and goto */
static void process_ast(struct ast_node* root) {
    if (!root) return;
    
    struct ast_node temp;
    volatile int use_memmove = 1;
    
    /* Label for goto into memory operation block */
    if (root->left) {
        goto mem_op_block;
    }
    
    normal_path:
    __builtin_memset(&temp, 0, sizeof(temp));
    return;
    
    mem_op_block:
    {
        /* This block contains builtin memmove with goto out */
        char src[64], dst[64];
        __builtin_memset(src, 0xAA, sizeof(src));
        
        if (use_memmove) {
            /* Force memmove redirection */
            __builtin_memmove(dst, src, sizeof(src));
            goto normal_path;
        }
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    const int num_threads = 4;
    int results[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        char buffer[128];
        volatile size_t local_size = g_mem_size / (tid + 1);
        
        /* Each thread uses different memory builtins */
        switch (tid % 3) {
            case 0:
                __builtin_memset(buffer, tid, local_size);
                break;
            case 1:
                {
                    char src[128];
                    __builtin_memset(src, 0xCC, sizeof(src));
                    __builtin_memcpy(buffer, src, local_size);
                }
                break;
            case 2:
                {
                    char data[128];
                    __builtin_memset(data, 0xDD, sizeof(data));
                    __builtin_memmove(buffer, data, local_size);
                }
                break;
        }
        
        /* Compute simple hash */
        int hash = 0;
        for (size_t i = 0; i < local_size && i < sizeof(buffer); i++) {
            hash += buffer[i];
        }
        results[tid] = hash;
    }
    
    /* Verify results */
    int total = 0;
    for (int i = 0; i < num_threads; i++) {
        total += results[i];
    }
    printf("OpenMP memory ops total hash: %d\n", total);
}

/* Multi-stage initialization with memory builtins */
static void initialize_data_structures(void) {
    /* Array of structures for memcpy operations */
    struct data_block {
        char header[16];
        int values[32];
        char footer[16];
    };
    
    struct data_block src_block, dst_block;
    
    /* Initialize source with builtin memset */
    __builtin_memset(&src_block, 0, sizeof(src_block));
    __builtin_memset(src_block.header, 'H', sizeof(src_block.header));
    __builtin_memset(src_block.footer, 'F', sizeof(src_block.footer));
    
    for (int i = 0; i < 32; i++) {
        src_block.values[i] = i * i;
    }
    
    /* Copy using builtin memcpy */
    __builtin_memcpy(&dst_block, &src_block, sizeof(src_block));
    
    /* Verify copy with memmove (overlapping regions) */
    __builtin_memmove(&dst_block.values[16], &dst_block.values[0], 16 * sizeof(int));
    
    /* Compute verification sum */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst_block.values[i];
    }
    printf("Data structure verification sum: %d\n", sum);
}

int main(void) {
    printf("=== ASAN Built-in Redirection Test Program ===\n");
    
    /* Stage 1: Initialize with constructor already called */
    
    /* Stage 2: Create and process recursive AST */
    struct ast_node* ast = create_ast(4, 1);
    if (ast) {
        process_ast(ast);
        
        /* Use goto to jump into memory operation */
        volatile int flag = 1;
        if (flag) {
            goto perform_memcpy;
        }
        
        back_from_goto:
        printf("AST processing complete\n");
        
        /* Cleanup */
        free(ast);
    }
    
    /* Stage 3: OpenMP parallel operations */
    parallel_memory_ops();
    
    /* Stage 4: Data structure initialization and verification */
    initialize_data_structures();
    
    printf("=== Test program completed successfully ===\n");
    return 0;
    
perform_memcpy:
    {
        /* Isolated block with builtin memcpy */
        char data1[64], data2[64];
        __builtin_memset(data1, 0x55, sizeof(data1));
        __builtin_memcpy(data2, data1, sizeof(data1));
        
        /* Jump back */
        goto back_from_goto;
    }
}
