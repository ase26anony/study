#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* AST-like recursive structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile int volatile_marker;  /* Prevent optimization */
    struct ASTNode *left;
    struct ASTNode *right;
    char data[64];
} ASTNode;

/* Global token array */
static volatile int token_array[256];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force initialization of builtins in constructor context */
    volatile char buffer[128];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    token_index = 42;  /* Non-zero initial value */
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    volatile char cleanup_buf[64];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth, volatile int* counter) {
    if (depth <= 0 || *counter >= 100) {
        ASTNode* leaf = malloc(sizeof(ASTNode));
        if (!leaf) return NULL;
        
        /* Use all three builtins in leaf creation */
        __builtin_memset(leaf, 0, sizeof(ASTNode));
        leaf->type = *counter;
        leaf->value = depth;
        
        /* Volatile prevents constant folding */
        volatile int pattern = 0xDEADBEEF;
        __builtin_memcpy(leaf->data, &pattern, sizeof(pattern));
        
        (*counter)++;
        return leaf;
    }
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    node->volatile_marker = *counter;
    
    /* Recursive calls */
    (*counter)++;
    node->left = parse_expression(depth - 1, counter);
    
    /* Jump label for goto testing */
    process_right:
    (*counter) += 2;
    node->right = parse_expression(depth - 2, counter);
    
    /* Copy data between nodes if both exist */
    if (node->left && node->right) {
        /* Use memmove for potentially overlapping regions */
        __builtin_memmove(node->data, node->left->data, 32);
        
        /* Conditional goto to test flow sensitivity */
        if (node->type % 3 == 0) {
            goto skip_copy;
        }
        
        /* Use memcpy for non-overlapping */
        __builtin_memcpy(node->right->data + 16, node->data, 16);
        
        skip_copy:
        /* Another memmove with goto */
        if (node->type % 5 == 0) {
            goto process_right;  /* Jump back */
        }
    }
    
    return node;
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    volatile char buffers[8][256];
    volatile int patterns[8];
    
    #pragma omp parallel for
    for (int i = 0; i < 8; i++) {
        patterns[i] = i * 0x11111111;
        
        /* Force all three builtins in parallel region */
        __builtin_memset(buffers[i], i, sizeof(buffers[i]));
        
        if (i % 2 == 0) {
            __builtin_memcpy(buffers[i] + 64, &patterns[i], sizeof(int));
        } else {
            /* Overlapping memmove */
            __builtin_memmove(buffers[i] + 128, buffers[i] + 32, 64);
        }
        
        /* Complex goto pattern in parallel region */
        if (i == 3) {
            goto special_case;
        }
        
        continue;
        
        special_case:
        __builtin_memset(buffers[i] + 192, 0xFF, 32);
    }
}

/* Tree traversal with memory operations */
static int traverse_and_hash(ASTNode* node, int depth) {
    if (!node) return 0;
    
    int hash = node->type ^ node->value;
    
    /* Volatile access prevents dead code elimination */
    volatile int temp = node->volatile_marker;
    
    /* Memory operation in traversal */
    char local_buf[48];
    __builtin_memcpy(local_buf, node->data, 32);
    
    /* Conditional goto around memmove */
    if (depth % 2 == 0) {
        goto skip_memmove;
    }
    
    __builtin_memmove(local_buf + 16, local_buf, 16);
    
    skip_memmove:
    hash ^= *(int*)local_buf;
    
    /* Recursive traversal */
    hash ^= traverse_and_hash(node->left, depth + 1);
    
    /* Another goto target */
    process_right_branch:
    hash ^= traverse_and_hash(node->right, depth + 1);
    
    /* Final memset before return */
    __builtin_memset(local_buf, 0, sizeof(local_buf));
    
    return hash;
}

/* Free tree with memory sanitization */
static void free_tree(ASTNode* node) {
    if (!node) return;
    
    free_tree(node->left);
    
    /* Goto for flow control */
    if (node->type < 0) {
        goto free_right;
    }
    
    free_tree(node->right);
    
    /* Clear memory before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
    return;
    
    free_right:
    free_tree(node->right);
    __builtin_memset(node, 0, sizeof(ASTNode));
    free(node);
}

int main(void) {
    volatile int counter = 0;
    int final_hash = 0;
    
    /* Initialize token array with memset */
    __builtin_memset((void*)token_array, 0xCC, sizeof(token_array));
    
    /* Create AST with recursive parser */
    ASTNode* root = parse_expression(5, &counter);
    
    /* Parallel memory operations */
    parallel_memory_ops();
    
    /* Traverse and compute hash */
    final_hash = traverse_and_hash(root, 0);
    
    /* Additional builtin calls in main */
    volatile char main_buffer[512];
    __builtin_memset(main_buffer, 0xAA, sizeof(main_buffer));
    
    /* Test all three builtins with goto */
    if (final_hash % 2 == 0) {
        goto use_memcpy;
    }
    
    __builtin_memmove(main_buffer + 256, main_buffer, 128);
    goto done;
    
    use_memcpy:
    __builtin_memcpy(main_buffer + 384, token_array, 64);
    
    done:
    /* Final memset */
    __builtin_memset(main_buffer + 448, final_hash & 0xFF, 32);
    
    /* Cleanup */
    free_tree(root);
    
    /* Print verification result */
    printf("Result: 0x%08X\n", final_hash);
    fflush(stdout);
    
    return final_hash != 0 ? 0 : 1;
}
