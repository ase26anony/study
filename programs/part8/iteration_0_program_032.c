/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array for parser simulation */
static const char* tokens[] = {"memcpy", "memset", "memmove", "data", "node"};
static const int token_count = 5;

/* Constructor/destructor for initialization coordination */
__attribute__((constructor)) static void init_asan_hooks(void) {
    volatile char init_buf[32];
    __builtin_memset(init_buf, 0, sizeof(init_buf));
    printf("Constructor: Initializing ASAN hooks\n");
}

__attribute__((destructor)) static void cleanup_asan_hooks(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth, int* token_idx) {
    if (depth <= 0 || *token_idx >= token_count) {
        return NULL;
    }
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(*node));
    node->id = depth * 100 + *token_idx;
    
    /* Copy token data using memcpy */
    const char* token = tokens[*token_idx];
    size_t len = strlen(token);
    if (len > 255) len = 255;
    __builtin_memcpy(node->data, token, len);
    node->data[len] = '\0';
    
    (*token_idx)++;
    
    /* Recursive calls */
    node->left = parse_expression(depth - 1, token_idx);
    node->right = parse_expression(depth - 1, token_idx);
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int use_copy = 1;
    
    /* Jump into block with memmove */
    if (g_use_memmove) {
        goto do_memmove;
    }
    
    /* Normal path */
    __builtin_memcpy(dst->data, src->data, sizeof(src->data));
    goto done;
    
do_memmove:
    /* Overlapping copy with memmove */
    char temp[512];
    __builtin_memcpy(temp, src->data, sizeof(src->data));
    __builtin_memmove(dst->data, temp, sizeof(dst->data));
    
    /* Jump out */
    if (dst->id > 0) {
        goto done;
    }
    
done:
    /* Final touch with memset */
    __builtin_memset(dst->data + 240, 0xFF, 16);
}

/* Parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i] && nodes[(i + 1) % count]) {
            volatile size_t op_size = g_mem_size;
            
            /* Force all three builtins in parallel region */
            __builtin_memset(nodes[i]->data, i, op_size % 256);
            
            if (i % 2 == 0) {
                __builtin_memcpy(nodes[(i + 1) % count]->data, 
                               nodes[i]->data, 
                               op_size % 256);
            } else {
                __builtin_memmove(nodes[(i + 1) % count]->data,
                                nodes[i]->data,
                                op_size % 256);
            }
        }
    }
}

/* Calculate hash from AST */
static unsigned long calculate_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    /* Process string */
    while (*ptr) {
        hash = ((hash << 5) + hash) + *ptr++;
    }
    
    hash ^= calculate_ast_hash(node->left);
    hash ^= calculate_ast_hash(node->right);
    hash ^= node->id;
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Initialize token index */
    int token_idx = 0;
    
    /* Create recursive AST */
    ASTNode* root = parse_expression(3, &token_idx);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create sibling nodes for memmove testing */
    ASTNode* nodes[4];
    nodes[0] = root;
    for (int i = 1; i < 4; i++) {
        nodes[i] = malloc(sizeof(ASTNode));
        if (nodes[i]) {
            __builtin_memset(nodes[i], 0, sizeof(ASTNode));
            nodes[i]->id = 200 + i;
            __builtin_memcpy(nodes[i]->data, "test", 5);
        }
    }
    
    /* Test goto with memmove */
    process_with_goto(nodes[0], nodes[1]);
    
    /* Parallel operations */
    parallel_memory_ops(nodes, 4);
    
    /* Additional builtin calls in main */
    volatile char buffer[128];
    volatile char buffer2[128];
    
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer2, buffer, sizeof(buffer));
    __builtin_memmove(buffer + 32, buffer + 16, 64);
    
    /* Complex overlapping case */
    if (g_use_memmove) {
        __builtin_memmove(buffer + 10, buffer + 5, 50);
    }
    
    /* Calculate and print result */
    unsigned long total_hash = 0;
    for (int i = 0; i < 4; i++) {
        if (nodes[i]) {
            total_hash ^= calculate_ast_hash(nodes[i]);
        }
    }
    
    printf("Result hash: 0x%08lx\n", total_hash);
    printf("Built-in redirection test completed\n");
    
    /* Cleanup */
    for (int i = 1; i < 4; i++) {
        free(nodes[i]);
    }
    
    return 0;
}
