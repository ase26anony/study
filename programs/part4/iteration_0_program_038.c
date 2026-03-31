#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* AST-like recursive structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile size_t size;  /* volatile to prevent optimization */
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
} ASTNode;

/* Global token array */
static volatile int token_array[256];
static volatile size_t token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Force initialization of memory builtins early */
    volatile char buffer[32];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 16);
    
    /* Initialize token array with non-foldable values */
    for (int i = 0; i < 256; i++) {
        token_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    volatile char cleanup_buf[64];
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
}

/* Recursive AST creation with memory operations */
static ASTNode* create_ast(int depth, volatile size_t* counter) {
    if (depth <= 0 || *counter >= 100) {
        ASTNode* leaf = malloc(sizeof(ASTNode));
        if (!leaf) return NULL;
        
        /* Use builtins with volatile size */
        __builtin_memset(leaf, 0, sizeof(*leaf));
        leaf->type = 1;
        leaf->value = (*counter)++;
        leaf->size = sizeof(*leaf);  /* volatile assignment */
        
        /* Copy data with builtin */
        const char* src = "LEAF_NODE_DATA";
        __builtin_memcpy(leaf->data, src, __builtin_strlen(src) + 1);
        
        return leaf;
    }
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(*node));
    node->type = 0;
    node->value = (*counter)++;
    node->size = sizeof(*node);
    
    /* Create children recursively */
    node->left = create_ast(depth - 1, counter);
    node->right = create_ast(depth - 1, counter);
    
    /* Copy node data with goto-controlled flow */
    volatile int use_memmove = (depth % 2 == 0);
    
    if (use_memmove) {
        goto use_memmove_block;
    } else {
        goto use_memcpy_block;
    }
    
use_memmove_block:
    {
        char temp[64];
        const char* src = "INTERNAL_NODE_MEMOVE";
        __builtin_memcpy(temp, src, __builtin_strlen(src) + 1);
        __builtin_memmove(node->data, temp, sizeof(node->data));
        goto after_copy;
    }
    
use_memcpy_block:
    {
        const char* src = "INTERNAL_NODE_MEMCPY";
        __builtin_memcpy(node->data, src, __builtin_strlen(src) + 1);
        /* fall through */
    }
    
after_copy:
    return node;
}

/* AST traversal with memory operations */
static int traverse_ast(ASTNode* node, int* sum) {
    if (!node) return 0;
    
    volatile int local_sum = *sum;
    
    /* Process current node */
    local_sum += node->value;
    
    /* Copy node data to local buffer using builtins */
    char buffer[64];
    volatile size_t copy_size = node->size < sizeof(buffer) ? node->size : sizeof(buffer);
    
    /* Conditional memcpy/memmove with goto */
    if (node->type == 0) {
        __builtin_memcpy(buffer, node->data, copy_size);
    } else {
        /* Use memmove for leaf nodes */
        __builtin_memmove(buffer, node->data, copy_size);
    }
    
    /* Add buffer contents to sum (prevent optimization) */
    for (size_t i = 0; i < copy_size && i < sizeof(buffer); i++) {
        local_sum += buffer[i];
    }
    
    *sum = local_sum;
    
    /* Recursive traversal */
    int left_sum = traverse_ast(node->left, sum);
    int right_sum = traverse_ast(node->right, sum);
    
    return left_sum + right_sum + node->value;
}

/* Free AST with memory clearing */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear sensitive data before free */
    volatile char* data = (volatile char*)node;
    __builtin_memset(data, 0, sizeof(*node));
    free(node);
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char src_buf[128];
        char dst_buf[128];
        
        /* Initialize with builtin */
        __builtin_memset(src_buf, thread_id, sizeof(src_buf));
        
        /* Copy with different builtins based on thread */
        if (thread_id % 3 == 0) {
            __builtin_memcpy(dst_buf, src_buf, sizeof(src_buf));
        } else if (thread_id % 3 == 1) {
            __builtin_memmove(dst_buf, src_buf, sizeof(src_buf));
        } else {
            /* Overlapping copy with memmove */
            __builtin_memmove(dst_buf + 32, dst_buf, 64);
        }
        
        /* Verify copy */
        volatile int verify = 0;
        for (size_t i = 0; i < sizeof(src_buf); i++) {
            verify += (src_buf[i] == dst_buf[i]);
        }
        
        #pragma omp atomic
        token_index += verify;
    }
}

/* Main test driver */
int main(void) {
    int final_sum = 0;
    volatile size_t node_counter = 0;
    
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Phase 1: Create and traverse AST */
    ASTNode* root = create_ast(4, &node_counter);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    int ast_sum = 0;
    traverse_ast(root, &ast_sum);
    final_sum += ast_sum;
    
    /* Phase 2: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Phase 3: Direct builtin calls with volatile control */
    volatile size_t op_size = 256;
    char* dynamic_buf1 = malloc(op_size);
    char* dynamic_buf2 = malloc(op_size);
    
    if (dynamic_buf1 && dynamic_buf2) {
        /* Chain of memory operations */
        __builtin_memset(dynamic_buf1, 0xCC, op_size);
        __builtin_memcpy(dynamic_buf2, dynamic_buf1, op_size);
        __builtin_memmove(dynamic_buf1 + 64, dynamic_buf1, 128);
        
        /* Verify with volatile access */
        volatile char* vbuf = (volatile char*)dynamic_buf1;
        for (size_t i = 0; i < op_size; i++) {
            final_sum += vbuf[i];
        }
    }
    
    free(dynamic_buf1);
    free(dynamic_buf2);
    
    /* Phase 4: Token array processing with goto jumps */
    volatile int process_tokens = 1;
    size_t idx = 0;
    
process_loop:
    if (idx >= 256) goto done;
    
    volatile int token = token_array[idx];
    char token_buf[16];
    
    /* Conditional jump to different memory operations */
    if (token % 5 == 0) {
        goto do_memset;
    } else if (token % 5 == 1) {
        goto do_memcpy;
    } else {
        goto do_memmove;
    }
    
do_memset:
    __builtin_memset(token_buf, token & 0xFF, sizeof(token_buf));
    goto next_token;
    
do_memcpy:
    {
        char src[16];
        for (int i = 0; i < 16; i++) src[i] = (token + i) & 0xFF;
        __builtin_memcpy(token_buf, src, sizeof(token_buf));
        goto next_token;
    }
    
do_memmove:
    {
        /* Overlapping move */
        __builtin_memmove(token_buf + 8, token_buf, 8);
        goto next_token;
    }
    
next_token:
    final_sum += token_buf[0];
    idx++;
    if (process_tokens) goto process_loop;
    
done:
    /* Cleanup */
    free_ast(root);
    
    /* Print verification result */
    printf("Test completed. Final sum: %d\n", final_sum);
    printf("Token index: %zu\n", (size_t)token_index);
    
    return 0;
}
