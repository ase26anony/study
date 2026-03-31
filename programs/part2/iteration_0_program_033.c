/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

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

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force builtin initialization in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_late(void) {
    volatile char final_check[8];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* create_tree(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Use builtins with volatile-controlled size */
    __builtin_memset(node->data, node->id, sizeof(node->data));
    
    /* Conditional memcpy based on depth */
    char temp[64];
    if (depth % 2 == 0) {
        __builtin_memcpy(temp, node->data, g_mem_size % sizeof(node->data));
        __builtin_memcpy(node->data + 16, temp, 16);
    }
    
    node->left = create_tree(depth - 1, counter);
    node->right = create_tree(depth - 1, counter);
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int use_copy = 1;
    
    if (src == NULL || dst == NULL) {
        goto cleanup;
    }
    
copy_block:
    /* This block contains the critical builtin */
    if (use_copy) {
        __builtin_memcpy(dst->data, src->data, sizeof(src->data));
        use_copy = 0;
        goto move_block;
    }
    
move_block:
    if (g_use_memmove) {
        /* Force memmove with overlapping regions */
        __builtin_memmove(dst->data + 16, dst->data, 32);
        goto finalize;
    }
    
finalize:
    /* Final memset */
    __builtin_memset(dst->data + 48, 0xCC, 16);
    return;
    
cleanup:
    __builtin_memset(dst->data, 0, sizeof(dst->data));
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        #pragma omp for
        for (int i = 0; i < count; i++) {
            volatile char local_buf[128];
            
            /* Each thread uses all three builtins */
            __builtin_memset(local_buf, tid, sizeof(local_buf));
            
            if (nodes[i]) {
                __builtin_memcpy(nodes[i]->data, local_buf, 
                               g_mem_size % sizeof(nodes[i]->data));
                
                /* Conditional memmove */
                if (i % 3 == 0) {
                    __builtin_memmove(local_buf + 64, local_buf, 64);
                    __builtin_memcpy(nodes[i]->data + 32, local_buf + 64, 32);
                }
            }
        }
        
        /* Thread-private memset */
        #pragma omp barrier
        volatile char sync_buf[64];
        __builtin_memset(sync_buf, 0xAA, sizeof(sync_buf));
    }
}

/* Main execution flow */
int main(void) {
    int counter = 1;
    long hash_sum = 0;
    
    /* Create recursive structure */
    ASTNode* root = create_tree(4, &counter);
    if (!root) return 1;
    
    /* Create array for parallel processing */
    ASTNode* nodes[8];
    nodes[0] = root;
    for (int i = 1; i < 8; i++) {
        nodes[i] = create_tree(3, &counter);
    }
    
    /* Test goto flow control */
    process_with_goto(root, nodes[1]);
    
    /* Force all three builtins in main */
    volatile char main_buf[256];
    __builtin_memset(main_buf, 0x11, sizeof(main_buf));
    __builtin_memcpy(main_buf + 128, main_buf, 128);
    __builtin_memmove(main_buf + 64, main_buf + 32, 128);
    
    /* Execute parallel section */
    parallel_memory_ops(nodes, 8);
    
    /* Calculate verification hash */
    for (int i = 0; i < 8; i++) {
        if (nodes[i]) {
            for (size_t j = 0; j < sizeof(nodes[i]->data); j++) {
                hash_sum += nodes[i]->data[j] * (i + 1);
            }
        }
    }
    
    /* Final builtin calls before exit */
    __builtin_memset(main_buf, hash_sum & 0xFF, 64);
    __builtin_memcpy(root->data, main_buf, sizeof(root->data));
    
    printf("Result hash: %ld\n", hash_sum);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        free(nodes[i]);
    }
    
    return 0;
}
