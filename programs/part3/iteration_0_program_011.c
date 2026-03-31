/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force early initialization of ASAN runtime */
    volatile char buffer[16];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_late(void) {
    /* Final memory operation to ensure coverage */
    volatile char final_buf[8];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_tree(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Fill data with pattern using memcpy */
    char pattern[64];
    for (int i = 0; i < 64; i++) pattern[i] = (char)(i + depth);
    __builtin_memcpy(node->data, pattern, 64);
    
    node->size = g_mem_size % 128 + 64;
    
    /* Recursive creation with goto for control flow */
    int use_left = depth % 2;
    
    if (use_left) {
        node->left = create_tree(depth - 1);
        goto skip_right;
    }
    
    node->right = create_tree(depth - 1);
    goto end;
    
skip_right:
    node->right = create_tree(depth - 2);
    
end:
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int mode = g_use_memmove;
    
    if (mode == 0) {
        /* Direct path */
        __builtin_memcpy(dst->data, src->data, src->size);
        return;
    }
    
    /* Jump into memmove block */
    goto memmove_block;
    
normal_path:
    __builtin_memcpy(dst->data + 32, src->data + 32, 32);
    return;
    
memmove_block:
    /* This tests flow-sensitivity of asan_memfn_rtls retrieval */
    __builtin_memmove(dst->data, src->data, src->size);
    goto normal_path;
}

/* Parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    #pragma omp parallel for
    for (int i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Volatile-controlled memory operations */
            volatile size_t local_size = g_mem_size;
            char temp[256];
            
            /* Mix of builtins */
            __builtin_memset(temp, i, local_size % 256);
            __builtin_memcpy(nodes[i]->data, temp, nodes[i]->size);
            
            /* Conditional memmove */
            if (i % 3 == 0) {
                __builtin_memmove(nodes[i]->data + 16, nodes[i]->data, 48);
            }
        }
    }
}

/* Complex initialization with multiple builtins */
static void initialize_token_array(char** tokens, int token_count) {
    for (int i = 0; i < token_count; i++) {
        tokens[i] = (char*)malloc(128);
        if (tokens[i]) {
            /* Pattern initialization */
            __builtin_memset(tokens[i], 0, 128);
            
            /* Fill with data using memcpy */
            char source[128];
            __builtin_memset(source, 'A' + (i % 26), 128);
            __builtin_memcpy(tokens[i], source, 64 + (i * 3) % 64);
            
            /* Overlap handling with memmove */
            if (i % 2 == 0) {
                __builtin_memmove(tokens[i] + 32, tokens[i], 64);
            }
        }
    }
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Phase 1: Tree creation with recursive memory ops */
    ASTNode* root = create_tree(4);
    ASTNode* copy = create_tree(3);
    
    /* Phase 2: Goto-based memory operations */
    process_with_goto(root, copy);
    
    /* Phase 3: Array of nodes for parallel processing */
    ASTNode* nodes[8];
    for (int i = 0; i < 8; i++) {
        nodes[i] = create_tree(2 + (i % 3));
    }
    
    /* Phase 4: Parallel memory operations */
    parallel_memory_ops(nodes, 8);
    
    /* Phase 5: Token array processing */
    char* tokens[16];
    initialize_token_array(tokens, 16);
    
    /* Verification: Compute hash/sum */
    unsigned long long hash = 0;
    for (int i = 0; i < 8 && nodes[i]; i++) {
        for (size_t j = 0; j < 64 && j < nodes[i]->size; j++) {
            hash += (unsigned long long)nodes[i]->data[j];
        }
    }
    
    for (int i = 0; i < 16 && tokens[i]; i++) {
        for (int j = 0; j < 64; j++) {
            hash += (unsigned long long)tokens[i][j];
        }
        free(tokens[i]);
    }
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        free(nodes[i]);
    }
    free(root);
    free(copy);
    
    printf("Verification hash: %llu\n", hash);
    printf("Test completed successfully\n");
    
    return (hash > 0) ? 0 : 1;
}
