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
    int id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {"memcpy", "memset", "memmove", "test", "data"};
static const int num_tokens = 5;

/* Constructor/destructor functions */
__attribute__((constructor)) static void init_globals(void) {
    printf("Initializing ASAN test environment...\n");
}

__attribute__((destructor)) static void cleanup_globals(void) {
    printf("Cleaning up ASAN test environment...\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_node(int id) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with memcpy */
    const char* src = tokens[id % num_tokens];
    __builtin_memcpy(node->data, src, strlen(src) + 1);
    
    return node;
}

static void copy_node_data(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use memcpy or memmove based on volatile flag */
    if (g_use_memmove) {
        __builtin_memmove(dest->data, src->data, sizeof(dest->data));
    } else {
        __builtin_memcpy(dest->data, src->data, sizeof(dest->data));
    }
}

static ASTNode* build_tree(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = create_node((*counter)++);
    if (!node) return NULL;
    
    /* Recursive tree construction */
    node->left = build_tree(depth - 1, counter);
    node->right = build_tree(depth - 1, counter);
    
    return node;
}

static void process_tree_parallel(ASTNode* root) {
    if (!root) return;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Parallel tree processing with memory operations */
            #pragma omp task
            {
                if (root->left && root->right) {
                    /* Force memmove usage with goto */
                    goto use_memmove;
                }
                /* Fall through to memcpy */
                __builtin_memcpy(root->data, "parallel", 9);
                goto end;
                
            use_memmove:
                __builtin_memmove(root->data, "memmove_used", 13);
            end:
                ;
            }
            
            #pragma omp task
            {
                process_tree_parallel(root->left);
            }
            
            #pragma omp task
            {
                process_tree_parallel(root->right);
            }
        }
    }
}

static unsigned long compute_tree_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    /* Simple hash computation */
    while (*ptr) {
        hash = ((hash << 5) + hash) + *ptr++;
    }
    
    /* Recursive hash combination */
    hash += compute_tree_hash(node->left);
    hash += compute_tree_hash(node->right);
    
    return hash;
}

static void free_tree(ASTNode* node) {
    if (!node) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

/* Memory stress function with varied contexts */
static void memory_stress_test(void) {
    char buffer1[512];
    char buffer2[512];
    volatile size_t size = g_mem_size;
    
    /* Test all three builtins in different contexts */
    
    /* 1. Basic memset */
    __builtin_memset(buffer1, 0xAA, size);
    
    /* 2. Memcpy with goto control flow */
    int use_copy = 1;
    if (use_copy) {
        goto do_copy;
    } else {
        goto do_move;
    }
    
do_copy:
    __builtin_memcpy(buffer2, buffer1, size);
    goto next;
    
do_move:
    __builtin_memmove(buffer2, buffer1, size);
    
next:
    /* 3. Overlapping memmove */
    __builtin_memmove(buffer1 + 128, buffer1, 256);
    
    /* 4. Small constant-sized operations (non-foldable due to volatile) */
    volatile int small_size = 16;
    __builtin_memset(buffer1 + 256, 0xCC, small_size);
    __builtin_memcpy(buffer2 + 256, buffer1 + 256, small_size);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize counter for tree building */
    int counter = 0;
    
    /* Build recursive tree structure */
    ASTNode* root = build_tree(3, &counter);
    if (!root) {
        fprintf(stderr, "Failed to build tree\n");
        return 1;
    }
    
    /* Perform memory stress test */
    memory_stress_test();
    
    /* Parallel processing with OpenMP */
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            /* Test memcpy in parallel section */
            char local_buf[128];
            __builtin_memset(local_buf, 0x11, sizeof(local_buf));
            __builtin_memcpy(local_buf + 64, local_buf, 64);
        }
        
        #pragma omp section
        {
            /* Test memmove in parallel section */
            char local_buf[128];
            __builtin_memset(local_buf, 0x22, sizeof(local_buf));
            __builtin_memmove(local_buf + 32, local_buf, 96);
        }
    }
    
    /* Process tree with parallel tasks */
    process_tree_parallel(root);
    
    /* Compute verification hash */
    unsigned long final_hash = compute_tree_hash(root);
    printf("Final tree hash: %lu\n", final_hash);
    
    /* Additional builtin calls in different scopes */
    {
        char scope_buf[64];
        __builtin_memset(scope_buf, 0x33, sizeof(scope_buf));
        __builtin_memcpy(scope_buf + 32, scope_buf, 32);
    }
    
    /* Cleanup */
    free_tree(root);
    
    printf("Test completed successfully\n");
    return 0;
}
