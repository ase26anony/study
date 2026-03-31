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

/* Global token array for initialization */
static const char* tokens[] = {"memcpy", "memset", "memmove", "asan", "hwasan"};
static const int token_count = sizeof(tokens)/sizeof(tokens[0]);

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[128];
    /* Force __builtin_memset in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    
    /* Use volatile to prevent dead code elimination */
    volatile int* dummy = (volatile int*)buffer;
    *dummy = 0xDEADBEEF;
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char final_check[32];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with __builtin_memset */
    __builtin_memset(node->data, id % 256, sizeof(node->data));
    
    /* Create identifier string with __builtin_memcpy */
    char id_str[16];
    __builtin_snprintf(id_str, sizeof(id_str), "ID%d", id);
    __builtin_memcpy(node->data + 32, id_str, __builtin_strlen(id_str) + 1);
    
    node->id = id;
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int use_copy = 1;
    
    /* Jump into memory operation block */
    if (src->id % 3 == 0) {
        goto do_memmove;
    }
    
    /* Normal path with memcpy */
    __builtin_memcpy(dst->data, src->data, sizeof(src->data));
    goto finish;
    
do_memmove:
    /* Jump target with memmove */
    if (g_use_memmove) {
        __builtin_memmove(dst->data, src->data, sizeof(src->data));
    }
    goto finish;
    
finish:
    /* Verify with volatile read */
    volatile char* check = (volatile char*)dst->data;
    (void)*check;
}

/* Parallel memory dispatch */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Mixed memory operations in parallel region */
                char temp[64];
                
                /* Use volatile size */
                size_t copy_size = g_mem_size % sizeof(temp);
                if (copy_size > sizeof(temp)) copy_size = sizeof(temp);
                
                __builtin_memcpy(temp, nodes[i]->data, copy_size);
                
                /* Conditional memset */
                if (tid % 2 == 0) {
                    __builtin_memset(nodes[i]->data + 16, tid, 32);
                }
                
                /* Potential overlapping memmove */
                if (i > 0 && nodes[i-1]) {
                    __builtin_memmove(nodes[i]->data, nodes[i-1]->data, 16);
                }
            }
        }
    }
}

/* Compute hash from AST */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    
    /* Hash node data */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    /* Recursive hash computation */
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create AST structure */
    ASTNode* root = create_ast(4, 1);
    ASTNode* copy = create_ast(4, 100);
    
    if (!root || !copy) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Initialize copy with different pattern */
    __builtin_memset(copy->data, 0xCC, sizeof(copy->data));
    
    /* Test goto edge cases */
    process_with_goto(root, copy);
    
    /* Create node array for parallel processing */
    ASTNode* nodes[8];
    nodes[0] = root;
    nodes[1] = copy;
    
    for (int i = 2; i < 8; i++) {
        nodes[i] = create_ast(3, 200 + i);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(nodes, 8);
    
    /* Additional built-in calls in main */
    char buffer1[256], buffer2[256];
    
    /* Use volatile-controlled sizes */
    volatile size_t op_size = g_mem_size;
    if (op_size > sizeof(buffer1)) op_size = sizeof(buffer1);
    
    __builtin_memset(buffer1, 0x11, op_size);
    __builtin_memcpy(buffer2, buffer1, op_size);
    
    /* Overlapping memmove */
    __builtin_memmove(buffer1 + 128, buffer1, 128);
    
    /* Compute and print verification hash */
    unsigned long total_hash = 0;
    total_hash += compute_ast_hash(root);
    total_hash += compute_ast_hash(copy);
    
    /* Hash buffer contents */
    for (size_t i = 0; i < op_size; i++) {
        total_hash = ((total_hash << 3) + total_hash) + buffer1[i];
        total_hash = ((total_hash << 3) + total_hash) + buffer2[i];
    }
    
    printf("Verification hash: %lu\n", total_hash);
    printf("Token count: %d\n", token_count);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        if (nodes[i]) {
            free(nodes[i]);
        }
    }
    
    return 0;
}
