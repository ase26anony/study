/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    printf("Constructor: Initializing ASAN environment\n");
    /* Force early initialization of memory builtins */
    char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    printf("Destructor: Cleaning up ASAN environment\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast_tree(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile to control memset size */
    volatile size_t clear_size = sizeof(node->data);
    __builtin_memset(node->data, 0, clear_size);
    
    /* Copy data with builtin memcpy */
    size_t copy_len = strlen(base_data);
    if (copy_len > sizeof(node->data) - 1)
        copy_len = sizeof(node->data) - 1;
    
    __builtin_memcpy(node->data, base_data, copy_len);
    node->data[copy_len] = '\0';
    node->size = copy_len;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        char left_data[256];
        char right_data[256];
        
        /* Complex goto pattern around memmove */
        int use_goto = 1;
        
        if (use_goto) {
            goto create_left;
        }
        
        /* This block should be jumped into */
        create_left: {
            snprintf(left_data, sizeof(left_data), "%s-L%d", base_data, depth);
            node->left = create_ast_tree(depth - 1, left_data);
            
            /* Jump out to avoid right creation initially */
            if (depth % 2 == 0) {
                goto skip_right;
            }
        }
        
        /* Right branch creation */
        {
            snprintf(right_data, sizeof(right_data), "%s-R%d", base_data, depth);
            node->right = create_ast_tree(depth - 1, right_data);
        }
        
        skip_right:
        /* Use memmove between nodes if both exist */
        if (node->left && node->right) {
            volatile size_t move_size = node->left->size;
            if (move_size > node->right->size)
                move_size = node->right->size;
            
            __builtin_memmove(node->right->data, 
                            node->left->data, 
                            move_size);
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_operations(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Force all three builtins in parallel context */
            char temp[256];
            volatile size_t op_size = nodes[i]->size;
            
            /* memset in parallel region */
            __builtin_memset(temp, 0xAA, op_size % 128);
            
            /* memcpy in parallel region */
            __builtin_memcpy(temp, nodes[i]->data, 
                           op_size > sizeof(temp) ? sizeof(temp) : op_size);
            
            /* memmove within the same buffer */
            if (op_size > 16) {
                __builtin_memmove(temp + 8, temp, op_size / 2);
            }
            
            /* Copy back to node */
            __builtin_memcpy(nodes[i]->data, temp, 
                           op_size > sizeof(temp) ? sizeof(temp) : op_size);
        }
    }
}

/* Calculate hash of AST tree */
static unsigned long calculate_tree_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    /* Simple DJB2 hash */
    while (*ptr) {
        hash = ((hash << 5) + hash) + *ptr++;
    }
    
    /* Recursive hash combination */
    unsigned long left_hash = calculate_tree_hash(node->left);
    unsigned long right_hash = calculate_tree_hash(node->right);
    
    return hash ^ (left_hash << 1) ^ (right_hash >> 1);
}

int main(void) {
    const int NUM_TREES = 8;
    const int TREE_DEPTH = 4;
    ASTNode* trees[NUM_TREES];
    unsigned long final_hash = 0;
    
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create multiple AST trees */
    for (int i = 0; i < NUM_TREES; i++) {
        char base_data[64];
        snprintf(base_data, sizeof(base_data), "Tree%d-Node0", i);
        trees[i] = create_ast_tree(TREE_DEPTH, base_data);
    }
    
    /* Perform parallel memory operations */
    printf("Executing parallel memory operations...\n");
    parallel_memory_operations(trees, NUM_TREES);
    
    /* Calculate combined hash */
    for (int i = 0; i < NUM_TREES; i++) {
        if (trees[i]) {
            final_hash ^= calculate_tree_hash(trees[i]);
            
            /* Additional memory operation in main */
            char verify_buffer[128];
            volatile size_t verify_size = trees[i]->size;
            
            __builtin_memcpy(verify_buffer, trees[i]->data, 
                           verify_size > sizeof(verify_buffer) ? 
                           sizeof(verify_buffer) : verify_size);
            
            /* Use all three builtins in sequence */
            __builtin_memset(verify_buffer + 64, 0, 32);
            __builtin_memmove(verify_buffer, verify_buffer + 32, 32);
        }
    }
    
    printf("Final hash: 0x%08lx\n", final_hash);
    
    /* Cleanup */
    for (int i = 0; i < NUM_TREES; i++) {
        /* Recursive free (simplified for example) */
        free(trees[i]);
    }
    
    return 0;
}
