/* ISO C99-compliant test program for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    struct ASTNode* parent;
} ASTNode;

/* Global token array */
static char g_token_buffer[4096];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token buffer with pattern */
    for (size_t i = 0; i < sizeof(g_token_buffer); i++) {
        g_token_buffer[i] = (char)((i * 13) & 0xFF);
    }
    printf("Constructor: Initialized token buffer\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[64];
    for (int i = 0; i < 64; i++) {
        pattern[i] = (char)((depth + i) & 0xFF);
    }
    __builtin_memcpy(node->data, pattern, 64);
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_left = (depth % 2 == 0);
        
        if (use_left) {
            goto create_left;
        } else {
            node->right = create_ast_node(depth - 1);
            goto skip_left;
        }
        
    create_left:
        node->left = create_ast_node(depth - 1);
        goto after_both;
        
    skip_left:
        /* Use __builtin_memmove to shift data */
        if (node->right) {
            __builtin_memmove(node->data + 32, node->data, 32);
        }
        
    after_both:
        if (node->left && node->right) {
            /* Copy between nodes using __builtin_memcpy */
            __builtin_memcpy(node->left->data + 16, 
                           node->right->data, 16);
        }
    }
    
    return node;
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    volatile size_t local_size = g_mem_size;
    char* src = (char*)malloc(local_size);
    char* dst = (char*)malloc(local_size);
    
    if (!src || !dst) {
        free(src);
        free(dst);
        return;
    }
    
    /* Initialize source with pattern */
    for (size_t i = 0; i < local_size; i++) {
        src[i] = (char)(i & 0xFF);
    }
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                __builtin_memcpy(dst + thread_id * 32, 
                               src + thread_id * 32, 32);
                break;
            case 1:
                __builtin_memset(dst + thread_id * 64, 
                               thread_id, 64);
                break;
            case 2:
                __builtin_memmove(dst + thread_id * 48,
                                src + thread_id * 48, 48);
                break;
        }
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Verify operations */
        #pragma omp single
        {
            long long checksum = 0;
            for (size_t i = 0; i < local_size; i++) {
                checksum += dst[i];
            }
            printf("Parallel checksum: %lld\n", checksum);
        }
    }
    
    free(src);
    free(dst);
}

/* Complex memory dispatch with goto */
static void complex_dispatch(void) {
    char buffer1[512];
    char buffer2[512];
    volatile int stage = 0;
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memset(buffer2, 0x55, sizeof(buffer2));
    
    /* Goto-based state machine */
    stage = 1;
    if (g_use_hwasan) {
        goto hwasan_path;
    }
    
normal_path:
    /* Standard memory operations */
    __builtin_memcpy(buffer1 + 128, buffer2 + 64, 128);
    stage = 2;
    goto next_stage;
    
hwasan_path:
    /* Different pattern for HWASAN testing */
    __builtin_memmove(buffer1, buffer2, 256);
    stage = 3;
    
next_stage:
    /* More operations with volatile control */
    volatile size_t copy_size = 96;
    __builtin_memcpy(buffer2, buffer1, copy_size);
    
    /* Jump back for additional coverage */
    if (stage == 2) {
        goto final_stage;
    }
    
    __builtin_memset(buffer1 + 256, 0xFF, 128);
    
final_stage:
    /* Final verification */
    int sum = 0;
    for (int i = 0; i < 512; i++) {
        sum += buffer1[i] + buffer2[i];
    }
    printf("Dispatch sum: %d (stage: %d)\n", sum, stage);
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Recursive AST operations */
    printf("\nPhase 1: Creating AST structure\n");
    ASTNode* root = create_ast_node(4);
    
    if (root) {
        /* Traverse and verify */
        int node_count = 0;
        ASTNode* stack[16];
        int top = 0;
        stack[top++] = root;
        
        while (top > 0) {
            ASTNode* current = stack[--top];
            node_count++;
            
            /* Verify node data */
            int data_sum = 0;
            for (int i = 0; i < 64; i++) {
                data_sum += current->data[i];
            }
            
            if (current->right) stack[top++] = current->right;
            if (current->left) stack[top++] = current->left;
        }
        printf("AST nodes: %d\n", node_count);
        
        /* Cleanup */
        free(root);
    }
    
    /* Phase 2: Parallel operations */
    printf("\nPhase 2: Parallel memory operations\n");
    parallel_memory_ops();
    
    /* Phase 3: Complex dispatch */
    printf("\nPhase 3: Complex dispatch with goto\n");
    complex_dispatch();
    
    /* Phase 4: Direct built-in calls with volatile */
    printf("\nPhase 4: Direct built-in calls\n");
    {
        volatile char src[100];
        volatile char dst[100];
        
        for (int i = 0; i < 100; i++) {
            src[i] = (char)(i * 3);
        }
        
        /* Force all three built-ins */
        __builtin_memcpy((void*)dst, (void*)src, 100);
        __builtin_memset((void*)(dst + 50), 0x7F, 25);
        __builtin_memmove((void*)(dst + 25), (void*)(dst + 75), 25);
        
        /* Verify */
        int final_sum = 0;
        for (int i = 0; i < 100; i++) {
            final_sum += dst[i];
        }
        printf("Final verification sum: %d\n", final_sum);
    }
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
