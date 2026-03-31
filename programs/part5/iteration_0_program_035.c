/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_trigger = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
char global_tokens[1024];
int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Force initialization of memory builtins in constructor context */
    char local_buf[32];
    __builtin_memset(local_buf, 0xAA, sizeof(local_buf));
    __builtin_memcpy(global_tokens, local_buf, 16);
    
    /* Initialize token array with pattern */
    for (int i = 0; i < sizeof(global_tokens); i++) {
        global_tokens[i] = (char)(i % 256);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operation in destructor */
    char final_buf[64];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtin memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy token data using builtin memcpy */
    size_t copy_len = (size_t)(volatile_trigger * 32);
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    __builtin_memcpy(node->data, &global_tokens[token_index], copy_len);
    
    node->id = id;
    token_index = (token_index + 32) % sizeof(global_tokens);
    
    /* Recursive creation with goto for flow control */
    int use_left = 1;
    
    if (depth > 2) {
        use_left = volatile_trigger & 1;
        if (use_left) {
            goto create_left;
        } else {
            goto create_right;
        }
    }
    
create_left:
    node->left = create_ast(depth - 1, id * 2);
    if (!use_left) goto skip_right;
    
create_right:
    node->right = create_ast(depth - 1, id * 2 + 1);
    
skip_right:
    return node;
}

/* Function with goto jumping into memory operation block */
void process_with_goto(ASTNode* src, ASTNode* dst) {
    int mode = volatile_trigger % 3;
    
    if (mode == 0) {
        goto direct_copy;
    } else if (mode == 1) {
        goto memset_first;
    } else {
        goto move_operation;
    }

direct_copy:
    /* Jump into memcpy block */
    __builtin_memcpy(dst->data, src->data, volatile_len % sizeof(src->data));
    goto after_ops;
    
memset_first:
    __builtin_memset(dst->data, 0xCC, volatile_len % sizeof(dst->data));
    goto move_operation;
    
move_operation:
    /* Use memmove for overlapping regions */
    if (src == dst) {
        __builtin_memmove(dst->data + 10, dst->data, 50);
    } else {
        __builtin_memmove(dst->data, src->data, volatile_len % sizeof(src->data));
    }
    
after_ops:
    return;
}

/* Parallel processing function */
void parallel_memory_operations(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes)
    for (i = 0; i < count - 1; i++) {
        /* Each thread uses memory builtins */
        size_t len = (volatile_len + i) % 128;
        
        if (i % 3 == 0) {
            __builtin_memcpy(nodes[i+1]->data, nodes[i]->data, len);
        } else if (i % 3 == 1) {
            __builtin_memset(nodes[i]->data, i, len);
        } else {
            __builtin_memmove(nodes[i]->data + 20, nodes[i]->data, len);
        }
    }
}

/* Calculate hash from AST */
unsigned long calculate_ast_hash(ASTNode* root) {
    if (!root) return 0;
    
    unsigned long hash = 5381;
    char* p = root->data;
    
    /* Process data with volatile length */
    for (size_t i = 0; i < (volatile_len % sizeof(root->data)); i++) {
        hash = ((hash << 5) + hash) + p[i];
    }
    
    hash += calculate_ast_hash(root->left);
    hash += calculate_ast_hash(root->right);
    
    return hash;
}

int main(void) {
    /* Initialize with volatile-dependent size */
    int node_count = (volatile_trigger * 8) % 16;
    if (node_count < 4) node_count = 4;
    
    /* Create AST nodes */
    ASTNode* nodes[16];
    for (int i = 0; i < node_count; i++) {
        nodes[i] = create_ast(3, i);
        if (!nodes[i]) {
            fprintf(stderr, "Failed to create node %d\n", i);
            return 1;
        }
    }
    
    /* Process with goto flow control */
    for (int i = 0; i < node_count - 1; i++) {
        process_with_goto(nodes[i], nodes[i+1]);
    }
    
    /* Parallel memory operations */
    parallel_memory_operations(nodes, node_count);
    
    /* Additional builtin calls in main */
    char temp_buffer[256];
    __builtin_memset(temp_buffer, 0xAA, sizeof(temp_buffer));
    __builtin_memcpy(temp_buffer, nodes[0]->data, 64);
    __builtin_memmove(temp_buffer + 32, temp_buffer, 64);
    
    /* Calculate and print result */
    unsigned long total_hash = 0;
    for (int i = 0; i < node_count; i++) {
        total_hash ^= calculate_ast_hash(nodes[i]);
    }
    
    printf("Result hash: %lu\n", total_hash);
    
    /* Cleanup */
    for (int i = 0; i < node_count; i++) {
        free(nodes[i]);
    }
    
    return 0;
}
