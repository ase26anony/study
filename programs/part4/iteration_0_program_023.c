/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int use_memcpy = 1;
volatile int use_memset = 0;
volatile int use_memmove = 1;

/* Recursive AST-like structure */
struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early() {
    volatile char buffer[128];
    /* Force builtin initialization early */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 32, buffer, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_late() {
    volatile char final_check[64];
    __builtin_memset(final_check, 0xFF, 32);
}

/* Recursive function with memory operations */
struct ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    struct ASTNode* node = (struct ASTNode*)malloc(sizeof(struct ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in data */
    for (int i = 0; i < 32; i++) {
        node->data[i] = (char)(id + i);
    }
    
    node->id = id;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    } else {
        node->left = NULL;
        node->right = NULL;
        goto finish_node;
    }
    
create_children:
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    /* Copy data between nodes using builtins */
    if (node->left && node->right) {
        __builtin_memcpy(node->right->data + 64, node->left->data, 32);
        __builtin_memmove(node->left->data + 32, node->left->data, 16);
    }
    
finish_node:
    return node;
}

/* Complex token array initialization */
void init_token_array(char* tokens, size_t size) {
    volatile size_t local_len = volatile_len;
    
    /* Use all three builtins in sequence */
    __builtin_memset(tokens, 0x00, size);
    
    for (size_t i = 0; i < size / 2; i++) {
        tokens[i] = (char)(i % 256);
    }
    
    __builtin_memcpy(tokens + size/2, tokens, size/4);
    __builtin_memmove(tokens + size/4, tokens + size/2, size/8);
}

/* Parallel memory operations */
void parallel_memory_ops(struct ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Mix of memory operations based on volatile conditions */
                if (use_memcpy) {
                    __builtin_memcpy(nodes[i]->data + 128, 
                                   nodes[i]->data, 
                                   volatile_len % 64);
                }
                
                if (use_memset) {
                    __builtin_memset(nodes[i]->data + 192, 
                                   thread_id, 
                                   32);
                }
                
                /* Goto for flow control around memmove */
                if (use_memmove && i % 2 == 0) {
                    goto do_memmove;
                } else {
                    goto skip_memmove;
                }
                
            do_memmove:
                __builtin_memmove(nodes[i]->data + 64,
                                nodes[i]->data + 32,
                                16);
                goto after_memmove;
                
            skip_memmove:
                /* Alternative operation */
                nodes[i]->data[0] = (char)thread_id;
                
            after_memmove:
                /* Continue with other operations */
                ;
            }
        }
    }
}

/* Calculate hash from AST */
unsigned long calculate_ast_hash(struct ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* p = node->data;
    
    /* Simple hash calculation */
    for (size_t i = 0; i < sizeof(node->data) && *p; i++) {
        hash = ((hash << 5) + hash) + *p++;
    }
    
    hash += calculate_ast_hash(node->left);
    hash += calculate_ast_hash(node->right);
    
    return hash;
}

int main(void) {
    const size_t token_size = 4096;
    char* tokens = (char*)malloc(token_size);
    if (!tokens) return 1;
    
    /* Initialize token array */
    init_token_array(tokens, token_size);
    
    /* Create AST structure */
    struct ASTNode* root = create_ast(4, 1);
    
    /* Create array of nodes for parallel processing */
    struct ASTNode* nodes[8];
    nodes[0] = root;
    
    /* Build additional nodes */
    for (int i = 1; i < 8; i++) {
        nodes[i] = create_ast(3, i + 100);
    }
    
    /* Perform parallel memory operations */
    parallel_memory_ops(nodes, 8);
    
    /* Additional builtin calls in main */
    volatile char temp_buf[256];
    __builtin_memset(temp_buf, 0, sizeof(temp_buf));
    __builtin_memcpy(temp_buf, tokens, 128);
    __builtin_memmove(temp_buf + 128, temp_buf, 64);
    
    /* Calculate and print result */
    unsigned long total_hash = 0;
    for (int i = 0; i < 8; i++) {
        total_hash ^= calculate_ast_hash(nodes[i]);
    }
    
    /* Include token array in hash */
    for (size_t i = 0; i < token_size; i++) {
        total_hash = ((total_hash << 3) + total_hash) + tokens[i];
    }
    
    printf("Result hash: %lu\n", total_hash);
    
    /* Cleanup */
    free(tokens);
    
    /* Free AST nodes recursively */
    for (int i = 0; i < 8; i++) {
        /* Simple cleanup - in real code would need proper recursive free */
        free(nodes[i]);
    }
    
    return 0;
}
