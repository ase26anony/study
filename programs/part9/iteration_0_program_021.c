/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan",
    "instrument", "redzone", "shadow", "builtin"
};
static const size_t num_tokens = sizeof(tokens)/sizeof(tokens[0]);

/* Constructor/destructor functions */
__attribute__((constructor))
static void init_asan_env(void) {
    /* Force early initialization */
    volatile char dummy[16];
    __builtin_memset(dummy, 0, sizeof(dummy));
}

__attribute__((destructor))
static void cleanup_asan_env(void) {
    /* Ensure destructor path is taken */
    volatile int dummy = 0;
    __builtin_memset(&dummy, 0, sizeof(dummy));
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth, const char** token_ptr) {
    if (depth <= 0 || *token_ptr >= tokens + num_tokens) {
        return NULL;
    }
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset for initialization */
    __builtin_memset(node, 0, sizeof(*node));
    node->id = depth;
    
    /* Copy token data with __builtin_memcpy */
    size_t len = strlen(*token_ptr);
    if (len > sizeof(node->data) - 1) len = sizeof(node->data) - 1;
    __builtin_memcpy(node->data, *token_ptr, len);
    node->data[len] = '\0';
    
    token_ptr++;
    
    /* Recursive calls with goto for flow control */
    if (depth > 1) {
        int use_left = (depth % 2 == 0);
        
        if (use_left) {
            goto parse_left;
        } else {
            goto parse_right;
        }
        
    parse_left:
        node->left = parse_expression(depth - 1, token_ptr);
        if (node->left && token_ptr < tokens + num_tokens - 1) {
            /* Use __builtin_memmove for overlapping copy */
            char temp[256];
            __builtin_memcpy(temp, node->data, sizeof(node->data));
            __builtin_memmove(node->data, node->left->data, sizeof(node->data));
            __builtin_memmove(node->left->data, temp, sizeof(node->data));
        }
        goto after_left;
        
    parse_right:
        node->right = parse_expression(depth - 2, token_ptr);
    after_left:
        ; /* Empty statement for label */
    }
    
    return node;
}

/* Parallel memory operations */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        size_t local_size = g_mem_size + thread_id;
        
        /* Thread-local buffers */
        char src[256], dst[256];
        
        /* Initialize with __builtin_memset */
        __builtin_memset(src, thread_id, sizeof(src));
        __builtin_memset(dst, 0, sizeof(dst));
        
        #pragma omp barrier
        
        /* Conditional memcpy/memmove */
        if (g_use_memmove) {
            /* Create overlapping regions */
            __builtin_memcpy(dst, src, local_size);
            __builtin_memmove(src + 32, src, local_size - 32);
        } else {
            __builtin_memcpy(dst, src, local_size);
        }
        
        /* Verify with volatile access */
        volatile char* vdst = dst;
        #pragma omp critical
        {
            vdst[0] = (char)thread_id;
        }
    }
}

/* Calculate hash from AST */
static size_t compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    size_t hash = 5381;
    for (size_t i = 0; i < sizeof(node->data) && node->data[i]; i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    /* Recursive computation */
    size_t left_hash = compute_ast_hash(node->left);
    size_t right_hash = compute_ast_hash(node->right);
    
    /* Use __builtin_memcpy for hash combination */
    char hash_buf[64];
    __builtin_memset(hash_buf, 0, sizeof(hash_buf));
    __builtin_memcpy(hash_buf, &hash, sizeof(hash));
    __builtin_memcpy(hash_buf + 32, &left_hash, sizeof(left_hash));
    __builtin_memcpy(hash_buf + 48, &right_hash, sizeof(right_hash));
    
    /* Final mix */
    size_t final_hash = 0;
    for (int i = 0; i < 64; i++) {
        final_hash = (final_hash * 31) + hash_buf[i];
    }
    
    return final_hash;
}

/* Free AST with memory operations */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    /* Clear data before free */
    volatile char* vdata = node->data;
    __builtin_memset(vdata, 0, sizeof(node->data));
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Use volatile to prevent dead store elimination */
    volatile ASTNode* vnode = node;
    __builtin_memset((void*)vnode, 0, sizeof(*node));
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Recursive parsing with memory operations */
    const char* token_ptr = tokens;
    ASTNode* ast = parse_expression(4, &token_ptr);
    
    /* Phase 2: Parallel memory operations */
    parallel_mem_ops();
    
    /* Phase 3: Compute and verify result */
    size_t hash = compute_ast_hash(ast);
    printf("AST hash: %zu\n", hash);
    
    /* Phase 4: Cleanup with memory operations */
    free_ast(ast);
    
    /* Final verification with all three builtins */
    char final_buf[128];
    char src_buf[128];
    
    __builtin_memset(src_buf, 0xAA, sizeof(src_buf));
    __builtin_memcpy(final_buf, src_buf, sizeof(src_buf));
    
    /* Conditional memmove with goto */
    if (hash % 2 == 0) {
        goto use_memmove;
    } else {
        __builtin_memcpy(final_buf + 64, final_buf, 64);
        goto after_memmove;
    }
    
use_memmove:
    __builtin_memmove(final_buf + 64, final_buf, 64);
after_memmove:
    
    /* Compute final checksum */
    size_t checksum = 0;
    for (int i = 0; i < 128; i++) {
        checksum += final_buf[i];
    }
    printf("Final checksum: %zu\n", checksum);
    
    return (checksum > 0) ? 0 : 1;
}
