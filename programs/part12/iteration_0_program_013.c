/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_selector = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) void init_asan_redirection() {
    printf("Constructor: Initializing ASAN redirection test environment\n");
    
    /* Force early built-in usage in constructor */
    char buffer1[128];
    char buffer2[128];
    
    /* Use all three built-ins in constructor */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1, buffer2, sizeof(buffer1));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) void cleanup_asan_test() {
    printf("Destructor: Cleaning up ASAN test resources\n");
}

/* Recursive function with memory operations */
ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with built-in memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with pattern */
    for (int i = 0; i < 255; i++) {
        node->data[i] = 'A' + (id + i) % 26;
    }
    node->data[255] = '\0';
    
    /* Recursive creation with goto for control flow testing */
    int use_goto = (id % 3 == 0);
    
    if (use_goto) {
        goto create_children;
    }
    
    node->left = create_ast(depth - 1, id * 2);
    node->right = NULL;
    
create_children:
    /* Jump target with memmove operation */
    if (node->right == NULL) {
        ASTNode temp_node;
        __builtin_memset(&temp_node, 0xCC, sizeof(ASTNode));
        
        /* Use volatile length */
        size_t copy_len = volatile_len % sizeof(ASTNode);
        if (copy_len > 0) {
            __builtin_memmove(node, &temp_node, copy_len);
        }
        
        node->right = create_ast(depth - 1, id * 2 + 1);
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
void goto_mem_operations(char* dest, char* src, size_t len) {
    int condition = volatile_selector;
    
    if (condition > 100) {
        goto direct_copy;
    }
    
    /* Normal path */
    __builtin_memset(dest, 0x55, len);
    return;
    
direct_copy:
    /* Jump target containing memcpy */
    __builtin_memcpy(dest, src, len);
    
    /* Jump back out */
    goto cleanup;
    
cleanup:
    /* Final memmove after goto */
    __builtin_memmove(src, dest, len / 2);
}

/* OpenMP parallel memory operations */
void parallel_memory_operations() {
    const int num_threads = 4;
    char thread_buffers[num_threads][256];
    char shared_buffer[1024];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread uses different built-ins */
        switch (tid % 3) {
            case 0:
                __builtin_memset(thread_buffers[tid], tid, 256);
                break;
            case 1:
                if (tid > 0) {
                    __builtin_memcpy(thread_buffers[tid], 
                                   thread_buffers[tid-1], 
                                   256);
                }
                break;
            case 2:
                __builtin_memmove(thread_buffers[tid],
                                shared_buffer + tid * 64,
                                128);
                break;
        }
        
        /* Barrier to ensure all built-ins are processed */
        #pragma omp barrier
        
        /* Mixed operations after barrier */
        __builtin_memcpy(shared_buffer + tid * 64,
                        thread_buffers[tid],
                        64);
    }
    
    /* Final memmove on shared buffer */
    __builtin_memmove(shared_buffer, shared_buffer + 512, 512);
}

/* Complex token processing with memory operations */
unsigned long process_tokens(char** tokens, int count) {
    unsigned long hash = 0;
    char buffer[512];
    char* current = buffer;
    
    for (int i = 0; i < count; i++) {
        size_t token_len = strlen(tokens[i]);
        
        /* Use different built-ins based on conditions */
        if (i % 3 == 0) {
            __builtin_memset(current, i, token_len);
        } else if (i % 3 == 1) {
            __builtin_memcpy(current, tokens[i], token_len);
        } else {
            __builtin_memmove(current, tokens[i], token_len);
        }
        
        /* Compute simple hash */
        for (size_t j = 0; j < token_len && j < 512; j++) {
            hash = hash * 31 + current[j];
        }
        
        current += token_len;
        if (current - buffer > 400) {
            __builtin_memmove(buffer, buffer + 256, 256);
            current -= 256;
        }
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Test 1: Basic built-in usage with volatile */
    char src[256], dest[256];
    size_t len = volatile_len % 256;
    
    __builtin_memset(src, 0x42, len);
    __builtin_memcpy(dest, src, len);
    __builtin_memmove(src + 32, src, len - 32);
    
    /* Test 2: Goto control flow */
    goto_mem_operations(dest, src, len);
    
    /* Test 3: Recursive AST operations */
    ASTNode* root = create_ast(4, 1);
    if (root) {
        /* Copy between AST nodes */
        if (root->left && root->right) {
            __builtin_memcpy(root->right, root->left, sizeof(ASTNode));
            __builtin_memmove(root->left, root->right, sizeof(ASTNode));
        }
        
        /* Free AST (simplified - real code would recurse) */
        free(root);
    }
    
    /* Test 4: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Test 5: Token processing */
    char* tokens[] = {
        "ASAN", "HWASAN", "memcpy", "memset", "memmove",
        "builtin", "redirection", "coverage", "test"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    unsigned long final_hash = process_tokens(tokens, token_count);
    
    /* Test 6: Mixed operations in loop */
    for (int i = 0; i < 10; i++) {
        char temp[128];
        volatile_selector = i;
        
        if (i % 2 == 0) {
            __builtin_memset(temp, i, sizeof(temp));
        } else {
            __builtin_memcpy(temp, dest + i * 8, 64);
            __builtin_memmove(dest + i * 8, temp, 64);
        }
    }
    
    printf("Test completed. Final hash: %lu\n", final_hash);
    printf("All built-in memory operations executed\n");
    
    return 0;
}
