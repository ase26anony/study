/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_trigger = 1;

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
    volatile char buffer[128];
    /* Force __builtin_memset initialization */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    printf("[Constructor] Initialized buffer\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    printf("[Destructor] Cleaning up\n");
}

/* Recursive function with memory operations */
static ASTNode* create_tree(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    node->left = create_tree(depth - 1, counter);
    node->right = create_tree(depth - 1, counter);
    
    /* Use __builtin_memset to initialize data */
    __builtin_memset(node->data, node->id & 0xFF, sizeof(node->data));
    
    return node;
}

/* Function with goto jumps around memmove */
static void jumpy_memmove(char* dest, char* src, size_t len) {
    if (len == 0) return;
    
    volatile int use_memmove = volatile_trigger;
    
    if (use_memmove) {
        goto do_copy;
    } else {
        goto skip_copy;
    }
    
do_copy:
    /* This should trigger the memmove redirection */
    __builtin_memmove(dest, src, len);
    goto after_copy;
    
skip_copy:
    dest[0] = src[0];
    
after_copy:
    /* Add some computation to prevent dead code elimination */
    dest[len-1] = src[len-1] ^ 0x55;
}

/* Parallel memory operations */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buf[256];
        char shared_buf[256];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        #pragma omp single
        {
            /* Master thread copies using __builtin_memcpy */
            __builtin_memcpy(shared_buf, local_buf, sizeof(shared_buf));
        }
        
        #pragma omp barrier
        
        /* All threads verify the copy */
        if (shared_buf[0] == 0) {
            __builtin_memset(shared_buf, 0xFF, volatile_len % 256);
        }
    }
}

/* Complex token processing with memory operations */
static unsigned long process_tokens(const char** tokens, int count) {
    unsigned long hash = 0xDEADBEEF;
    char buffer[512];
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        if (len > sizeof(buffer)) len = sizeof(buffer);
        
        /* Alternate between memcpy and memset */
        if (i % 2 == 0) {
            __builtin_memcpy(buffer, tokens[i], len);
        } else {
            __builtin_memset(buffer, tokens[i][0], len);
        }
        
        /* Mix in memmove occasionally */
        if (i % 3 == 0) {
            char temp[512];
            jumpy_memmove(temp, buffer, len);
            __builtin_memcpy(buffer, temp, len);
        }
        
        /* Compute hash */
        for (size_t j = 0; j < len; j++) {
            hash = (hash * 31) + buffer[j];
        }
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Tree operations */
    int counter = 1;
    ASTNode* root = create_tree(3, &counter);
    
    if (root) {
        /* Copy between tree nodes */
        __builtin_memcpy(root->left->data, root->right->data, 
                        volatile_len % sizeof(root->data));
        
        /* Process tree data */
        unsigned long tree_hash = 0;
        ASTNode* nodes[] = {root, root->left, root->right, root->left->left};
        
        for (int i = 0; i < 4; i++) {
            if (nodes[i]) {
                tree_hash ^= process_tokens((const char**)&nodes[i]->data, 1);
            }
        }
        printf("Tree hash: 0x%lx\n", tree_hash);
    }
    
    /* Phase 2: Parallel operations */
    printf("Starting parallel memory operations\n");
    parallel_mem_ops();
    
    /* Phase 3: Token processing */
    const char* tokens[] = {
        "ASAN_TEST_STRING_1",
        "BUILTIN_MEMCPY",
        "BUILTIN_MEMSET",
        "BUILTIN_MEMMOVE",
        "HWASAN_REDIRECT",
        "VOLATILE_LENGTH",
        "GOTO_FLOW_CONTROL",
        "OPENMP_PARALLEL"
    };
    
    unsigned long final_hash = process_tokens(tokens, 
                                            sizeof(tokens)/sizeof(tokens[0]));
    
    printf("Final hash: 0x%lx\n", final_hash);
    printf("Test completed\n");
    
    /* Cleanup */
    /* Note: In real ASAN, memory would be automatically checked for leaks */
    
    return 0;
}
