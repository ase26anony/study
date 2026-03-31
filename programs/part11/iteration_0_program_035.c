/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    volatile char buffer[32];
    /* Force memset builtin in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    g_use_hwasan = (getenv("USE_HWASAN") != NULL);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_globals(void) {
    volatile char final_buffer[16];
    /* Force memcpy builtin in destructor */
    char src[16] = "DESTRUCT";
    __builtin_memcpy(final_buffer, src, sizeof(src));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    node->left = node->right = NULL;
    
    /* Initialize node data with memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in data */
    snprintf(node->data, sizeof(node->data), 
             "Node%d_Depth%d", node->id, depth);
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    skip_left:
    node->right = create_ast(depth - 1, counter);
    return node;
    
    create_children:
    node->left = create_ast(depth - 1, counter);
    
    /* Jump back with goto */
    if (create_left) {
        create_left = 0;
        goto skip_left;
    }
    
    return node;
}

/* Function with complex memory operations and goto */
static void process_ast_nodes(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int use_memmove = 0;
    
    /* Copy data between nodes using memcpy */
    __builtin_memcpy(dst->data, src->data, sizeof(src->data));
    
    /* Conditional goto around memmove */
    if (src->id % 3 == 0) {
        goto skip_memmove;
    }
    
    /* Use memmove with overlapping regions */
    char temp[128];
    __builtin_memcpy(temp, src->data, 32);
    __builtin_memmove(temp + 16, temp, 32);
    use_memmove = 1;
    
    skip_memmove:
    
    /* Process children recursively */
    if (src->left && dst->left) {
        process_ast_nodes(src->left, dst->left);
    }
    
    /* Jump back if we skipped memmove */
    if (!use_memmove && src->id % 2 == 0) {
        goto final_copy;
    }
    
    /* Additional copy */
    __builtin_memcpy(dst->data + 32, src->data + 32, 16);
    
    final_copy:
    return;
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    volatile size_t local_size = g_mem_size;
    char* buffers[4];
    
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp critical
        {
            buffers[tid] = malloc(local_size);
            if (buffers[tid]) {
                /* Each thread uses different builtins */
                if (tid % 3 == 0) {
                    __builtin_memset(buffers[tid], tid, local_size);
                } else if (tid % 3 == 1) {
                    char pattern[64];
                    __builtin_memset(pattern, 0xCC, sizeof(pattern));
                    __builtin_memcpy(buffers[tid], pattern, 
                                   sizeof(pattern) < local_size ? 
                                   sizeof(pattern) : local_size);
                } else {
                    /* Create overlapping copy with memmove */
                    __builtin_memset(buffers[tid], 0xFF, local_size);
                    __builtin_memmove(buffers[tid] + local_size/2,
                                    buffers[tid], local_size/4);
                }
            }
        }
        
        #pragma omp barrier
        
        /* Verify and compute checksum */
        unsigned long checksum = 0;
        if (buffers[tid]) {
            for (size_t i = 0; i < local_size && i < 128; i++) {
                checksum += (unsigned char)buffers[tid][i];
            }
            free(buffers[tid]);
        }
        
        #pragma omp critical
        {
            printf("Thread %d checksum: %lu\n", tid, checksum);
        }
    }
}

/* Main test driver */
int main(void) {
    int counter = 1;
    unsigned long total_hash = 0;
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Create and process AST */
    ASTNode* ast1 = create_ast(4, &counter);
    ASTNode* ast2 = create_ast(4, &counter);
    
    if (ast1 && ast2) {
        process_ast_nodes(ast1, ast2);
        
        /* Compute hash from AST data */
        for (int i = 0; i < 64; i++) {
            total_hash += (unsigned char)ast2->data[i];
        }
    }
    
    /* Phase 2: Large memory operations */
    volatile size_t dynamic_size = g_mem_size * 2;
    char* large_buf1 = malloc(dynamic_size);
    char* large_buf2 = malloc(dynamic_size);
    
    if (large_buf1 && large_buf2) {
        /* Test all three builtins with volatile sizes */
        __builtin_memset(large_buf1, 0xAB, dynamic_size);
        __builtin_memcpy(large_buf2, large_buf1, dynamic_size);
        
        /* Overlapping memmove */
        __builtin_memmove(large_buf1 + dynamic_size/3,
                         large_buf1, dynamic_size/2);
        
        /* Add to hash */
        for (size_t i = 0; i < dynamic_size && i < 256; i++) {
            total_hash += (unsigned char)large_buf1[i];
        }
        
        free(large_buf1);
        free(large_buf2);
    }
    
    /* Phase 3: Parallel operations */
    printf("Starting parallel section...\n");
    parallel_memory_ops();
    
    /* Phase 4: Final verification */
    volatile char final_buf[256];
    char src_pattern[256];
    
    for (int i = 0; i < 256; i++) {
        src_pattern[i] = (char)(total_hash + i) & 0xFF;
    }
    
    __builtin_memcpy(final_buf, src_pattern, 256);
    __builtin_memset(final_buf + 128, 0, 64);
    __builtin_memmove(final_buf + 64, final_buf + 32, 96);
    
    /* Final hash computation */
    for (int i = 0; i < 256; i++) {
        total_hash += (unsigned char)final_buf[i];
    }
    
    printf("Total hash: %lu\n", total_hash);
    printf("Test completed.\n");
    
    /* Cleanup */
    /* Note: In real code, you'd want to free AST nodes properly */
    
    return (total_hash > 0) ? 0 : 1;
}
