/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char *data;
    size_t len;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_constructor(void) {
    g_init_flag = 1;
    printf("Constructor: Initialized volatile flag\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_destructor(void) {
    printf("Destructor: Program completed\n");
}

/* Recursive tree creation with memory operations */
static ASTNode* create_tree(int depth, const char *base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile to control allocation size */
    node->len = g_mem_size / (depth + 1);
    node->data = malloc(node->len);
    
    if (!node->data) {
        free(node);
        return NULL;
    }
    
    /* Force builtin memset initialization */
    __builtin_memset(node->data, 0, node->len);
    
    /* Copy base pattern using builtin memcpy */
    size_t copy_len = node->len < strlen(base_data) ? node->len : strlen(base_data);
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Recursive creation with goto for control flow testing */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
create_children:
    node->left = create_tree(depth - 1, base_data);
    
    /* Jump back for right child */
    if (depth % 2 == 0) {
        goto create_right;
    } else {
        node->right = create_tree(depth - 2, base_data);
        goto done;
    }
    
create_right:
    node->right = create_tree(depth - 1, base_data);
    
done:
    return node;
}

/* Tree traversal with memory operations */
static size_t traverse_and_hash(ASTNode *node) {
    if (!node) return 0;
    
    size_t hash = 0;
    char *temp_buf = malloc(node->len);
    
    if (temp_buf) {
        /* Use builtin memmove for overlapping regions */
        __builtin_memmove(temp_buf, node->data, node->len);
        
        /* Process data */
        for (size_t i = 0; i < node->len; i++) {
            hash = (hash * 31) + temp_buf[i];
        }
        
        /* Move data back */
        __builtin_memmove(node->data, temp_buf, node->len);
        free(temp_buf);
    }
    
    /* Recursive traversal */
    size_t left_hash = traverse_and_hash(node->left);
    size_t right_hash = traverse_and_hash(node->right);
    
    return hash + left_hash + right_hash;
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_ops(void) {
    const int num_buffers = 8;
    char *buffers[num_buffers];
    size_t sizes[num_buffers];
    
    #pragma omp parallel for
    for (int i = 0; i < num_buffers; i++) {
        /* Volatile-based size calculation */
        sizes[i] = g_mem_size / (i + 1);
        buffers[i] = malloc(sizes[i]);
        
        if (buffers[i]) {
            /* Pattern initialization with builtin memset */
            __builtin_memset(buffers[i], i % 256, sizes[i]);
            
            /* Conditional memcpy between buffers */
            if (i > 0) {
                size_t copy_size = sizes[i] < sizes[i-1] ? sizes[i] : sizes[i-1];
                __builtin_memcpy(buffers[i], buffers[i-1], copy_size);
            }
        }
    }
    
    /* Cleanup */
    #pragma omp parallel for
    for (int i = 0; i < num_buffers; i++) {
        if (buffers[i]) free(buffers[i]);
    }
}

/* Complex token processing with goto jumps */
static void process_tokens(char **tokens, int count) {
    char *accumulator = malloc(g_mem_size);
    if (!accumulator) return;
    
    __builtin_memset(accumulator, 0, g_mem_size);
    
    int i = 0;
    
process_loop:
    if (i >= count) goto finish;
    
    /* Jump into memory operation block */
    if (tokens[i]) {
        goto do_memmove;
    }
    
do_memmove:
    {
        size_t token_len = strlen(tokens[i]);
        size_t offset = i * 16 % g_mem_size;
        
        /* Use builtin memmove for potential overlap */
        if (offset + token_len <= g_mem_size) {
            __builtin_memmove(accumulator + offset, tokens[i], token_len);
        }
        
        i++;
        
        /* Conditional jump out */
        if (i % 3 == 0) {
            goto process_loop;
        } else {
            goto do_memmove;
        }
    }
    
finish:
    free(accumulator);
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Phase 1: Tree operations */
    ASTNode *root = create_tree(4, "ASAN_TEST_BASE");
    if (!root) {
        fprintf(stderr, "Failed to create tree\n");
        return 1;
    }
    
    size_t tree_hash = traverse_and_hash(root);
    printf("Tree hash: %zu\n", tree_hash);
    
    /* Phase 2: Parallel memory operations */
    parallel_memory_ops();
    
    /* Phase 3: Token processing with goto */
    char *tokens[] = {"token1", "token2", "token3", "token4", "token5"};
    process_tokens(tokens, 5);
    
    /* Phase 4: Direct builtin calls with volatile control */
    char buffer1[512];
    char buffer2[512];
    
    /* Use volatile to prevent constant folding */
    volatile size_t op_size = g_mem_size;
    
    __builtin_memset(buffer1, 0xAA, op_size);
    __builtin_memcpy(buffer2, buffer1, op_size);
    __builtin_memmove(buffer1, buffer2, op_size);
    
    /* Verify operations */
    int verify = 1;
    for (size_t i = 0; i < op_size && i < sizeof(buffer1); i++) {
        if (buffer1[i] != 0xAA) {
            verify = 0;
            break;
        }
    }
    
    printf("Memory verification: %s\n", verify ? "PASS" : "FAIL");
    printf("Final result: %zu\n", tree_hash + verify);
    
    /* Cleanup */
    /* Note: Full tree cleanup omitted for brevity */
    
    return 0;
}
