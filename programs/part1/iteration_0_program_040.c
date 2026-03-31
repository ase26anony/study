/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char *volatile_dest = NULL;
static volatile char *volatile_src = NULL;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[32];
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Global token array */
static const char *tokens[] = {"memcpy", "memset", "memmove", "test", "data"};
static const int token_count = 5;

/* Constructor/destructor functions */
__attribute__((constructor)) static void init_globals(void) {
    printf("Constructor: Initializing global buffers\n");
    volatile_dest = malloc(volatile_len * 2);
    volatile_src = malloc(volatile_len * 2);
    
    if (volatile_dest && volatile_src) {
        /* Force builtin usage in constructor */
        __builtin_memset(volatile_dest, 0, volatile_len * 2);
        __builtin_memcpy(volatile_src, "Source data for testing", 24);
    }
}

__attribute__((destructor)) static void cleanup_globals(void) {
    printf("Destructor: Cleaning up\n");
    free((void*)volatile_dest);
    free((void*)volatile_src);
}

/* Recursive parser with memory operations */
static ASTNode* parse_tokens(int depth, int idx) {
    if (depth <= 0 || idx >= token_count) return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile lengths */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data with builtin */
    int copy_len = strlen(tokens[idx]) + 1;
    if (copy_len > 32) copy_len = 32;
    __builtin_memcpy(node->data, tokens[idx], copy_len);
    
    node->type = idx;
    
    /* Recursive calls */
    node->left = parse_tokens(depth - 1, (idx + 1) % token_count);
    node->right = parse_tokens(depth - 1, (idx + 2) % token_count);
    
    return node;
}

/* Function with goto jumps around memmove */
static void goto_memmove_test(char *buf1, char *buf2, int size) {
    int use_memmove = 0;
    
    if (size > 16) {
        use_memmove = 1;
        goto do_operation;
    }
    
skip_operation:
    __builtin_memset(buf1, 0xFF, size);
    return;
    
do_operation:
    /* This memmove should be intercepted */
    __builtin_memmove(buf1, buf2, size);
    
    if (use_memmove) {
        goto skip_operation;
    }
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    const int num_buffers = 8;
    char *buffers[num_buffers];
    
    #pragma omp parallel for
    for (int i = 0; i < num_buffers; i++) {
        buffers[i] = malloc(128);
        if (buffers[i]) {
            /* Mix of builtins in parallel region */
            __builtin_memset(buffers[i], i, 128);
            
            if (i % 2 == 0) {
                __builtin_memcpy(buffers[i] + 64, buffers[i], 64);
            } else {
                __builtin_memmove(buffers[i] + 32, buffers[i], 96);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_buffers; i++) {
        free(buffers[i]);
    }
}

/* Complex memory dispatch with all three builtins */
static unsigned int memory_dispatch_logic(void) {
    unsigned int hash = 0;
    char buffer1[256];
    char buffer2[256];
    
    /* Initialize with memset */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memset(buffer2, 0xBB, sizeof(buffer2));
    
    /* Copy with memcpy */
    __builtin_memcpy(buffer1 + 64, buffer2, 128);
    
    /* Overlapping move with memmove */
    __builtin_memmove(buffer1 + 32, buffer1 + 16, 192);
    
    /* Use volatile variables */
    if (volatile_dest && volatile_src) {
        int len = volatile_len;
        if (len > 128) len = 128;
        __builtin_memcpy((char*)volatile_dest, (char*)volatile_src, len);
    }
    
    /* Calculate simple hash */
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        hash = (hash * 31) + (unsigned char)buffer1[i];
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* 1. Parse recursive structure */
    ASTNode *root = parse_tokens(3, 0);
    
    /* 2. Perform goto-based memmove test */
    char test_buf1[100], test_buf2[100];
    __builtin_memset(test_buf2, 0xCC, sizeof(test_buf2));
    goto_memmove_test(test_buf1, test_buf2, sizeof(test_buf1));
    
    /* 3. Execute parallel operations */
    parallel_memory_ops();
    
    /* 4. Run main dispatch logic */
    unsigned int result = memory_dispatch_logic();
    
    /* 5. Additional builtin calls with AST nodes */
    if (root && root->left) {
        __builtin_memcpy(root->right->data, root->left->data, 32);
        __builtin_memmove(root->data, root->right->data, 32);
    }
    
    /* Cleanup AST */
    /* ... recursive free omitted for brevity ... */
    
    printf("Result hash: %u\n", result);
    printf("Test completed successfully\n");
    
    return 0;
}
