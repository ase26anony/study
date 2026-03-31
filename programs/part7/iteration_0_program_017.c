#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile size_t size;  /* volatile to prevent optimization */
    struct ASTNode* left;
    struct ASTNode* right;
    char padding[32];      /* Ensure size for memory operations */
} ASTNode;

/* Global volatile variables to prevent constant folding */
volatile size_t g_mem_size = 64;
volatile int g_init_flag = 0;

/* Token array for parser simulation */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "alloc", "free", "process"
};
#define TOKEN_COUNT (sizeof(tokens)/sizeof(tokens[0]))

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_environment(void) {
    /* Force early initialization of ASAN runtime */
    volatile char buffer[128];
    __builtin_memset(buffer, 0, sizeof(buffer));
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operation to ensure destructor path is taken */
    volatile int final_check[16];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth, const char** token_ptr) {
    if (depth <= 0 || **token_ptr == '\0') {
        return NULL;
    }
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with volatile size */
    node->size = g_mem_size + depth;
    node->type = depth % 3;
    node->value = (**token_ptr) * depth;
    
    /* Use all three builtins in different contexts */
    if (depth % 3 == 0) {
        /* memcpy between padding areas */
        char temp[32];
        __builtin_memcpy(temp, node->padding, sizeof(temp));
        __builtin_memcpy(node->padding, temp, sizeof(node->padding));
    } else if (depth % 3 == 1) {
        /* memset the padding */
        __builtin_memset(node->padding, node->value % 256, sizeof(node->padding));
    }
    
    /* Recursive calls */
    (*token_ptr)++;
    node->left = parse_expression(depth - 1, token_ptr);
    
    /* Jump label for goto testing */
    process_right:
    (*token_ptr)++;
    node->right = parse_expression(depth - 2, token_ptr);
    
    return node;
}

/* Function with goto for flow control testing */
static void process_with_goto(ASTNode* node) {
    if (!node) return;
    
    volatile char buffer1[256];
    volatile char buffer2[256];
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memset(buffer2, 0xBB, sizeof(buffer2));
    
    /* Goto into block with memmove */
    if (node->type == 0) {
        goto do_memmove;
    }
    
    /* Normal path with memcpy */
    __builtin_memcpy((void*)buffer1, (void*)buffer2, node->size % 128);
    goto skip_memmove;
    
do_memmove:
    /* This block should be reached via goto */
    __builtin_memmove((void*)buffer1, (void*)buffer2, node->size % 128);
    
skip_memmove:
    /* Use memset after goto */
    __builtin_memset((void*)buffer1, node->value, 64);
}

/* Parallel processing function */
static void parallel_memory_operations(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            volatile char local_buf[512];
            size_t op_size = nodes[i]->size % 256;
            
            /* Mix of all three builtins in parallel region */
            __builtin_memset(local_buf, i, sizeof(local_buf));
            
            if (i % 2 == 0) {
                __builtin_memcpy(nodes[i]->padding, local_buf, op_size);
            } else {
                __builtin_memmove(nodes[i]->padding, local_buf, op_size);
            }
            
            /* Additional memcpy for good measure */
            __builtin_memcpy(local_buf + 128, nodes[i]->padding, op_size);
        }
    }
}

/* Compute hash from AST */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    int i;
    
    /* Hash the padding area */
    for (i = 0; i < sizeof(node->padding); i++) {
        hash = ((hash << 5) + hash) + node->padding[i];
    }
    
    /* Recursive hash computation */
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

int main(void) {
    const char* token_ptr = tokens[0];
    ASTNode* root = NULL;
    ASTNode* node_array[10];
    int i;
    unsigned long final_hash = 0;
    
    printf("Starting ASAN coverage test...\n");
    
    /* Phase 1: Recursive parsing with memory operations */
    root = parse_expression(5, &token_ptr);
    
    /* Phase 2: Build node array for parallel processing */
    ASTNode* current = root;
    for (i = 0; i < 10 && current; i++) {
        node_array[i] = current;
        current = current->left ? current->left : current->right;
    }
    
    /* Fill remaining slots with copies */
    for (; i < 10; i++) {
        node_array[i] = root;
    }
    
    /* Phase 3: Goto-based flow control */
    for (i = 0; i < 5; i++) {
        process_with_goto(node_array[i % 3]);
    }
    
    /* Phase 4: OpenMP parallel memory operations */
    parallel_memory_operations(node_array, 10);
    
    /* Phase 5: Final memory operations in main */
    volatile char final_buffer[1024];
    volatile char src_buffer[1024];
    
    /* Use all three builtins in sequence */
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memset(src_buffer, 0xCC, sizeof(src_buffer));
    
    __builtin_memcpy(final_buffer, src_buffer, g_mem_size);
    __builtin_memmove(final_buffer + 128, src_buffer + 64, g_mem_size / 2);
    __builtin_memset(final_buffer + 256, 0xFF, 128);
    
    /* Compute and print result */
    final_hash = compute_ast_hash(root);
    printf("AST hash: %lu\n", final_hash);
    printf("Test completed.\n");
    
    /* Cleanup */
    /* Note: In real ASAN, memory would be freed here */
    
    return (final_hash != 0) ? 0 : 1;
}
