/* ISO C99-compliant test program for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
struct ast_node {
    int type;
    char data[32];
    struct ast_node *left;
    struct ast_node *right;
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Force initialization of memory built-ins */
    char buf1[128], buf2[128];
    
    /* Use all three built-ins in constructor */
    __builtin_memset(buf1, 0xAA, sizeof(buf1));
    __builtin_memcpy(buf2, buf1, sizeof(buf1));
    __builtin_memmove(buf1 + 10, buf1, 50);
    
    printf("Constructor: ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Destructor: ASAN environment cleaned up\n");
}

/* Recursive function with memory operations */
static struct ast_node* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Initialize with built-in memset */
    __builtin_memset(node, 0, sizeof(struct ast_node));
    
    node->type = depth;
    
    /* Fill data with pattern using volatile length */
    int len = volatile_len % 32;
    for (int i = 0; i < len; i++) {
        node->data[i] = 'A' + (depth % 26);
    }
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_goto = 1;
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1);
        node->right = create_ast(depth - 1);
        
        create_children:
        /* Jump into block with memmove */
        char temp[32];
        __builtin_memcpy(temp, node->data, sizeof(temp));
        __builtin_memmove(node->data + 5, node->data, 10);
        __builtin_memcpy(node->data, temp, 5);
        
        node->left = create_ast(depth - 1);
        node->right = create_ast(depth - 1);
        
        /* Copy between nodes */
        if (node->left && node->right) {
            __builtin_memcpy(node->right->data, node->left->data, 
                           sizeof(node->left->data));
        }
    }
    
    return node;
}

/* Function with complex control flow using goto */
static void process_with_goto(struct ast_node* node) {
    if (!node) return;
    
    int state = 0;
    
    start_processing:
    switch(state) {
        case 0:
            /* Use built-in in switch case */
            __builtin_memset(node->data, 'X', 16);
            state = 1;
            goto start_processing;
            
        case 1:
            /* Jump over memmove */
            if (node->left) {
                goto skip_memmove;
            }
            
            __builtin_memmove(node->data + 8, node->data, 8);
            
            skip_memmove:
            state = 2;
            goto start_processing;
            
        case 2:
            /* Final copy */
            if (node->right) {
                __builtin_memcpy(node->right->data, node->data, 16);
            }
            break;
    }
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_operations(void) {
    int i;
    char buffers[8][128];
    
    #pragma omp parallel private(i)
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread uses built-ins */
        __builtin_memset(buffers[thread_id], thread_id, 128);
        
        #pragma omp barrier
        
        /* Copy between buffers */
        int src_thread = (thread_id + 1) % omp_get_num_threads();
        __builtin_memcpy(buffers[thread_id], buffers[src_thread], 64);
        
        /* Move within buffer */
        __builtin_memmove(buffers[thread_id] + 32, buffers[thread_id], 32);
    }
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Basic built-in usage */
    printf("Phase 1: Basic built-in operations\n");
    {
        char buffer1[256], buffer2[256];
        
        /* Force all three built-ins */
        __builtin_memset(buffer1, 0x55, sizeof(buffer1));
        __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
        __builtin_memmove(buffer1 + 128, buffer1, 128);
        
        /* Use volatile variables */
        int len = volatile_len;
        __builtin_memset(volatile_dest, 0xCC, len);
        __builtin_memcpy((char*)volatile_src, (char*)volatile_dest, len);
    }
    
    /* Phase 2: Recursive AST operations */
    printf("Phase 2: Recursive AST operations\n");
    struct ast_node* root = create_ast(4);
    if (root) {
        process_with_goto(root);
        
        /* Calculate hash/sum for verification */
        unsigned long hash = 0;
        struct ast_node* stack[32];
        int top = 0;
        stack[top++] = root;
        
        while (top > 0) {
            struct ast_node* node = stack[--top];
            
            /* Add node data to hash */
            for (int i = 0; i < 32; i++) {
                hash += (unsigned long)node->data[i];
            }
            
            if (node->right) stack[top++] = node->right;
            if (node->left) stack[top++] = node->left;
            
            /* Free with cleanup */
            __builtin_memset(node, 0xFF, sizeof(struct ast_node));
            free(node);
        }
        
        printf("AST hash sum: %lu\n", hash);
    }
    
    /* Phase 3: OpenMP parallel operations */
    printf("Phase 3: OpenMP parallel operations\n");
    #ifdef _OPENMP
    parallel_memory_operations();
    #else
    printf("OpenMP not available, skipping parallel phase\n");
    #endif
    
    /* Phase 4: Edge cases with goto jumping */
    printf("Phase 4: Control flow edge cases\n");
    {
        char data[100];
        int do_jump = 1;
        
        __builtin_memset(data, 'Z', sizeof(data));
        
        if (do_jump) {
            goto jump_over;
        }
        
        /* This should be jumped over */
        __builtin_memcpy(data + 50, data, 25);
        
        jump_over:
        /* Jump back in */
        __builtin_memmove(data + 10, data + 90, 10);
        
        /* Nested goto */
        {
            int inner = 0;
            goto inner_label;
            
            inner_label:
            __builtin_memset(data + 75, 0, 10);
        }
    }
    
    /* Final verification */
    printf("Phase 5: Final verification\n");
    {
        volatile char final_check[64];
        __builtin_memset((char*)final_check, 0x42, 64);
        __builtin_memcpy((char*)final_check + 32, (char*)final_check, 32);
        __builtin_memmove((char*)final_check, (char*)final_check + 16, 16);
        
        int sum = 0;
        for (int i = 0; i < 64; i++) {
            sum += final_check[i];
        }
        printf("Final check sum: %d\n", sum);
    }
    
    printf("=== Test completed successfully ===\n");
    return 0;
}
