/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
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
    int depth;
} ASTNode;

/* Constructor function with __attribute__ */
static void __attribute__((constructor)) init_globals(void) {
    printf("Constructor: Initializing global state\n");
}

/* Destructor function */
static void __attribute__((destructor)) cleanup_globals(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive AST creation with memory operations */
ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with volatile size */
    size_t copy_len = g_mem_size % 64;
    if (copy_len > 63) copy_len = 63;
    
    __builtin_memcpy(node->data, base_data, copy_len);
    node->data[copy_len] = '\0';
    node->depth = depth;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = depth % 2;
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, "left_branch");
        node->right = create_ast(depth - 1, "right_branch");
        return node;
        
    create_children:
        /* Jump target with __builtin_memmove */
        char temp[64];
        __builtin_memcpy(temp, node->data, sizeof(temp));
        __builtin_memmove(node->data, temp, sizeof(node->data));
        
        node->left = create_ast(depth - 1, "goto_left");
        node->right = create_ast(depth - 1, "goto_right");
    }
    
    return node;
}

/* Function with OpenMP parallel section */
void parallel_memory_operations(void) {
    const int array_size = 1024;
    char* src = (char*)malloc(array_size);
    char* dst = (char*)malloc(array_size);
    
    if (!src || !dst) return;
    
    /* Initialize source with pattern */
    for (int i = 0; i < array_size; i++) {
        src[i] = (char)(i % 256);
    }
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int chunk_size = array_size / omp_get_num_threads();
        int start = thread_id * chunk_size;
        
        /* Each thread performs memory operations */
        __builtin_memcpy(dst + start, src + start, chunk_size);
        
        /* Modify with memset */
        __builtin_memset(dst + start + chunk_size/2, thread_id, chunk_size/4);
        
        /* Use memmove for overlapping regions */
        if (chunk_size > 100) {
            __builtin_memmove(dst + start + 50, dst + start, 50);
        }
    }
    
    /* Verify by calculating checksum */
    unsigned long long checksum = 0;
    for (int i = 0; i < array_size; i++) {
        checksum += (unsigned char)dst[i];
    }
    
    printf("Parallel checksum: %llu\n", checksum);
    
    free(src);
    free(dst);
}

/* Complex token processing with goto jumps */
void process_tokens_with_goto(char** tokens, int count) {
    char buffer[256];
    int i = 0;
    
    /* Jump into memory operation block */
    if (count > 0) goto start_processing;
    
    return;
    
start_processing:
    while (i < count) {
        /* Clear buffer with memset */
        __builtin_memset(buffer, 0, sizeof(buffer));
        
        /* Copy token with memcpy */
        size_t len = strlen(tokens[i]);
        if (len > sizeof(buffer) - 1) len = sizeof(buffer) - 1;
        __builtin_memcpy(buffer, tokens[i], len);
        
        /* Jump out if token is special */
        if (strcmp(buffer, "EXIT") == 0) goto cleanup;
        
        /* Move data around with memmove */
        if (i > 0 && i < count - 1) {
            __builtin_memmove(buffer + 10, buffer, 20);
        }
        
        i++;
        
        /* Conditional jump back */
        if (i % 3 == 0) goto start_processing;
    }
    
cleanup:
    /* Final memory operation after goto */
    __builtin_memset(buffer, 0xFF, sizeof(buffer));
}

/* Main test driver */
int main(void) {
    printf("=== ASAN/HWASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Recursive AST operations */
    printf("\nPhase 1: Creating recursive AST\n");
    ASTNode* root = create_ast(4, "root_data");
    
    if (root) {
        /* Copy entire AST structure */
        ASTNode copy;
        __builtin_memcpy(&copy, root, sizeof(ASTNode));
        
        /* Move data between nodes */
        if (root->left && root->right) {
            __builtin_memmove(root->left->data, root->right->data, 
                            sizeof(root->left->data));
        }
        
        /* Recursive cleanup would be here in real code */
        free(root);
    }
    
    /* Phase 2: OpenMP parallel operations */
    printf("\nPhase 2: Parallel memory operations\n");
    parallel_memory_operations();
    
    /* Phase 3: Token processing with goto */
    printf("\nPhase 3: Token processing with goto jumps\n");
    char* tokens[] = {"TOKEN1", "TOKEN2", "EXIT", "TOKEN3"};
    process_tokens_with_goto(tokens, 4);
    
    /* Phase 4: Direct built-in calls with volatile control */
    printf("\nPhase 4: Direct built-in calls\n");
    {
        volatile char v_src[128];
        volatile char v_dst[128];
        
        for (volatile int i = 0; i < 128; i++) {
            v_src[i] = (char)(i * 3);
        }
        
        /* Force all three built-ins with volatile sizes */
        volatile size_t op_size = g_mem_size % 128;
        __builtin_memcpy((void*)v_dst, (void*)v_src, op_size);
        __builtin_memset((void*)v_dst, 0x42, op_size / 2);
        __builtin_memmove((void*)(v_dst + 32), (void*)v_dst, 64);
        
        /* Calculate and print verification hash */
        unsigned int hash = 0;
        for (volatile int i = 0; i < 128; i++) {
            hash = hash * 31 + (unsigned char)v_dst[i];
        }
        printf("Verification hash: %u\n", hash);
    }
    
    printf("\n=== Test completed ===\n");
    return 0;
}
