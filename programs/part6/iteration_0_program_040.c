/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256
#define MAX_DEPTH 5

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* AST-like recursive structure */
typedef struct TreeNode {
    char buffer[64];
    int value;
    struct TreeNode* left;
    struct TreeNode* right;
    struct TreeNode* parent;
} TreeNode;

/* Global token array */
static char g_token_array[ARRAY_SIZE];

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token array with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        g_token_array[i] = (char)((i * 7) & 0xFF);
    }
    printf("Constructor: Token array initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Clear sensitive data */
    __builtin_memset(g_token_array, 0, ARRAY_SIZE);
    printf("Destructor: Cleanup completed\n");
}

/* Recursive tree creation */
static TreeNode* create_tree(int depth, TreeNode* parent) {
    if (depth >= MAX_DEPTH) return NULL;
    
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    if (!node) return NULL;
    
    /* Initialize node with built-in memset */
    __builtin_memset(node, 0, sizeof(TreeNode));
    
    /* Copy parent's buffer if exists */
    if (parent) {
        __builtin_memcpy(node->buffer, parent->buffer, sizeof(node->buffer));
    } else {
        /* Fill with pattern */
        for (int i = 0; i < sizeof(node->buffer); i++) {
            node->buffer[i] = (char)(depth * 16 + i);
        }
    }
    
    node->value = depth * 100;
    node->parent = parent;
    
    /* Recursive creation with goto for flow control */
    if (depth % 2 == 0) {
        goto create_left;
    } else {
        goto create_right;
    }
    
create_left:
    node->left = create_tree(depth + 1, node);
    goto after_left;
    
create_right:
    node->right = create_tree(depth + 1, node);
    goto after_right;
    
after_left:
    node->right = create_tree(depth + 1, node);
    goto done;
    
after_right:
    node->left = create_tree(depth + 1, node);
    goto done;
    
done:
    return node;
}

/* Complex memory operations with goto jumps */
static void perform_memory_operations(TreeNode* node) {
    char local_buf[128];
    char* dyn_buf = NULL;
    
    if (!node) return;
    
    /* Jump into memory operation block */
    goto start_ops;
    
bypass_init:
    /* This label is jumped to, testing flow sensitivity */
    dyn_buf = (char*)malloc(g_mem_size);
    if (!dyn_buf) goto cleanup;
    
    /* Use all three built-ins with volatile sizes */
    volatile size_t copy_size = g_mem_size / 2;
    
    __builtin_memset(dyn_buf, 0xAA, g_mem_size);
    __builtin_memcpy(local_buf, node->buffer, sizeof(node->buffer));
    
    /* Jump around memmove */
    if (node->value > 200) {
        goto skip_memmove;
    }
    
    __builtin_memmove(dyn_buf + 10, dyn_buf, copy_size);
    
skip_memmove:
    /* Copy between tree nodes */
    if (node->left && node->right) {
        __builtin_memcpy(node->left->buffer, node->right->buffer, 
                        sizeof(node->buffer));
    }
    
    /* Jump back for another round */
    if (node->value < 400) {
        goto bypass_init;
    }
    
cleanup:
    if (dyn_buf) {
        __builtin_memset(dyn_buf, 0, g_mem_size);
        free(dyn_buf);
    }
    return;
    
start_ops:
    /* Initial setup */
    __builtin_memset(local_buf, 0, sizeof(local_buf));
    goto bypass_init;
}

/* OpenMP parallel memory dispatcher */
static void parallel_memory_dispatch(TreeNode* root) {
    int sum = 0;
    
    #pragma omp parallel reduction(+:sum)
    {
        int thread_id = omp_get_thread_num();
        char thread_buf[64];
        
        /* Each thread performs memory operations */
        __builtin_memset(thread_buf, thread_id, sizeof(thread_buf));
        
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            char temp[32];
            
            /* Mix of built-ins in parallel region */
            __builtin_memset(temp, i, sizeof(temp));
            
            if (i % 3 == 0) {
                __builtin_memcpy(temp, thread_buf, 16);
            } else if (i % 3 == 1) {
                __builtin_memmove(temp + 8, temp, 16);
            }
            
            sum += temp[i % 32];
        }
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Additional memory ops after barrier */
        if (thread_id == 0 && root) {
            perform_memory_operations(root);
        }
    }
    
    printf("Parallel sum: %d\n", sum);
}

