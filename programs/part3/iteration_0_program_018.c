/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 32;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[100];
    /* Force builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 10, "constructor", 11);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    volatile char buf[50];
    __builtin_memset(buf, 0xFF, sizeof(buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile lengths */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    
    /* Fill data with pattern */
    for (int i = 0; i < 255; i++) {
        node->data[i] = (char)((depth + i) % 256);
    }
    node->data[255] = '\0';
    
    /* Recursive creation */
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 2);
    
    return node;
}

/* Function with goto jumps around memmove */
static void test_goto_memmove(void* dst, void* src, size_t len) {
    volatile int flag = 1;
    
    goto skip_first;
    
    block_with_memmove:
        /* This should trigger the uncovered logic */
        __builtin_memmove(dst, src, len);
        goto after_block;
    
    skip_first:
        if (flag) {
            flag = 0;
            goto block_with_memmove;
        }
    
    after_block:
        /* Another memmove after goto */
        __builtin_memmove(src, dst, len / 2);
}

/* Parallel memory operations */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[1024];
        char shared_buf[1024];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, tid, sizeof(local_buf));
        
        #pragma omp barrier
        
        #pragma omp for
        for (int i = 0; i < 16; i++) {
            volatile size_t len = g_memcpy_len + i;
            __builtin_memcpy(&shared_buf[i * 64], local_buf, len % 256);
        }
        
        #pragma omp single
        {
            /* Master thread uses memmove */
            __builtin_memmove(shared_buf + 512, shared_buf, 256);
        }
    }
}

/* Complex token processing */
static unsigned long process_tokens(const char** tokens, int count) {
    unsigned long hash = 5381;
    char buffer[512];
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        volatile size_t op_len = len % 256;
        
        /* Mix of builtins */
        __builtin_memset(buffer, 0, sizeof(buffer));
        __builtin_memcpy(buffer, tokens[i], op_len);
        
        if (i % 3 == 0) {
            __builtin_memmove(buffer + 100, buffer, op_len / 2);
        }
        
        /* Update hash */
        for (int j = 0; j < op_len; j++) {
            hash = ((hash << 5) + hash) + buffer[j];
        }
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* 1. Initialize complex token array */
    const char* tokens[] = {
        "memcpy", "memset", "memmove", "asan", "hwasan",
        "instrumentation", "redzone", "shadow", "granule",
        "builtin", "function", "declaration", "rtl", "tree"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* 2. Create recursive AST */
    ASTNode* root = create_ast(5);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* 3. Test goto with memmove */
    char src_buf[1024], dst_buf[1024];
    for (int i = 0; i < sizeof(src_buf); i++) {
        src_buf[i] = (char)(i % 256);
    }
    
    test_goto_memmove(dst_buf, src_buf, g_memmove_len);
    
    /* 4. Parallel memory operations */
    parallel_mem_ops();
    
    /* 5. Copy between AST nodes */
    if (root->left && root->right) {
        volatile size_t copy_len = sizeof(root->left->data);
        __builtin_memcpy(root->right->data, root->left->data, copy_len);
        
        /* Nested memmove */
        __builtin_memmove(root->left->data + 128, root->left->data, 64);
    }
    
    /* 6. Process tokens with mixed builtins */
    unsigned long final_hash = process_tokens(tokens, token_count);
    
    /* 7. Additional builtin stress */
    volatile char final_buf[2048];
    __builtin_memset(final_buf, 0xCC, sizeof(final_buf));
    __builtin_memcpy(final_buf + 512, src_buf, g_memcpy_len);
    __builtin_memmove(final_buf + 1024, final_buf + 512, g_memmove_len);
    
    /* Verify and print result */
    printf("Token hash: %lu\n", final_hash);
    printf("AST root type: %d\n", root->type);
    printf("First bytes of final buffer: 0x%02x 0x%02x\n", 
           (unsigned char)final_buf[0], (unsigned char)final_buf[1]);
    
    /* Cleanup */
    /* Note: In real ASAN, this would detect leaks */
    
    printf("Test completed\n");
    return 0;
}
