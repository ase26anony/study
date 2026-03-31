/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ast_node {
    char data[32];
    struct ast_node* left;
    struct ast_node* right;
    uint32_t checksum;
} ast_node_t;

/* Global token array */
static char g_token_buffer[256];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
static void __attribute__((constructor)) init_asan_test(void) {
    /* Initialize token buffer with pattern */
    for (int i = 0; i < 256; i++) {
        g_token_buffer[i] = (char)((i * 13) & 0xFF);
    }
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
static void __attribute__((destructor)) cleanup_asan_test(void) {
    /* Clear sensitive data */
    __builtin_memset(g_token_buffer, 0, sizeof(g_token_buffer));
}

/* Recursive parser with memory operations */
static ast_node_t* parse_expression(int depth) {
    if (depth <= 0) return NULL;
    
    ast_node_t* node = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Initialize node with builtin memset */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    
    /* Copy data from token buffer using builtin memcpy */
    size_t copy_size = (g_mem_size < 32) ? g_mem_size : 32;
    __builtin_memcpy(node->data, &g_token_buffer[g_token_index], copy_size);
    
    /* Update token index with overflow check */
    g_token_index = (g_token_index + (int)copy_size) % 256;
    
    /* Recursive calls */
    node->left = parse_expression(depth - 1);
    node->right = parse_expression(depth - 1);
    
    /* Calculate checksum */
    uint32_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint32_t)node->data[i];
    }
    node->checksum = sum;
    
    return node;
}

/* Function with goto and memory operations */
static void process_with_goto(ast_node_t* dest, ast_node_t* src) {
    if (!dest || !src) return;
    
    int state = 0;
    
    /* Jump into memory operation block */
    goto start_copy;
    
copy_block:
    /* Use builtin memmove for overlapping regions */
    __builtin_memmove(dest->data + 8, dest->data, 16);
    state = 1;
    goto finish;
    
start_copy:
    /* Copy using builtin memcpy with goto */
    if (state == 0) {
        __builtin_memcpy(dest->data, src->data, 24);
        goto copy_block;
    }
    
finish:
    /* Finalize with memset */
    __builtin_memset(dest->data + 24, 0xAA, 8);
}

/* Parallel memory dispatch logic */
static uint64_t parallel_memory_ops(void) {
    uint64_t total_sum = 0;
    ast_node_t* nodes[4] = {0};
    
    /* Create AST nodes */
    for (int i = 0; i < 4; i++) {
        nodes[i] = parse_expression(3);
    }
    
    /* OpenMP parallel region */
    #pragma omp parallel reduction(+:total_sum)
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        
        /* Each thread processes different memory operations */
        #pragma omp for
        for (int i = 0; i < 4; i++) {
            if (nodes[i]) {
                /* Force builtin usage in parallel context */
                char temp[32];
                
                /* Mixed memory operations */
                __builtin_memcpy(temp, nodes[i]->data, 16);
                __builtin_memset(nodes[i]->data + 16, thread_id, 8);
                __builtin_memmove(nodes[i]->data + 8, temp, 16);
                
                /* Update checksum */
                uint32_t sum = 0;
                for (int j = 0; j < 32; j++) {
                    sum += (uint32_t)nodes[i]->data[j];
                }
                nodes[i]->checksum = sum;
                
                total_sum += sum;
            }
        }
        
        /* Additional memory operations outside loop */
        if (thread_id == 0) {
            char thread_buffer[64];
            volatile size_t local_size = g_mem_size;
            
            __builtin_memset(thread_buffer, 0, sizeof(thread_buffer));
            __builtin_memcpy(thread_buffer, g_token_buffer, 
                           (local_size < 64) ? local_size : 64);
            __builtin_memmove(thread_buffer + 32, thread_buffer, 32);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        if (nodes[i]) {
            free(nodes[i]);
        }
    }
    
    return total_sum;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Wait for constructor */
    while (!g_init_flag) {
        /* Busy wait - ensures constructor runs */
    }
    
    /* Test 1: Basic builtin operations */
    char buffer1[128], buffer2[128];
    volatile int use_size = (int)g_mem_size;
    
    __builtin_memset(buffer1, 0xCC, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, use_size);
    __builtin_memmove(buffer1 + 32, buffer1, 64);
    
    /* Test 2: Recursive parser with memory ops */
    ast_node_t* root = parse_expression(4);
    if (root) {
        ast_node_t* copy = parse_expression(2);
        if (copy) {
            process_with_goto(root, copy);
            free(copy);
        }
        
        /* Verify checksum */
        uint32_t verify = 0;
        for (int i = 0; i < 32; i++) {
            verify += (uint32_t)root->data[i];
        }
        
        if (verify != root->checksum) {
            printf("Checksum mismatch: %u vs %u\n", verify, root->checksum);
        }
        
        free(root);
    }
    
    /* Test 3: Parallel operations */
    uint64_t parallel_sum = parallel_memory_ops();
    printf("Parallel operations sum: %llu\n", (unsigned long long)parallel_sum);
    
    /* Test 4: Edge cases with volatile sizes */
    volatile size_t dynamic_size = g_mem_size;
    char* dynamic_buf = (char*)malloc(dynamic_size * 2);
    if (dynamic_buf) {
        __builtin_memset(dynamic_buf, 0xAA, dynamic_size);
        __builtin_memcpy(dynamic_buf + dynamic_size, dynamic_buf, dynamic_size);
        __builtin_memmove(dynamic_buf, dynamic_buf + dynamic_size, dynamic_size);
        
        /* Verify copy */
        int errors = 0;
        for (size_t i = 0; i < dynamic_size; i++) {
            if (dynamic_buf[i] != 0xAA) errors++;
        }
        printf("Dynamic buffer errors: %d\n", errors);
        
        free(dynamic_buf);
    }
    
    /* Final verification */
    uint32_t final_hash = 0;
    for (int i = 0; i < 256; i++) {
        final_hash = (final_hash * 31) + (uint32_t)g_token_buffer[i];
    }
    printf("Final token hash: 0x%08X\n", final_hash);
    
    printf("ASAN test completed successfully.\n");
    return 0;
}
