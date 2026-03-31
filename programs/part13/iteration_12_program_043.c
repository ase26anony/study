/* complex_gty_test.c - Test file for exercising gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.c"

/* Test 1: Function pointer with nested parameter lists */
typedef void (*GTY((user)) complex_func_ptr)(
    int (*GTY((skip)) callback)(char GTY((length("10"))) [10]),
    struct GTY(()) inner_struct { 
        int x; 
        void (*nested_fn)(int[5]); 
    }
);

/* Test 2: Deeply nested parentheses in array dimensions */
struct GTY(()) outer_struct {
    /* Array with expression containing parentheses */
    int arr1[GTY((user)) (10 + sizeof(struct GTY(()) temp { int a; }))];
    
    /* Multi-dimensional array with nested brackets */
    char arr2[5][(2 * 3)][sizeof(int*)];
    
    /* Function pointer array with complex signature */
    void (*func_array[3])(
        struct GTY(()) param1 { 
            int x[(2+3)]; 
        }, 
        int (*)(char[][10])
    );
};

/* Test 3: Nested structures with all delimiter types */
union GTY(()) complex_union {
    struct GTY(()) nested1 {
        /* Braces within initializer */
        int x GTY((default("0")));
        
        /* Parentheses in bit-field */
        unsigned int bits : (sizeof(int) * 8 - 1);
        
        /* Array of function pointers */
        int (*callbacks[5])(void);
    } s;
    
    struct GTY(()) nested2 {
        /* Nested array declaration */
        double matrix[3][(2+2)];
        
        /* Pointer to array */
        int (*ptr_to_arr)[10];
        
        /* Anonymous struct with initializer-like syntax in GTY */
        struct GTY((user)) { 
            short a; 
            long b[(4)]; 
        } anon;
    } t;
};

/* Test 4: Template-like macro expansion with delimiters */
#define DECLARE_VECTOR(TYPE, SIZE) \
    struct GTY(()) vector_##TYPE { \
        TYPE data[(SIZE)]; \
        int (*compare)(TYPE*, TYPE*); \
    }

DECLARE_VECTOR(int, 10);
DECLARE_VECTOR(double, (5+5));

/* Test 5: Chain of pointers with nested attributes */
struct GTY((chain_next("next"), chain_prev("prev"))) linked_node {
    int value;
    struct linked_node * GTY((skip)) next;
    struct linked_node *prev;
    
    /* Callback with complex return type */
    struct GTY(()) result* (*processor)(
        int param[(sizeof(int)+2)],
        void (*cleanup)(struct GTY(()) cleanup_data*)
    );
};

/* Test 6: Multiple balanced delimiters in single declaration */
struct GTY(()) delimiter_test {
    /* Combination: (*, ), [, ], (, ), {, } */
    void (*complex_member[3])(
        int arg1,
        struct GTY(()) { 
            char data[10]; 
            int* ptrs[(5)]; 
        } arg2
    );
    
    /* Nested initializer-like syntax in type */
    struct GTY((user)) initialized {
        int x;
        int y;
    } values[2] GTY((user)) = { {0, 1}, {2, 3} };
};

/* Test 7: Recursive structure with function pointers */
struct GTY(()) tree_node {
    int data;
    struct tree_node* GTY((user)) left;
    struct tree_node* right;
    
    /* Visitor function with nested parameter */
    void (*visit)(
        struct tree_node*,
        void (*action)(int, char[10])
    );
    
    /* Array of child processors */
    int (*children[5])(
        struct GTY(()) child_params {
            int count;
            char* names[];
        }
    );
};

/* Test 8: Union with anonymous structs and arrays */
union GTY(()) mixed_types {
    /* Anonymous struct with array */
    struct GTY((user)) {
        float coords[3][(2+1)];
        void (*transform)(float[][3]);
    };
    
    /* Function pointer union member */
    union GTY((user)) func_union {
        int (*as_int)(void);
        double (*as_double)(int[(3)]);
    } funcs;
    
    /* Nested array with parenthesized size */
    char buffer[sizeof(struct { int a; double b; })];
};

/* Test 9: Attribute list with nested parentheses */
typedef struct GTY((
    user,
    desc("%1.data"),
    maybe_undef
)) attributed_struct {
    int data[(5 * 2)];
    struct attributed_struct* GTY((skip)) next;
    
    /* Method with attributes */
    int (* GTY((user)) method)(
        struct GTY(()) params { int x; }*
    ) GTY((default("NULL")));
} attributed_struct_t;

/* Test 10: Multiple levels of nesting */
struct GTY(()) level1 {
    struct GTY(()) level2 {
        struct GTY(()) level3 {
            int (*level4[2])(
                struct GTY(()) level5 {
                    char data[ { 10 } ];  /* Braces in array size */
                }*
            );
            
            union GTY(()) level5_union {
                int a;
                struct GTY(()) { short b; long c; } nested;
            } u;
        } l3;
        
        /* Array with designators */
        int arr[5] GTY((user)) = { [0] = 1, [4] = 5 };
    } l2;
    
    /* Complex function pointer signature */
    void (*final_callback)(
        int,
        ...  /* Variadic - will have special handling */
    );
};

/* Test 11: Initialize static variable with nested braces */
static struct GTY(()) static_var = {
    .x = 10,
    .y = { 
        .a = 20, 
        .b = { 1, 2, 3 } 
    },
    .z = (struct GTY(()) inner { int i; }){ .i = 30 }
};

/* Test 12: Macro that expands to delimiter-heavy code */
#define DEFINE_CALLBACK(NAME, RET, PARAMS) \
    RET (*GTY((user)) NAME) PARAMS

DEFINE_CALLBACK(my_callback, int, (int a, char b[(10)]));

/* Test 13: All three delimiters in sequence */
struct GTY(()) all_delimiters {
    /* Pattern: ( [ { } ] ) */
    int (*func1)(int arr[3][ { 2 } ]);
    
    /* Pattern: { ( [ ] ) } */
    struct GTY(()) { 
        void (*fn)(short data[(5)]); 
    } container;
    
    /* Pattern: [ ( { } ) ] */
    void (*func_array2[(2)])(
        struct GTY(()) { int x; } param
    );
};

/* Main function to make file compilable */
int main() {
    return 0;
}
