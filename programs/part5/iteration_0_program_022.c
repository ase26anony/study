/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_constructor(void) {
    volatile char buffer[128];
    /* Force __builtin_memset in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    printf("Constructor initialized\n");
}

/* Destructor for cleanup */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    volatile char buffer[64];
    __builtin_memset(buffer, 0xFF, sizeof(buffer));
    printf("Destructor cleaning up\n");
}

/* Function with goto jumping into memory operation block */
static void goto_memmove_test(char* dest, const char* src, size_t n) {
    int use_memmove = 0;
    
    if (volatile_flag > 0) {
        use_memmove = 1;
        goto perform_copy;
    }
    
    /* Normal path */
    __builtin_memcpy(dest, src, n);
    return;
    
perform_copy:
    /* Jumped-into block with __builtin_memmove */
    __builtin_memmove(dest, src, n);
    
    /* Jump out to different location */
    if (n > 32) {
        goto cleanup;
    }
    
    /* Additional memory operation after jump */
    __builtin_memset(dest + n/2, 0xCC, n/2);
    return;
    
cleanup:
    __builtin_memset(dest, 0x00, n);
}

/* Recursive function with memory operations on AST */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with __builtin_memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[32];
    __builtin_memset(pattern, 'A' + (depth % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(node->data));
    
    /* Recursive creation with volatile length control */
    size_t child_len = (size_t)volatile_flag * 16;
    if (child_len > sizeof(node->data)) child_len = sizeof(node->data);
    
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    /* Copy between nodes if both children exist */
    if (node->left && node->right) {
        __builtin_memcpy(node->right->data, node->left->data, child_len);
    }
    
    return node;
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(char* buffer, size_t size) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        size_t chunk = size / omp_get_num_threads();
        size_t offset = tid * chunk;
        
        if (offset + chunk > size) chunk = size - offset;
        
        /* Each thread uses different builtins */
        switch (tid % 3) {
            case 0:
                __builtin_memset(buffer + offset, tid, chunk);
                break;
            case 1:
                if (offset > 0) {
                    __builtin_memcpy(buffer + offset, buffer + offset - 1, chunk);
                }
                break;
            case 2:
                if (offset + chunk < size) {
                    __builtin_memmove(buffer + offset, buffer + offset + 1, chunk);
                }
                break;
        }
        
        #pragma omp barrier
        
        /* Collective operation after barrier */
        #pragma omp single
        {
            __builtin_memset(buffer + size/2, 0x55, size/4);
        }
    }
}

/* Complex token processing with control flow */
static size_t process_tokens(char** tokens, int count) {
    size_t hash = 0;
    char temp_buffer[256];
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        
        /* Use volatile to control which builtin is called */
        if ((volatile_flag + i) % 3 == 0) {
            __builtin_memcpy(temp_buffer, tokens[i], len);
        } else if ((volatile_flag + i) % 3 == 1) {
            __builtin_memset(temp_buffer, tokens[i][0], len);
        } else {
            __builtin_memmove(temp_buffer, tokens[i], len);
        }
        
        /* Compute hash from buffer */
        for (size_t j = 0; j < len && j < sizeof(temp_buffer); j++) {
            hash = hash * 31 + temp_buffer[j];
        }
        
        /* Goto-based control flow around memory ops */
        if (i == count/2) {
            goto special_case;
        }
        continue;
        
    special_case:
        __builtin_memset(temp_buffer, 0xFF, sizeof(temp_buffer));
        hash ^= 0xDEADBEEF;
    }
    
    return hash;
}

int main(void) {
    const size_t buffer_size = 1024;
    char* main_buffer = (char*)malloc(buffer_size);
    if (!main_buffer) return 1;
    
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Basic builtin calls */
    __builtin_memset(main_buffer, 0x00, buffer_size);
    __builtin_memcpy(main_buffer, "Test Pattern", 12);
    __builtin_memmove(main_buffer + 20, main_buffer, 12);
    
    /* Phase 2: Goto-based memory operations */
    goto_memmove_test(main_buffer + 100, main_buffer, 50);
    
    /* Phase 3: Recursive AST operations */
    ASTNode* root = create_ast(4);
    if (root) {
        /* Copy between tree nodes */
        if (root->left && root->right) {
            size_t copy_len = (size_t)volatile_len % 32;
            __builtin_memcpy(root->right->data, root->left->data, copy_len);
        }
        
        /* TODO: Add AST cleanup */
    }
    
    /* Phase 4: OpenMP parallel operations */
    parallel_memory_ops(main_buffer, buffer_size);
    
    /* Phase 5: Token processing */
    char* tokens[] = {"token1", "token2", "token3", "token4", "token5"};
    size_t final_hash = process_tokens(tokens, 5);
    
    /* Final verification */
    size_t sum = 0;
    for (size_t i = 0; i < buffer_size; i++) {
        sum += (unsigned char)main_buffer[i];
    }
    
    printf("Buffer sum: %zu, Token hash: %zu\n", sum, final_hash);
    printf("Test completed successfully\n");
    
    free(main_buffer);
    /* TODO: Free AST nodes */
    
    return 0;
}
