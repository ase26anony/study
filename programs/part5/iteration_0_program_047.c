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
struct ast_node {
    int type;
    char data[256];
    struct ast_node *left;
    struct ast_node *right;
    struct ast_node *next;
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force __builtin_memset initialization */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    printf("[constructor] Initialized early ASAN hooks\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_late(void) {
    volatile char buffer[16];
    /* Force __builtin_memcpy in destructor */
    char src[] = "destructor";
    __builtin_memcpy(buffer, src, sizeof(src));
    printf("[destructor] Cleaned up ASAN resources\n");
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    struct ast_node* node = (struct ast_node*)malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Initialize with __builtin_memset */
    __builtin_memset(node, 0, sizeof(*node));
    node->type = depth;
    
    /* Use volatile length for memset */
    volatile size_t local_len = 128;
    __builtin_memset(node->data, 'A' + depth, local_len % sizeof(node->data));
    
    /* Build children recursively */
    node->left = build_ast(depth + 1, max_depth);
    node->right = build_ast(depth + 2, max_depth);
    
    /* Copy data between nodes if both children exist */
    if (node->left && node->right) {
        volatile size_t copy_len = g_memcpy_len;
        if (copy_len > sizeof(node->left->data))
            copy_len = sizeof(node->left->data);
        
        /* Force __builtin_memcpy with volatile length */
        __builtin_memcpy(node->right->data, node->left->data, copy_len);
    }
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(struct ast_node* node) {
    if (!node) return;
    
    char temp[512];
    int use_memmove = 1;
    
    /* Jump into block with memmove */
    if (node->type % 2 == 0) {
        goto memmove_block;
    }
    
    skip_memmove:
    /* Normal processing */
    __builtin_memcpy(temp, node->data, g_memcpy_len % sizeof(temp));
    return;
    
    memmove_block:
    {
        volatile size_t move_len = g_memmove_len;
        if (move_len > sizeof(temp)) move_len = sizeof(temp);
        
        /* This should trigger __builtin_memmove redirection */
        __builtin_memmove(temp, node->data, move_len);
        
        if (use_memmove) {
            use_memmove = 0;
            goto skip_memmove;  /* Jump out of block */
        }
    }
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(struct ast_node** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            char buffer[256];
            volatile size_t op_len = (g_memcpy_len + i) % sizeof(buffer);
            
            /* Mix of memory builtins in parallel region */
            __builtin_memset(buffer, i, op_len);
            __builtin_memcpy(nodes[i]->data, buffer, op_len);
            
            /* Conditional memmove */
            if (i % 3 == 0) {
                char temp[256];
                __builtin_memmove(temp, nodes[i]->data, op_len);
                __builtin_memcpy(nodes[i]->data, temp, op_len);
            }
        }
    }
}

/* Multi-stage processing function */
static unsigned long process_ast(struct ast_node* node) {
    unsigned long hash = 0;
    struct ast_node* current = node;
    
    while (current) {
        /* Process current node */
        for (size_t i = 0; i < sizeof(current->data); i++) {
            hash = (hash * 31) + current->data[i];
        }
        
        /* Recursive processing with memory operations */
        if (current->left) {
            char temp[256];
            volatile size_t len = g_memcpy_len % sizeof(temp);
            __builtin_memcpy(temp, current->left->data, len);
            hash += process_ast(current->left);
        }
        
        if (current->right) {
            hash += process_ast(current->right);
        }
        
        /* Move to next sibling with memmove */
        if (current->next) {
            char buffer[512];
            volatile size_t move_len = g_memmove_len % sizeof(buffer);
            __builtin_memmove(buffer, current->data, move_len);
        }
        
        current = current->next;
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Build AST structure */
    struct ast_node* root = build_ast(0, 4);
    if (!root) {
        fprintf(stderr, "Failed to build AST\n");
        return 1;
    }
    
    /* Create node array for parallel processing */
    struct ast_node* nodes[8];
    nodes[0] = root;
    for (int i = 1; i < 8; i++) {
        nodes[i] = build_ast(i % 3, 3);
    }
    
    /* Test goto flow control */
    process_with_goto(root);
    
    /* Parallel memory operations */
    parallel_memory_ops(nodes, 8);
    
    /* Multi-stage processing */
    unsigned long result = process_ast(root);
    printf("Processing result: %lu\n", result);
    
    /* Final memory operations in main */
    volatile char final_buffer[1024];
    volatile size_t final_len = g_memset_len;
    if (final_len > sizeof(final_buffer))
        final_len = sizeof(final_buffer);
    
    __builtin_memset(final_buffer, 0xCC, final_len);
    
    /* Use all three builtins in sequence */
    char src[1024];
    __builtin_memset(src, 0xDD, sizeof(src));
    __builtin_memcpy(final_buffer, src, g_memcpy_len % sizeof(final_buffer));
    __builtin_memmove(final_buffer + 128, final_buffer, 256);
    
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* ... cleanup code would go here ... */
    
    return 0;
}
