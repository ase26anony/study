/* complex_gty_test.c - Test file to exercise gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Complex function pointer with nested parentheses */
typedef void (*GTY((user)) complex_func_ptr)(
    int (*GTY((skip)) nested_callback)(char param[10]),
    struct GTY(()) inner_struct { int x; } *arg2
);

/* Test 2: Array with complex dimension expression containing parentheses */
struct GTY(()) array_test {
    int arr1[(10 + sizeof(struct GTY(()) temp { int a; char b; }))];
    char arr2[5][(sizeof(int) * 2)];
};

/* Test 3: Nested structures with all delimiter types */
struct GTY(()) outer_struct {
    /* Case for '(' */
    void (*func_ptr_array[3])(
        int param1,
        struct GTY(()) { 
            int x; 
            char y[(sizeof(int) + 1)]; 
        } param2
    );
    
    /* Case for '[' */
    int (*matrix_ptr)[10][(2 * 5)];
    
    /* Case for '{' */
    union GTY((desc ("%1.type"))) inner_union {
        int ival;
        struct GTY(()) {
            float f;
            char c[({ int x = 5; x + 1; })];
        } s;
    } u;
};

/* Test 4: Multiple balanced delimiters in single declaration */
struct GTY(()) complex_member {
    /* Contains: (*, ), [5], (, [][10], ) */
    void (*fn_array[5])(int param[][10]);
    
    /* Initializer with nested braces */
    struct GTY(()) init_example {
        int a;
        int b;
    } init = { 
        .a = ({ int x = 1; x + 2; }),
        .b = 4 
    };
};

/* Test 5: Deeply nested parentheses in attribute lists */
struct GTY((chain_next ("next"), 
            chain_prev ("prev"),
            user ("user_data"))) linked_list {
    struct linked_list * GTY((skip)) next;
    struct linked_list * GTY((skip)) prev;
    
    /* Function pointer with complex signature */
    int (* GTY((user)) compare_func)(
        const void * GTY((skip)) a,
        const void * GTY((skip)) b,
        void * GTY((skip)) user_data
    );
    
    /* Array of function pointers */
    void (*callbacks[3])(
        struct GTY(()) { int id; char name[20]; } *data
    );
};

/* Test 6: Template-like pattern using nested types */
#define DECLARE_CONTAINER(TYPE) \
    struct GTY(()) container_##TYPE { \
        TYPE * GTY((tag ("0"))) items; \
        int (* GTY((skip)) alloc_func)(size_t size); \
        void (* GTY((skip)) free_func)(void *ptr); \
    }

DECLARE_CONTAINER(int);
DECLARE_CONTAINER(char);

/* Test 7: Union with nested anonymous struct containing arrays */
union GTY((user)) data_union {
    struct GTY(()) {
        int type;
        char name[(sizeof("default") + 1)];
        struct GTY(()) {
            int x, y;
            int coords[2][({ int dim = 2; dim; })];
        } point;
    } s;
    
    struct GTY(()) {
        float values[4];
        void (*processor)(float *arr, int len);
    } f;
};

/* Test 8: Recursive structure with function pointer */
struct GTY(()) tree_node {
    int value;
    struct tree_node * GTY((skip)) left;
    struct tree_node * GTY((skip)) right;
    
    /* Visitor function with complex parameters */
    void (* GTY((user)) visitor)(
        struct tree_node *node,
        void (* GTY((skip)) callback)(int, char *),
        int depth
    );
    
    /* Array of child processors */
    int (*processors[3])(
        struct GTY(()) { 
            int id; 
            int params[3]; 
        } *config
    );
};

/* Test 9: Structure with bit-fields and initializers */
struct GTY(()) bitfield_test {
    unsigned int flag:1;
    unsigned int mode:3;
    
    /* Anonymous union with array */
    union {
        int ival;
        char str[({ int len = 10; len + 1; })];
    } data;
    
    /* Complex initializer */
    struct GTY(()) {
        int x;
        int y[2];
    } point = { 
        .x = 0, 
        .y = {[0] = 1, [1] = ({ int z = 2; z; })} 
    };
};

/* Test 10: Multiple levels of nesting */
struct GTY(()) level1 {
    struct GTY(()) level2 {
        struct GTY(()) level3 {
            void (*level3_func)(
                int (*nested[2])(
                    char param[({ int s = 5; s; })]
                )
            );
            
            union GTY(()) {
                int a;
                struct GTY(()) {
                    float b;
                    char c[3][(sizeof(double) / 2)];
                } s;
            } u;
        } l3;
        
        int (*l2_func)(struct level3 *ptr);
    } l2;
    
    struct level2 * GTY((skip)) next;
};

/* Main function to make file compilable */
int main(void) {
    /* These structures will be processed by gengtype */
    struct outer_struct os;
    struct complex_member cm;
    struct linked_list ll;
    struct tree_node tn;
    struct level1 l1;
    
    return 0;
}
