/* complex_gty_test.c - Test file for exercising gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter list and attributes */
typedef void GTY((user)) (*complex_func_ptr)(
    int (*GTY((skip)) callback)(char GTY((user))[10]), 
    struct GTY((user)) { 
        int x; 
        union GTY((desc("%1.tag"))) { 
            int i; 
            char * GTY((length("%1.len"))) str; 
        } data; 
    }
);

/* Test 2: Array with complex dimension expression containing parentheses */
struct GTY((user)) ArrayTest {
    int arr1[(10 + sizeof(struct GTY((user)) Inner { int a; }))];
    char * GTY((length("strlen(%h.arr2[%1]) + 1"))) arr2[5][(sizeof(int*) * 2)];
};

/* Test 3: Deeply nested structure with all delimiter types */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) NestedStruct {
    struct NestedStruct *next;
    struct NestedStruct *prev;
    
    /* Function pointer array */
    void (* GTY((user)) func_array[3])(
        int param1,
        char param2[][10],
        struct GTY((user)) { 
            double d; 
            int arr[((2 * 3) + 4)]; 
        }
    );
    
    /* Nested union with bitfields */
    union GTY((desc("%1.type"))) {
        struct GTY((user)) {
            unsigned int : 4;
            unsigned int flag1: 1;
            unsigned int flag2: 1;
        } bits;
        
        struct GTY((user)) {
            int (* GTY((skip)) compare)(
                const void *,
                const void *,
                int (*)(const char *, const char *[])
            );
            char * GTY((length("%h.len"))) name;
        } data;
    } variant;
};

/* Test 4: Template-like macro with nested delimiters */
#define DECLARE_VECTOR(TYPE) \
    struct GTY((user)) vector_##TYPE { \
        TYPE * GTY((length("%h.capacity"))) items; \
        size_t size; \
        size_t capacity; \
        int (* GTY((skip)) resize)(struct vector_##TYPE *, size_t); \
    }

DECLARE_VECTOR(int);
DECLARE_VECTOR(struct GTY((user)) { int x; double y; char z[10]; });

/* Test 5: Complex initializer with nested braces */
static const struct GTY((user)) ComplexInit {
    int matrix[2][3];
    struct GTY((user)) {
        char *name;
        int values[5];
    } nested;
} complex_init_instance = {
    .matrix = {
        {1, 2, 3},
        {(4 + 1), 5, 6}
    },
    .nested = {
        .name = "test",
        .values = {[0] = 10, [4] = 20}
    }
};

/* Test 6: Multiple balanced delimiters in single declaration */
struct GTY((user)) MultiDelimiter {
    /* Contains: (*, ), [5], (, [][10], ) */
    void (* GTY((user)) fn_array[5])(int GTY((user))[][10]);
    
    /* Mixed array and function pointer */
    int (* GTY((user)) mixed[3][2])(
        char * GTY((length("%1.len"))) str,
        struct GTY((user)) { 
            int a[2]; 
            struct GTY((user)) { 
                double b; 
            } inner; 
        }
    )[4];
    
    /* Triple nested */
    struct GTY((user)) {
        union GTY((desc("%1.tag"))) {
            int i;
            struct GTY((user)) {
                char * GTY((length("%h.len"))) s;
                int len;
            } str;
        } data[2];
    } container;
};

/* Test 7: Recursive structure with function pointer */
struct GTY((user)) TreeNode {
    char * GTY((length("%h.name_len"))) name;
    int name_len;
    
    struct TreeNode * GTY((user)) children[4];
    
    int (* GTY((skip)) traverse)(
        struct TreeNode *,
        void (*)(struct TreeNode *, void *),
        void *
    );
    
    /* Callback with array parameter */
    void (* GTY((skip)) on_visit)(
        int depth,
        char path[][256],
        struct GTY((user)) {
            int count;
            struct TreeNode *nodes[];
        } *context
    );
};

/* Test 8: Union containing array of function pointers */
union GTY((desc("%1.type"))) FunctionUnion {
    int (* GTY((skip)) int_func)(int, int);
    char * (* GTY((length("strlen(%0) + 1"))) str_func)(
        const char *,
        int (*)(const char *, ...)
    );
    void (* GTY((skip)) void_func)(
        void *,
        struct GTY((user)) { 
            int id; 
            char data[100]; 
        }
    );
};

/* Test 9: Structure with attribute containing nested parentheses */
struct GTY((
    user,
    chain_next("%h.next"),
    chain_prev("%h.prev"),
    desc("((%1.type == 0) ? \"int\" : \"struct\")")
)) AttrTest {
    struct AttrTest *next;
    struct AttrTest *prev;
    int type;
    
    union GTY((desc("%1.type"))) {
        int value;
        struct GTY((user)) {
            char * GTY((length("%h.len"))) text;
            int len;
        } text;
    } data;
};

/* Test 10: Complex typedef with multiple nested levels */
typedef struct GTY((user)) Outer {
    int id;
    
    struct GTY((user)) Middle {
        double value;
        
        struct GTY((user)) Inner {
            char * GTY((length("%h.str_len"))) str;
            int str_len;
            int (* GTY((skip)) validate)(struct Inner *, const char *[]);
        } inner;
        
        struct Inner * GTY((user)) inner_array[2];
    } middle[3];
    
    /* Function returning function pointer */
    int (*(* GTY((skip)) get_callback)(int type))(
        struct Middle *,
        char buffer[][512]
    );
} OuterType;

/* Test 11: Initialize with designators containing array indices */
static struct GTY((user)) DesignatorTest {
    int points[3][2];
    struct GTY((user)) {
        char label[20];
        int values[4];
    } metadata[2];
} designator_instance = {
    .points = {
        [0] = {1, 2},
        [1][0] = 3, [1][1] = 4,
        [2] = {5, 6}
    },
    .metadata = {
        [0] = {
            .label = "first",
            .values = {[0] = 10, [3] = 40}
        },
        [1] = {
            .label = "second",
            .values = {[1] = 20, [2] = 30}
        }
    }
};

/* Test 12: Macro expansion with nested delimiters */
#define DEFINE_CALLBACK(RET, NAME, PARAMS) \
    RET (* GTY((skip)) NAME) PARAMS

struct GTY((user)) MacroTest {
    DEFINE_CALLBACK(int, cb1, (int, char * GTY((length("strlen(%1) + 1")))));
    DEFINE_CALLBACK(void, cb2, (
        struct GTY((user)) { 
            int x[2]; 
            struct GTY((user)) { 
                double y; 
            } inner; 
        }
    ));
};

/* Test 13: Structure with all three delimiter types in sequence */
struct GTY((user)) AllDelimiters {
    /* Sequence: { [ ( ) ] } */
    struct GTY((user)) {
        int (* GTY((skip)) processors[2])(
            void *data[10]
        );
    } container;
    
    /* Sequence: ( { [ ] } ) */
    void (* GTY((skip)) initialize)(
        struct GTY((user)) {
            int settings[3];
            char * GTY((length("%h.name_len"))) name;
        }
    );
    
    /* Sequence: [ ( { } ) ] */
    struct GTY((user)) {
        void (* GTY((skip)) callbacks[3])(
            struct GTY((user)) { int id; }
        );
    } handlers[2];
};

/* Dummy main to make file compilable */
int main(void) {
    return 0;
}
