/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (i % 256) ^ 0x55;
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use memset in destructor */
    __builtin_memset(token_pool, 0, sizeof(token_pool));
    printf("Destructor: Token pool cleared\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtin memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy pattern using builtin memcpy */
    int copy_len = volatile_len % 128;
    if (copy_len > 0) {
        __builtin_memcpy(node->data, token_pool + token_index, copy_len);
        token_index = (token_index + copy_len) % sizeof(token_pool);
    }
    
    node->id = id;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = volatile_flag;
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        node->right = NULL;
        return node;
        
    create_children:
        node->left = create_ast(depth - 1, id * 2);
        node->right = create_ast(depth - 1, id * 2 + 1);
        
        /* Copy between nodes using builtin memmove */
        if (node->left && node->right) {
            int move_len = sizeof(node->left->data) / 2;
            __builtin_memmove(node->right->data + move_len, 
                            node->left->data, move_len);
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with complex control flow and memory ops */
static void process_ast(ASTNode* root, int* result) {
    if (!root) return;
    
    ASTNode* stack[32];
    int top = 0;
    stack[top++] = root;
    
    while (top > 0) {
        ASTNode* current = stack[--top];
        
        /* Process node data */
        int sum = 0;
        for (int i = 0; i < 64; i++) {
            sum += current->data[i];
        }
        *result += sum ^ current->id;
        
        /* Push children */
        if (current->right) {
            /* Use goto to jump into memory operation block */
            if (volatile_flag) {
                goto push_right;
            }
            
            stack[top++] = current->right;
            continue;
            
        push_right:
            /* This block tests goto into memmove context */
            stack[top++] = current->right;
            
            /* Copy data between stack slots if possible */
            if (top >= 2) {
                int copy_size = 32;
                __builtin_memmove((char*)stack[top-1], 
                                (char*)stack[top-2], 
                                copy_size);
            }
        }
        
        if (current->left) {
            stack[top++] = current->left;
        }
    }
}

/* Parallel processing function */
static void parallel_memory_ops(void) {
    int buffer1[1024];
    int buffer2[1024];
    int buffer3[1024];
    
    /* Initialize buffers */
    for (int i = 0; i < 1024; i++) {
        buffer1[i] = i;
        buffer2[i] = i * 2;
        buffer3[i] = i * 3;
    }
    
    /* OpenMP parallel region with memory operations */
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                /* Use builtin memcpy */
                __builtin_memcpy(buffer1 + thread_id * 16, 
                               buffer2, 
                               volatile_len % 256);
                break;
            case 1:
                /* Use builtin memset */
                __builtin_memset(buffer3 + thread_id * 8, 
                               thread_id, 
                               volatile_len % 128);
                break;
            case 2:
                /* Use builtin memmove with overlap */
                __builtin_memmove(buffer1 + 64, 
                                buffer1, 
                                volatile_len % 192);
                break;
        }
        
        /* Barrier to synchronize */
        #pragma omp barrier
        
        /* Combined operation after barrier */
        #pragma omp for
        for (int i = 0; i < 256; i++) {
            int idx = i * 4;
            if (idx + 64 < 1024) {
                __builtin_memcpy(buffer2 + idx, 
                               buffer3, 
                               64);
            }
        }
    }
    
    /* Verify operations */
    int check_sum = 0;
    for (int i = 0; i < 256; i++) {
        check_sum += buffer1[i] + buffer2[i] + buffer3[i];
    }
    printf("Parallel ops checksum: %d\n", check_sum);
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Recursive AST operations */
    printf("\nPhase 1: Creating AST structure...\n");
    ASTNode* root = create_ast(4, 1);
    
    int ast_result = 0;
    process_ast(root, &ast_result);
    printf("AST processing result: %d\n", ast_result);
    
    /* Phase 2: Direct builtin calls with volatile control */
    printf("\nPhase 2: Direct builtin memory operations...\n");
    
    char test_buf1[512];
    char test_buf2[512];
    
    /* Series of builtin calls */
    __builtin_memset(test_buf1, 0xAA, sizeof(test_buf1));
    __builtin_memcpy(test_buf2, test_buf1, sizeof(test_buf1));
    
    /* Overlapping memmove */
    __builtin_memmove(test_buf1 + 128, test_buf1, 256);
    
    /* Verify with regular memcmp */
    if (memcmp(test_buf1 + 128, test_buf2, 256) == 0) {
        printf("Memmove verification passed\n");
    }
    
    /* Phase 3: Parallel operations */
    printf("\nPhase 3: Parallel memory operations...\n");
    parallel_memory_ops();
    
    /* Phase 4: Edge case with goto and memory ops */
    printf("\nPhase 4: Control flow edge cases...\n");
    
    int goto_test = 0;
    char goto_buf1[100];
    char goto_buf2[100];
    
    __builtin_memset(goto_buf1, 'A', sizeof(goto_buf1));
    
    if (volatile_flag) {
        goto perform_memmove;
    }
    
    __builtin_memcpy(goto_buf2, goto_buf1, 50);
    goto_test = 1;
    
perform_memmove:
    /* This label tests goto into memmove context */
    __builtin_memmove(goto_buf1 + 20, goto_buf1, 60);
    
    /* Calculate final hash */
    unsigned long final_hash = 0;
    for (int i = 0; i < sizeof(goto_buf1); i++) {
        final_hash = final_hash * 31 + goto_buf1[i];
    }
    final_hash += ast_result;
    
    printf("\nFinal verification hash: %lu\n", final_hash);
    printf("Test completed successfully!\n");
    
    /* Cleanup */
    /* Note: In real code, would need to free AST recursively */
    
    return 0;
}
