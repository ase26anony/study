/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int g_volatile_len = 64;
static volatile char g_volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_tokens[1024];
static int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize tokens with pattern */
    for (int i = 0; i < sizeof(g_tokens); i++) {
        g_tokens[i] = (i % 26) + 'A';
    }
    
    /* Use builtins in constructor */
    char local_buf[128];
    __builtin_memset(local_buf, 0xCC, sizeof(local_buf));
    __builtin_memcpy(local_buf, "CONSTRUCTOR_INIT", 16);
    
    printf("Constructor initialized tokens\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Clear sensitive data */
    __builtin_memset(g_tokens, 0, sizeof(g_tokens));
    printf("Destructor cleared tokens\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = depth;
    
    /* Copy data with volatile length control */
    int copy_len = g_volatile_len % 256;
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Create children recursively */
    node->left = create_ast_node(depth - 1, base_data);
    node->right = create_ast_node(depth - 1, base_data);
    
    /* Copy between nodes if both exist */
    if (node->left && node->right) {
        int move_len = (g_volatile_len / 2) % 256;
        __builtin_memmove(node->right->data, node->left->data, move_len);
    }
    
    return node;
}

/* Function with goto jumps around memmove */
static void goto_memmove_test(char* dest, char* src, size_t len) {
    int use_memmove = g_volatile_flag;
    
    if (use_memmove) {
        goto do_copy;
    } else {
        goto skip_copy;
    }
    
do_copy:
    /* This should trigger ASAN memmove redirection */
    __builtin_memmove(dest, src, len);
    goto after_copy;
    
skip_copy:
    /* Alternative path */
    __builtin_memset(dest, 0, len);
    
after_copy:
    /* Common continuation */
    dest[0] = 'X';
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buf[512];
        char src_buf[512];
        
        /* Initialize source with thread-specific pattern */
        for (int i = 0; i < sizeof(src_buf); i++) {
            src_buf[i] = (i + thread_id) % 256;
        }
        
        /* Use all three builtins in parallel region */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        __builtin_memcpy(local_buf + 128, src_buf, 256);
        
        /* Conditional memmove with volatile */
        if (g_volatile_flag) {
            __builtin_memmove(local_buf, local_buf + 64, 128);
        }
        
        /* Verify by computing checksum */
        unsigned long sum = 0;
        for (int i = 0; i < sizeof(local_buf); i++) {
            sum += (unsigned char)local_buf[i];
        }
        
        #pragma omp critical
        {
            printf("Thread %d: checksum = %lu\n", thread_id, sum);
        }
    }
}

/* Multi-stage initialization */
static void initialize_system(void) {
    /* Stage 1: Direct builtin calls */
    char stage1_buf[1024];
    __builtin_memset(stage1_buf, 0xAA, sizeof(stage1_buf));
    
    /* Stage 2: Copy from global tokens */
    __builtin_memcpy(stage1_buf, g_tokens, 512);
    
    /* Stage 3: Move within buffer */
    __builtin_memmove(stage1_buf + 256, stage1_buf, 256);
    
    /* Stage 4: Recursive structure operations */
    ASTNode* root = create_ast_node(3, "AST_BASE_DATA");
    if (root) {
        /* Copy between tree nodes */
        if (root->left && root->right) {
            int copy_size = g_volatile_len % 128;
            __builtin_memcpy(root->right->data, root->left->data, copy_size);
        }
        
        /* Free recursively */
        free(root->left);
        free(root->right);
        free(root);
    }
}

int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Basic builtin calls */
    char buffer1[4096];
    char buffer2[4096];
    
    /* Force initialization of asan_memfn_rtls cache */
    __builtin_memset(buffer1, 0, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1, buffer2, sizeof(buffer1));
    
    /* Phase 2: Goto flow control test */
    goto_memmove_test(buffer1 + 1024, buffer2 + 512, 256);
    
    /* Phase 3: OpenMP parallel section */
    printf("\n--- OpenMP Parallel Section ---\n");
    parallel_memory_ops();
    
    /* Phase 4: Complex initialization */
    printf("\n--- Multi-stage Initialization ---\n");
    initialize_system();
    
    /* Phase 5: Final verification */
    printf("\n--- Final Verification ---\n");
    unsigned long final_hash = 0;
    for (int i = 0; i < sizeof(buffer1); i++) {
        final_hash = final_hash * 31 + (unsigned char)buffer1[i];
    }
    printf("Final buffer hash: %lu\n", final_hash);
    
    /* Additional builtin calls in different contexts */
    {
        char small_buf[32];
        __builtin_memset(small_buf, 0xFF, sizeof(small_buf));
        __builtin_memcpy(small_buf, "END", 3);
    }
    
    printf("=== Test Complete ===\n");
    return 0;
}
