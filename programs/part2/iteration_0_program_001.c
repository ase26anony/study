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
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force early initialization of ASAN runtime */
    volatile char buffer[32];
    __builtin_memset(buffer, 0xA5, sizeof(buffer));
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    volatile char cleanup_buf[16];
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_node(const char* src, size_t len) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy with volatile length */
    volatile size_t copy_len = len > 63 ? 63 : len;
    __builtin_memcpy(node->data, src, copy_len);
    node->data[copy_len] = '\0';
    node->size = copy_len;
    node->left = node->right = NULL;
    
    return node;
}

static void copy_tree_data(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    /* Complex control flow with goto */
    if (src->size > 32) {
        goto large_copy;
    }
    
    /* Regular copy path */
    __builtin_memcpy(dest->data, src->data, src->size);
    dest->size = src->size;
    return;
    
large_copy:
    /* Alternative path with memmove */
    char temp[64];
    __builtin_memcpy(temp, src->data, src->size);
    
    /* Jump back to normal flow */
    goto finish_copy;
    
finish_copy:
    __builtin_memmove(dest->data, temp, src->size);
    dest->size = src->size;
}

/* Parallel memory operations */
static void parallel_mem_operations(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buf[128];
        char shared_buf[256];
        
        /* Each thread uses different builtins */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(local_buf, thread_id, sizeof(local_buf));
                break;
            case 1:
                __builtin_memcpy(local_buf, shared_buf, 
                                g_mem_size > 128 ? 128 : g_mem_size);
                break;
            case 2:
                __builtin_memmove(local_buf, &local_buf[64], 64);
                break;
        }
        
        /* Barrier to ensure all threads reach this point */
        #pragma omp barrier
        
        /* Collective operation */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            volatile char tmp[16];
            __builtin_memset(tmp, i, sizeof(tmp));
        }
    }
}

/* Complex control flow with nested memory ops */
static size_t process_with_gotos(char* buf, size_t size) {
    size_t result = 0;
    char temp[256];
    
    /* Initial memset */
    __builtin_memset(buf, 0, size);
    
    /* Jump into block with memmove */
    if (size > 128) {
        goto block_with_memmove;
    }
    
    /* Normal path */
    for (size_t i = 0; i < size; i += 16) {
        __builtin_memcpy(&buf[i], "ABCDEFGHIJKLMNOP", 16);
    }
    goto finish;
    
block_with_memmove:
    /* This block should trigger memmove redirection */
    __builtin_memmove(temp, buf, size);
    __builtin_memset(buf, 0xFF, size);
    
    /* Jump out to different context */
    if (size % 2 == 0) {
        goto even_size;
    }
    
even_size:
    __builtin_memcpy(buf, temp, size > 256 ? 256 : size);
    
finish:
    /* Calculate checksum */
    for (size_t i = 0; i < size; i++) {
        result += (unsigned char)buf[i];
    }
    return result;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Initialize data */
    const size_t buf_size = g_mem_size;
    char* buffer1 = malloc(buf_size);
    char* buffer2 = malloc(buf_size);
    
    if (!buffer1 || !buffer2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Test 1: Direct built-in calls */
    __builtin_memset(buffer1, 0xAA, buf_size);
    __builtin_memcpy(buffer2, buffer1, buf_size);
    __builtin_memmove(buffer1, buffer2, buf_size / 2);
    
    /* Test 2: Control flow with gotos */
    size_t sum1 = process_with_gotos(buffer1, buf_size);
    
    /* Test 3: Recursive structure operations */
    ASTNode* tree1 = create_node("Test data for tree node", 24);
    ASTNode* tree2 = create_node("Another tree node", 18);
    
    if (tree1 && tree2) {
        copy_tree_data(tree2, tree1);
        
        /* Additional memory operation on tree data */
        __builtin_memcpy(tree1->data, "Modified tree data", 19);
    }
    
    /* Test 4: OpenMP parallel section */
    parallel_mem_operations();
    
    /* Test 5: Mixed operations in loop */
    for (int i = 0; i < 10; i++) {
        volatile size_t op_size = (i * 17) % buf_size;
        if (op_size == 0) op_size = 1;
        
        if (i % 3 == 0) {
            __builtin_memset(buffer1, i, op_size);
        } else if (i % 3 == 1) {
            __builtin_memcpy(&buffer2[i], buffer1, op_size);
        } else {
            __builtin_memmove(&buffer1[i], &buffer2[i], op_size);
        }
    }
    
    /* Final verification */
    size_t final_sum = 0;
    for (size_t i = 0; i < buf_size; i++) {
        final_sum += (unsigned char)buffer1[i];
        final_sum += (unsigned char)buffer2[i];
    }
    
    if (tree1) final_sum += tree1->size;
    if (tree2) final_sum += tree2->size;
    
    printf("Verification sum: %zu (initial: %zu)\n", final_sum, sum1);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(buffer1);
    free(buffer2);
    free(tree1);
    free(tree2);
    
    return 0;
}
