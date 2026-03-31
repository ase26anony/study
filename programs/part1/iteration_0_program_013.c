/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[32];
    uint32_t hash;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force early initialization of memory functions */
    char buffer[16];
    __builtin_memset(buffer, 0xA5, sizeof(buffer));
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Final memory operation */
    volatile char final_buf[8];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_tree(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy pattern data */
    char pattern[32];
    __builtin_memset(pattern, 'A' + depth, sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(node->data));
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
create_children:
    if (create_left) {
        node->left = create_tree(depth - 1);
        create_left = 0;
        goto create_children; /* Jump back */
    } else {
        node->right = create_tree(depth - 1);
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int use_memmove = 0;
    
    /* Jump into memory operation block */
    goto entry_point;
    
memmove_block:
    {
        char temp[32];
        /* This should trigger the memmove redirection */
        __builtin_memmove(temp, src->data, sizeof(temp));
        __builtin_memcpy(dst->data, temp, sizeof(dst->data));
    }
    goto after_operation;
    
entry_point:
    if (src == dst) {
        use_memmove = 1;
        goto memmove_block;
    }
    
    /* Normal memcpy path */
    __builtin_memcpy(dst->data, src->data, sizeof(dst->data));
    
after_operation:
    /* Compute hash */
    uint32_t hash = 0;
    for (size_t i = 0; i < sizeof(dst->data); i++) {
        hash = (hash * 31) + dst->data[i];
    }
    dst->hash = hash;
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(ASTNode** nodes, size_t count) {
    #pragma omp parallel
    {
        #pragma omp for
        for (size_t i = 0; i < count; i++) {
            volatile size_t local_size = g_mem_size;
            char buffer[128];
            
            /* Mix of memory operations */
            __builtin_memset(buffer, i, local_size % 128);
            
            if (nodes[i]) {
                __builtin_memcpy(nodes[i]->data, buffer, 
                               sizeof(nodes[i]->data) < 32 ? 
                               sizeof(nodes[i]->data) : 32);
                
                /* Conditional memmove */
                if (i > 0 && nodes[i-1]) {
                    char temp[32];
                    __builtin_memmove(temp, nodes[i-1]->data, sizeof(temp));
                    __builtin_memcpy(buffer, temp, sizeof(temp));
                }
            }
        }
    }
}

/* Multi-stage initialization */
static void initialize_system(ASTNode* root) {
    static volatile int stage = 0;
    
    /* Stage 1: Basic initialization */
    char init_buf[256];
    __builtin_memset(init_buf, 0, sizeof(init_buf));
    stage = 1;
    
    /* Stage 2: Tree processing */
    if (root) {
        process_with_goto(root, root); /* Self-copy with memmove */
        stage = 2;
    }
    
    /* Stage 3: Parallel processing */
    ASTNode* nodes[4] = {root, NULL, NULL, NULL};
    if (root) {
        nodes[1] = create_tree(2);
        nodes[2] = create_tree(3);
        nodes[3] = nodes[1]; /* Create alias for testing */
    }
    
    parallel_memory_ops(nodes, 4);
    stage = 3;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create complex tree structure */
    ASTNode* root = create_tree(4);
    if (!root) {
        fprintf(stderr, "Failed to create tree\n");
        return 1;
    }
    
    /* Multi-stage processing */
    initialize_system(root);
    
    /* Additional memory operations in main */
    volatile char main_buf[512];
    volatile size_t op_size = g_mem_size;
    
    /* Force all three builtins */
    __builtin_memset(main_buf, 0xCC, op_size % 512);
    
    char src_buf[256];
    __builtin_memset(src_buf, 0xAA, sizeof(src_buf));
    __builtin_memcpy(main_buf + 128, src_buf, sizeof(src_buf));
    
    /* Overlapping memmove */
    __builtin_memmove(main_buf + 64, main_buf + 32, 128);
    
    /* Process tree with gotos */
    ASTNode* copy = (ASTNode*)malloc(sizeof(ASTNode));
    if (copy) {
        __builtin_memset(copy, 0, sizeof(ASTNode));
        
        int use_goto = 1;
        
    goto_label:
        if (use_goto) {
            process_with_goto(root, copy);
            use_goto = 0;
            goto goto_label; /* Jump back */
        }
        
        /* Verify result */
        uint32_t final_hash = 0;
        for (size_t i = 0; i < sizeof(copy->data); i++) {
            final_hash = final_hash * 31 + copy->data[i];
        }
        
        printf("Final hash: %u\n", final_hash);
        printf("Memory operations completed successfully\n");
        
        free(copy);
    }
    
    /* Cleanup */
    free(root);
    
    return 0;
}
