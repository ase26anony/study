/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Force compiler to use builtins instead of library calls */
#define USE_BUILTINS

/* Recursive AST-like structure */
typedef struct Node {
    int id;
    volatile int data[16];          /* volatile to prevent optimization */
    struct Node* left;
    struct Node* right;
} Node;

/* Global volatile variables to control memory operation sizes */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 32;
volatile size_t g_memmove_len = 48;

/* Token array for parser simulation */
static const char* tokens[] = {"mem", "cpy", "set", "move", "data", "node"};
static const int token_count = 6;

/* Constructor/destructor for initialization coordination */
__attribute__((constructor)) static void init_asan_hooks(void) {
    printf("Constructor: Initializing ASAN hooks\n");
}

__attribute__((destructor)) static void cleanup_asan_hooks(void) {
    printf("Destructor: Cleaning up ASAN hooks\n");
}

/* Recursive parser with goto control flow */
static int parse_tokens(int idx, int depth, char* buffer) {
    if (depth >= 3 || idx >= token_count) return idx;
    
    int start_idx = idx;
    
    /* Label for goto jumping into memory operation block */
memmove_block:
    if (idx + 1 < token_count) {
        char temp[128];
        
        /* __builtin_memmove with goto entry point */
        __builtin_memmove(temp, tokens[idx], strlen(tokens[idx]) + 1);
        
        /* Jump target from outside */
        idx++;
        
        /* Complex control flow with goto */
        if (idx % 2 == 0) {
            goto recursive_call;
        }
    }
    
recursive_call:
    /* Recursive call with memory operation in parameters */
    idx = parse_tokens(idx, depth + 1, buffer);
    
    /* Jump back to memmove block */
    if (idx < token_count && depth == 1) {
        goto memmove_block;
    }
    
    return idx;
}

/* Create recursive tree structure */
static Node* create_tree(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    Node* node = (Node*)malloc(sizeof(Node));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Initialize data with __builtin_memset */
    __builtin_memset(node->data, node->id % 256, sizeof(node->data));
    
    /* Create children */
    node->left = create_tree(depth - 1, counter);
    node->right = create_tree(depth - 1, counter);
    
    return node;
}

/* Copy data between tree nodes using __builtin_memcpy */
static void copy_node_data(Node* dest, const Node* src) {
    if (!dest || !src) return;
    
    /* Use volatile length to prevent optimization */
    size_t len = g_memcpy_len;
    if (len > sizeof(dest->data)) len = sizeof(dest->data);
    
    __builtin_memcpy(dest->data, src->data, len);
}

/* Process tree with OpenMP parallelization */
static int process_tree_parallel(Node* root) {
    int total = 0;
    
    #pragma omp parallel reduction(+:total)
    {
        #pragma omp single
        {
            /* Dispatch memory operations in parallel sections */
            #pragma omp task
            {
                if (root && root->left) {
                    char buffer[256];
                    size_t len = g_memmove_len;
                    if (len > sizeof(buffer)) len = sizeof(buffer);
                    
                    /* __builtin_memmove in parallel task */
                    __builtin_memmove(buffer, root->left->data, len);
                    
                    /* Compute checksum */
                    for (size_t i = 0; i < len; i++) {
                        total += buffer[i];
                    }
                }
            }
            
            #pragma omp task
            {
                if (root && root->right) {
                    volatile char fill_pattern = 0xAB;
                    size_t len = g_memset_len;
                    char temp[128];
                    
                    if (len > sizeof(temp)) len = sizeof(temp);
                    
                    /* __builtin_memset in parallel task */
                    __builtin_memset(temp, fill_pattern, len);
                    
                    for (size_t i = 0; i < len; i++) {
                        total += temp[i];
                    }
                }
            }
            
            #pragma omp taskwait
        }
    }
    
    return total;
}

/* Free tree memory */
static void free_tree(Node* node) {
    if (!node) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Recursive token parsing with goto */
    char parse_buffer[1024];
    int parsed = parse_tokens(0, 0, parse_buffer);
    printf("Parsed %d tokens\n", parsed);
    
    /* Phase 2: Create and process recursive tree */
    int counter = 1;
    Node* tree = create_tree(3, &counter);
    
    if (tree && tree->left && tree->right) {
        /* Test __builtin_memcpy between nodes */
        copy_node_data(tree->right, tree->left);
        
        /* Additional __builtin_memcpy with different sizes */
        volatile size_t dynamic_len = 24;
        char extra_buffer[64];
        __builtin_memcpy(extra_buffer, tree->data, dynamic_len);
        
        /* Test __builtin_memset on node data */
        __builtin_memset(tree->left->data, 0xFF, 8);
    }
    
    /* Phase 3: OpenMP parallel processing */
    int parallel_result = 0;
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            parallel_result = process_tree_parallel(tree);
        }
    }
    
    /* Phase 4: Additional built-in calls in main */
    volatile char main_buffer[256];
    
    /* Sequence of all three builtins */
    __builtin_memset(main_buffer, 0x11, g_memset_len);
    __builtin_memcpy(main_buffer + 64, main_buffer, g_memcpy_len);
    __builtin_memmove(main_buffer + 128, main_buffer, g_memmove_len);
    
    /* Compute final hash */
    uint32_t hash = 0;
    for (size_t i = 0; i < sizeof(main_buffer); i++) {
        hash = (hash * 31) + main_buffer[i];
    }
    
    printf("Parallel result: %d\n", parallel_result);
    printf("Final hash: 0x%08X\n", hash);
    printf("Tree nodes created: %d\n", counter - 1);
    
    /* Cleanup */
    free_tree(tree);
    
    return 0;
}
