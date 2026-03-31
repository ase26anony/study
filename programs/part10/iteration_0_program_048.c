/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
struct ASTNode {
    int type;
    char data[32];
    struct ASTNode *left;
    struct ASTNode *right;
};

/* Attribute constructors/destructors for initialization complexity */
void __attribute__((constructor)) init_globals(void) {
    printf("Constructor: Initializing memory subsystem\n");
}

void __attribute__((destructor)) cleanup_globals(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive function with memory operations */
struct ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    struct ASTNode* node = malloc(sizeof(struct ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset with volatile size */
    __builtin_memset(node, 0, sizeof(struct ASTNode));
    
    node->type = depth;
    
    /* Use builtin memcpy with non-foldable pattern */
    const char* pattern = "AST_NODE_DATA_";
    __builtin_memcpy(node->data, pattern, 
                     g_mem_size < 32 ? g_mem_size : 15);
    
    /* Recursive creation with goto for flow complexity */
    int create_children = 1;
    
    if (depth > 2) {
        goto create_left;
    } else {
        node->left = NULL;
        goto create_right;
    }
    
create_left:
    node->left = create_ast(depth - 1);
    
create_right:
    node->right = create_ast(depth - 2);
    
    return node;
}

/* Function with goto jumping around memmove */
void process_with_goto(struct ASTNode* src, struct ASTNode* dst) {
    if (!src || !dst) return;
    
    int stage = 0;
    
start:
    if (stage == 0) {
        /* Jump over memmove */
        goto skip_memmove;
    }
    
    /* This memmove should be reachable via goto */
    if (g_use_memmove) {
        __builtin_memmove(dst->data, src->data, g_mem_size < 32 ? g_mem_size : 32);
    }
    
skip_memmove:
    if (stage == 0) {
        stage = 1;
        /* Jump back to execute memmove */
        goto start;
    }
    
    /* Another memmove after the goto logic */
    if (dst->left && src->left) {
        __builtin_memmove(dst->left->data, src->left->data, 16);
    }
}

/* OpenMP parallel section with memory operations */
void parallel_memory_ops(struct ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Force builtin usage in parallel context */
            char buffer[64];
            volatile size_t local_size = g_mem_size;
            
            __builtin_memset(buffer, i, local_size < 64 ? local_size : 64);
            
            if (i > 0 && nodes[i-1]) {
                __builtin_memcpy(nodes[i]->data, buffer, 32);
            }
            
            /* Conditional memmove */
            if (i % 3 == 0 && i + 1 < count && nodes[i+1]) {
                __builtin_memmove(nodes[i+1]->data, nodes[i]->data, 24);
            }
        }
    }
}

/* Complex initialization with varied memory operations */
void initialize_token_array(char** tokens, int token_count) {
    for (int i = 0; i < token_count; i++) {
        tokens[i] = malloc(128);
        if (tokens[i]) {
            /* Pattern of different builtin uses */
            switch (i % 4) {
                case 0:
                    __builtin_memset(tokens[i], 'A', g_mem_size < 128 ? g_mem_size : 100);
                    break;
                case 1:
                    if (i > 0) {
                        __builtin_memcpy(tokens[i], tokens[i-1], 64);
                    }
                    break;
                case 2:
                    if (i > 1) {
                        __builtin_memmove(tokens[i], tokens[i-2], 32);
                    }
                    break;
                case 3:
                    /* Mixed operations */
                    __builtin_memset(tokens[i], 'Z', 16);
                    if (i > 0) {
                        __builtin_memcpy(tokens[i] + 16, tokens[i-1] + 16, 16);
                    }
                    break;
            }
        }
    }
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Create recursive structures */
    struct ASTNode* root = create_ast(4);
    struct ASTNode* copy = malloc(sizeof(struct ASTNode));
    
    if (root && copy) {
        /* Test goto flow with memmove */
        __builtin_memcpy(copy, root, sizeof(struct ASTNode));
        process_with_goto(root, copy);
        
        /* Create array for parallel processing */
        struct ASTNode* nodes[8];
        nodes[0] = root;
        nodes[1] = copy;
        for (int i = 2; i < 8; i++) {
            nodes[i] = create_ast(3);
        }
        
        /* OpenMP parallel operations */
        parallel_memory_ops(nodes, 8);
        
        /* Complex token array */
        char* tokens[16];
        initialize_token_array(tokens, 16);
        
        /* Compute verification hash */
        unsigned long hash = 0;
        for (int i = 0; i < 8 && nodes[i]; i++) {
            for (int j = 0; j < 32; j++) {
                hash += (unsigned long)nodes[i]->data[j];
            }
        }
        
        for (int i = 0; i < 16 && tokens[i]; i++) {
            for (int j = 0; j < 64; j++) {
                hash += (unsigned long)tokens[i][j];
            }
            free(tokens[i]);
        }
        
        printf("Verification hash: %lu\n", hash);
        
        /* Cleanup */
        free(copy);
        for (int i = 2; i < 8; i++) {
            free(nodes[i]);
        }
    }
    
    /* Free root recursively */
    /* In production code, implement proper recursive free */
    free(root);
    
    return 0;
}
