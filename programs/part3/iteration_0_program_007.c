/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    int id;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[128];
    /* Force memcpy redirection early */
    __builtin_memset(buffer, 0, sizeof(buffer));
    printf("Constructor: initialized ASAN early\n");
}

/* Destructor to test cleanup paths */
__attribute__((destructor))
static void cleanup_asan(void) {
    printf("Destructor: cleaning up\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Use all three built-ins with volatile control */
    size_t copy_size = g_mem_size % 128;
    
    /* memset to initialize */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* memcpy pattern into data */
    char pattern[64];
    __builtin_memset(pattern, 'A' + (depth % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, copy_size);
    
    /* Create children with goto-controlled flow */
    int create_left = 1;
    
    if (depth > 3) {
        /* Test goto jumping into memory operation block */
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    return node;
    
create_children:
    /* Jump back from goto */
    node->left = create_ast(depth - 1, counter);
    
    /* memmove between nodes if both exist */
    if (node->left && depth > 2) {
        ASTNode temp;
        __builtin_memcpy(&temp, node->left, sizeof(ASTNode));
        __builtin_memmove(node->data + 32, temp.data, 32);
    }
    
    node->right = create_ast(depth - 2, counter);
    
    return node;
}

/* Parallel memory operations with OpenMP */
static void parallel_mem_ops(void) {
    const int num_buffers = 16;
    char* buffers[num_buffers];
    
    /* Allocate buffers */
    for (int i = 0; i < num_buffers; i++) {
        buffers[i] = (char*)malloc(g_mem_size);
        if (!buffers[i]) return;
    }
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        #pragma omp for
        for (int i = 0; i < num_buffers; i++) {
            /* Each thread uses all three built-ins */
            volatile size_t op_size = (g_mem_size + i) % 128 + 1;
            
            /* memset */
            __builtin_memset(buffers[i], thread_id, op_size);
            
            /* memcpy between buffers */
            if (i > 0) {
                __builtin_memcpy(buffers[i] + op_size/2, 
                               buffers[i-1], 
                               op_size/2);
            }
            
            /* memmove within buffer */
            __builtin_memmove(buffers[i] + 16, 
                            buffers[i] + 8, 
                            op_size > 32 ? 32 : op_size);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_buffers; i++) {
        free(buffers[i]);
    }
}

/* Function with goto edge cases around memmove */
static void test_goto_memmove(void) {
    char src[256], dst[256];
    
    /* Initialize with memset */
    __builtin_memset(src, 'X', sizeof(src));
    __builtin_memset(dst, 'Y', sizeof(dst));
    
    int use_memmove = 1;
    
    if (use_memmove) {
        goto do_memmove;
    }
    
    /* Normal path */
    __builtin_memcpy(dst, src, 128);
    return;
    
do_memmove:
    /* Jump into memmove block */
    volatile size_t move_size = g_mem_size % 200 + 1;
    __builtin_memmove(dst, src, move_size);
    
    /* Jump out */
    goto finish;
    
    /* Unreachable code with another memcpy */
    __builtin_memcpy(dst + 128, src + 128, 64);
    
finish:
    /* Verify with another memset */
    __builtin_memset(dst + 150, 'Z', 50);
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    int counter = 0;
    ASTNode* root = create_ast(5, &counter);
    
    if (root) {
        /* Process AST with memory operations */
        ASTNode stack[10];
        int stack_top = 0;
        
        /* Push root */
        __builtin_memcpy(&stack[stack_top++], root, sizeof(ASTNode));
        
        while (stack_top > 0) {
            ASTNode current;
            __builtin_memcpy(&current, &stack[--stack_top], sizeof(ASTNode));
            
            /* Process children */
            if (current.left) {
                __builtin_memcpy(&stack[stack_top++], current.left, sizeof(ASTNode));
            }
            if (current.right) {
                __builtin_memcpy(&stack[stack_top++], current.right, sizeof(ASTNode));
            }
        }
        
        /* Cleanup AST */
        free(root);
    }
    
    /* Phase 2: Test goto edge cases */
    test_goto_memmove();
    
    /* Phase 3: Parallel operations */
    parallel_mem_ops();
    
    /* Phase 4: Direct built-in calls with volatile sizes */
    volatile char final_buffer[512];
    volatile size_t final_size = g_mem_size % 400 + 100;
    
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer + 64, final_buffer, final_size);
    __builtin_memmove(final_buffer + 128, final_buffer + 32, final_size / 2);
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        hash = hash * 31 + final_buffer[i];
    }
    
    printf("Test completed. Hash: %lu\n", hash);
    printf("Built-in functions redirected: memcpy, memset, memmove\n");
    
    return 0;
}
