/* complex_gty_test.c - Test file for exercising gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with deeply nested parameter lists */
typedef void (*GTY(()) complex_func_ptr)(
    int (*GTY(()) nested_callback)(
        char GTY(()) param1[10],
        struct GTY(()) { 
            int x; 
            union GTY(()) { 
                float f; 
                double d; 
            } u; 
        } param2
    ),
    int arr[5][(10 + sizeof(struct { char c; }))]
);

/* Test 2: Structure with multiple nested delimiter types */
struct GTY((chain_next, chain_prev)) outer_struct {
    /* Function pointer array with complex signature */
    void (*GTY(()) fn_array[5])(
        int GTY(()) matrix[][10],
        struct GTY(()) inner {
            int count;
            char * GTY((length("count"))) data;
        } GTY(()) *config
    );
    
    /* Nested union with array of function pointers */
    union GTY(()) {
        int (*GTY(()) callbacks[3])(
            char GTY(()) buffer[],
            struct GTY(()) { 
                int flags; 
                void (*GTY(()) action)(void); 
            } GTY(()) ctx
        );
        struct GTY(()) {
            int dimensions[2][(sizeof(int*) * 4)];
            void (*GTY(()) initialize)(
                int GTY(()) params[],
                struct GTY(()) { 
                    int x; 
                    int y; 
                    int z; 
                } GTY(()) coords
            );
        } GTY(()) setup;
    } GTY(()) nested_union;
    
    /* Complex array declaration with nested size expression */
    unsigned char GTY(()) bitfield_data[
        (1 << (sizeof(int) * 8 - 1)) / 
        (sizeof(struct GTY(()) { char a; short b; int c; }) + 1)
    ];
};

/* Test 3: Typedef chain with multiple balanced delimiters */
typedef struct GTY(()) node {
    struct node * GTY((skip)) next;
    struct node * GTY((skip)) prev;
    enum GTY(()) node_type {
        TYPE_FUNC = 1,
        TYPE_ARRAY = 2,
        TYPE_STRUCT = 3
    } GTY(()) type;
    
    /* Anonymous union with complex members */
    union GTY(()) {
        struct GTY(()) {
            void (*GTY(()))(*func_ptr)(
                int, 
                char GTY(()) *argv[],
                ... /* variadic function */
            );
            int param_count;
        } GTY(()) func_info;
        
        struct GTY(()) {
            int * GTY((length("dim[0]"))) elements;
            int dim[3];
            struct GTY(()) bounds {
                int lower;
                int upper;
            } GTY(()) range[2];
        } GTY(()) array_info;
        
        struct GTY(()) {
            char * GTY(()) name;
            struct GTY(()) field {
                char * GTY(()) field_name;
                int offset;
                int size;
            } GTY(()) *fields;
            int field_count;
        } GTY(()) struct_info;
    } GTY(()) data;
} *GTY(()) node_ptr;

/* Test 4: Complex initializer with nested braces */
static const struct GTY(()) complex_init_example {
    int matrix[2][3];
    struct GTY(()) point {
        int x;
        int y;
        int z;
    } GTY(()) points[2];
    void (*GTY(()) operations[2])(void);
} GTY(()) init_example = {
    .matrix = {
        {1, 2, 3},
        {4, 5, 6}
    },
    .points = {
        { .x = 10, .y = 20, .z = 30 },
        { .x = 40, .y = 50, .z = 60 }
    },
    .operations = {
        (void (*)(void))0x1000,
        (void (*)(void))0x2000
    }
};

/* Test 5: Template-like macro with nested delimiters */
#define DECLARE_CONTAINER(TYPE, SIZE) \
    struct GTY(()) container_##TYPE { \
        TYPE GTY(()) items[(SIZE)]; \
        int (*GTY(()) compare)(TYPE GTY(()) a, TYPE GTY(()) b); \
        struct GTY(()) { \
            int capacity; \
            TYPE * GTY((length("capacity"))) buffer; \
        } GTY(()) storage; \
    }

/* Instantiate the macro with complex types */
DECLARE_CONTAINER(
    struct GTY(()) { 
        int id; 
        char * GTY(()) name; 
        float scores[3]; 
    },
    (10 * sizeof(void*)) / sizeof(int)
);

/* Test 6: Recursive structure with function pointer containing array parameters */
struct GTY(()) tree_node {
    char * GTY(()) name;
    struct tree_node * GTY((skip)) children;
    struct tree_node * GTY((skip)) parent;
    
    /* Function pointer with array parameter and nested structure */
    int (*GTY(()) traverse)(
        struct tree_node * GTY(()) nodes[],
        int (*GTY(()))(*visitor)(
            struct tree_node * GTY(()) node,
            void * GTY(()) context,
            int depth
        ),
        struct GTY(()) {
            int max_depth;
            int options;
            void (*GTY(()) callback)(int, char *[]);
        } GTY(()) config
    );
    
    /* Array of function pointers with complex signatures */
    void (*GTY(()) handlers[4])(
        struct GTY(()) event {
            int type;
            union GTY(()) {
                int int_val;
                float float_val;
                char * GTY(()) str_val;
                struct GTY(()) { int x; int y; } GTY(()) coords;
            } GTY(()) data;
        } GTY(()) *event_ptr
    );
};

/* Test 7: Union containing nested structures with all delimiter types */
union GTY(()) mega_union {
    /* Case 1: Function pointer with nested parameter list */
    int (*GTY(()) complex_fn)(
        int matrix[3][(2 + sizeof(double))],
        struct GTY(()) {
            void (*GTY(()) action)(int, char *[]);
            int params[2];
        } GTY(()) *ctx
    );
    
    /* Case 2: Structure with array of function pointers */
    struct GTY(()) {
        void (*GTY(()) (*callback_table[5]))(
            int,
            struct GTY(()) { int a; int b; } GTY(()) pair
        );
        int settings[(sizeof(void*) * 8)];
    } GTY(()) handler_set;
    
    /* Case 3: Nested anonymous structure with initializer-like braces */
    struct GTY(()) {
        struct GTY(()) nested {
            int values[4];
            struct GTY(()) { int start; int end; } GTY(()) range;
        } GTY(()) data[2];
        char flags[((1 << 4) - 1)];
    } GTY(()) config_block;
};

/* Test 8: Typedef with deeply nested parentheses */
typedef int (*(*(*GTY(()) deep_func_ptr)(void))[5])(
    char GTY(()) *args[],
    struct GTY(()) context {
        int id;
        void * GTY(()) user_data;
        struct GTY(()) {
            int (*GTY(()) get_value)(void);
            void (*GTY(()) set_value)(int);
        } GTY(()) ops;
    } GTY(()) *ctx_ptr
);

/* Main function to make the file compilable */
int main(void) {
    /* Empty main - the purpose is to exercise gengtype parser */
    return 0;
}