/* Recursive tree traversal with memory operations */
static int traverse_and_process(TreeNode* node, int depth) {
    if (!node) return 0;
    
    int result = node->value;
    char temp_buf[48];
    
    /* Conditional jumps around memory ops */
    if (depth % 2 == 0) {
        goto even_depth_ops;
    }
    
    /* Odd depth: use memcpy */
    __builtin_memcpy(temp_buf, node->buffer, sizeof(temp_buf));
    goto after_copy;
    
even_depth_ops:
    /* Even depth: use memset and memmove */
    __builtin_memset(temp_buf, depth, sizeof(temp_buf));
    
    if (node->parent) {
        __builtin_memmove(temp_buf + 16, node->parent->buffer, 32);
    }
    
after_copy:
    /* Process buffer */
    for (int i = 0; i < sizeof(temp_buf); i++) {
        result += temp_buf[i];
    }
    
    /* Recursive traversal */
    result += traverse_and_process(node->left, depth + 1);
    result += traverse_and_process(node->right, depth + 1);
    
    return result;
}

/* Free tree recursively */
static void free_tree(TreeNode* node) {
    if (!node) return;
    
    free_tree(node->left);
    free_tree(node->right);
    
    /* Clear node memory before free */
    __builtin_memset(node, 0, sizeof(TreeNode));
    free(node);
}

int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Create recursive tree structure */
    TreeNode* root = create_tree(0, NULL);
    if (!root) {
        fprintf(stderr, "Failed to create tree\n");
        return 1;
    }
    
    /* Phase 1: Individual memory operations */
    printf("Phase 1: Individual built-in tests\n");
    
    char test_buf1[128], test_buf2[128];
    volatile size_t op_size = g_mem_size;
    
    /* Force all three built-ins to be used */
    __builtin_memset(test_buf1, 0x55, sizeof(test_buf1));
    __builtin_memcpy(test_buf2, test_buf1, op_size);
    __builtin_memmove(test_buf1 + 32, test_buf1, op_size / 2);
    
    /* Phase 2: Recursive tree processing */
    printf("Phase 2: Tree traversal with memory ops\n");
    int tree_result = traverse_and_process(root, 0);
    printf("Tree traversal result: %d\n", tree_result);
    
    /* Phase 3: OpenMP parallel operations */
    printf("Phase 3: Parallel memory dispatch\n");
    parallel_memory_dispatch(root);
    
    /* Phase 4: Complex flow with goto */
    printf("Phase 4: Flow-sensitive memory operations\n");
    perform_memory_operations(root);
    
    /* Phase 5: Token array processing */
    printf("Phase 5: Global array operations\n");
    char processed[ARRAY_SIZE];
    
    /* Multiple memory operations on global array */
    __builtin_memcpy(processed, g_token_array, ARRAY_SIZE);
    
    for (int i = 0; i < 10; i++) {
        size_t offset = (i * 23) % ARRAY_SIZE;
        size_t len = (ARRAY_SIZE - offset) / 2;
        
        if (i % 2 == 0) {
            __builtin_memset(processed + offset, i, len);
        } else {
            __builtin_memmove(processed + offset, processed, len);
        }
    }
    
    /* Calculate final hash */
    uint32_t final_hash = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_hash = (final_hash * 31) + processed[i];
    }
    
    printf("Final hash: 0x%08X\n", (unsigned int)final_hash);
    
    /* Cleanup */
    free_tree(root);
    
    printf("=== Test completed successfully ===\n");
    return 0;
}
