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
static const char* tokens[] = {"memcpy", "memset", "memmove", "asan", "hwasan"};
static const int token_count = 5;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_sanitizer_hook(void) {
    volatile char buffer[128];
    /* Force builtin initialization in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 64, buffer, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_sanitizer_hook(void) {
    volatile char final_buf[16];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Create pattern in data */
    __builtin_memset(node->data, 'A' + (id % 26), 128);
    __builtin_memcpy(node->data + 128, node->data, 128);
    
    /* Recursive creation with goto for flow control */
    int use_left = 1;
    
    if (depth > 2) {
        use_left = 0;
        goto skip_left;
    }
    
    node->left = create_ast(depth - 1, id * 2);
    
skip_left:
    if (use_left) {
        /* This path uses memmove */
        if (g_use_memmove) {
            char temp[256];
            __builtin_memcpy(temp, node->data, sizeof(temp));
            __builtin_memmove(node->data, temp, sizeof(temp));
        }
    }
    
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ASTNode* node) {
    if (!node) return;
    
    volatile int jump_flag = 1;
    
    if (jump_flag) {
        goto memory_block;
    }
    
    /* Normal path */
    __builtin_memset(node->data, 'X', 64);
    return;
    
memory_block:
    {
        /* Jumped-into block with memmove */
        char buffer[128];
        __builtin_memcpy(buffer, node->data, sizeof(buffer));
        
        /* Conditional memmove */
        if (node->id % 3 == 0) {
            __builtin_memmove(node->data + 32, node->data, 96);
        } else if (node->id % 3 == 1) {
            __builtin_memmove(node->data, buffer, 64);
        }
        
        /* Jump back out */
        goto finish;
    }
    
finish:
    /* Final memset */
    __builtin_memset(node->data + 192, 0, 64);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        volatile size_t local_size = g_mem_size + (i * 8);
        
        if (nodes[i]) {
            /* Mix of memory operations */
            __builtin_memset(nodes[i]->data, i, local_size);
            
            if (i % 2 == 0) {
                char temp[256];
                __builtin_memcpy(temp, nodes[i]->data, local_size);
                __builtin_memcpy(nodes[i]->data + 128, temp, local_size / 2);
            }
            
            if (i % 3 == 0 && g_use_memmove) {
                __builtin_memmove(nodes[i]->data + 64, nodes[i]->data, 128);
            }
        }
    }
}

/* Calculate hash from AST */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    int i;
    
    for (i = 0; i < 256; i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

int main(void) {
    const int ast_depth = 4;
    const int node_count = 15;
    ASTNode* nodes[node_count];
    int i;
    unsigned long total_hash = 0;
    
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Create AST nodes */
    for (i = 0; i < node_count; i++) {
        nodes[i] = create_ast(ast_depth, i + 1);
        if (!nodes[i]) {
            fprintf(stderr, "Failed to create node %d\n", i);
            return 1;
        }
    }
    
    /* Process with goto flow control */
    for (i = 0; i < node_count; i += 2) {
        process_with_goto(nodes[i]);
    }
    
    /* OpenMP parallel memory operations */
    parallel_memory_ops(nodes, node_count);
    
    /* Additional builtin calls in main */
    volatile char main_buffer[512];
    __builtin_memset(main_buffer, 0xCC, sizeof(main_buffer));
    
    /* Test all three builtins */
    __builtin_memcpy(main_buffer + 256, main_buffer, 128);
    __builtin_memset(main_buffer + 384, 0xDD, 64);
    
    if (g_use_memmove) {
        __builtin_memmove(main_buffer + 128, main_buffer, 256);
    }
    
    /* Token processing with memory ops */
    char token_buffer[1024];
    size_t offset = 0;
    
    for (i = 0; i < token_count; i++) {
        size_t len = strlen(tokens[i]) + 1;
        __builtin_memcpy(token_buffer + offset, tokens[i], len);
        offset += len;
        
        /* Interleaved memset */
        __builtin_memset(token_buffer + offset, '-', 4);
        offset += 4;
    }
    
    /* Compute verification hash */
    for (i = 0; i < node_count; i++) {
        total_hash += compute_ast_hash(nodes[i]);
    }
    
    /* Include token buffer in hash */
    for (i = 0; i < offset && i < sizeof(token_buffer); i++) {
        total_hash = ((total_hash << 3) + total_hash) + token_buffer[i];
    }
    
    printf("Verification hash: %lu\n", total_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    for (i = 0; i < node_count; i++) {
        free(nodes[i]);
    }
    
    return 0;
}
