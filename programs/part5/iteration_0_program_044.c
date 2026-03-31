#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Complex token structure for AST-like operations */
typedef struct TokenNode {
    char *data;
    size_t length;
    volatile int checksum;
    struct TokenNode *left;
    struct TokenNode *right;
} TokenNode;

/* Global volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN globals\n");
    /* Force compiler to consider memory builtins */
    char *dummy = malloc(64);
    if (dummy) {
        __builtin_memset(dummy, 0xAA, 64);
        __builtin_memcpy(dummy + 32, dummy, 32);
        free(dummy);
    }
}

/* Destructor for cleanup verification */
__attribute__((destructor)) 
static void cleanup_asan(void) {
    printf("Destructor: ASAN cleanup completed\n");
}

/* Recursive AST creation with memory operations */
static TokenNode* create_ast(int depth, const char *base_data) {
    if (depth <= 0) return NULL;
    
    TokenNode *node = malloc(sizeof(TokenNode));
    if (!node) return NULL;
    
    /* Use volatile to prevent constant folding */
    volatile size_t data_len = (size_t)(depth * 16);
    node->length = data_len;
    node->data = malloc(data_len + 1);
    
    if (node->data) {
        /* Force memcpy/memset through builtins */
        __builtin_memset(node->data, 0, data_len + 1);
        
        /* Copy base data with memcpy */
        size_t copy_len = strlen(base_data);
        if (copy_len > data_len) copy_len = data_len;
        __builtin_memcpy(node->data, base_data, copy_len);
        
        /* Calculate checksum with memory access */
        node->checksum = 0;
        for (size_t i = 0; i < copy_len; i++) {
            node->checksum += node->data[i];
        }
    }
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        goto create_children;
        
        create_children:
        node->left = create_ast(depth - 1, base_data);
        
        /* Jump back and forth to test flow sensitivity */
        if (depth % 2 == 0) {
            goto skip_right;
        }
        
        node->right = create_ast(depth - 1, base_data);
        goto children_done;
        
        skip_right:
        node->right = NULL;
        
        children_done:
        /* Use memmove for overlapping regions */
        if (node->data && node->length > 32) {
            __builtin_memmove(node->data + 16, node->data, 16);
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_operations(TokenNode **nodes, int count) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i] && nodes[i]->data) {
                /* Varied memory operations per thread */
                switch (thread_id % 3) {
                    case 0: /* memcpy between nodes */
                        if (i > 0 && nodes[i-1]) {
                            size_t copy_len = nodes[i]->length;
                            if (nodes[i-1]->length < copy_len) 
                                copy_len = nodes[i-1]->length;
                            __builtin_memcpy(nodes[i]->data, 
                                           nodes[i-1]->data, 
                                           copy_len);
                        }
                        break;
                        
                    case 1: /* memset pattern */
                        __builtin_memset(nodes[i]->data, 
                                       thread_id + 'A', 
                                       nodes[i]->length);
                        break;
                        
                    case 2: /* memmove with overlap */
                        if (nodes[i]->length > 64) {
                            __builtin_memmove(nodes[i]->data + 32,
                                            nodes[i]->data,
                                            32);
                        }
                        break;
                }
            }
        }
        
        /* Barrier with additional memory ops */
        #pragma omp barrier
        
        #pragma omp single
        {
            /* Master thread does special memmove */
            if (count > 1 && nodes[0] && nodes[1]) {
                size_t move_len = nodes[0]->length;
                if (nodes[1]->length < move_len) 
                    move_len = nodes[1]->length;
                __builtin_memmove(nodes[1]->data,
                                nodes[0]->data,
                                move_len);
            }
        }
    }
}

/* Complex control flow with goto around memory ops */
static void control_flow_test(char *buffer, size_t size) {
    volatile int condition = size > 128;
    
    if (condition) {
        goto large_buffer;
    }
    
    /* Small buffer path */
    __builtin_memset(buffer, 0xCC, size);
    goto done;
    
    large_buffer:
    /* Large buffer with memcpy */
    char *temp = malloc(size);
    if (temp) {
        __builtin_memset(temp, 0xDD, size);
        __builtin_memcpy(buffer, temp, size);
        
        /* Jump back for memmove */
        if (size > 256) {
            goto do_memmove;
        }
        free(temp);
    }
    goto done;
    
    do_memmove:
    __builtin_memmove(buffer + 128, buffer, 128);
    free(temp);
    
    done:
    return;
}

/* Main execution with diverse memory operations */
int main(void) {
    const int NODES_COUNT = 8;
    const int AST_DEPTH = 4;
    
    printf("Starting ASAN memory operation tests\n");
    
    /* Create AST nodes */
    TokenNode *nodes[NODES_COUNT];
    for (int i = 0; i < NODES_COUNT; i++) {
        char base[32];
        snprintf(base, sizeof(base), "Node%d_Data_%zu", i, g_mem_size);
        nodes[i] = create_ast(AST_DEPTH, base);
    }
    
    /* Test control flow with goto */
    char *test_buffer = malloc(512);
    if (test_buffer) {
        control_flow_test(test_buffer, 512);
        control_flow_test(test_buffer, 64);
        free(test_buffer);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_operations(nodes, NODES_COUNT);
    
    /* Verify results and calculate final hash */
    unsigned long final_hash = 0;
    for (int i = 0; i < NODES_COUNT; i++) {
        if (nodes[i]) {
            final_hash += nodes[i]->checksum;
            
            /* Additional memory ops in verification */
            if (nodes[i]->data && i % 2 == 0) {
                __builtin_memset(nodes[i]->data + nodes[i]->length - 16, 
                               0xFF, 16);
            }
            
            /* Cleanup */
            free(nodes[i]->data);
            free(nodes[i]);
        }
    }
    
    printf("Final hash: %lu\n", final_hash);
    printf("Tests completed successfully\n");
    
    return 0;
}
