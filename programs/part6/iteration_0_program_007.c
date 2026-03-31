/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {"memcpy", "memset", "memmove", "test", "data"};
static const int token_count = 5;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_constructor(void) {
    g_init_flag = 1;
    printf("Constructor: Initializing ASAN environment\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_destructor(void) {
    printf("Destructor: Cleaning up\n");
    g_init_flag = 0;
}

/* Recursive parser with memory operations */
static ASTNode* create_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[64];
    __builtin_memset(pattern, 'A' + (id % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(node->data));
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_node(depth - 1, id * 2);
        node->right = create_node(depth - 1, id * 2 + 1);
        
        if (use_goto) {
            create_children:
            /* Jump into block with __builtin_memmove */
            ASTNode temp;
            if (node->left) {
                __builtin_memmove(&temp, node->left, sizeof(ASTNode));
                __builtin_memcpy(node->data + 32, temp.data, 32);
            }
            node->right = create_node(depth - 1, id * 2 + 1);
        }
    }
    
    return node;
}

/* Parallel memory operations with OpenMP */
static void parallel_mem_operations(char* buffer1, char* buffer2, size_t size) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        size_t chunk = size / omp_get_num_threads();
        size_t start = tid * chunk;
        
        /* Each thread performs different memory operations */
        switch (tid % 3) {
            case 0:
                __builtin_memset(buffer1 + start, tid, chunk);
                break;
            case 1:
                __builtin_memcpy(buffer2 + start, buffer1 + start, chunk);
                break;
            case 2:
                __builtin_memmove(buffer1 + start, buffer2 + start, chunk);
                break;
        }
        
        /* Barrier to synchronize */
        #pragma omp barrier
        
        /* Cross-thread memory operation */
        if (tid == 0) {
            for (int i = 1; i < omp_get_num_threads(); i++) {
                size_t src_start = i * chunk;
                __builtin_memcpy(buffer1 + src_start, buffer2, chunk);
            }
        }
    }
}

/* Complex memory dispatch with goto edge cases */
static void memory_dispatch_logic(void) {
    char* buffers[4];
    size_t sizes[4];
    
    /* Allocate with volatile sizes */
    for (int i = 0; i < 4; i++) {
        sizes[i] = g_mem_size * (i + 1);
        buffers[i] = (char*)malloc(sizes[i]);
        if (!buffers[i]) abort();
    }
    
    /* Label for goto jumps */
    dispatch_start:
    
    /* Pattern 1: Sequential built-in calls */
    __builtin_memset(buffers[0], 0xAA, sizes[0]);
    __builtin_memcpy(buffers[1], buffers[0], sizes[0] < sizes[1] ? sizes[0] : sizes[1]);
    __builtin_memmove(buffers[2], buffers[1], sizes[1] < sizes[2] ? sizes[1] : sizes[2]);
    
    /* Pattern 2: Nested calls with goto */
    int mode = g_init_flag % 3;
    
    if (mode == 0) {
        goto use_memcpy;
    } else if (mode == 1) {
        goto use_memset;
    } else {
        goto use_memmove;
    }
    
    use_memcpy:
    __builtin_memcpy(buffers[3], buffers[0], sizes[0]);
    goto dispatch_end;
    
    use_memset:
    __builtin_memset(buffers[3], 0xBB, sizes[3]);
    goto dispatch_end;
    
    use_memmove:
    __builtin_memmove(buffers[3], buffers[2], sizes[2]);
    /* Fall through */
    
    dispatch_end:
    
    /* Pattern 3: Overlapping regions */
    size_t overlap_size = sizes[0] / 2;
    __builtin_memmove(buffers[0] + overlap_size, buffers[0], overlap_size);
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        free(buffers[i]);
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Initialize token processing */
    int token_hash = 0;
    for (int i = 0; i < token_count; i++) {
        size_t len = strlen(tokens[i]);
        char buffer[64];
        
        /* Force built-in calls with token data */
        __builtin_memset(buffer, 0, sizeof(buffer));
        __builtin_memcpy(buffer, tokens[i], len);
        __builtin_memmove(buffer + 10, buffer, len);
        
        for (size_t j = 0; j < len; j++) {
            token_hash += buffer[j];
        }
    }
    
    /* Create recursive AST */
    ASTNode* root = create_node(4, 1);
    
    /* Perform AST memory operations */
    if (root && root->left && root->right) {
        /* Copy between nodes */
        __builtin_memcpy(root->right->data, root->left->data, sizeof(root->left->data));
        
        /* Move within node with overlap */
        __builtin_memmove(root->data + 16, root->data, 32);
    }
    
    /* Parallel memory operations */
    size_t buffer_size = g_mem_size * 8;
    char* parallel_buf1 = (char*)malloc(buffer_size);
    char* parallel_buf2 = (char*)malloc(buffer_size);
    
    if (parallel_buf1 && parallel_buf2) {
        parallel_mem_operations(parallel_buf1, parallel_buf2, buffer_size);
    }
    
    /* Memory dispatch with goto edge cases */
    memory_dispatch_logic();
    
    /* Calculate verification result */
    int result = token_hash;
    if (root) {
        for (int i = 0; i < 64; i++) {
            result += root->data[i];
        }
    }
    
    if (parallel_buf1) {
        for (size_t i = 0; i < 64 && i < buffer_size; i++) {
            result += parallel_buf1[i];
        }
    }
    
    /* Cleanup */
    free(parallel_buf1);
    free(parallel_buf2);
    
    /* Free AST recursively */
    /* ... (implementation omitted for brevity) */
    
    printf("Result: %d\n", result);
    printf("Test completed successfully\n");
    
    return 0;
}
