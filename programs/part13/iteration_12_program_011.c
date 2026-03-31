/* complex_gty_test.c - Test file for exercising gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Complex function pointer with nested parentheses */
typedef void (*GTY((user)) complex_func_ptr)(
    int (*GTY((skip)) nested_callback)(char GTY((length("strlen(param)+1"))) param[]),
    struct GTY(()) inner_struct { 
        int x; 
        void (*GTY((skip)) another_fn)(int, float); 
    } *arg2
);

/* Test 2: Array declarations with complex dimensions containing parentheses */
struct GTY(()) ArrayTest {
    /* Array dimension with parenthesized expression */
    int arr1[(10 + sizeof(struct GTY(()) Temp { int a; }))];
    
    /* Multi-dimensional array with function call-like syntax in dimension */
    char *arr2[3][(5 * 2)];
    
    /* Pointer to array with nested structure initializer */
    struct GTY(()) NestedArr {
        int data[5];
    } (*arr_ptr)[(sizeof(int) > 4) ? 8 : 4];
};

/* Test 3: Deeply nested structures with all delimiter types */
union GTY(()) DeepNested {
    struct GTY(()) Level1 {
        int a;
        struct GTY(()) Level2 {
            float b;
            union GTY(()) Level3 {
                double c;
                struct GTY(()) Level4 {
                    /* Mix of all delimiters in one member */
                    void (*func_array[3])(
                        int param1,
                        char param2[],
                        struct GTY(()) { int x; float y; } param3
                    );
                    
                    /* Initializer-like brace structure */
                    struct GTY(()) InitStyle {
                        int values[5];
                        struct GTY(()) { short s; long l; } nested;
                    } data;
                } d;
            } e;
        } f;
    } g;
    
    /* Alternative union member with complex type */
    complex_func_ptr h;
};

/* Test 4: Template-like pattern using nested parentheses and brackets */
#define DECLARE_VECTOR(TYPE, SIZE) \
    struct GTY(()) vector_##TYPE { \
        TYPE data[(SIZE)]; \
        int (*compare)(TYPE GTY((user)) a, TYPE GTY((user)) b); \
    }

/* Instantiate template-like structures */
DECLARE_VECTOR(int, 10);
DECLARE_VECTOR(struct GTY(()) { int x; double y; }, 5);

/* Test 5: GTY annotations with nested attribute lists */
struct GTY((chain_next("next"), chain_prev("prev"))) LinkedList {
    int value;
    
    /* Nested GTY annotation inside a function pointer type */
    void (*GTY((callback)) notify)(
        struct LinkedList *GTY((skip)) self,
        int GTY((user)) status
    );
    
    /* Pointer with multiple nested attributes */
    struct LinkedList *GTY((chain_next, chain_prev)) next;
    struct LinkedList *GTY((chain_next, chain_prev)) prev;
    
    /* Array of function pointers with GTY attributes */
    int (*GTY((length("count"))) handlers[5])(
        char *GTY((string)) message,
        int GTY((user)) priority
    );
};

/* Test 6: Mixed delimiters in single declaration */
struct GTY(()) UltimateTest {
    /* Contains: (*, ), [5], (, [][10], ) */
    void (*fn_array[5])(int[][10]);
    
    /* Nested initializer with designators */
    struct GTY(()) Config {
        int sizes[3];
        struct GTY(()) { 
            char *name; 
            float values[2][2]; 
        } settings;
    } config = {
        .sizes = {10, 20, 30},
        .settings = {
            .name = "test",
            .values = {{1.0, 2.0}, {3.0, 4.0}}
        }
    };
    
    /* Complex type with all delimiters intertwined */
    union GTY(()) {
        struct GTY(()) {
            int (*processor)(
                char input[],
                struct GTY(()) { 
                    int flags; 
                    void (*action)(void); 
                } state
            );
        } s;
        float matrix[2][(3 + 2)];
    } u;
};

/* Test 7: Recursive structures with balanced delimiters */
typedef struct GTY(()) TreeNode TreeNode;
struct GTY(()) TreeNode {
    int value;
    
    /* Array of child pointers */
    TreeNode *GTY((length("child_count"))) children[4];
    
    /* Function pointer with recursive type in parameters */
    void (*GTY((skip)) visit)(
        TreeNode *GTY((user)) node,
        int (*GTY((skip)) should_stop)(TreeNode *GTY((user)), int depth)
    );
    
    /* Nested anonymous union */
    union GTY(()) {
        char *GTY((string)) str_data;
        int (*GTY((skip)) int_func)(int, int);
        struct GTY(()) {
            float x, y;
            void (*draw)(float, float);
        } point;
    } data;
};

/* Test 8: Multiple balanced delimiter types in sequence */
struct GTY(()) SequenceTest {
    /* Sequence: { [ ( ) ] } */
    struct GTY(()) {
        int *array[(5 + (3 * 2))];
    } nested;
    
    /* Sequence: ( { [ ] } ) */
    void (*initialize)(
        struct GTY(()) {
            char buffer[256];
            int (*validators[3])(char *);
        } config
    );
    
    /* Complex array of structures with initializers */
    struct GTY(()) Item {
        int id;
        char name[50];
        void (*action)(void);
    } items[3] = {
        {1, "first", 0},
        {2, "second", 0},
        {3, "third", 0}
    };
};

/* Test 9: Attribute lists with nested parentheses */
struct GTY(( 
    user,
    desc("test_struct"),
    maybe_undef,
    tag("TEST_STRUCT"),
    variable_size  /* This attribute itself doesn't take params but shows nesting */
)) TestWithAttrs {
    int count;
    
    /* Field with nested attribute */
    char *GTY((length("count"), string)) strings;
    
    /* Pointer with chain attributes */
    struct TestWithAttrs *GTY((chain_next("next_in_chain"))) next_in_chain;
    
    /* Callback with GTY attribute in parameter */
    void (*GTY((callback)) on_update)(
        struct TestWithAttrs *GTY((user)) self,
        int GTY((user)) new_count,
        char **GTY((length("new_count"), string)) new_strings
    );
};

/* Test 10: Edge case - empty balanced delimiters */
struct GTY(()) EmptyDelimiters {
    void (*empty_fn)(void);  /* Empty parentheses */
    int empty_array[0];      /* Zero-length array */
    struct GTY(()) {} empty_struct;  /* Empty structure */
    
    /* Function with empty parameter list returning pointer to array */
    int (*(*complex_empty)(void))[];
    
    /* Nested empty delimiters */
    struct GTY(()) {
        union GTY(()) {
            struct GTY(()) { } s;
        } u;
    } wrapper;
};

/* Main function to make the file compilable (though GTY expands to nothing normally) */
int main(void) {
    /* These structs would normally be used by GCC's garbage collector */
    struct LinkedList list = {0};
    struct UltimateTest test = {0};
    struct TreeNode node = {0};
    
    return 0;
}
