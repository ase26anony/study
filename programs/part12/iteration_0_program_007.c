/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static const char* tokens[] = {"memcpy", "memset", "memmove", "asan", "hwasan"};
static const size_t num_tokens = sizeof(tokens)/sizeof(tokens[0]);

/* Constructor/destructor functions */
__attribute__((constructor)) static void init_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 256 + (rand() % 768);
}

__attribute__((destructor)) static void cleanup(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset to initialize */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data using builtin memcpy */
    size_t idx = depth % num_tokens;
    __builtin_memcpy(node->data, tokens[idx], strlen(tokens[idx]) + 1);
    
    node->size = g_mem_size / (depth + 1);
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int use_memmove = 1;
    
    if (use_memmove) {
        goto do_copy;
    } else {
        goto skip_copy;
    }
    
do_copy:
    /* Builtin memmove with overlapping regions */
    __builtin_memmove(dst->data + 10, dst->data, 50);
    goto after_copy;
    
skip_copy:
    printf("Skipping memmove\n");
    
after_copy:
    /* Another memmove after label */
    if (src != dst) {
        __builtin_memmove(dst->data, src->data, 100);
    }
}

/* Parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, size_t count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (size_t i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Force builtin usage with volatile size */
                volatile size_t op_size = g_mem_size / (i + 2);
                
                /* Use all three builtins */
                __builtin_memset(nodes[i]->data + 100, tid, op_size % 128);
                
                if (i > 0) {
                    __builtin_memcpy(nodes[i]->data, nodes[i-1]->data, 64);
                }
                
                if (i < count - 1) {
                    __builtin_memmove(nodes[i]->data + 64, 
                                     nodes[i]->data, 32);
                }
            }
        }
    }
}

/* Complex memory dispatch */
static size_t memory_dispatch(ASTNode* root) {
    size_t hash = 0;
    ASTNode* stack[64];
    int top = 0;
    
    if (root) stack[top++] = root;
    
    while (top > 0) {
        ASTNode* current = stack[--top];
        
        /* Process current node */
        for (size_t i = 0; i < sizeof(current->data); i++) {
            hash += (size_t)current->data[i];
        }
        
        /* Add children to stack */
        if (current->right) {
            /* Copy right child data before pushing */
            ASTNode temp;
            __builtin_memcpy(&temp, current->right, sizeof(ASTNode));
            stack[top++] = current->right;
            
            /* Use memmove on overlapping data */
            if (top < 64) {
                __builtin_memmove(current->data, 
                                 current->data + 128, 64);
            }
        }
        
        if (current->left) {
            stack[top++] = current->left;
            
            /* Another memset */
            __builtin_memset(current->data + 192, 
                           top % 256, 32);
        }
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Create AST */
    ASTNode* root = create_ast(4);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create node array for parallel ops */
    ASTNode* nodes[8];
    nodes[0] = root;
    for (int i = 1; i < 8; i++) {
        nodes[i] = create_ast(2);
    }
    
    /* Test goto flow with memmove */
    process_with_goto(root, nodes[1]);
    
    /* Execute parallel memory operations */
    parallel_memory_ops(nodes, 8);
    
    /* Perform memory dispatch */
    size_t result = memory_dispatch(root);
    printf("Result hash: %zu\n", result);
    
    /* Additional builtin calls in different contexts */
    char buffer1[512], buffer2[512];
    volatile size_t copy_size = g_mem_size % 256;
    
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memset(buffer2, 0xBB, sizeof(buffer2));
    
    __builtin_memcpy(buffer1 + 128, buffer2, copy_size);
    __builtin_memmove(buffer2 + 64, buffer2, copy_size / 2);
    
    /* Final verification */
    int sum = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        sum += buffer1[i];
        sum += buffer2[i];
    }
    printf("Buffer checksum: %d\n", sum % 1000);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        if (nodes[i]) free(nodes[i]);
    }
    
    return 0;
}
