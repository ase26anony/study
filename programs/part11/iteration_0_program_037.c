/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_selector = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
char global_tokens[1024];
int token_hash = 0;

/* Constructor/destructor functions */
__attribute__((constructor)) void init_global_tokens(void) {
    /* Use memset in constructor */
    __builtin_memset(global_tokens, 'A', sizeof(global_tokens));
    global_tokens[sizeof(global_tokens)-1] = '\0';
}

__attribute__((destructor)) void cleanup_resources(void) {
    /* Use memcpy in destructor */
    char buffer[64];
    __builtin_memcpy(buffer, global_tokens, 63);
    buffer[63] = '\0';
}

/* Recursive parser with memory operations */
ASTNode* create_ast_node(const char* src, size_t len) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use memcpy with volatile length */
    size_t copy_len = len < 255 ? len : 255;
    __builtin_memcpy(node->data, src, copy_len);
    node->data[copy_len] = '\0';
    node->size = copy_len;
    node->left = node->right = NULL;
    
    return node;
}

void copy_ast_nodes(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use memmove for overlapping copy scenario */
    if (dest->data + 10 == src->data) {
        __builtin_memmove(dest->data, src->data, 
                         src->size < 200 ? src->size : 200);
    } else {
        __builtin_memcpy(dest->data, src->data, 
                        src->size < 200 ? src->size : 200);
    }
}

/* Function with goto jumps around memmove */
void goto_memmove_test(char* buf1, char* buf2, size_t len) {
    int use_memmove = 0;
    
    if (volatile_selector > 0) {
        use_memmove = 1;
        goto do_operation;
    }
    
    /* Normal path */
    __builtin_memset(buf1, 'X', len);
    goto skip_operation;
    
do_operation:
    /* Jump target with memmove */
    __builtin_memmove(buf2, buf1, len);
    
skip_operation:
    /* Use memset after label */
    __builtin_memset(buf1 + len/2, 'Y', len/4);
}

/* OpenMP parallel memory operations */
void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[128];
        char shared_buf[256];
        
        /* Each thread uses memory builtins */
        __builtin_memset(local_buf, tid, sizeof(local_buf));
        
        #pragma omp barrier
        
        #pragma omp for
        for (int i = 0; i < 16; i++) {
            char temp[32];
            __builtin_memset(temp, i, 32);
            __builtin_memcpy(&shared_buf[i*16], temp, 16);
            
            if (i % 3 == 0) {
                __builtin_memmove(&shared_buf[i*8], &shared_buf[i*16], 8);
            }
        }
    }
}

/* Multi-stage initialization */
void initialize_system(void) {
    /* Force multiple builtin calls */
    char init_buf[512];
    
    /* Chain of memory operations */
    __builtin_memset(init_buf, 0, sizeof(init_buf));
    __builtin_memcpy(init_buf, global_tokens, 256);
    __builtin_memmove(init_buf + 128, init_buf, 128);
    
    /* Recursive structure creation */
    ASTNode* root = create_ast_node(init_buf, 128);
    ASTNode* child = create_ast_node(init_buf + 64, 64);
    
    if (root && child) {
        copy_ast_nodes(root, child);
        
        /* Goto test */
        goto_memmove_test(root->data, child->data, 50);
        
        free(child);
        free(root);
    }
}

int main(void) {
    /* Initialize token hash */
    for (size_t i = 0; i < sizeof(global_tokens); i++) {
        token_hash += global_tokens[i];
    }
    
    printf("Initial token hash: %d\n", token_hash);
    
    /* Stage 1: Basic builtin calls */
    char buffer1[256];
    char buffer2[256];
    
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 64, buffer1, 128);
    
    /* Stage 2: OpenMP parallel section */
    parallel_memory_ops();
    
    /* Stage 3: Complex initialization */
    initialize_system();
    
    /* Stage 4: Volatile-controlled operations */
    size_t dyn_len = volatile_len % 128;
    char dyn_buf[128];
    
    __builtin_memset(dyn_buf, 0xFF, dyn_len);
    __builtin_memcpy(dyn_buf + 32, dyn_buf, dyn_len > 32 ? 32 : dyn_len);
    
    /* Final verification */
    int final_hash = token_hash;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        final_hash += buffer1[i];
    }
    
    printf("Final computed hash: %d\n", final_hash);
    printf("Program completed successfully.\n");
    
    return 0;
}
