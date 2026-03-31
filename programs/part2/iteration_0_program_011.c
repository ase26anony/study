/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Token array for parser simulation */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "data", "node", "copy", "clear", "move"
};
static const int token_count = 8;

/* Global memory buffers */
static char buffer1[1024];
static char buffer2[1024];
static char buffer3[1024];

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_sanitizer_environment(void) {
    /* Force initialization of sanitizer runtime */
    volatile char init_buf[16];
    __builtin_memset(init_buf, 0, sizeof(init_buf));
    
    /* Create initial redzone-like pattern */
    for (int i = 0; i < 16; i++) {
        init_buf[i] = (char)(i * 17);
    }
    
    /* Copy pattern to global buffer */
    __builtin_memcpy(buffer1, init_buf, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_sanitizer_environment(void) {
    /* Clear sensitive data */
    __builtin_memset(buffer1, 0, sizeof(buffer1));
    __builtin_memset(buffer2, 0, sizeof(buffer2));
    __builtin_memset(buffer3, 0, sizeof(buffer3));
}

/* Recursive AST creation and manipulation */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with volatile-controlled pattern */
    int data_len = volatile_len % 128;
    if (data_len > 255) data_len = 255;
    
    /* Use builtins with volatile control */
    __builtin_memset(node->data, id % 256, data_len);
    
    /* Copy token data with memcpy */
    const char* token = tokens[id % token_count];
    size_t token_len = strlen(token);
    if (token_len < 256) {
        __builtin_memcpy(node->data, token, token_len);
    }
    
    node->id = id;
    node->left = create_ast_node(depth - 1, id * 2);
    node->right = create_ast_node(depth - 1, id * 2 + 1);
    
    return node;
}

/* Complex memory operation with goto flow control */
static void complex_memory_operations(void) {
    char local_buf1[512];
    char local_buf2[512];
    int use_memmove = 0;
    
    /* Initialize with memset */
    __builtin_memset(local_buf1, 0xAA, sizeof(local_buf1));
    __builtin_memset(local_buf2, 0x55, sizeof(local_buf2));
    
    /* Goto-based flow control */
    if (volatile_flag) {
        goto use_memcpy_block;
    } else {
        goto use_memset_block;
    }
    
use_memcpy_block:
    {
        /* Copy with overlapping regions to force memmove consideration */
        size_t len = volatile_len % 256;
        __builtin_memcpy(local_buf1 + 100, local_buf1, len);
        use_memmove = 1;
    }
    
    if (use_memmove) {
        goto use_memmove_block;
    }
    
use_memset_block:
    {
        /* Clear middle section */
        size_t len = (volatile_len * 2) % 256;
        __builtin_memset(local_buf1 + 128, 0, len);
        goto final_block;
    }
    
use_memmove_block:
    {
        /* Actually use memmove with overlapping regions */
        size_t len = volatile_len % 128;
        __builtin_memmove(local_buf1 + 64, local_buf1 + 32, len);
    }
    
final_block:
    /* Copy final result to global buffer */
    __builtin_memcpy(buffer2, local_buf1, 256);
}

/* OpenMP parallel memory operations */
static void parallel_memory_dispatch(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char thread_buf[128];
        
        /* Each thread uses different builtins */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(thread_buf, thread_id, sizeof(thread_buf));
                break;
            case 1:
                __builtin_memcpy(thread_buf, buffer1, 
                                volatile_len % sizeof(thread_buf));
                break;
            case 2:
                __builtin_memmove(thread_buf + 32, thread_buf, 64);
                break;
        }
        
        /* Copy to global buffer with synchronization */
        #pragma omp critical
        {
            size_t offset = (thread_id * 128) % (sizeof(buffer3) - 128);
            __builtin_memcpy(buffer3 + offset, thread_buf, 128);
        }
    }
}

/* AST traversal with memory operations between nodes */
static int traverse_and_manipulate_ast(ASTNode* node, int depth) {
    if (!node) return 0;
    
    int hash = node->id;
    
    /* Copy data between left and right children if both exist */
    if (node->left && node->right) {
        size_t copy_len = volatile_len % 128;
        if (copy_len > 255) copy_len = 255;
        
        /* Use memmove for potentially overlapping regions */
        __builtin_memmove(node->right->data, node->left->data, copy_len);
        
        /* Then clear left node */
        __builtin_memset(node->left->data, depth % 256, copy_len);
    }
    
    /* Process children recursively */
    hash ^= traverse_and_manipulate_ast(node->left, depth + 1);
    hash ^= traverse_and_manipulate_ast(node->right, depth + 1);
    
    return hash;
}

/* Free AST memory */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    /* Clear node data before freeing */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Initialize and create AST */
    ASTNode* root = create_ast_node(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Complex memory operations with goto */
    complex_memory_operations();
    
    /* Phase 3: Parallel memory dispatch */
    parallel_memory_dispatch();
    
    /* Phase 4: AST traversal and manipulation */
    int ast_hash = traverse_and_manipulate_ast(root, 0);
    
    /* Phase 5: Final verification operations */
    /* Mix all three builtins in sequence */
    size_t final_len = volatile_len % 512;
    
    __builtin_memset(buffer1 + 256, 0xFF, final_len);
    __builtin_memcpy(buffer2 + 128, buffer1 + 256, final_len / 2);
    __builtin_memmove(buffer3 + 64, buffer2 + 128, final_len / 4);
    
    /* Calculate verification hash */
    unsigned long verification_hash = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        verification_hash = (verification_hash * 31) + buffer1[i];
    }
    for (size_t i = 0; i < sizeof(buffer2); i++) {
        verification_hash = (verification_hash * 31) + buffer2[i];
    }
    for (size_t i = 0; i < sizeof(buffer3); i++) {
        verification_hash = (verification_hash * 31) + buffer3[i];
    }
    
    verification_hash ^= (unsigned long)ast_hash;
    
    printf("Verification hash: 0x%08lx\n", verification_hash);
    printf("AST traversal hash: %d\n", ast_hash);
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
