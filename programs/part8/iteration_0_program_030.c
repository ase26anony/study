/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 128;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[64];
    /* Force __builtin_memset redirection early */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    printf("[constructor] Initialized ASAN early buffer\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    volatile char final_buf[32];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
    printf("[destructor] Cleanup complete\n");
}

/* Recursive tree manipulation with memory operations */
ASTNode* create_node(int id, const char* pattern) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for node data initialization */
    __builtin_memcpy(node->data, pattern, strlen(pattern) + 1);
    node->left = node->right = NULL;
    node->id = id;
    
    return node;
}

/* Complex memory operation with goto flow control */
void process_with_goto(ASTNode* dest, ASTNode* src) {
    int use_copy = 1;
    
    /* Jump into memory operation block */
    if (dest && src) {
        goto perform_op;
    }
    
    return;
    
perform_op:
    /* This block tests flow-sensitive ASAN handling */
    if (use_copy) {
        __builtin_memcpy(dest->data, src->data, sizeof(dest->data));
    } else {
        volatile char temp[256];
        __builtin_memcpy(temp, src->data, sizeof(temp));
        __builtin_memcpy(dest->data, temp, sizeof(dest->data));
    }
    
    /* Jump out */
    goto finish;
    
finish:
    /* Additional memset to ensure coverage */
    __builtin_memset(src->data + 128, 0, 32);
}

/* OpenMP parallel memory operations */
void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        volatile char local_buf[256];
        volatile char shared_buf[512];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, tid, sizeof(local_buf));
        
        #pragma omp barrier
        
        /* Conditional memmove based on thread ID */
        if (tid % 2 == 0) {
            __builtin_memmove(shared_buf + tid * 64, 
                            local_buf, 
                            g_mem_size % 128);
        } else {
            __builtin_memcpy(shared_buf + tid * 64, 
                           local_buf, 
                           g_mem_size % 128);
        }
        
        /* Verify with memset */
        __builtin_memset(local_buf, 0, 16);
    }
}

/* Multi-stage processing with different builtins */
size_t process_token_array(const char** tokens, int count) {
    volatile char accumulator[1024] = {0};
    size_t total_len = 0;
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]) + 1;
        
        /* Alternate between memcpy and memmove */
        if (g_use_memmove && i > 0) {
            __builtin_memmove(accumulator + total_len, 
                            accumulator + total_len - 32, 
                            len < 32 ? len : 32);
        }
        
        __builtin_memcpy(accumulator + total_len, tokens[i], len);
        total_len += len;
        
        /* Periodic memset to clear sections */
        if (i % 3 == 0) {
            __builtin_memset(accumulator + total_len - 16, 0, 16);
        }
    }
    
    return total_len;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* 1. Initialize token array */
    const char* tokens[] = {
        "ASAN_TEST_STRING_1",
        "HWASAN_BUILTIN_REDIRECT",
        "MEMCPY_MEMSET_MEMMOVE",
        "GOTO_FLOW_CONTROL",
        "OPENMP_PARALLEL_OPS",
        "RECURSIVE_AST_NODES",
        "VOLATILE_CONSTANTS",
        "CONSTRUCTOR_DESTRUCTOR"
    };
    
    /* 2. Process tokens with memory operations */
    size_t total_size = process_token_array(tokens, 
                        sizeof(tokens)/sizeof(tokens[0]));
    printf("Token array processed: %zu bytes\n", total_size);
    
    /* 3. Create and manipulate AST structures */
    ASTNode* root = create_node(1, "ROOT_NODE_DATA");
    ASTNode* child1 = create_node(2, "CHILD_1_DATA_PATTERN");
    ASTNode* child2 = create_node(3, "CHILD_2_DATA_PATTERN");
    
    if (root && child1 && child2) {
        /* Test goto flow control with memory ops */
        process_with_goto(root, child1);
        process_with_goto(child2, root);
        
        /* Complex memmove between nodes */
        __builtin_memmove(root->data + 64, 
                         child2->data + 32, 
                         g_mem_size % 192);
        
        /* Verify with memset */
        __builtin_memset(child1->data, 0xCC, 128);
    }
    
    /* 4. Execute OpenMP parallel section */
    parallel_mem_ops();
    
    /* 5. Final verification hash */
    unsigned long hash = 0;
    if (root) {
        for (int i = 0; i < 256; i++) {
            hash = (hash * 31) + root->data[i];
        }
    }
    
    printf("Final hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(root);
    free(child1);
    free(child2);
    
    return 0;
}
