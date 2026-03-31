/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_tokens[][32] = {
    "memcpy_test_token_1",
    "memset_test_token_2", 
    "memmove_test_token_3",
    "asan_coverage_token_4"
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    volatile char init_buf[128];
    /* Force memset built-in in constructor */
    __builtin_memset(init_buf, 0xAA, sizeof(init_buf));
    
    /* Copy initialization pattern */
    const char* pattern = "ASAN_INIT_PATTERN";
    __builtin_memcpy(init_buf + 16, pattern, __builtin_strlen(pattern) + 1);
    
    /* Move data around */
    __builtin_memmove(init_buf + 32, init_buf + 16, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[64];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive parser with memory operations */
static ASTNode* parse_tokens_recursive(const char** tokens, int depth, int index) {
    if (depth <= 0 || tokens[index] == NULL) {
        return NULL;
    }
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use memset built-in for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data using memcpy built-in */
    size_t token_len = __builtin_strlen(tokens[index]);
    if (token_len > sizeof(node->data) - 1) {
        token_len = sizeof(node->data) - 1;
    }
    __builtin_memcpy(node->data, tokens[index], token_len);
    node->data[token_len] = '\0';
    
    node->id = index * 1000 + depth;
    
    /* Control flow with goto to test flow sensitivity */
    if (depth % 2 == 0) {
        goto process_left;
    } else {
        goto process_right;
    }
    
process_left:
    node->left = parse_tokens_recursive(tokens, depth - 1, (index + 1) % 4);
    /* Jump back */
    goto continue_processing;
    
process_right:
    node->right = parse_tokens_recursive(tokens, depth - 1, (index + 3) % 4);
    /* Another goto to create complex flow */
    if (node->right) {
        goto copy_between_nodes;
    }
    
continue_processing:
    /* Additional memory operation after goto */
    if (node->left && node->right) {
        volatile char temp_buf[64];
        __builtin_memcpy(temp_buf, node->left->data, sizeof(temp_buf));
        __builtin_memmove(node->right->data, temp_buf, sizeof(temp_buf));
    }
    return node;
    
copy_between_nodes:
    /* Memory move between nodes using goto block */
    if (node->left) {
        __builtin_memmove(node->data + 32, node->left->data, 32);
    }
    goto continue_processing;
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_operations(void) {
    const int num_blocks = 8;
    char* blocks[num_blocks];
    
    #pragma omp parallel for
    for (int i = 0; i < num_blocks; i++) {
        blocks[i] = (char*)malloc(g_mem_size);
        if (blocks[i]) {
            /* Use all three built-ins in parallel region */
            __builtin_memset(blocks[i], i, g_mem_size);
            
            /* Copy between blocks with offset */
            if (i > 0) {
                __builtin_memcpy(blocks[i] + 32, blocks[i-1], 64);
            }
            
            /* Move data within block */
            __builtin_memmove(blocks[i] + 128, blocks[i] + 64, 64);
        }
    }
    
    /* Cleanup */
    #pragma omp parallel for
    for (int i = 0; i < num_blocks; i++) {
        if (blocks[i]) {
            free(blocks[i]);
        }
    }
}

/* Complex memory dispatch with varied contexts */
static unsigned long execute_memory_dispatch(ASTNode* root) {
    unsigned long hash = 0;
    volatile char buffer[512];
    volatile char shadow[512];
    
    /* Phase 1: Direct built-in calls */
    __builtin_memset(buffer, 0xCC, sizeof(buffer));
    
    /* Copy token data */
    for (int i = 0; i < 4; i++) {
        __builtin_memcpy(buffer + i * 64, g_tokens[i], 32);
    }
    
    /* Move data around with overlapping regions */
    __builtin_memmove(buffer + 128, buffer, 256);
    
    /* Phase 2: Conditional built-in calls */
    ASTNode* current = root;
    int level = 0;
    while (current) {
        volatile char node_buf[128];
        
        /* Copy node data */
        __builtin_memcpy(node_buf, current->data, 64);
        
        /* Conditional memset based on node ID */
        if (current->id % 3 == 0) {
            __builtin_memset(node_buf + 64, 0xAA, 32);
        } else if (current->id % 3 == 1) {
            __builtin_memset(node_buf + 64, 0xBB, 32);
        }
        
        /* Move to shadow buffer */
        __builtin_memmove(shadow + level * 64, node_buf, 64);
        
        /* Update hash */
        for (int j = 0; j < 64; j++) {
            hash = (hash * 31) + shadow[level * 64 + j];
        }
        
        current = current->left;
        level++;
    }
    
    /* Phase 3: Nested memory operations */
    volatile char nested_buf[4][256];
    for (int i = 0; i < 4; i++) {
        __builtin_memset(nested_buf[i], i * 16, sizeof(nested_buf[i]));
        
        for (int j = 0; j < i; j++) {
            __builtin_memcpy(nested_buf[i] + j * 32, 
                           nested_buf[j] + i * 32, 32);
            __builtin_memmove(nested_buf[j] + i * 32,
                            nested_buf[i] + j * 32, 32);
        }
        
        /* Add to hash */
        for (int k = 0; k < 256; k++) {
            hash = (hash * 17) + nested_buf[i][k];
        }
    }
    
    return hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize token pointers */
    const char* token_ptrs[] = {
        g_tokens[0], g_tokens[1], g_tokens[2], g_tokens[3], NULL
    };
    
    /* Create recursive AST */
    ASTNode* root = parse_tokens_recursive(token_ptrs, 5, 0);
    
    /* Execute parallel operations */
    parallel_memory_operations();
    
    /* Execute memory dispatch and get result */
    unsigned long result = execute_memory_dispatch(root);
    
    /* Additional memory operations in main */
    volatile char final_buf[1024];
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    
    /* Copy all tokens */
    for (int i = 0; i < 4; i++) {
        __builtin_memcpy(final_buf + i * 256, g_tokens[i], 32);
    }
    
    /* Move overlapping regions */
    __builtin_memmove(final_buf + 512, final_buf, 512);
    
    /* Update result with final buffer */
    for (int i = 0; i < 256; i++) {
        result = (result * 13) + final_buf[i * 4];
    }
    
    printf("Test completed. Result hash: %lu\n", result);
    
    /* Cleanup */
    free_ast(root);
    
    return 0;
}
