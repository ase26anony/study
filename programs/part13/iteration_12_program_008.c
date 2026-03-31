/* complex-gty-test.c - Test file for exercising gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter lists and attributes */
typedef void (*GTY((user)) complex_func_ptr)(
    int (*GTY((skip)) nested_callback)(char GTY((length("strlen(param)+1"))) param[10]),
    struct GTY(()) inner_struct { 
        int x; 
        char *GTY((tag("0"))) y;
    }
);

/* Test 2: Array declarations with complex size expressions */
struct GTY(()) array_test {
    /* Array with size containing parentheses expression */
    int arr1[(10 + sizeof(struct GTY(()) dummy { int a; }))];
    
    /* Multi-dimensional array with function pointer elements */
    complex_func_ptr (*GTY((chain_next)) arr2[3][2])(
        int GTY((user)) param,
        char GTY((length("10"))) buf[]
    );
    
    /* Array of pointers to arrays */
    int (*GTY((skip)) arr3[5])[10];
};

/* Test 3: Nested structures and unions with complex members */
union GTY(()) outer_union {
    struct GTY(()) nested_struct {
        /* Bit-field with complex expression */
        unsigned int flags : (sizeof(int) * 8 - 1);
        
        /* Anonymous union inside struct */
        union {
            int GTY((tag("1"))) ival;
            float GTY((tag("2"))) fval;
            struct GTY(()) { 
                char *GTY((length("len"))) str; 
                int len; 
            } GTY((tag("3"))) sval;
        } GTY((tag("0"))) data;
        
        /* Function pointer array */
        int (*GTY((user)) handlers[5])(
            void *GTY((skip)) context,
            char GTY((length("*(int*)context"))) buffer[]
        );
    } GTY((tag("STRUCT"))) s;
    
    /* Union variant with nested initializer-like syntax in comments */
    struct GTY(()) other_variant {
        /* Complex type involving all delimiters */
        void (*(*GTY((chain_next)) fn_table[3])[2])(
            int matrix[][10],
            struct { 
                int (*compare)(const void *, const void *); 
            } GTY((skip)) config
        );
    } GTY((tag("OTHER"))) ov;
};

/* Test 4: Template-like macro patterns with nested delimiters */
#define DECLARE_GTY_VECTOR(TYPE, SIZE) \
    struct GTY(()) vector_##TYPE { \
        TYPE GTY((length("capacity"))) *data; \
        int size; \
        int capacity; \
        void (*GTY((skip)) resize)(struct vector_##TYPE *GTY((user)) self, \
                                   int GTY((user)) new_capacity); \
    }

/* Instantiate with complex type */
DECLARE_GTY_VECTOR(
    struct GTY(()) pair {
        char *GTY((length("key_len"))) key;
        int *GTY((length("value_count"))) values;
        int value_count;
        int key_len;
    }, 
    (10 * sizeof(void*))
);

/* Test 5: Multiple delimiter types in single declaration */
struct GTY(()) delimiter_test {
    /* Combination: (*fn_array[size])(param[][dim]) */
    void (*(*GTY((chain_next)) complex_array[5])[3])(
        int GTY((user)) param1[][(2 + 3) * sizeof(int)],
        struct GTY(()) { 
            union { 
                int x[10]; 
                char y[20]; 
            } data; 
        } GTY((skip)) param2
    );
    
    /* Nested initializer pattern in type declaration */
    struct GTY(()) config {
        int (*GTY((user)) get_value)(int index, char buffer[256]);
        struct {
            int min;
            int max;
            int (*validate)(int value, void *GTY((skip)) context);
        } GTY((tag("RANGE"))) range;
    } GTY((user)) settings[4];
};

