/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* AST-like recursive structure */
typedef struct Node {
    char *data;
    size_t size;
    struct Node *left;
    struct Node *right;
} Node;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_globals(void) {
    printf("Constructor: Initializing ASAN/HWASAN test environment\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_globals(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive tree manipulation with memory operations */
static Node* create_node(size_t size) {
    Node *node = (Node*)malloc(sizeof(Node));
    if (!node) return NULL;
    
    node->data = (char*)malloc(size);
    node->size = size;
    node->left = node->right = NULL;
    
    /* Use __builtin_memset to initialize */
    if (node->data) {
        __builtin_memset(node->data, 0xAA, size);
    }
    
    return node;
}

static void copy_node_data(Node *dest, Node *src) {
    if (!dest || !src || !dest->data || !src->data) return;
    
    size_t copy_size = dest->size < src->size ? dest->size : src->size;
    
    /* Force __builtin_memcpy usage */
    __builtin_memcpy(dest->data, src->data, copy_size);
}

static void recursive_tree_ops(Node *root, int depth) {
    if (!root || depth <= 0) return;
    
    /* Create children */
    root->left = create_node(g_mem_size / 2);
    root->right = create_node(g_mem_size / 2);
    
    if (root->left && root->right) {
        /* Copy data between nodes using goto for flow control */
        int use_memmove = g_use_memmove;
        
        if (use_memmove) {
            goto use_memmove_block;
        } else {
            copy_node_data(root->left, root->right);
            goto skip_memmove;
        }
        
    use_memmove_block:
        /* This block tests memmove redirection with goto */
        if (root->left->data && root->right->data) {
            __builtin_memmove(root->left->data, root->right->data, 
                            root->left->size < root->right->size ? 
                            root->left->size : root->right->size);
        }
        
    skip_memmove:
        /* Continue recursion */
        recursive_tree_ops(root->left, depth - 1);
        recursive_tree_ops(root->right, depth - 1);
    }
}

static void free_tree(Node *root) {
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root->data);
    free(root);
}

/* Parallel memory operations using OpenMP */
static void parallel_mem_operations(void) {
    const int num_threads = 4;
    const size_t block_size = 1024;
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        char *src = (char*)malloc(block_size);
        char *dst = (char*)malloc(block_size);
        
        if (src && dst) {
            /* Each thread uses different builtins */
            switch (tid % 3) {
                case 0:
                    __builtin_memset(src, tid, block_size);
                    __builtin_memcpy(dst, src, block_size);
                    break;
                case 1:
                    __builtin_memset(dst, tid + 1, block_size);
                    __builtin_memmove(src, dst, block_size / 2);
                    break;
                case 2:
                    __builtin_memset(src, 0xFF, block_size);
                    __builtin_memcpy(dst, src, block_size - 64);
                    __builtin_memset(dst + block_size - 64, 0x00, 64);
                    break;
            }
            
            /* Verify with volatile read */
            volatile char check = dst[0];
            (void)check;
        }
        
        free(src);
        free(dst);
    }
}

/* Complex token processing with memory builtins */
static size_t process_tokens(const char **tokens, int count) {
    size_t hash = 0;
    char buffer[512];
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        
        /* Clear buffer with memset */
        __builtin_memset(buffer, 0, sizeof(buffer));
        
        /* Copy token with memcpy */
        if (len < sizeof(buffer)) {
            __builtin_memcpy(buffer, tokens[i], len);
            
            /* Move data around with memmove */
            if (i > 0 && len > 16) {
                __builtin_memmove(buffer + 8, buffer, len - 8);
            }
        }
        
        /* Simple hash computation */
        for (size_t j = 0; j < len && j < sizeof(buffer); j++) {
            hash = hash * 31 + buffer[j];
        }
    }
    
    return hash;
}

int main(void) {
    printf("=== ASAN/HWASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Tree operations with recursive memory copies */
    Node *root = create_node(g_mem_size);
    if (root) {
        recursive_tree_ops(root, 3);
        
        /* Additional memory operation between tree nodes */
        if (root->left && root->right) {
            size_t min_size = root->left->size < root->right->size ? 
                             root->left->size : root->right->size;
            __builtin_memcpy(root->left->data, root->right->data, min_size);
        }
        
        free_tree(root);
    }
    
    /* Phase 2: Parallel memory operations */
    printf("Starting parallel memory operations...\n");
    parallel_mem_operations();
    
    /* Phase 3: Token processing */
    const char *tokens[] = {
        "memcpy", "memset", "memmove", "asan", "hwasan",
        "instrumentation", "redzone", "builtin", "coverage"
    };
    
    size_t token_hash = process_tokens(tokens, 
                                      sizeof(tokens)/sizeof(tokens[0]));
    printf("Token hash: %zu\n", token_hash);
    
    /* Phase 4: Variable-sized memory operations */
    volatile size_t dynamic_size = 128;
    char *dynamic_src = (char*)malloc(dynamic_size * 2);
    char *dynamic_dst = dynamic_src + dynamic_size;
    
    if (dynamic_src) {
        /* Chain of memory operations */
        __builtin_memset(dynamic_src, 0xCC, dynamic_size * 2);
        __builtin_memcpy(dynamic_dst, dynamic_src, dynamic_size);
        
        /* Conditional memmove with goto */
        if (g_use_memmove) {
            goto do_memmove;
        }
        
        __builtin_memset(dynamic_src, 0xDD, dynamic_size);
        goto skip_final;
        
    do_memmove:
        __builtin_memmove(dynamic_src, dynamic_dst, dynamic_size);
        
    skip_final:
        free(dynamic_src);
    }
    
    printf("Test completed successfully\n");
    return 0;
}
