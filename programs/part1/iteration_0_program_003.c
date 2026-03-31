/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 96;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Global token array */
static char g_token_array[4096];

/* Constructor attribute to force early initialization */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token array with pattern */
    for (size_t i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)((i * 31) & 0xFF);
    }
    
    /* Use __builtin_memset in constructor context */
    __builtin_memset(g_token_array + 512, 0xAA, 128);
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Final memory operation in destructor */
    volatile char final_buf[64];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ASTNode* parse_recursive(int depth, const char* tokens, size_t len) {
    if (depth <= 0 || len < 16) {
        return NULL;
    }
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data with volatile length */
    volatile size_t copy_len = len > 256 ? 256 : len;
    __builtin_memcpy(node->data, tokens, copy_len);
    
    node->type = depth;
    node->value = (int)(tokens[0] + tokens[len-1]);
    
    /* Recursive calls with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 3 == 0);
        
        if (use_goto) {
            goto recursive_call;
        }
        
        node->left = parse_recursive(depth - 1, tokens + 16, len - 16);
        
        recursive_call:
        node->right = parse_recursive(depth - 2, tokens + 32, len - 32);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Memory operation dispatcher with OpenMP */
static void dispatch_memory_ops(ASTNode* nodes[], size_t count) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (size_t i = 0; i < count; i++) {
            if (nodes[i] && nodes[i]->left && nodes[i]->right) {
                /* Use all three builtins with volatile lengths */
                volatile size_t op_len = g_memcpy_len + thread_id;
                
                /* memcpy between child nodes */
                __builtin_memcpy(nodes[i]->left->data, 
                               nodes[i]->right->data, 
                               op_len > 256 ? 256 : op_len);
                
                /* memset on node data */
                __builtin_memset(nodes[i]->data + 64, 
                               thread_id, 
                               g_memset_len > 192 ? 192 : g_memset_len);
                
                /* memmove with overlapping regions */
                volatile char temp_buf[512];
                __builtin_memcpy(temp_buf, nodes[i]->data, 256);
                __builtin_memmove(nodes[i]->data + 32, 
                                nodes[i]->data, 
                                g_memmove_len > 224 ? 224 : g_memmove_len);
                __builtin_memcpy(nodes[i]->data, temp_buf, 256);
            }
        }
        
        /* Additional memory ops outside parallel loop */
        #pragma omp single
        {
            volatile char single_buf[1024];
            __builtin_memset(single_buf, 0xCC, sizeof(single_buf));
            
            /* Goto jumping into memory operation block */
            int do_jump = 1;
            if (do_jump) {
                goto jump_point;
            }
            
            __builtin_memset(single_buf + 256, 0xDD, 128);
            
            jump_point:
            __builtin_memmove(single_buf + 384, single_buf + 128, 256);
        }
    }
}

/* Calculate hash from AST structure */
static unsigned long calculate_hash(ASTNode* root) {
    if (!root) return 0;
    
    unsigned long hash = 5381;
    
    /* Process node data */
    for (size_t i = 0; i < sizeof(root->data) && root->data[i]; i++) {
        hash = ((hash << 5) + hash) + (unsigned long)root->data[i];
    }
    
    hash ^= (unsigned long)root->type;
    hash ^= (unsigned long)root->value << 8;
    
    /* Recursive hash combination */
    unsigned long left_hash = calculate_hash(root->left);
    unsigned long right_hash = calculate_hash(root->right);
    
    hash ^= (left_hash << 16) ^ (right_hash << 8);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create array of AST nodes */
    const size_t node_count = 8;
    ASTNode* nodes[node_count];
    
    /* Initialize nodes recursively */
    for (size_t i = 0; i < node_count; i++) {
        size_t offset = i * 128;
        size_t len = sizeof(g_token_array) - offset;
        if (len > 512) len = 512;
        
        nodes[i] = parse_recursive(4, g_token_array + offset, len);
    }
    
    /* Dispatch memory operations with OpenMP */
    dispatch_memory_ops(nodes, node_count);
    
    /* Additional standalone builtin calls */
    volatile char extra_buf1[256], extra_buf2[256];
    
    __builtin_memset(extra_buf1, 0x11, sizeof(extra_buf1));
    __builtin_memcpy(extra_buf2, extra_buf1, sizeof(extra_buf2));
    
    /* Complex goto pattern with memmove */
    {
        volatile int counter = 0;
        
        start_loop:
        if (counter++ < 3) {
            __builtin_memmove(extra_buf1 + 64, extra_buf1, 128);
            goto start_loop;
        }
    }
    
    /* Calculate and print verification result */
    unsigned long total_hash = 0;
    for (size_t i = 0; i < node_count; i++) {
        if (nodes[i]) {
            total_hash ^= calculate_hash(nodes[i]);
            
            /* Cleanup */
            free(nodes[i]->left);
            free(nodes[i]->right);
            free(nodes[i]);
        }
    }
    
    printf("Verification hash: 0x%08lx\n", total_hash);
    printf("Test completed\n");
    
    return (total_hash != 0) ? 0 : 1;
}