/* Test 6: Deeply nested parentheses in attribute lists */
typedef struct GTY((user,
    chain_next = "next",
    chain_prev = "prev",
    desc("%0.print()"),
    skip = "{(void)0;}"
)) linked_node {
    struct linked_node *GTY((user)) next;
    struct linked_node *GTY((user)) prev;
    void *GTY((skip)) data;
    void (*GTY((user)) print)(struct linked_node *GTY((user)) self,
                              FILE *GTY((skip)) stream,
                              int (*GTY((user)) formatter)(
                                  const void *GTY((skip)) data,
                                  char GTY((length("bufsize"))) buffer[],
                                  int bufsize
                              ));
} linked_node_t;

/* Test 7: Complex initializer (static variable) */
static struct GTY(()) global_config {
    int debug_level;
    void (**GTY((length("handler_count"))) handlers)(
        const char *GTY((length("strlen(msg)"))) msg,
        va_list GTY((skip)) args
    );
    int handler_count;
} GTY((user)) the_config = {
    .debug_level = 3,
    .handlers = (void (**)()){ NULL, NULL, NULL },
    .handler_count = 3
};

/* Test 8: Recursive type with nested delimiters */
struct GTY(()) tree_node {
    char *GTY((length("name_len"))) name;
    int name_len;
    struct tree_node **GTY((length("child_count"))) children;
    int child_count;
    int (*GTY((user)) traverse)(
        struct tree_node *GTY((user)) node,
        void *GTY((skip)) context,
        int (*GTY((user)) visitor)(
            struct tree_node *GTY((user)) current,
            int GTY((user)) depth,
            void *GTY((skip)) ctx
        )
    );
};

/* Test 9: Mixed delimiters in array dimensions and function parameters */
union GTY(()) mixed_delimiters {
    struct {
        /* Array of function pointers returning pointers to arrays */
        int (*(*GTY((chain_next)) api[10])(void))[20];
        
        /* Nested structure with bit-fields and arrays */
        struct GTY(()) {
            unsigned int : (sizeof(int) * 8 - 4);
            unsigned int mode : 4;
            char (*GTY((length("strlen(path)+1"))) path)[256];
        } GTY((tag("FILE_INFO"))) file_info;
    } s;
    
    /* Union alternative with complex callback */
    struct {
        void (*GTY((user)) on_event)(
            int GTY((user)) event_type,
            union {
                struct { int x; int y; } point;
                struct { char *GTY((length("len"))) data; int len; } buffer;
            } GTY((tag("event_data"))) data
        );
    } e;
};

/* Test 10: Multiple levels of nested GTY annotations */
typedef struct GTY(()) outer_container {
    struct GTY((user,
        skip = "{(void)0;}",
        desc("Inner container with callback")
    )) inner_container {
        void *GTY((skip)) user_data;
        int (*GTY((user)) processor)(
            struct inner_container *GTY((user)) self,
            int (*GTY((user)) step_callback)(
                int GTY((user)) step,
                char GTY((length("step*10"))) output[],
                struct GTY(()) { int retry_count; } *GTY((skip)) config
            )
        );
        struct GTY(()) { int count; char **GTY((length("count"))) items; } *GTY((user)) items;
    } GTY((chain_next)) *inner;
    
    /* Array of pointers to functions returning structures */
    struct GTY(()) result {
        int status;
        char *GTY((length("message_len"))) message;
        int message_len;
    } (*(*GTY((chain_next)) workers[5])(void *GTY((skip)) arg))[3];
} outer_container_t;

/* Main function to make the file compilable (GTY macros expand to nothing normally) */
int main(void) {
    /* Reference variables to avoid unused warnings */
    struct array_test at = {0};
    union outer_union ou = {0};
    struct delimiter_test dt = {0};
    linked_node_t ln = {0};
    struct tree_node tn = {0};
    union mixed_delimiters md = {0};
    outer_container_t oc = {0};
    
    (void)at;
    (void)ou;
    (void)dt;
    (void)ln;
    (void)tn;
    (void)md;
    (void)oc;
    
    return 0;
}
