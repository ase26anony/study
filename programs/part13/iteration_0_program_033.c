/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[128];
    /* Force __builtin_memset redirection early */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    printf("Constructor: initialized buffer\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    printf("Destructor: ASAN cleanup complete\n");
}

/* Recursive function with memory operations */
ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[32];
    __builtin_memset(pattern, 'A' + (id % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(pattern));
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        node->right = NULL;
        
        create_children:
        /* Jump target with __builtin_memmove */
        if (use_memmove) {
            ASTNode temp;
            __builtin_memcpy(&temp, node, sizeof(ASTNode));
            __builtin_memmove(node->data + 32, node->data, 32);
            __builtin_memcpy(node->data, temp.data, 32);
        }
        
        if (!use_goto) {
            node->right = create_ast(depth - 1, id * 2 + 1);
        }
    }
    
    return node;
}

/* Parallel memory operation dispatcher */
void parallel_mem_operations(void) {
    const int num_threads = 4;
    char* buffers[num_threads];
    size_t sizes[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        sizes[tid] = (tid + 1) * volatile_len;
        buffers[tid] = (char*)malloc(sizes[tid]);
        
        if (buffers[tid]) {
            /* Each thread uses different builtins */
            switch (tid % 3) {
                case 0:
                    __builtin_memset(buffers[tid], tid, sizes[tid]);
                    break;
                case 1:
                    __builtin_memcpy(buffers[tid], "SOURCE_DATA", 12);
                    break;
                case 2:
                    __builtin_memmove(buffers[tid], buffers[tid] + 8, sizes[tid] - 8);
                    break;
            }
            
            /* Cross-thread memory operation */
            #pragma omp barrier
            
            if (tid == 0) {
                for (int i = 1; i < num_threads; i++) {
                    __builtin_memcpy(buffers[0] + i * 16, buffers[i], 16);
                }
            }
        }
        
        #pragma omp barrier
        
        if (buffers[tid]) {
            free(buffers[tid]);
        }
    }
}

/* Complex token processing with goto jumps */
void process_tokens(char* tokens[], int count) {
    char buffer[512];
    int i = 0;
    
    start_processing:
    if (i >= count) goto done;
    
    /* Jump into memory operation block */
    if (tokens[i][0] == 'C') {
        goto copy_block;
    } else if (tokens[i][0] == 'M') {
        goto move_block;
    }
    
    copy_block:
    {
        /* Force __builtin_memcpy redirection */
        __builtin_memcpy(buffer, tokens[i], strlen(tokens[i]) + 1);
        i++;
        if (i < count && tokens[i][0] == 'S') {
            goto set_block;
        }
        goto start_processing;
    }
    
    move_block:
    {
        /* Force __builtin_memmove redirection */
        size_t len = strlen(tokens[i]);
        __builtin_memmove(buffer, buffer + 32, len);
        __builtin_memcpy(buffer + len, tokens[i], len);
        i++;
        goto start_processing;
    }
    
    set_block:
    {
        /* Force __builtin_memset redirection */
        __builtin_memset(buffer + 128, 0xFF, volatile_len);
        i++;
        goto start_processing;
    }
    
    done:
    return;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* 1. Create recursive AST structure */
    ASTNode* root = create_ast(4, 1);
    
    /* 2. Process tokens with goto control flow */
    char* tokens[] = {"COPY_ME", "SET_ME", "MOVE_ME", "COPY_AGAIN"};
    process_tokens(tokens, 4);
    
    /* 3. Execute parallel memory operations */
    parallel_mem_operations();
    
    /* 4. Complex memory operations between AST nodes */
    if (root && root->left && root->right) {
        /* Cross-node memory operations */
        __builtin_memcpy(root->right->data, root->left->data, 128);
        __builtin_memmove(root->left->data + 64, root->left->data, 64);
        __builtin_memset(root->data + 192, 0xCC, 64);
        
        /* Calculate verification hash */
        unsigned long hash = 0;
        for (int i = 0; i < 256; i++) {
            hash = (hash * 31) + root->data[i];
        }
        printf("Verification hash: %lu\n", hash);
    }
    
    /* Cleanup */
    /* ... recursive free implementation would go here ... */
    
    printf("Test completed successfully\n");
    return 0;
}
