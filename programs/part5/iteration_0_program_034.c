/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early() {
    volatile char buffer[128];
    /* Force builtin calls in constructor context */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 32, buffer, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late() {
    volatile char final_buf[64];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_tree(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile length */
    __builtin_memset(node->data, depth, volatile_len % sizeof(node->data));
    node->size = volatile_len % sizeof(node->data);
    
    /* Conditional goto for flow control */
    if (depth % 2 == 0) {
        goto skip_left;
    }
    
    node->left = create_tree(depth - 1);
    
skip_left:
    node->right = create_tree(depth - 1);
    
    /* Copy between nodes if both exist */
    if (node->left && node->right) {
        size_t copy_len = node->left->size < node->right->size ? 
                         node->left->size : node->right->size;
        __builtin_memcpy(node->right->data, node->left->data, copy_len);
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void goto_mem_operations(char* dest, const char* src, size_t n) {
    int do_copy = 1;
    
    if (n == 0) {
        goto no_op;
    }
    
    if (do_copy) {
        goto perform_copy;
    }
    
no_op:
    return;
    
perform_copy:
    /* This block is entered via goto */
    if (use_memmove) {
        __builtin_memmove(dest, src, n);
    } else {
        __builtin_memcpy(dest, src, n);
    }
    
    /* Jump out of block */
    goto cleanup;
    
cleanup:
    __builtin_memset(dest + n - 1, 0, 1);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(int num_threads) {
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        char local_buf[256];
        char shared_buf[256];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, tid, sizeof(local_buf));
        
        #pragma omp barrier
        
        #pragma omp single
        {
            __builtin_memset(shared_buf, 0xCC, sizeof(shared_buf));
        }
        
        #pragma omp barrier
        
        /* Conditional memmove with goto */
        if (tid % 2 == 0) {
            goto use_memmove_block;
        } else {
            __builtin_memcpy(local_buf, shared_buf, 128);
            goto after_copy;
        }
        
    use_memmove_block:
        __builtin_memmove(local_buf + 64, local_buf, 64);
        
    after_copy:
        /* Final builtin in parallel region */
        __builtin_memset(local_buf + 192, 0xFF, 64);
    }
}

/* Multi-stage processing function */
static size_t process_token_array(const char** tokens, int count) {
    char buffer[1024];
    size_t total_hash = 0;
    size_t offset = 0;
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        
        /* Use all three builtins in sequence */
        if (i == 0) {
            __builtin_memset(buffer, 0, sizeof(buffer));
        }
        
        __builtin_memcpy(buffer + offset, tokens[i], len);
        offset += len;
        
        if (i > 0 && offset > len) {
            __builtin_memmove(buffer + len, buffer, offset - len);
        }
        
        /* Compute simple hash */
        for (size_t j = 0; j < len; j++) {
            total_hash += (size_t)buffer[offset - len + j];
        }
    }
    
    return total_hash;
}

int main(void) {
    const char* tokens[] = {
        "ASAN_TEST_STRING_1",
        "BUILTIN_REDIRECTION",
        "MEMORY_OPERATIONS",
        "GOTO_FLOW_CONTROL",
        "OPENMP_PARALLEL",
        "RECURSIVE_AST_NODES",
        "VOLATILE_VARIABLES",
        "CONSTRUCTOR_DESTRUCTOR"
    };
    int num_tokens = sizeof(tokens) / sizeof(tokens[0]);
    
    printf("Starting ASAN/HWASAN builtin redirection test...\n");
    
    /* Stage 1: Process token array */
    size_t hash1 = process_token_array(tokens, num_tokens);
    printf("Token array hash: %zu\n", hash1);
    
    /* Stage 2: Create recursive tree */
    ASTNode* root = create_tree(4);
    
    if (root) {
        /* Copy between tree nodes */
        if (root->left && root->right) {
            size_t copy_size = root->left->size < 128 ? root->left->size : 128;
            __builtin_memmove(root->right->data + 64, root->left->data, copy_size);
        }
        
        /* TODO: Add tree cleanup */
    }
    
    /* Stage 3: Goto-based memory operations */
    char src[256], dest[256];
    for (int i = 0; i < sizeof(src); i++) {
        src[i] = (char)(i % 256);
    }
    
    goto_mem_operations(dest, src, volatile_len % sizeof(dest));
    
    /* Stage 4: OpenMP parallel operations */
    parallel_memory_ops(4);
    
    /* Stage 5: Final builtin calls with different sizes */
    char final_buffer[512];
    __builtin_memset(final_buffer, 0xAA, 256);
    __builtin_memcpy(final_buffer + 256, final_buffer, 128);
    __builtin_memmove(final_buffer + 128, final_buffer + 64, 192);
    
    /* Compute verification result */
    size_t final_hash = hash1;
    for (int i = 0; i < 256; i++) {
        final_hash += (size_t)dest[i] + (size_t)final_buffer[i];
    }
    
    printf("Final verification hash: %zu\n", final_hash);
    printf("Test completed successfully.\n");
    
    return (final_hash > 0) ? 0 : 1;
}
