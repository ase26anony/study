/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char *data;
    size_t len;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_global() {
    g_init_flag = 1;
    printf("Constructor: Global initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_global() {
    printf("Destructor: Cleaning up\n");
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_node(const char *src, size_t len) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->data = malloc(len + 1);
    if (!node->data) {
        free(node);
        return NULL;
    }
    
    /* Force __builtin_memcpy usage */
    __builtin_memcpy(node->data, src, len);
    node->data[len] = '\0';
    node->len = len;
    node->left = node->right = NULL;
    
    return node;
}

/* Complex function with goto and memory operations */
static void process_with_goto(ASTNode *dest, ASTNode *src) {
    volatile int use_memmove = 1;
    
    if (dest->len != src->len) {
        goto resize_operation;
    }
    
copy_operation:
    /* This should trigger memcpy redirection */
    __builtin_memcpy(dest->data, src->data, dest->len);
    goto finish;
    
resize_operation:
    if (use_memmove) {
        /* Force __builtin_memmove with overlapping regions */
        size_t min_len = dest->len < src->len ? dest->len : src->len;
        char *temp = malloc(min_len);
        __builtin_memcpy(temp, src->data, min_len);
        __builtin_memmove(dest->data, temp, min_len);
        free(temp);
    }
    goto finish;
    
finish:
    /* Initialize with memset */
    __builtin_memset(src->data + src->len/2, 0, src->len/4);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode **nodes, size_t count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (size_t i = 0; i < count; i++) {
            volatile size_t local_size = g_mem_size + tid;
            
            /* Mixed built-in usage in parallel region */
            if (i % 3 == 0) {
                __builtin_memset(nodes[i]->data, tid, local_size % nodes[i]->len);
            } else if (i % 3 == 1) {
                size_t target = (i + 1) % count;
                __builtin_memcpy(nodes[i]->data, nodes[target]->data, 
                               nodes[i]->len < nodes[target]->len ? 
                               nodes[i]->len : nodes[target]->len);
            } else {
                /* Create overlapping regions for memmove */
                size_t half = nodes[i]->len / 2;
                __builtin_memmove(nodes[i]->data + half, nodes[i]->data, half);
            }
        }
    }
}

/* Multi-stage processing with different memory patterns */
static unsigned long compute_hash(ASTNode *node) {
    unsigned long hash = 5381;
    
    for (size_t i = 0; i < node->len; i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    /* Additional memory operation in hash computation */
    volatile char temp[32];
    __builtin_memset(temp, hash & 0xFF, sizeof(temp));
    __builtin_memcpy(&temp[16], node->data, node->len > 16 ? 16 : node->len);
    
    return hash;
}

int main(void) {
    const char *test_data = "ASAN_TEST_STRING_FOR_MEMORY_OPERATIONS_1234567890";
    size_t data_len = strlen(test_data);
    
    /* Create tree structure */
    ASTNode *root = create_node(test_data, data_len);
    ASTNode *left = create_node(test_data + 10, data_len - 10);
    ASTNode *right = create_node(test_data + 5, data_len - 5);
    
    if (!root || !left || !right) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    root->left = left;
    root->right = right;
    
    /* Array for parallel processing */
    ASTNode *node_array[] = {root, left, right};
    size_t array_size = sizeof(node_array) / sizeof(node_array[0]);
    
    printf("Starting parallel memory operations...\n");
    
    /* Stage 1: Process with goto jumps */
    process_with_goto(left, right);
    
    /* Stage 2: OpenMP parallel operations */
    parallel_memory_ops(node_array, array_size);
    
    /* Stage 3: Additional built-in calls outside parallel region */
    volatile size_t dynamic_size = g_mem_size;
    char *extra_buffer = malloc(dynamic_size);
    if (extra_buffer) {
        __builtin_memset(extra_buffer, 0xAA, dynamic_size);
        __builtin_memcpy(extra_buffer + 8, root->data, 
                        root->len < dynamic_size - 8 ? root->len : dynamic_size - 8);
        __builtin_memmove(extra_buffer, extra_buffer + 4, dynamic_size - 4);
        free(extra_buffer);
    }
    
    /* Compute and verify results */
    unsigned long total_hash = 0;
    for (size_t i = 0; i < array_size; i++) {
        total_hash ^= compute_hash(node_array[i]);
    }
    
    printf("Result hash: 0x%lx\n", total_hash);
    printf("Verification: %s\n", total_hash != 0 ? "PASS" : "FAIL");
    
    /* Cleanup */
    for (size_t i = 0; i < array_size; i++) {
        free(node_array[i]->data);
        free(node_array[i]);
    }
    
    return total_hash != 0 ? 0 : 1;
}
