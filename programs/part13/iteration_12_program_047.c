/* complex_gty_test.c - Test file for exercising gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Complex function pointer with nested parentheses */
typedef void GTY(()) (*complex_func_ptr)(
    int (* GTY((callback)) nested_callback)(char GTY((length("10"))) buffer[10]),
    struct GTY(()) inner_struct { 
        int x; 
        void (* GTY((chain_next)) another_fn)(int, double);
    } param
);

/* Test 2: Array with complex dimension containing parentheses */
struct GTY(()) array_test {
    int GTY((length("(10 + sizeof(struct array_test))"))) 
        complex_array[(10 + sizeof(struct array_test))];
    
    /* Nested array of function pointers */
    void (* GTY((length("5"))) fn_array[5])(
        int GTY((length("10"))) multi_dim[][10]
    );
};

/* Test 3: Deeply nested structure with all delimiter types */
struct GTY(()) outer_struct {
    /* Case '(': Function pointer with complex signature */
    int (* GTY((chain_next, chain_prev)) process)(
        struct GTY(()) data { 
            int values[5]; 
            struct { 
                char * GTY((tag("0"))) name; 
            } info;
        } *input,
        void (* GTY((callback)) completion)(int result[3], ...)
    );
    
    /* Case '[': Multi-dimensional arrays with complex indices */
    double GTY((length("compute_size()"))) matrix[][
        (sizeof(int) * 2)
    ][10];
    
    /* Case '{': Nested anonymous struct with initializer-like syntax */
    struct {
        union GTY(()) {
            struct {
                int (* GTY((chain_next)) handlers[3])(
                    char GTY((string)) *args[]
                );
            } s;
            long GTY((skip)) bits;
        } u;
        
        /* Array with designated initializer pattern in comment */
        int config[ /* [0] = 1, [1] = 2 */ 5 ];
    } anonymous;
    
    /* Mixed delimiters in single declaration */
    void (*(* GTY((user)) complex_decl)[5])(
        int param[(sizeof(struct outer_struct) + 1)]
    );
};

/* Test 4: Union with nested GTY annotations */
union GTY(()) complex_union {
    struct GTY(()) {
        int (* GTY((chain_next)) compare)(
            const void * GTY((skip)) a,
            const void * GTY((skip)) b,
            int (* GTY((callback)) custom_cmp)(
                const struct { int x; double y; } *p1,
                const struct { int x; double y; } *p2
            )
        );
    } func_union;
    
    struct GTY(()) data_container {
        /* Array of structs containing arrays */
        struct GTY(()) item {
            char * GTY((length("dim"))) names[];
            int dimensions[3];
        } GTY((length("count"))) items[10];
        
        /* Pointer to array of function pointers */
        void (*(* GTY((user)) operation_set)[3])(
            int (* GTY((callback)) steps[])(void)
        );
    } container;
};

/* Test 5: Template-like macro patterns (C-style) */
#define DECLARE_CALLBACK(type, name) \
    type (* GTY((user)) name)(type (* GTY((callback)) transform)(type))

struct GTY(()) template_test {
    /* This will expand to: int (*callback_fn)(int (*transform)(int)) */
    DECLARE_CALLBACK(int, callback_fn);
    
    /* Nested macro expansion with more delimiters */
    #define ARRAY_PTR(type, size) type (* GTY((length(#size))) ptr)[size]
    ARRAY_PTR(struct { int x; char buf[10]; }, 5) data_ptr;
};

/* Test 6: Attribute lists with nested parentheses */
typedef struct GTY((chain_next("next"), chain_prev("prev"))) linked_node {
    struct linked_node * GTY((skip)) next;
    struct linked_node * GTY((skip)) prev;
    
    /* GTY annotation with deeply nested attribute list */
    void * GTY((user("allocator"), 
                desc("1"), 
                param_is(struct GTY(()) alloc_info {
                    size_t size;
                    void (* GTY((callback)) out_of_memory)(void);
                } *))) 
           user_data;
} linked_node_t;

/* Test 7: Initializer-style constructs (in comments to avoid compilation errors) */
static const struct GTY(()) init_example {
    int values[3];
    struct GTY(()) {
        char * GTY((string)) name;
        double (* GTY((user)) compute)(int x);
    } helper;
} /* Example initializer that would trigger brace handling:
   = {
       .values = {1, 2, 3},
       .helper = {
           .name = "test",
           .compute = NULL
       }
   }
   */;

/* Test 8: Multiple balanced delimiters in sequence */
struct GTY(()) ultimate_test {
    /* Combination: ( [ { } ] ) */
    int (* GTY((user)) process_array[3])(
        struct GTY(()) {
            int data[ /* [0] = 1 */ 10 ];
            void (* GTY((callback)) notify)(
                char message[ /* {"test"} */ 50 ]
            );
        } args[]
    );
    
    /* Reverse combination: { [ ( ) ] } */
    struct {
        void *pointers[
            (sizeof(void *) * 2)
        ];
        union {
            int (*func)(int (*inner)(double));
            char buffer[100];
        } choice;
    } mixed;
};

/* Test 9: Function with __attribute__ containing parentheses */
typedef void GTY(()) (*attr_func)(
    int param
) __attribute__((deprecated("Use new_func instead")));

struct GTY(()) attr_test {
    attr_func old_func GTY((user));
    
    /* Multiple attributes with nested parentheses */
    int GTY((aligned(
        sizeof(void *) * 2
    ))) aligned_data;
};

/* Test 10: Recursive structure with function pointer to itself */
struct GTY(()) tree_node {
    char * GTY((string)) name;
    struct tree_node * GTY((child)) left;
    struct tree_node * GTY((child)) right;
    
    /* Function that takes a pointer to same type */
    void (* GTY((user)) traverse)(
        struct tree_node *root,
        void (* GTY((callback)) visit)(
            struct tree_node *node,
            int depth,
            void * GTY((skip)) context
        ),
        void * GTY((skip)) context
    );
    
    /* Array of function pointers for operations */
    int (* GTY((length("op_count"))) operations[5])(
        struct tree_node *target,
        int options[3]
    );
};

/* Main function to make the file compilable */
int main() {
    return 0;
}
