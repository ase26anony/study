/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
static char g_tokens[][32] = {
    "token1", "token2", "token3", "token4",
    "token5", "token6", "token7", "token8"
};

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_environment(void) {
    volatile char buffer[128];
    
    /* Force memcpy built-in initialization */
    __builtin_memcpy(buffer, "constructor_init", 16);
    
    printf("Constructor: ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[64];
    
    /* Force memset built-in initialization */
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
    
    printf("Destructor: ASAN environment cleaned up\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->id = (*counter)++;
    
    /* Copy token data with memcpy */
    size_t token_idx = node->id % 8;
    __builtin_memcpy(node->data, g_tokens[token_idx], 
                     sizeof(g_tokens[0]));
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 2 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, counter);
        node->right = NULL;
        return node;
        
    create_children:
        node->left = create_ast(depth - 1, counter);
        node->right = create_ast(depth - 1, counter);
        
        /* Copy between nodes using memmove with goto */
        if (g_use_memmove) {
            volatile char temp[64];
            goto do_memmove;
        }
    }
    
    return node;
    
do_memmove:
    /* This tests the flow-sensitivity of asan_memfn_rtls retrieval */
    __builtin_memmove(temp, node->data, sizeof(node->data));
    __builtin_memmove(node->data, temp, sizeof(node->data));
    return node;
}

/* Parallel memory operations */
static void parallel_memory_operations(void) {
    volatile size_t local_size = g_mem_size;
    char* buffers[4];
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread allocates and operates on memory */
        buffers[thread_id] = (char*)malloc(local_size);
        if (!buffers[thread_id]) return;
        
        /* Force all three built-ins in parallel context */
        #pragma omp sections
        {
            #pragma omp section
            {
                /* memcpy section */
                char pattern[] = "memcpy_pattern";
                __builtin_memcpy(buffers[thread_id], pattern, 
                                sizeof(pattern));
            }
            
            #pragma omp section
            {
                /* memset section */
                __builtin_memset(buffers[thread_id] + 64, 0xAA, 32);
            }
            
            #pragma omp section
            {
                /* memmove section with volatile control */
                volatile int do_memmove = 1;
                if (do_memmove) {
                    __builtin_memmove(buffers[thread_id] + 96,
                                     buffers[thread_id] + 64, 32);
                }
            }
        }
        
        /* Additional memcpy with non-constant size */
        volatile size_t copy_size = local_size / 2;
        char* temp = (char*)malloc(copy_size);
        if (temp) {
            __builtin_memcpy(temp, buffers[thread_id], copy_size);
            free(temp);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        if (buffers[i]) free(buffers[i]);
    }
}

/* Complex memory operation with goto jumping */
static void complex_memory_pattern(void) {
    volatile char buffer1[256];
    volatile char buffer2[256];
    volatile int stage = 0;
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 'A', sizeof(buffer1));
    __builtin_memset(buffer2, 'B', sizeof(buffer2));
    
    /* Goto-based state machine with memory operations */
    stage = 1;
    if (stage == 1) goto stage1;
    
stage2:
    /* memmove with goto jumping into block */
    __builtin_memmove((void*)buffer1, (void*)buffer2, 128);
    goto finish;
    
stage1:
    /* memcpy in first stage */
    __builtin_memcpy((void*)buffer2, (void*)buffer1, 64);
    stage = 2;
    goto stage2;
    
finish:
    /* Final memset */
    __builtin_memset(buffer1 + 192, 0, 64);
}

/* Main execution flow */
int main(void) {
    int counter = 0;
    unsigned long hash = 0;
    
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create recursive AST structure */
    ASTNode* root = create_ast(4, &counter);
    
    /* Perform parallel memory operations */
    parallel_memory_operations();
    
    /* Execute complex memory pattern with goto */
    complex_memory_pattern();
    
    /* Calculate verification hash from AST */
    if (root) {
        ASTNode* stack[32];
        int top = 0;
        stack[top++] = root;
        
        while (top > 0) {
            ASTNode* node = stack[--top];
            
            /* Add node data to hash */
            for (size_t i = 0; i < sizeof(node->data); i++) {
                hash = (hash * 31) + node->data[i];
            }
            hash += node->id;
            
            if (node->right) stack[top++] = node->right;
            if (node->left) stack[top++] = node->left;
            
            /* Copy node data before freeing */
            char temp[64];
            __builtin_memcpy(temp, node->data, sizeof(node->data));
            
            free(node);
        }
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Built-in redirection test completed\n");
    
    return (int)(hash % 256);
}
