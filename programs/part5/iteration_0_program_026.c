/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
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

/* Global token array */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "instrument"
};
static const int token_count = 6;

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_sanitizer_hook(void) {
    printf("Constructor: Initializing sanitizer hooks\n");
    /* Force initialization of sanitizer runtime */
    volatile char dummy[16];
    __builtin_memset(dummy, 0, sizeof(dummy));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_sanitizer_hook(void) {
    printf("Destructor: Cleaning up sanitizer state\n");
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth, int* counter) {
    if (depth <= 0 || *counter >= 100) {
        ASTNode* leaf = malloc(sizeof(ASTNode));
        if (!leaf) return NULL;
        
        /* Use __builtin_memset to initialize node */
        __builtin_memset(leaf, 0, sizeof(ASTNode));
        leaf->id = (*counter)++;
        
        /* Copy token data using __builtin_memcpy */
        int token_idx = leaf->id % token_count;
        __builtin_memcpy(leaf->data, tokens[token_idx], 
                        strlen(tokens[token_idx]) + 1);
        
        return leaf;
    }
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with __builtin_memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = (*counter)++;
    
    /* Build left and right subtrees */
    node->left = parse_expression(depth - 1, counter);
    node->right = parse_expression(depth - 1, counter);
    
    /* Copy data between nodes using goto for flow control */
    int use_goto = (node->id % 3 == 0);
    
    if (use_goto) {
        goto copy_block;
    } else {
        /* Direct copy */
        if (node->left && node->right) {
            __builtin_memcpy(node->data, node->left->data, 64);
        }
        goto skip_copy;
    }
    
copy_block:
    /* This block tests goto into memory operation */
    if (node->left && node->right) {
        /* Use __builtin_memmove for overlapping regions */
        __builtin_memmove(node->data, node->left->data, 32);
        __builtin_memmove(node->data + 32, node->right->data, 32);
    }
    
skip_copy:
    return node;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

/* Compute hash of AST */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    while (*ptr) {
        hash = ((hash << 5) + hash) + *ptr++;
    }
    
    hash ^= compute_ast_hash(node->left);
    hash ^= compute_ast_hash(node->right);
    hash ^= node->id;
    
    return hash;
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_operations(void) {
    const int buffer_count = 8;
    char* buffers[buffer_count];
    
    /* Allocate buffers */
    for (int i = 0; i < buffer_count; i++) {
        buffers[i] = malloc(g_mem_size);
        if (!buffers[i]) {
            fprintf(stderr, "Failed to allocate buffer %d\n", i);
            return;
        }
    }
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        #pragma omp for
        for (int i = 0; i < buffer_count; i++) {
            /* Each thread performs different memory operations */
            switch (i % 3) {
                case 0:
                    /* memset pattern */
                    __builtin_memset(buffers[i], thread_id + 'A', g_mem_size);
                    break;
                case 1:
                    /* memcpy between buffers */
                    if (i > 0) {
                        __builtin_memcpy(buffers[i], buffers[i-1], g_mem_size);
                    }
                    break;
                case 2:
                    /* memmove with overlap */
                    if (i > 0 && g_mem_size > 32) {
                        __builtin_memmove(buffers[i] + 16, buffers[i], g_mem_size - 16);
                    }
                    break;
            }
            
            /* Additional goto-controlled memory operation */
            if (thread_id % 2 == 0) {
                goto do_extra_copy;
            } else {
                goto skip_extra;
            }
            
        do_extra_copy:
            /* This tests goto in parallel region */
            if (i + 1 < buffer_count) {
                __builtin_memcpy(buffers[i] + 64, buffers[i+1], 32);
            }
            goto end_label;
            
        skip_extra:
            /* Alternative path */
            __builtin_memset(buffers[i] + 96, 'X', 32);
            
        end_label:
            /* Continue execution */
            ;
        }
    }
    
    /* Verify and compute checksum */
    unsigned long checksum = 0;
    for (int i = 0; i < buffer_count; i++) {
        for (size_t j = 0; j < g_mem_size; j++) {
            checksum += (unsigned char)buffers[i][j];
        }
        free(buffers[i]);
    }
    
    printf("Parallel checksum: %lu\n", checksum);
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Build and process AST */
    int counter = 0;
    ASTNode* root = parse_expression(4, &counter);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    unsigned long ast_hash = compute_ast_hash(root);
    printf("AST hash: %lu (nodes: %d)\n", ast_hash, counter);
    
    /* Phase 2: Perform parallel memory operations */
    parallel_memory_operations();
    
    /* Phase 3: Additional built-in usage with volatile control */
    volatile char src[128];
    volatile char dst[128];
    
    /* Initialize with volatile pattern */
    for (volatile int i = 0; i < 128; i++) {
        src[i] = (char)(i % 26 + 'A');
    }
    
    /* Force all three built-ins with goto */
    int operation = 0;
    
operation_switch:
    switch (operation) {
        case 0:
            __builtin_memcpy((void*)dst, (void*)src, 128);
            operation++;
            goto operation_switch;
        case 1:
            __builtin_memset((void*)dst, 0x42, 64);
            operation++;
            goto operation_switch;
        case 2:
            __builtin_memmove((void*)(dst + 32), (void*)dst, 64);
            break;
        default:
            break;
    }
    
    /* Compute final verification sum */
    unsigned long final_sum = ast_hash;
    for (volatile int i = 0; i < 128; i++) {
        final_sum += (unsigned char)dst[i];
    }
    
    printf("Final verification sum: %lu\n", final_sum);
    
    /* Cleanup */
    free_ast(root);
    
    return 0;
}
