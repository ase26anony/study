/* complex_gty_test.c - Test file for gengtype delimiter parsing coverage */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Complex function pointer with nested parentheses */
typedef void (*GTY((user)) complex_func_ptr)(
    int (*GTY((skip)) nested_callback)(char param[10]),
    struct GTY(()) inner_struct { int x; }
);

/* Test 2: Array with complex dimension expression containing parentheses */
struct GTY(()) array_test {
    int arr1[(10 + sizeof(struct GTY(()) size_struct { int a; double b; }))];
    char arr2[5][(sizeof(int) * 2)];
};

/* Test 3: Nested structures with multiple delimiter types */
struct GTY(()) outer_struct {
    /* Function pointer array */
    void (*GTY((chain_next)) fp_array[5])(int matrix[][10]);
    
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int flags : 4;
            unsigned int : 4;  /* unnamed bit-field */
        } GTY((tag("0"))) bits;
        int GTY((default)) raw_value;
    } GTY((desc("%1.bits.flags"))) bit_union;
    
    /* Pointer to array of function pointers */
    int (*(*GTY((skip)) complex_ptr)[3])(float, double);
};

/* Test 4: Deeply nested parentheses in type declarations */
typedef int (*(*(*GTY((user)) deep_nested_fp)(void))[5])(
    struct GTY(()) param_struct {
        int data[10];
        void (*GTY((callback)) handler)(int, char);
    } *
);

/* Test 5: Multiple balanced delimiters in single declaration */
struct GTY(()) delimiter_mix {
    /* Contains: (*, ), [5], (, [][10], ) */
    void (*GTY((chain_prev, chain_next)) fn_matrix[3][2])(
        int (*GTY((skip)) param_cb)(char buffer[]),
        double coefficients[][10]
    );
    
    /* Initializer with nested braces */
    struct GTY(()) init_example {
        int x;
        int y;
        int z;
    } GTY((default)) instance = { 
        .x = 10, 
        .y = {20, 30, 40},  /* Nested initializer */
        .z = 50 
    };
};

/* Test 6: Template-like pattern simulation using macros */
#define GTY_TEMPLATE(type) struct GTY(()) template_##type { type data; }

GTY_TEMPLATE(int);
GTY_TEMPLATE(double);
GTY_TEMPLATE(char*);

/* Test 7: Union with complex nested types */
union GTY((desc("%0.kind"))) complex_union {
    enum { KIND_INT, KIND_PTR, KIND_ARRAY } kind;
    
    struct GTY((tag("KIND_INT"))) int_data {
        int value;
        int (*GTY((skip)) validators[3])(int);
    } id;
    
    struct GTY((tag("KIND_PTR"))) ptr_data {
        void *GTY((skip)) ptr;
        size_t size;
        char description[(sizeof("Complex pointer") + 10)];
    } pd;
    
    struct GTY((tag("KIND_ARRAY"))) array_data {
        int *GTY((length("%0.count"))) elements;
        int count;
        int dimensions[3];
    } ad;
};

/* Test 8: Recursive structure with function pointers */
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((left)) left;
    struct tree_node *GTY((right)) right;
    void (*GTY((callback)) visit)(struct tree_node *, 
                                  void (*)(int, char[10]));
};

/* Test 9: Attribute lists within nested contexts */
struct GTY(()) container {
    struct GTY((for_user)) nested {
        char *GTY((length("%0.len"))) data;
        int len;
        void (*GTY((user)) processors[2])(
            struct nested *,
            int options[(sizeof(int) * 4)]
        );
    } *GTY((chain_next)) items;
    
    /* GTY annotation with multiple nested attributes */
    struct nested *GTY((chain_prev("prev"), 
                       chain_next("next"), 
                       skip("skip"))) current;
};

