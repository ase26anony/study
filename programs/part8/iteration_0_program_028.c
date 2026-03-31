#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    char *data;
    size_t data_len;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_flag = 1;

/* Global token array */
static char global_tokens[256];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(global_tokens, 0, sizeof(global_tokens));
    
    /* Fill with pattern using builtin memcpy */
    char pattern[] = "ASAN_TEST_PATTERN_1234567890";
    for (int i = 0; i < sizeof(global_tokens); i += sizeof(pattern)) {
        size_t copy_len = (sizeof(global_tokens) - i < sizeof(pattern)) ? 
                         sizeof(global_tokens) - i : sizeof(pattern);
        __builtin_memcpy(&global_tokens[i], pattern, copy_len);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    /* Clear sensitive data with builtin memset */
    __builtin_memset(global_tokens, 0xAA, sizeof(global_tokens));
}

/* Recursive parser with goto flow control */
static ASTNode* parse_expression(int depth) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    if (depth > 0) {
        node->type = depth;
        node->value = depth * 10;
        
        /* Allocate data with volatile length */
        node->data_len = volatile_len % 128;
        node->data = malloc(node->data_len + 1);
        
        if (node->data) {
            /* Use builtin memset with volatile length */
            __builtin_memset(node->data, 'A' + (depth % 26), node->data_len);
            node->data[node->data_len] = '\0';
        }
        
        /* Jump label for goto testing */
        process_left:
        node->left = parse_expression(depth - 1);
        
        /* Conditional goto to test flow sensitivity */
        if (volatile_flag) {
            goto process_right;
        }
        
        /* This should never execute but tests compiler analysis */
        __builtin_memmove(node->data, global_tokens, 16);
        
        process_right:
        node->right = parse_expression(depth - 1);
        
        /* Copy between nodes using builtin memcpy */
        if (node->left && node->right && 
            node->left->data && node->right->data) {
            size_t copy_size = (node->left->data_len < node->right->data_len) ?
                              node->left->data_len : node->right->data_len;
            __builtin_memcpy(node->right->data, node->left->data, copy_size);
        }
    }
    
    return node;
}

/* Free AST recursively */
static void free_ast(ASTNode *node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    if (node->data) {
        /* Clear before free with builtin memset */
        __builtin_memset(node->data, 0, node->data_len);
        free(node->data);
    }
    
    /* Clear node memory */
    __builtin_memset(node, 0, sizeof(ASTNode));
    free(node);
}

/* Compute hash of AST */
static unsigned long compute_ast_hash(ASTNode *node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char *ptr = (char*)node;
    
    /* Hash node structure */
    for (size_t i = 0; i < sizeof(ASTNode); i++) {
        hash = ((hash << 5) + hash) + ptr[i];
    }
    
    /* Hash data if present */
    if (node->data) {
        for (size_t i = 0; i < node->data_len && i < 256; i++) {
            hash = ((hash << 5) + hash) + node->data[i];
        }
    }
    
    /* Recursive hash computation */
    hash ^= compute_ast_hash(node->left);
    hash ^= compute_ast_hash(node->right);
    
    return hash;
}

/* Parallel memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf1[128];
        char local_buf2[128];
        
        /* Initialize with builtin memset */
        __builtin_memset(local_buf1, thread_id, sizeof(local_buf1));
        __builtin_memset(local_buf2, 0xFF, sizeof(local_buf2));
        
        /* Copy with builtin memcpy */
        __builtin_memcpy(local_buf2, local_buf1, 
                        volatile_len % sizeof(local_buf1));
        
        /* Move with builtin memmove (overlapping regions) */
        size_t move_len = sizeof(local_buf1) / 2;
        __builtin_memmove(&local_buf1[move_len/2], local_buf1, move_len);
        
        /* Complex pattern with goto */
        if (thread_id % 2 == 0) {
            goto even_thread;
        } else {
            goto odd_thread;
        }
        
        even_thread:
        /* Additional memory operation for even threads */
        __builtin_memcpy(&local_buf1[64], &local_buf2[32], 32);
        goto thread_done;
        
        odd_thread:
        /* Different operation for odd threads */
        __builtin_memset(&local_buf1[32], 0xCC, 64);
        
        thread_done:
        /* Final operation for all threads */
        __builtin_memmove(local_buf2, &local_buf1[16], 48);
        
        /* Critical section for result aggregation */
        #pragma omp critical
        {
            /* Copy to global array */
            size_t offset = (thread_id * 16) % sizeof(global_tokens);
            __builtin_memcpy(&global_tokens[offset], local_buf1, 16);
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN coverage test...\n");
    
    /* Phase 1: Recursive AST parsing */
    ASTNode *root = parse_expression(4);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Compute and print hash */
    unsigned long hash = compute_ast_hash(root);
    printf("AST Hash: %lu\n", hash);
    
    /* Phase 3: Parallel memory operations */
    printf("Running parallel memory operations...\n");
    parallel_memory_operations();
    
    /* Phase 4: Verify global tokens */
    int token_sum = 0;
    for (size_t i = 0; i < sizeof(global_tokens); i++) {
        token_sum += (unsigned char)global_tokens[i];
    }
    printf("Token array sum: %d\n", token_sum);
    
    /* Phase 5: Additional builtin tests with volatile control */
    char test_buf[256];
    volatile size_t test_len = volatile_len % 128;
    
    /* Test all three builtins in sequence */
    __builtin_memset(test_buf, 0x55, test_len);
    __builtin_memcpy(&test_buf[64], test_buf, 32);
    __builtin_memmove(test_buf, &test_buf[32], 64);
    
    /* Cleanup */
    free_ast(root);
    
    /* Final verification */
    int final_check = 0;
    __builtin_memset(&final_check, token_sum & 0xFF, sizeof(final_check));
    
    printf("Test completed. Final check: %d\n", final_check);
    return 0;
}
