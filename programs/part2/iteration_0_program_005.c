/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_trigger = 1;

/* Recursive AST-like structure */
struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
};

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[128];
    /* Force memcpy built-in in constructor */
    __builtin_memcpy(buffer, "constructor_init", 17);
    printf("[Constructor] Initialized\n");
}

/* Destructor for cleanup coordination */
__attribute__((destructor))
static void cleanup_asan(void) {
    printf("[Destructor] Cleanup complete\n");
}

/* Recursive function with memory operations */
struct ASTNode* build_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    struct ASTNode* node = (struct ASTNode*)malloc(sizeof(struct ASTNode));
    if (!node) return NULL;
    
    /* Use memset built-in for initialization */
    __builtin_memset(node, 0, sizeof(struct ASTNode));
    node->id = id;
    
    /* Fill data with pattern using memcpy */
    char pattern[32];
    __builtin_snprintf(pattern, sizeof(pattern), "Node_%d_depth_%d", id, depth);
    __builtin_memcpy(node->data, pattern, strlen(pattern) + 1);
    
    /* Recursive construction */
    node->left = build_ast(depth - 1, id * 2);
    node->right = build_ast(depth - 1, id * 2 + 1);
    
    return node;
}

/* Function with goto jumps around memmove */
void goto_memmove_operations(char* dest, char* src, size_t len) {
    int use_memmove = 0;
    
    /* Jump into block with memmove */
    if (volatile_trigger) goto do_memmove;
    
    normal_path:
    __builtin_memcpy(dest, src, len);
    return;
    
    do_memmove:
    /* This tests flow sensitivity */
    __builtin_memmove(dest, src, len);
    
    /* Jump out */
    if (len > 32) goto normal_path;
    
    /* Another memmove in same basic block */
    char temp[128];
    __builtin_memmove(temp, dest, len / 2);
}

/* OpenMP parallel section with memory operations */
void parallel_memory_operations(struct ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Mix of memory built-ins in parallel region */
                char buffer[256];
                
                if (i % 3 == 0) {
                    __builtin_memcpy(buffer, nodes[i]->data, 
                                   volatile_len % 128);
                } else if (i % 3 == 1) {
                    __builtin_memset(buffer, tid, 
                                   volatile_len % 128);
                } else {
                    /* Create overlapping regions for memmove */
                    __builtin_memmove(&buffer[64], &buffer[32], 
                                    volatile_len % 64);
                }
                
                /* Modify node data */
                __builtin_memcpy(nodes[i]->data, buffer, 64);
            }
        }
    }
}

/* Multi-stage processing with different memory operations */
size_t process_ast(struct ASTNode* root) {
    if (!root) return 0;
    
    size_t hash = 0;
    char combined[512];
    
    /* Process left subtree with memcpy */
    if (root->left) {
        __builtin_memcpy(combined, root->left->data, 128);
        for (int i = 0; i < 128 && combined[i]; i++) {
            hash += combined[i];
        }
    }
    
    /* Process right subtree with memset/memmove combo */
    if (root->right) {
        char temp[256];
        __builtin_memset(temp, 0, sizeof(temp));
        __builtin_memcpy(temp, root->right->data, 128);
        
        /* Overlapping memmove */
        __builtin_memmove(&temp[64], &temp[32], 96);
        
        for (int i = 0; i < 128 && temp[i]; i++) {
            hash += temp[i] * 31;
        }
    }
    
    /* Process current node */
    for (int i = 0; root->data[i]; i++) {
        hash = hash * 31 + root->data[i];
    }
    
    return hash + process_ast(root->left) + process_ast(root->right);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Build AST structure */
    struct ASTNode* root = build_ast(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to build AST\n");
        return 1;
    }
    
    /* Phase 2: Goto-based memory operations */
    char src_buffer[256], dst_buffer[256];
    for (int i = 0; i < 256; i++) {
        src_buffer[i] = i % 26 + 'A';
    }
    
    /* Force multiple memmove redirections */
    for (int i = 0; i < 5; i++) {
        goto_memmove_operations(dst_buffer, src_buffer, volatile_len + i * 16);
    }
    
    /* Phase 3: Create node array for parallel processing */
    struct ASTNode* node_array[32];
    node_array[0] = root;
    
    /* Build additional nodes with different patterns */
    for (int i = 1; i < 32; i++) {
        node_array[i] = build_ast(3, i + 100);
        
        /* Inter-node memory operations */
        if (i > 1 && node_array[i] && node_array[i-1]) {
            __builtin_memcpy(node_array[i]->data, 
                           node_array[i-1]->data, 128);
        }
    }
    
    /* Phase 4: Parallel memory operations */
    parallel_memory_operations(node_array, 32);
    
    /* Phase 5: Final processing and verification */
    size_t final_hash = process_ast(root);
    
    /* Additional built-in calls in cleanup */
    char verify_buffer[128];
    __builtin_memset(verify_buffer, 0xAA, sizeof(verify_buffer));
    __builtin_memcpy(&verify_buffer[64], &verify_buffer[0], 64);
    
    /* One more memmove with volatile length */
    size_t dynamic_len = volatile_len;
    __builtin_memmove(&verify_buffer[32], &verify_buffer[0], dynamic_len);
    
    printf("Final hash: %zu\n", final_hash);
    printf("Verification buffer[0]=0x%02x, buffer[64]=0x%02x\n", 
           (unsigned char)verify_buffer[0], 
           (unsigned char)verify_buffer[64]);
    
    /* Cleanup */
    for (int i = 0; i < 32; i++) {
        free(node_array[i]);
    }
    
    printf("Test completed successfully\n");
    return 0;
}
