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
    char *data;
    size_t len;
    struct ASTNode *left;
    struct ASTNode *right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    printf("Constructor: Initializing ASAN/HWASAN test environment\n");
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    printf("Destructor: Cleaning up test environment\n");
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->len = g_mem_size + id;
    node->data = (char*)malloc(node->len);
    node->id = id;
    
    /* Use __builtin_memset to initialize node data */
    if (node->data) {
        __builtin_memset(node->data, id % 256, node->len);
    }
    
    /* Recursive creation with goto for control flow testing */
    int create_left = 1;
    if (id % 3 == 0) goto skip_left;
    
    node->left = create_node(depth - 1, id * 2);
    create_left = 0;
    
skip_left:
    if (create_left) {
        node->left = NULL;
    }
    
    /* Another goto for right child */
    if (id % 4 == 0) {
        goto create_right;
    } else {
        node->right = NULL;
        goto done;
    }
    
create_right:
    node->right = create_node(depth - 1, id * 2 + 1);
    
done:
    return node;
}

/* Copy data between nodes using __builtin_memcpy */
static void copy_node_data(ASTNode *dest, const ASTNode *src) {
    if (!dest || !src || !dest->data || !src->data) return;
    
    size_t copy_len = dest->len < src->len ? dest->len : src->len;
    
    /* Force __builtin_memcpy call */
    __builtin_memcpy(dest->data, src->data, copy_len);
    
    /* Also test __builtin_memmove with overlapping regions */
    if (dest->len >= 64) {
        __builtin_memmove(dest->data + 32, dest->data, 32);
    }
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_ops(ASTNode **nodes, int count) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Use volatile to prevent optimization */
                volatile char *ptr = nodes[i]->data;
                volatile size_t len = nodes[i]->len;
                
                /* Test all three builtins in parallel context */
                if (len > 0) {
                    /* Create temporary buffer */
                    char *temp = (char*)malloc(len);
                    if (temp) {
                        /* __builtin_memcpy */
                        __builtin_memcpy(temp, (void*)ptr, len);
                        
                        /* __builtin_memset on temp */
                        __builtin_memset(temp + len/2, thread_id, len/2);
                        
                        /* __builtin_memmove back */
                        __builtin_memmove((void*)ptr, temp, len);
                        
                        free(temp);
                    }
                }
            }
        }
    }
}

/* Complex control flow with goto around memory operations */
static void test_goto_memmove(ASTNode *node) {
    char buffer[512];
    volatile int use_memmove = 1;
    
    if (!node || !node->data) return;
    
    /* Initialize buffer */
    __builtin_memset(buffer, 0, sizeof(buffer));
    
    /* Jump into memory operation block */
    if (node->id % 2 == 0) {
        goto do_memmove;
    }
    
    /* Normal path */
    __builtin_memcpy(buffer, node->data, node->len < 512 ? node->len : 512);
    goto done;
    
do_memmove:
    /* This path tests __builtin_memmove with goto */
    if (use_memmove) {
        __builtin_memmove(buffer, node->data, node->len < 512 ? node->len : 512);
        
        /* Jump out to another operation */
        if (node->len > 256) {
            goto extra_ops;
        }
    }
    
    goto done;
    
extra_ops:
    /* Additional memory operations after goto */
    __builtin_memset(buffer + 256, 0xFF, 128);
    /* fall through */
    
done:
    /* Verify operation */
    int sum = 0;
    for (size_t i = 0; i < (node->len < 512 ? node->len : 512); i++) {
        sum += buffer[i];
    }
    printf("Node %d buffer sum: %d\n", node->id, sum);
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create tree of nodes */
    ASTNode *root = create_node(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create root node\n");
        return 1;
    }
    
    /* Build array of nodes for parallel processing */
    ASTNode *nodes[8];
    int node_count = 0;
    
    /* Collect nodes recursively */
    ASTNode *stack[32];
    int stack_top = 0;
    stack[stack_top++] = root;
    
    while (stack_top > 0 && node_count < 8) {
        ASTNode *current = stack[--stack_top];
        nodes[node_count++] = current;
        
        if (current->right) stack[stack_top++] = current->right;
        if (current->left) stack[stack_top++] = current->left;
    }
    
    /* Phase 1: Test goto with memmove */
    printf("\nPhase 1: Testing goto with memmove\n");
    for (int i = 0; i < node_count; i++) {
        test_goto_memmove(nodes[i]);
    }
    
    /* Phase 2: Copy data between nodes */
    printf("\nPhase 2: Copying data between nodes\n");
    for (int i = 0; i < node_count - 1; i++) {
        copy_node_data(nodes[i], nodes[i + 1]);
    }
    
    /* Phase 3: Parallel memory operations */
    printf("\nPhase 3: Parallel memory operations\n");
    parallel_memory_ops(nodes, node_count);
    
    /* Phase 4: Direct built-in calls with volatile */
    printf("\nPhase 4: Direct built-in calls\n");
    volatile char direct_buf[1024];
    volatile size_t op_size = g_mem_size;
    
    /* All three builtins in sequence */
    __builtin_memset((void*)direct_buf, 0xAA, op_size);
    __builtin_memcpy((void*)(direct_buf + 256), (void*)direct_buf, op_size / 2);
    __builtin_memmove((void*)(direct_buf + 128), (void*)direct_buf, op_size / 4);
    
    /* Calculate verification hash */
    unsigned long long hash = 0;
    for (int i = 0; i < node_count; i++) {
        if (nodes[i] && nodes[i]->data) {
            for (size_t j = 0; j < nodes[i]->len && j < 64; j++) {
                hash = (hash * 31 + nodes[i]->data[j]) % 1000000007;
            }
        }
    }
    
    printf("\nFinal verification hash: %llu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real ASAN/HWASAN tests, memory leaks would be detected */
    
    return 0;
}
