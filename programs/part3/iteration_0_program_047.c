/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
struct ASTNode {
    char *data;
    size_t len;
    struct ASTNode *left;
    struct ASTNode *right;
    int id;
};

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force initialization of ASAN runtime before main */
    volatile char buffer[64];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    /* Final memory operation in destructor */
    volatile char final_buf[32];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static struct ASTNode* build_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    struct ASTNode *node = malloc(sizeof(struct ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->len = g_mem_size % 512 + 64;
    node->data = malloc(node->len);
    
    if (!node->data) {
        free(node);
        return NULL;
    }
    
    /* Use all three builtins with volatile control */
    volatile int use_pattern = id % 3;
    
    if (use_pattern == 0) {
        __builtin_memset(node->data, id % 256, node->len);
    } else if (use_pattern == 1) {
        char pattern[128];
        __builtin_memset(pattern, id % 128, sizeof(pattern));
        __builtin_memcpy(node->data, pattern, 
                        node->len < sizeof(pattern) ? node->len : sizeof(pattern));
    } else {
        char temp[256];
        __builtin_memset(temp, id % 64, sizeof(temp));
        __builtin_memmove(node->data, temp, 
                         node->len < sizeof(temp) ? node->len : sizeof(temp));
    }
    
    /* Recursive construction */
    node->left = build_ast(depth - 1, id * 2);
    node->left = build_ast(depth - 1, id * 2 + 1);
    
    return node;
}

/* Function with goto jumping into memory operation blocks */
static void process_with_goto(struct ASTNode *a, struct ASTNode *b) {
    if (!a || !b || !a->data || !b->data) return;
    
    size_t copy_len = (a->len < b->len) ? a->len : b->len;
    
    /* Jump into memmove block */
    goto memmove_block;
    
memcpy_block:
    __builtin_memcpy(a->data, b->data, copy_len);
    goto end;
    
memmove_block:
    if (g_use_memmove) {
        __builtin_memmove(a->data, b->data, copy_len);
        goto memset_block;
    } else {
        goto memcpy_block;
    }
    
memset_block:
    __builtin_memset(b->data, 0xCC, copy_len / 2);
    /* Jump out to different context */
    goto end;
    
end:
    return;
}

/* Parallel processing with OpenMP */
static unsigned long long parallel_memory_ops(struct ASTNode **nodes, int count) {
    unsigned long long total_hash = 0;
    
    #pragma omp parallel reduction(+:total_hash)
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i] && nodes[i]->data) {
                /* Mixed builtin usage in parallel region */
                volatile int op_type = (i + tid) % 4;
                
                if (op_type == 0) {
                    __builtin_memset(nodes[i]->data, tid, nodes[i]->len / 4);
                } else if (op_type == 1) {
                    char local_buf[128];
                    __builtin_memset(local_buf, i % 256, sizeof(local_buf));
                    __builtin_memcpy(nodes[i]->data + 32, local_buf, 
                                    nodes[i]->len > 160 ? 128 : nodes[i]->len - 32);
                } else if (op_type == 2) {
                    /* Overlapping memmove */
                    size_t move_len = nodes[i]->len / 2;
                    if (move_len > 0) {
                        __builtin_memmove(nodes[i]->data + move_len/2, 
                                         nodes[i]->data, 
                                         move_len);
                    }
                } else {
                    /* Combination of operations */
                    __builtin_memset(nodes[i]->data, 0xAA, nodes[i]->len / 8);
                    __builtin_memcpy(nodes[i]->data + nodes[i]->len/8, 
                                    nodes[i]->data, 
                                    nodes[i]->len/8);
                }
                
                /* Compute simple hash */
                for (size_t j = 0; j < nodes[i]->len && j < 64; j++) {
                    total_hash += (unsigned long long)nodes[i]->data[j] * (i + 1);
                }
            }
        }
    }
    
    return total_hash;
}

/* Free AST recursively */
static void free_ast(struct ASTNode *node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node->data);
    free(node);
}

int main(void) {
    const int ast_depth = 4;
    const int node_count = 8;
    
    /* Build multiple ASTs */
    struct ASTNode *nodes[node_count];
    for (int i = 0; i < node_count; i++) {
        nodes[i] = build_ast(ast_depth, i + 1);
    }
    
    /* Process with goto jumps */
    for (int i = 0; i < node_count - 1; i += 2) {
        process_with_goto(nodes[i], nodes[i + 1]);
    }
    
    /* Additional direct builtin calls */
    volatile char direct_buf[512];
    volatile char src_buf[256];
    
    __builtin_memset(src_buf, 0x55, sizeof(src_buf));
    __builtin_memcpy(direct_buf, src_buf, sizeof(src_buf));
    __builtin_memmove(direct_buf + 128, direct_buf, 128);
    __builtin_memset(direct_buf + 384, 0x77, 128);
    
    /* Parallel processing */
    unsigned long long hash_result = parallel_memory_ops(nodes, node_count);
    
    /* Final verification memcpy */
    char final_result[64];
    __builtin_memset(final_result, 0, sizeof(final_result));
    __builtin_memcpy(final_result, &hash_result, 
                    sizeof(hash_result) < sizeof(final_result) ? 
                    sizeof(hash_result) : sizeof(final_result));
    
    /* Print result */
    printf("Memory operations completed. Hash: %llu\n", hash_result);
    
    /* Cleanup */
    for (int i = 0; i < node_count; i++) {
        free_ast(nodes[i]);
    }
    
    return 0;
}
