/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    volatile char buffer[128];
    /* Force __builtin_memset in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    volatile_flag = 0;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    volatile char cleanup_buf[64];
    /* Force __builtin_memcpy in destructor */
    __builtin_memcpy(cleanup_buf, "DESTRUCTOR", 10);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for node initialization */
    __builtin_memcpy(node->data, base_data, strlen(base_data) + 1);
    
    node->size = strlen(base_data) + 1;
    node->left = create_ast(depth - 1, "LEFT");
    node->right = create_ast(depth - 1, "RIGHT");
    
    return node;
}

/* Function with goto statements and __builtin_memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    char temp[512];
    int use_memmove = 0;
    
    /* Jump into memory operation block */
    if (volatile_flag) goto memmove_block;
    
    normal_path:
    /* Standard __builtin_memcpy */
    __builtin_memcpy(dst->data, src->data, src->size);
    return;
    
    memmove_block:
    /* Use __builtin_memmove with overlapping regions */
    __builtin_memcpy(temp, src->data, src->size);
    
    /* Jump out and back in */
    if (use_memmove) goto after_memmove;
    
    /* Force __builtin_memmove with goto */
    __builtin_memmove(dst->data, temp, src->size);
    use_memmove = 1;
    goto memmove_block;
    
    after_memmove:
    /* Cleanup with __builtin_memset */
    __builtin_memset(temp, 0, sizeof(temp));
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        char local_buf[256];
        char src_buf[256];
        
        /* Initialize source with __builtin_memset */
        __builtin_memset(src_buf, omp_get_thread_num() + 'A', sizeof(src_buf));
        
        /* Copy with __builtin_memcpy */
        __builtin_memcpy(local_buf, src_buf, volatile_len % 256);
        
        /* Move with __builtin_memmove (potential overlap) */
        __builtin_memmove(local_buf + 128, local_buf, 128);
        
        #pragma omp barrier
        
        /* Final memset */
        __builtin_memset(local_buf + 192, 0, 64);
    }
}

/* Multi-stage processing with different builtins */
static size_t process_ast_tree(ASTNode* root) {
    if (!root) return 0;
    
    size_t total = 0;
    char buffer[1024];
    
    /* Stage 1: Process left subtree with memcpy */
    if (root->left) {
        __builtin_memcpy(buffer, root->left->data, root->left->size);
        total += root->left->size;
    }
    
    /* Stage 2: Process current node with memset */
    __builtin_memset(root->data + root->size - 16, 0, 16);
    
    /* Stage 3: Process right subtree with memmove */
    if (root->right) {
        /* Create overlapping region for memmove */
        char* overlap_src = root->right->data;
        char* overlap_dst = root->right->data + 32;
        size_t move_len = (root->right->size > 32) ? root->right->size - 32 : 0;
        
        if (move_len > 0) {
            __builtin_memmove(overlap_dst, overlap_src, move_len);
        }
        total += root->right->size;
    }
    
    return total + process_ast_tree(root->left) + process_ast_tree(root->right);
}

/* Main test driver */
int main(void) {
    size_t total_size = 0;
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create recursive AST structure */
    ASTNode* tree1 = create_ast(3, "ROOT_NODE_DATA_HERE");
    ASTNode* tree2 = create_ast(2, "SECOND_TREE_ROOT");
    
    if (!tree1 || !tree2) {
        fprintf(stderr, "Failed to create AST trees\n");
        return 1;
    }
    
    /* Test 1: Goto-based memory operations */
    process_with_goto(tree1, tree2);
    
    /* Test 2: OpenMP parallel operations */
    #ifdef _OPENMP
    parallel_memory_ops();
    #endif
    
    /* Test 3: Recursive tree processing */
    total_size = process_ast_tree(tree1);
    total_size += process_ast_tree(tree2);
    
    /* Test 4: Direct builtin calls with volatile control */
    char final_buffer[1024];
    volatile int dynamic_size = volatile_len * 2;
    
    __builtin_memset(final_buffer, 0xCC, dynamic_size % 1024);
    __builtin_memcpy(final_buffer + 256, tree1->data, tree1->size);
    __builtin_memmove(final_buffer, final_buffer + 128, 384);
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        hash = (hash * 31) + final_buffer[i];
    }
    
    printf("Processing complete. Total size: %zu, Hash: 0x%08lx\n", 
           total_size, hash & 0xFFFFFFFF);
    
    /* Cleanup */
    /* Note: In real code, you'd need proper tree freeing logic */
    
    return 0;
}
