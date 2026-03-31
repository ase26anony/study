/* ISO C99-compliant test program for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ast_node {
    int type;
    char data[32];
    struct ast_node *left;
    struct ast_node *right;
    struct ast_node *next;
} ast_node_t;

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_asan_hooks(void) {
    /* Force initialization of ASAN memory function RTLS */
    char buf1[64], buf2[64];
    
    /* Use all three builtins in constructor */
    __builtin_memset(buf1, 0xAA, sizeof(buf1));
    __builtin_memcpy(buf2, buf1, sizeof(buf1));
    __builtin_memmove(buf1 + 16, buf1, 32);
    
    printf("Constructor: ASAN hooks initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_asan(void) {
    printf("Destructor: ASAN cleanup complete\n");
}

/* Recursive parser with memory operations */
static ast_node_t* parse_expression(int depth, const char *input) {
    if (depth <= 0 || *input == '\0') {
        return NULL;
    }
    
    ast_node_t *node = malloc(sizeof(*node));
    if (!node) return NULL;
    
    /* Use builtins with volatile length */
    int len = volatile_len % 32;
    __builtin_memset(node, 0, sizeof(*node));
    __builtin_memcpy(node->data, input, len);
    node->type = depth;
    
    /* Recursive parsing with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 3 == 0);
        
        if (use_goto) {
            goto parse_children;
        }
        
        node->left = parse_expression(depth - 1, input + 1);
        node->right = parse_expression(depth - 2, input + 2);
        
        parse_children:
        /* Jump target with memmove operation */
        if (node->left && node->right) {
            char temp[32];
            __builtin_memcpy(temp, node->left->data, 16);
            __builtin_memmove(node->right->data + 8, temp, 16);
            __builtin_memcpy(node->data + 16, node->right->data, 16);
        }
    }
    
    return node;
}

/* Complex token processing */
static void process_tokens(char tokens[][64], int count) {
    for (int i = 0; i < count - 1; i++) {
        /* Mixed builtin usage with volatile dest/src */
        __builtin_memcpy((void*)volatile_dest, tokens[i], 32);
        __builtin_memset(tokens[i + 1] + 16, 0xFF, 16);
        __builtin_memmove(tokens[i], tokens[i + 1], 32);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[128];
        char shared_buf[256];
        
        /* Each thread uses builtins differently */
        switch (tid % 3) {
            case 0:
                __builtin_memset(local_buf, tid, sizeof(local_buf));
                __builtin_memcpy(shared_buf + tid * 32, local_buf, 32);
                break;
            case 1:
                __builtin_memcpy(local_buf, shared_buf, 64);
                __builtin_memmove(local_buf + 16, local_buf, 48);
                break;
            case 2:
                __builtin_memset(shared_buf + 128, 0xCC, 64);
                __builtin_memcpy(local_buf, shared_buf + 64, 64);
                __builtin_memmove(shared_buf, shared_buf + 128, 64);
                break;
        }
        
        #pragma omp barrier
        
        /* Collective operation */
        #pragma omp for
        for (int i = 0; i < 4; i++) {
            __builtin_memset(shared_buf + i * 32, i * 0x11, 32);
        }
    }
}

/* Free AST recursively */
static void free_ast(ast_node_t *node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

/* Calculate hash from AST */
static unsigned long ast_hash(ast_node_t *node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    for (int i = 0; i < 32 && node->data[i]; i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    return hash + ast_hash(node->left) + ast_hash(node->right);
}

int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Initialize complex token array */
    char tokens[8][64];
    for (int i = 0; i < 8; i++) {
        __builtin_memset(tokens[i], 'A' + i, 63);
        tokens[i][63] = '\0';
    }
    
    /* Process tokens with builtins */
    process_tokens(tokens, 8);
    
    /* Create recursive AST */
    ast_node_t *ast = parse_expression(5, "TestExpressionForASANBuiltinRedirection");
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Calculate and print verification result */
    unsigned long total_hash = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 64; j++) {
            total_hash += tokens[i][j];
        }
    }
    
    if (ast) {
        total_hash += ast_hash(ast);
        free_ast(ast);
    }
    
    printf("Verification hash: %lu\n", total_hash);
    printf("Test completed successfully\n");
    
    return 0;
}
