/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    size_t size;
    uint32_t hash;
} ASTNode;

/* Global token array */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan",
    "test", "coverage", "builtin", "volatile", "recursive"
};
static const size_t g_token_count = 10;

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_test(void) {
    printf("ASAN Test Constructor: Initializing test environment\n");
    /* Force early initialization of ASAN runtime */
    volatile char dummy[16];
    __builtin_memset(dummy, 0, sizeof(dummy));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    printf("ASAN Test Destructor: Cleaning up\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* data, size_t depth) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using builtin memcpy */
    size_t len = strlen(data);
    if (len > sizeof(node->data) - 1)
        len = sizeof(node->data) - 1;
    __builtin_memcpy(node->data, data, len);
    node->data[len] = '\0';
    
    node->size = len;
    
    /* Calculate simple hash */
    uint32_t hash = 0;
    for (size_t i = 0; i < len; i++) {
        hash = (hash * 31) + (uint8_t)node->data[i];
    }
    node->hash = hash;
    
    /* Recursive creation with depth limit */
    if (depth > 0) {
        char left_data[32], right_data[32];
        snprintf(left_data, sizeof(left_data), "%s_L%d", data, (int)depth);
        snprintf(right_data, sizeof(right_data), "%s_R%d", data, (int)depth);
        
        node->left = create_ast_node(left_data, depth - 1);
        node->right = create_ast_node(right_data, depth - 1);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with goto edge cases */
static void test_goto_memmove(void) {
    volatile char buffer1[256];
    volatile char buffer2[256];
    volatile int condition = 1;
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 'A', sizeof(buffer1));
    __builtin_memset(buffer2, 'B', sizeof(buffer2));
    
    goto start_block;
    
memmove_block:
    /* This block contains the critical memmove operation */
    __builtin_memmove((void*)buffer1, (void*)buffer2, 
                     g_mem_size % sizeof(buffer1));
    goto end_test;
    
start_block:
    if (condition) {
        /* Jump into memmove block */
        goto memmove_block;
    }
    
    /* Unreachable but tests control flow */
    __builtin_memcpy((void*)buffer2, (void*)buffer1, 64);
    
end_test:
    /* Verify the move worked */
    volatile char verify = buffer1[0];
    (void)verify; /* Suppress unused warning */
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    const size_t num_buffers = 8;
    volatile char buffers[num_buffers][512];
    volatile size_t sizes[num_buffers];
    
    /* Initialize sizes */
    for (size_t i = 0; i < num_buffers; i++) {
        sizes[i] = (i * 64 + 128) % 512;
    }
    
    #pragma omp parallel for
    for (size_t i = 0; i < num_buffers; i++) {
        /* Each thread uses builtins */
        __builtin_memset(buffers[i], i, sizes[i]);
        
        /* Rotate buffers using memcpy/memmove */
        size_t next = (i + 1) % num_buffers;
        if (i % 2 == 0) {
            __builtin_memcpy(buffers[next], buffers[i], 
                           sizes[i] < sizes[next] ? sizes[i] : sizes[next]);
        } else {
            __builtin_memmove(buffers[next], buffers[i], 
                            sizes[i] < sizes[next] ? sizes[i] : sizes[next]);
        }
    }
}

/* Complex memory dispatch with varied contexts */
static uint64_t dispatch_memory_operations(void) {
    uint64_t result_hash = 0;
    ASTNode* ast_root = create_ast_node("root", 3);
    
    if (!ast_root) return 0;
    
    /* Process AST with different memory operations */
    ASTNode* current = ast_root;
    ASTNode* temp_nodes[4];
    
    /* Create temporary nodes */
    for (int i = 0; i < 4; i++) {
        char name[16];
        snprintf(name, sizeof(name), "temp%d", i);
        temp_nodes[i] = create_ast_node(name, 1);
    }
    
    /* Complex pattern of memory operations */
    int stage = 0;
    while (current) {
        switch (stage % 4) {
            case 0:
                /* Copy node data using builtin memcpy */
                if (temp_nodes[0]) {
                    __builtin_memcpy(temp_nodes[0]->data, current->data,
                                   sizeof(current->data));
                    result_hash ^= temp_nodes[0]->hash;
                }
                break;
                
            case 1:
                /* Clear using builtin memset */
                if (temp_nodes[1]) {
                    __builtin_memset(temp_nodes[1]->data, stage,
                                   sizeof(temp_nodes[1]->data) / 2);
                }
                break;
                
            case 2:
                /* Move data around with builtin memmove */
                if (temp_nodes[2] && temp_nodes[3]) {
                    __builtin_memmove(temp_nodes[2]->data, temp_nodes[3]->data,
                                    sizeof(temp_nodes[2]->data));
                }
                break;
                
            case 3:
                /* Copy between AST nodes */
                if (current->left && current->right) {
                    size_t copy_size = current->left->size < current->right->size ?
                                      current->left->size : current->right->size;
                    __builtin_memcpy(current->right->data, current->left->data,
                                   copy_size);
                }
                break;
        }
        
        result_hash = (result_hash * 6364136223846793005ULL) + current->hash;
        current = (stage % 2 == 0) ? current->left : current->right;
        stage++;
        
        if (stage > 20) break; /* Safety limit */
    }
    
    /* Cleanup */
    /* Recursive free implementation omitted for brevity */
    
    return result_hash;
}

/* Helper to free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Force initialization of all three builtins early */
    volatile char init_buf[32];
    __builtin_memset(init_buf, 0, sizeof(init_buf));
    __builtin_memcpy(init_buf, "init", 5);
    __builtin_memmove(init_buf + 1, init_buf, 4);
    
    /* Test control flow with goto */
    test_goto_memmove();
    
    /* Execute parallel operations */
    parallel_memory_ops();
    
    /* Dispatch complex memory operations */
    uint64_t final_hash = dispatch_memory_operations();
    
    /* Additional stress test with token array */
    char token_buffer[1024];
    size_t offset = 0;
    
    for (size_t i = 0; i < g_token_count; i++) {
        size_t len = strlen(g_tokens[i]);
        if (offset + len < sizeof(token_buffer)) {
            __builtin_memcpy(token_buffer + offset, g_tokens[i], len);
            offset += len;
            token_buffer[offset++] = ' ';
        }
    }
    
    /* Final verification memset */
    __builtin_memset(token_buffer + offset, 0, 
                    sizeof(token_buffer) - offset);
    
    printf("Test completed. Final hash: 0x%016llx\n", 
           (unsigned long long)final_hash);
    printf("Token buffer starts with: %.32s\n", token_buffer);
    
    return 0;
}