/* Test 10: Multiple levels of nested delimiters */
typedef void (*(*(*GTY((user)) extreme_nesting)[
    (sizeof(struct { int a; double b; }) > 10 ? 5 : 3)
])(
    int (*(*callbacks)[2])(
        char param[][(10 + sizeof(int))],
        void (*)(int, int)
    )
))[10];

/* Test 11: Mixed delimiters with initializers */
static struct GTY(()) initialized_struct {
    int matrix[2][3] = {
        {1, 2, {3, 4, 5}},  /* Nested initializer */
        {6, 7, 8}
    };
    
    struct GTY(()) point {
        int x;
        int y;
    } points[3] = {
        [0] = {.x = 10, .y = 20},
        [1] = {.x = 30, .y = {40, 50}},  /* Another nested initializer */
        [2] = {.x = 60, .y = 70}
    };
    
    void (*GTY((default)) handlers[2])(int) = { NULL, NULL };
} GTY((user)) global_instance;

/* Test 12: Complex typedef with all delimiter types */
typedef union GTY((desc("%0.type"))) {
    int type;
    struct GTY((tag("1"))) {
        int (*GTY((skip)) compare)(const void *, const void *);
        void *GTY((length("%0.size"))) data;
        size_t size;
    } ptr_data;
    struct GTY((tag("2"))) {
        int array[10][(sizeof(double) + 2)];
        void (*GTY((user)) mapper)(int[][10]);
    } array_data;
} generic_container_t;

/* Test 13: Structure with embedded GTY annotations in nested positions */
struct GTY(()) master_struct {
    /* Field with GTY annotation containing nested parentheses */
    struct GTY((chain_next("next"), 
               chain_prev("prev"),
               skip("if (%0.data == NULL)"))) slave {
        void *GTY((user)) data;
        int size;
        void (*GTY((callback)) cleanup)(struct slave *, 
                                       int options[(5 * sizeof(void*))]);
    } *GTY((length("%0.count"))) slaves;
    
    int count;
    
    /* Array of function pointers with complex signatures */
    int (*(*GTY((skip)) operations[3])(int, void*))[
        (sizeof(struct { char a; int b; }) + 10)
    ];
};

/* Test 14: Multiple GTY annotations at different nesting levels */
struct GTY(()) level1 {
    struct GTY((for_user)) level2 {
        struct GTY((tag("inner"))) level3 {
            char *GTY((length("%0.len"), skip)) buffer;
            int len;
            void (*GTY((user)) processors[2])(
                struct level3 *self,
                int params[][(10 * sizeof(char))]
            );
        } *GTY((chain_next)) inner;
        
        int depth;
    } *GTY((chain_prev)) middle;
    
    float ratio;
};

/* Test 15: Final comprehensive test with all delimiter patterns */
struct GTY(()) comprehensive_test {
    /* 1. Function pointer with nested parameter list */
    void (*GTY((user)) startup)(int argc, 
                               char *argv[],
                               void (*GTY((callback)) init)(void));
    
    /* 2. Array with computed size containing parentheses */
    int buffer[(MAX(10, 20) * sizeof(struct GTY(()) temp { int x; }))];
    
    /* 3. Nested structure initializer */
    struct GTY(()) config {
        int values[3];
        struct {
            char *name;
            int id;
        } GTY((skip)) entries[5];
    } GTY((default)) settings = {
        .values = {1, 2, 3},
        .entries = {{"a", 1}, {"b", 2}, {"c", 3}, {"d", 4}, {"e", 5}}
    };
    
    /* 4. Complex pointer to array of function pointers */
    char (*(*GTY((chain_next)) string_ops[2][3])(int, float))[];
    
    /* 5. Union with anonymous struct containing bit-fields */
    union {
        struct {
            unsigned int : 2;
            unsigned int mode : 3;
            unsigned int : 3;
        } GTY((tag("bits"))) flags;
        unsigned char GTY((default)) raw_byte;
    } GTY((desc("%1.flags.mode"))) status;
};

/* Dummy main function to make the file compilable */
int main(void) {
    return 0;
}
