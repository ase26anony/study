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
    unsigned char redzone[32];  /* Simulate ASAN redzone */
} ASTNode;

/* Global token array */
static const char *tokens[] = {"memcpy", "memset", "memmove", "test", "data"};
static const size_t num_tokens = sizeof(tokens)/sizeof(tokens[0]);

/* Constructor/destructor for initialization coordination */
__attribute__((constructor)) static void init_asan_hooks(void) {
    g_init_flag = 1;
    fprintf(stderr, "Constructor: ASAN hooks initialized\n");
}

__attribute__((destructor)) static void cleanup_asan_hooks(void) {
    fprintf(stderr, "Destructor: ASAN hooks cleaned up\n");
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth, const char **token_ptr) {
    if (depth <= 0 || *token_ptr >= tokens + num_tokens) {
        return NULL;
    }
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile to control memory size */
    node->len = g_mem_size / (depth + 1);
    node->data = malloc(node->len + 1);
    node->left = node->right = NULL;
    
    if (!node->data) {
        free(node);
        return NULL;
    }
    
    /* Force builtin usage with goto for flow sensitivity */
    const char *current_token = *token_ptr;
    
    if (depth % 3 == 0) {
        /* Use __builtin_memset */
        __builtin_memset(node->data, 'A', node->len);
        node->data[node->len] = '\0';
    } else {
        /* Use __builtin_memcpy with goto */
        static const char pattern[] = "MEMORY_PATTERN_123";
        size_t pattern_len = sizeof(pattern) - 1;
        
        if (node->len > 0) {
            /* Jump label for goto */
            copy_start:
            __builtin_memcpy(node->data, pattern, 
                           pattern_len < node->len ? pattern_len : node->len);
            
            /* Conditional goto to test flow sensitivity */
            if (node->len > pattern_len && depth > 2) {
                /* Fill remainder with different pattern */
                goto fill_remainder;
            }
        }
        
        fill_remainder:
        if (node->len > pattern_len) {
            __builtin_memset(node->data + pattern_len, 'X', 
                           node->len - pattern_len);
        }
    }
    
    /* Recursive parsing */
    (*token_ptr)++;
    node->left = parse_expression(depth - 1, token_ptr);
    
    if (*token_ptr < tokens + num_tokens) {
        (*token_ptr)++;
        node->right = parse_expression(depth - 2, token_ptr);
    }
    
    return node;
}

/* Memory operation between AST nodes */
static void copy_ast_data(ASTNode *dest, ASTNode *src) {
    if (!dest || !src || !dest->data || !src->data) return;
    
    size_t copy_len = dest->len < src->len ? dest->len : src->len;
    
    /* Use __builtin_memmove for overlapping regions */
    if (dest->data + copy_len > src->data && dest->data < src->data + copy_len) {
        __builtin_memmove(dest->data, src->data, copy_len);
    } else {
        __builtin_memcpy(dest->data, src->data, copy_len);
    }
}

/* Parallel memory dispatch */
static unsigned long parallel_memory_ops(ASTNode **nodes, size_t count) {
    unsigned long hash_sum = 0;
    
    #pragma omp parallel reduction(+:hash_sum)
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for schedule(dynamic)
        for (size_t i = 0; i < count; i++) {
            if (nodes[i] && nodes[i]->data) {
                /* Thread-specific memory operations */
                char *temp = malloc(nodes[i]->len);
                if (temp) {
                    /* Force builtin usage in parallel region */
                    __builtin_memcpy(temp, nodes[i]->data, nodes[i]->len);
                    
                    /* Modify data */
                    if (tid % 2 == 0) {
                        __builtin_memset(temp + (nodes[i]->len/2), 
                                       '0' + tid, nodes[i]->len/4);
                    }
                    
                    /* Copy back with memmove */
                    __builtin_memmove(nodes[i]->data, temp, nodes[i]->len);
                    
                    /* Compute hash */
                    for (size_t j = 0; j < nodes[i]->len; j++) {
                        hash_sum += (unsigned long)nodes[i]->data[j];
                    }
                    
                    free(temp);
                }
            }
        }
    }
    
    return hash_sum;
}

/* Free AST recursively */
static void free_ast(ASTNode *node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node->data);
    free(node);
}

int main(void) {
    fprintf(stderr, "Starting ASAN builtin redirection test\n");
    
    /* Initialize token pointer */
    const char *token_ptr = tokens;
    
    /* Create recursive AST */
    ASTNode *root = parse_expression(5, &token_ptr);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    ASTNode *node_array[32];
    size_t node_count = 0;
    
    /* Collect nodes recursively */
    void collect_nodes(ASTNode *node) {
        if (!node || node_count >= sizeof(node_array)/sizeof(node_array[0])) {
            return;
        }
        node_array[node_count++] = node;
        collect_nodes(node->left);
        collect_nodes(node->right);
    }
    collect_nodes(root);
    
    /* Perform memory operations between nodes */
    for (size_t i = 0; i + 1 < node_count; i += 2) {
        copy_ast_data(node_array[i], node_array[i + 1]);
    }
    
    /* Execute parallel memory operations */
    unsigned long result = parallel_memory_ops(node_array, node_count);
    
    /* Additional builtin usage in main */
    char buffer[512];
    volatile size_t buf_size = sizeof(buffer);
    
    /* Test all three builtins */
    __builtin_memset(buffer, 0, buf_size);
    __builtin_memcpy(buffer, "Test pattern", 12);
    __builtin_memmove(buffer + 10, buffer, 12);
    
    /* Complex goto pattern with builtin */
    int use_memmove = 1;
    
    if (use_memmove) {
        goto use_memmove_block;
    }
    
    /* This should be skipped */
    __builtin_memset(buffer, 'Z', 50);
    
    use_memmove_block:
    __builtin_memmove(buffer + 100, buffer + 50, 50);
    
    /* Print verification result */
    printf("Memory operations completed. Hash sum: %lu\n", result);
    printf("Buffer[0:20] = ");
    for (int i = 0; i < 20; i++) {
        printf("%02x ", (unsigned char)buffer[i]);
    }
    printf("\n");
    
    /* Cleanup */
    free_ast(root);
    
    return 0;
}
