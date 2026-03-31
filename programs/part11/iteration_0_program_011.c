/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
struct ast_node {
    char data[256];
    struct ast_node *left;
    struct ast_node *right;
    int id;
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 128;  /* Force non-constant size */
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Function with goto statements for flow control */
static void test_goto_memmove(void) {
    char buffer1[256];
    char buffer2[256];
    int use_memmove = 1;
    
    /* Initialize buffers with pattern */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memset(buffer2, 0xBB, sizeof(buffer2));
    
    goto jump_point;
    
no_move:
    printf("Skipping memmove\n");
    return;
    
jump_point:
    if (use_memmove) {
        /* This should trigger ASAN memmove redirection */
        __builtin_memmove(buffer1, buffer2, g_mem_size);
    } else {
        goto no_move;
    }
    
    /* Jump back */
    goto after_move;
    
after_move:
    /* Verify the move happened */
    if (buffer1[0] == 0xBB) {
        printf("Memmove successful via goto\n");
    }
}

/* Recursive function with memory operations */
static struct ast_node* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Initialize node data with memset */
    __builtin_memset(node->data, id, sizeof(node->data));
    
    node->id = id;
    
    /* Create left child with different pattern */
    node->left = create_ast(depth - 1, id * 2);
    if (node->left) {
        /* Copy data between nodes using memcpy */
        __builtin_memcpy(node->data + 128, 
                        node->left->data, 
                        g_mem_size / 2);
    }
    
    /* Create right child */
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    return node;
}

/* Function to traverse and process AST */
static int process_ast(struct ast_node* node, int* sum) {
    if (!node) return 0;
    
    int local_sum = 0;
    
    /* Process current node data with memset */
    char temp[256];
    __builtin_memcpy(temp, node->data, sizeof(temp));
    
    /* Modify with memset */
    __builtin_memset(node->data + 64, node->id, 32);
    
    /* Calculate checksum */
    for (int i = 0; i < 64; i++) {
        local_sum += node->data[i];
    }
    
    /* Recursive processing */
    local_sum += process_ast(node->left, sum);
    local_sum += process_ast(node->right, sum);
    
    *sum += local_sum;
    return local_sum;
}

/* OpenMP parallel section with memory operations */
static void parallel_mem_operations(void) {
    const int num_threads = 4;
    char* buffers[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread allocates and operates on its own buffer */
        buffers[tid] = malloc(1024);
        if (buffers[tid]) {
            /* Use all three builtins in parallel */
            __builtin_memset(buffers[tid], tid, 1024);
            
            if (tid > 0) {
                /* Copy from previous thread's buffer */
                __builtin_memcpy(buffers[tid], 
                               buffers[tid-1], 
                               512);
            }
            
            /* Move data within buffer */
            __builtin_memmove(buffers[tid] + 256,
                            buffers[tid],
                            256);
                            
            /* Verify operation */
            int check = 0;
            for (int i = 0; i < 256; i++) {
                check += buffers[tid][i];
            }
            
            #pragma omp critical
            {
                printf("Thread %d: checksum = %d\n", tid, check);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_threads; i++) {
        if (buffers[i]) free(buffers[i]);
    }
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Test 1: Basic built-in calls */
    printf("\n1. Testing basic memory builtins:\n");
    char src[256], dst[256];
    
    __builtin_memset(src, 0xCC, sizeof(src));
    __builtin_memcpy(dst, src, g_mem_size);
    __builtin_memmove(dst + 128, dst, 64);
    
    /* Test 2: Goto flow control */
    printf("\n2. Testing goto with memmove:\n");
    test_goto_memmove();
    
    /* Test 3: Recursive AST operations */
    printf("\n3. Testing recursive AST with memory ops:\n");
    struct ast_node* root = create_ast(3, 1);
    int ast_sum = 0;
    process_ast(root, &ast_sum);
    printf("AST checksum: %d\n", ast_sum);
    
    /* Test 4: OpenMP parallel operations */
    printf("\n4. Testing OpenMP parallel memory ops:\n");
    parallel_mem_operations();
    
    /* Test 5: Mixed operations in loops */
    printf("\n5. Testing mixed operations in loops:\n");
    char circular_buffer[1024];
    volatile int offset = 0;
    
    for (int i = 0; i < 10; i++) {
        offset = (offset + 64) % 512;
        
        __builtin_memset(circular_buffer + offset, i, 128);
        
        if (i % 2 == 0) {
            __builtin_memcpy(circular_buffer + 256,
                           circular_buffer + offset,
                           64);
        } else {
            __builtin_memmove(circular_buffer + 384,
                            circular_buffer + offset,
                            64);
        }
    }
    
    /* Final verification */
    int final_check = 0;
    for (int i = 0; i < 512; i++) {
        final_check += circular_buffer[i];
    }
    printf("Final buffer checksum: %d\n", final_check);
    
    /* Cleanup */
    /* Note: In real code, you'd need to free the AST properly */
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
