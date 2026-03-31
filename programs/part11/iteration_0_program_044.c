#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* AST-like recursive structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile int volatile_flag;  /* Prevent optimization */
    struct ASTNode *left;
    struct ASTNode *right;
    char padding[32];  /* Ensure size for memcpy operations */
} ASTNode;

/* Global token array */
volatile int token_array[256];
volatile int token_index = 0;

/* Function prototypes */
ASTNode* create_ast_node(int type, int value);
void recursive_parser(ASTNode *node, int depth);
void parallel_memory_operations(void);
void __attribute__((constructor)) init_tokens(void);
void __attribute__((destructor)) cleanup(void);

/* Constructor - runs before main */
void __attribute__((constructor)) init_tokens(void)
{
    volatile int i;
    for (i = 0; i < 256; i++) {
        token_array[i] = i * 3 + 1;
    }
    
    /* Force builtin initialization with volatile */
    volatile char buffer1[64];
    volatile char buffer2[64];
    
    /* Initialize asan_memfn_rtls cache */
    __builtin_memset((void*)buffer1, 0, sizeof(buffer1));
    __builtin_memcpy((void*)buffer2, (void*)buffer1, sizeof(buffer1));
    __builtin_memmove((void*)buffer1, (void*)buffer2, sizeof(buffer1));
}

/* Create AST node with memory initialization */
ASTNode* create_ast_node(int type, int value)
{
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile to prevent optimization */
    volatile size_t node_size = sizeof(ASTNode);
    __builtin_memset(node, 0, node_size);
    
    node->type = type;
    node->value = value;
    node->volatile_flag = 1;  /* Volatile write */
    node->left = NULL;
    node->right = NULL;
    
    return node;
}

/* Recursive parser with goto and memory operations */
void recursive_parser(ASTNode *node, int depth)
{
    if (depth >= 5 || !node) return;
    
    ASTNode *temp = create_ast_node(depth, token_array[depth % 256]);
    
    /* Complex control flow with goto */
    if (depth % 3 == 0) {
        goto copy_block;
    } else {
        goto normal_block;
    }
    
copy_block:
    {
        /* Memory copy between nodes with volatile length */
        volatile size_t copy_size = sizeof(ASTNode) - 16;
        __builtin_memcpy(&node->padding, &temp->padding, copy_size);
        
        /* Jump back */
        if (depth % 2 == 0) goto normal_block;
    }
    
normal_block:
    {
        /* More builtin usage */
        volatile char local_buf[128];
        volatile int buf_size = 64 + (depth * 4);
        
        __builtin_memset(local_buf, depth, buf_size);
        __builtin_memmove(local_buf + 32, local_buf, buf_size - 32);
        
        /* Update volatile member */
        node->volatile_flag = depth;
    }
    
    /* Recursive calls */
    if (depth < 4) {
        node->left = create_ast_node(depth + 1, token_array[(depth + 1) % 256]);
        node->right = create_ast_node(depth + 2, token_array[(depth + 2) % 256]);
        
        recursive_parser(node->left, depth + 1);
        recursive_parser(node->right, depth + 2);
    }
    
    /* Final memory operation with goto */
    if (temp) {
        goto final_copy;
    }
    
final_copy:
    {
        volatile size_t final_size = 24;
        __builtin_memcpy(node->padding + 8, temp->padding + 8, final_size);
        free(temp);
    }
}

/* Parallel memory operations with OpenMP */
void parallel_memory_operations(void)
{
    volatile int i;
    volatile char buffers[8][256];
    volatile int results[8] = {0};
    
    #pragma omp parallel for num_threads(4)
    for (i = 0; i < 8; i++) {
        volatile int thread_id = omp_get_thread_num();
        volatile size_t op_size = 128 + (thread_id * 16);
        
        /* Each thread uses all three builtins */
        __builtin_memset(buffers[i], thread_id + 'A', op_size);
        
        if (i > 0) {
            __builtin_memcpy(buffers[i] + 64, buffers[i-1], op_size / 2);
        }
        
        /* memmove with overlapping regions */
        __builtin_memmove(buffers[i] + 32, buffers[i] + 16, op_size - 48);
        
        /* Compute checksum */
        for (int j = 0; j < op_size; j++) {
            results[i] += buffers[i][j];
        }
    }
    
    /* Verify parallel results */
    volatile int total = 0;
    for (i = 0; i < 8; i++) {
        total += results[i];
    }
    token_array[0] = total % 1000;
}

/* Destructor - runs after main */
void __attribute__((destructor)) cleanup(void)
{
    /* Final memory operations in destructor */
    volatile char final_buf[512];
    volatile size_t final_size = 256;
    
    __builtin_memset(final_buf, 0xFF, final_size);
    __builtin_memcpy(final_buf + 128, final_buf, final_size - 128);
    __builtin_memmove(final_buf, final_buf + 64, final_size - 64);
}

/* Main execution flow */
int main(void)
{
    volatile int result = 0;
    
    /* Initialize AST */
    ASTNode *root = create_ast_node(0, token_array[0]);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST root\n");
        return 1;
    }
    
    /* Recursive parsing with memory operations */
    recursive_parser(root, 0);
    
    /* Parallel memory dispatch */
    parallel_memory_operations();
    
    /* Compute verification hash */
    ASTNode *current = root;
    int depth = 0;
    
    while (current && depth < 10) {
        result ^= current->value;
        result += current->volatile_flag;
        result = (result * 31) & 0xFFFF;
        
        /* Traverse */
        if (depth % 2 == 0 && current->left) {
            current = current->left;
        } else if (current->right) {
            current = current->right;
        } else {
            break;
        }
        depth++;
    }
    
    /* Additional builtin calls in main */
    volatile char main_buf[1024];
    volatile size_t main_size = 512;
    
    __builtin_memset(main_buf, 0xAA, main_size);
    __builtin_memcpy(main_buf + 256, main_buf, main_size - 256);
    __builtin_memmove(main_buf, main_buf + 128, main_size - 128);
    
    /* Incorporate buffer into result */
    for (int i = 0; i < 64; i++) {
        result += main_buf[i * 8];
    }
    
    /* Cleanup */
    free(root);
    
    /* Print verification result */
    printf("Verification result: %d\n", result);
    
    return 0;
}
