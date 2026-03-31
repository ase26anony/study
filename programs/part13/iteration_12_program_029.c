/* complex_gty_test.c - Test file to exercise gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter lists */
typedef int (*GTY((chain_next)) complex_func_ptr)(
    void (*GTY((chain_prev)) nested_callback)(
        char data[10][(sizeof(int) + 5)],
        struct { int x; double y; } param
    ),
    int array_dim[(10 + sizeof(struct { char a; int b; }))]
);

/* Test 2: Structure with deeply nested delimiters */
struct GTY((for_user)) outer_struct {
    /* Array with complex dimension expression */
    int matrix[5][(2 * sizeof(double))];
    
    /* Function pointer array */
    void (*GTY((skip)) handlers[3])(
        int param,
        char buffer[][(10 + 2)]
    );
    
    /* Nested anonymous struct with initializer-like designators */
    struct {
        int (*compare)(const char *a[(10)], const char *b[20]);
        union {
            long (*transform)(int matrix[3][3][3]);
            short data[((sizeof(int) > 4) ? 8 : 4)];
        } GTY((tag("0"))) u;
    } GTY((desc("%1.u"))) nested;
    
    /* Pointer to array of function pointers */
    int (*(*GTY((chain_next)) complex_array)[5])(
        struct { int x; char y[10]; } arg
    );
};

/* Test 3: Union with multiple nested constructs */
union GTY((desc("%0.kind"))) complex_union {
    int kind;
    
    /* Case 1: Array of structures with bit-fields */
    struct {
        unsigned int flags : 3;
        unsigned int : 5;  /* unnamed bit-field */
        unsigned int count : 8;
        char name[(sizeof(struct { int x; }) + 10)];
    } GTY((tag("1"))) s;
    
    /* Case 2: Function pointer with nested attributes */
    void (*GTY((skip)) action)(
        int (*steps[3])(
            char input[],
            int (*callback)(void)
        ),
        union {
            int i;
            void *p;
        } data
    );
    
    /* Case 3: Multi-dimensional array initializer pattern */
    int matrix[2][(3 + sizeof(char*))][4];
};

/* Test 4: Typedef with template-like pattern using nested parentheses */
typedef struct GTY(()) node {
    struct node *GTY((chain_next)) next;
    struct node *GTY((chain_prev)) prev;
    
    /* Complex member with all delimiter types */
    union {
        /* Braces for initializer pattern */
        struct { int start; int end; } range;
        
        /* Parentheses for function type */
        void (*processor)(
            int (*transformers[])(char *data),
            struct { int count; } metadata
        );
        
        /* Brackets for array type */
        char buffer[100][(50 + sizeof(int*))];
    } GTY((tag("%0.type"))) data;
    
    /* Array with designators in comments (simulating initializers) */
    int values[10]; /* Could be initialized as = { [0] = 1, [9] = 10 } */
} *GTY((length("%h.count"))) node_ptr;

/* Test 5: Structure with attribute in nested context */
struct GTY((for_user)) container {
    /* GTY annotation inside a complex type */
    struct GTY((chain_next)) element {
        struct element *next;
        
        /* Function with array parameter */
        void (*GTY((skip)) cleanup)(
            void *resources[(10)],
            int count
        );
        
        /* Nested structure with bit-fields and array */
        struct {
            unsigned int : 16;  /* padding */
            unsigned int size : 8;
            char id[(20 + sizeof(long))];
        } info;
    } *GTY((length("%h.elem_count"))) elements;
    
    int elem_count;
    
    /* Callback with nested GTY annotation */
    int (*GTY((callback)) validate)(
        struct container *self,
        int (*checks[])(const char *criteria[][10])
    );
};

/* Test 6: Extreme nesting case */
typedef int (*(*(**GTY((chain_next)) extreme_nesting)[
    (sizeof(struct { char a; int b; double c; }) > 16 ? 2 : 4)
])(
    void (*callbacks[3])(
        int matrix[][(5)][3],
        struct { union { int x; long y; } u; } param
    )
))[10])(
    char data[100][(50)],
    void (*finalizer)(
        struct { int status; } result
    )
);

/* Test 7: Structure with initializer-like comments (triggers brace handling) */
struct GTY(()) config {
    /* Members that might appear in initializers */
    int modes[4]; /* = { 1, 2, 3, 4 } */
    struct {
        int x;
        int y;
    } position; /* = { .x = 0, .y = 0 } */
    
    /* Union with tag for discriminant */
    union {
        int int_value;
        char *string_value;
        void (*func_value)(int);
    } GTY((tag("%0.type"))) value;
    
    int type;
};

/* Test 8: Array of function pointers with complex signatures */
static void (*GTY((skip)) signal_handlers[5])(
    int signal,
    void *context[(10)],
    struct {
        int pid;
        char name[(20)];
    } *process_info
) = { NULL, NULL, NULL, NULL, NULL };

/* Test 9: Recursive structure with complex members */
struct GTY((chain_next)) tree_node {
    char *GTY((length("%h.name_len"))) name;
    int name_len;
    
    /* Array of child nodes */
    struct tree_node *GTY((length("%h.child_count"))) children[
        (MAX_CHILDREN > 0 ? MAX_CHILDREN : 10)
    ];
    int child_count;
    
    /* Function pointer for node operations */
    void (*GTY((callback)) traverse)(
        struct tree_node *node,
        void (*visit)(struct tree_node *, int depth),
        int options[(5)]
    );
    
    /* Nested data structure */
    union {
        int int_data;
        double float_data;
        struct {
            char *buffer;
            int size;
        } blob_data;
    } GTY((tag("%0.data_type"))) data;
    
    int data_type;
    
    struct tree_node *next;
};

/* Test 10: Macro-like patterns that expand to delimiter sequences */
#define DECLARE_CALLBACK(name, type) \
    type (*GTY((skip)) name)(type arg[(sizeof(type))], \
                             struct { type min; type max; } range)

/* Instantiate the macro with different types */
DECLARE_CALLBACK(int_callback, int);
DECLARE_CALLBACK(double_callback, double);

/* Test 11: Structure with all delimiter types in one member */
struct GTY(()) delimiter_test {
    /* Contains: parentheses, brackets, and braces */
    int (*(*complex_member)[
        (sizeof(int*) * 2)
    ])(
        char input[][(10)],
        struct { int flag; } options
    )[(5)];
    
    /* Another complex example */
    union {
        struct { int x[(3)]; } s;
        void (*func)(int (*arg)(void));
    } u;
};

/* Test 12: Typedef chain with increasing complexity */
typedef int simple_type;
typedef simple_type (*type_level1)(int);
typedef type_level1 (*type_level2)(char[], type_level1);
typedef type_level2 (*type_level3)(
    struct { type_level2 prev; } history,
    type_level1 fallback
);
typedef type_level3 (*GTY((chain_next)) type_level4)[10];

/* Dummy main function to make file compilable */
int main(void) {
    return 0;
}
