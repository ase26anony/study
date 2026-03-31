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
    }
);

/* Test 2: Array with complex dimension expression containing parentheses */
struct GTY(()) ArrayTest {
    int arr1[(10 + sizeof(struct GTY(()) SizeStruct { int a; double b; }))];
    char * GTY((length("arr2[0]"))) arr2[5][(sizeof(int*) * 2)];
};

/* Test 3: Deeply nested structure with multiple delimiter types */
struct GTY(()) OuterStruct {
    /* Nested anonymous union with braces */
    union GTY(()) {
        struct GTY(()) {
            int (* GTY((chain_next)) fp_array[3])(
                int matrix[][(2 * sizeof(double))],
                union GTY(()) { 
                    long l; 
                    char c[(4 + (2 * sizeof(int)))]; 
                } u
            );
        } s;
        double dbl_arr[(sizeof(struct { char c; int i; }) + 5)];
    } nested_union;
    
    /* Member with all three delimiter types in sequence */
    void (*(* GTY((user)) complex_member)(
        int param1,
        struct GTY(()) { 
            short s; 
            long long ll; 
        } param2
    ))[10])(int, char);
};

/* Test 4: GTY annotation with nested attribute lists */
struct GTY((chain_next, chain_prev("prev_ptr"), user)) ListNode {
    struct ListNode * GTY((skip)) next;
    struct ListNode * GTY((skip("prev_ptr"))) prev;
    
    /* Function pointer field with GTY attributes */
    void (* GTY((callback, user("user_data"))) handler)(
        struct ListNode * GTY((skip)) node,
        int data[(sizeof(void*) * 8)]
    );
    
    /* Array with designated initializer-like GTY annotation */
    int GTY((length("count"))) *dynamic_array;
    int count;
};

/* Test 5: Multiple balanced delimiters in single declaration */
union GTY(()) MultiDelimiterUnion {
    /* Contains: (*, ), [5], (, [][10], ) */
    void (* GTY((user)) fn_array[5])(
        int GTY((length("dim1"))) matrix[][10],
        struct GTY(()) { 
            int dim1; 
            char buf[(20 + (5 * sizeof(int)))]; 
        } meta
    );
    
    /* Complex initializer-like structure */
    struct GTY(()) {
        struct GTY(()) inner {
            int values[3][(2 + sizeof(double))];
            void (* GTY((chain_next)) methods[2])(
                union MultiDelimiterUnion * GTY((skip)) self,
                int index
            );
        } nested;
        
        /* Braces within braces */
        struct GTY(()) config {
            int flags;
            struct GTY(()) { 
                char *name; 
                int id; 
            } items[5];
        } cfg;
    } complex_data;
};

/* Test 6: Template-like pattern simulation using nested types */
#define DECLARE_CONTAINER(TYPE, SIZE) \
    struct GTY(()) Container_##TYPE { \
        TYPE GTY((length("used"))) items[SIZE]; \
        int used; \
        int (* GTY((user)) compare)(TYPE *a, TYPE *b); \
    }

/* Instantiate with complex types */
DECLARE_CONTAINER(
    struct GTY(()) ComplexItem {
        int id;
        char * GTY((length("len"))) name;
        int len;
        void (* GTY((callback)) notify)(
            struct ComplexItem * GTY((skip)) item,
            int status[(4 + sizeof(struct { int code; char msg[50]; }))]
        );
    }, 
    (10 * sizeof(void*)) / sizeof(struct ComplexItem)
);

/* Test 7: Recursive structure with function pointers */
struct GTY(()) Tree {
    struct Tree * GTY((left, right)) children[2];
    char * GTY((string)) data;
    
    /* Visitor function pointer with complex signature */
    int (* GTY((user)) visitor)(
        struct Tree * GTY((skip)) node,
        void (* GTY((callback)) action)(
            char * GTY((string)) msg,
            int level,
            struct GTY(()) Context { 
                int depth; 
                int path[100]; 
            } ctx
        ),
        struct GTY(()) { 
            int options; 
            char filter[(20 + (10 * sizeof(char)))]; 
        } config
    );
};

/* Test 8: Mixed delimiters in typedef */
typedef union GTY(()) {
    int (* GTY((user)) funcs[3])(
        int,
        char * GTY((string)) args[],
        struct GTY(()) { 
            int count; 
            double values[5]; 
        } params
    );
    struct GTY(()) {
        void (* GTY((chain_next)) handlers[2])(
            union GTY(()) * GTY((skip)) self,
            int signal[(sizeof(int) + 2)]
        );
    } callback_group;
} MixedUnion GTY((user));

/* Test 9: Structure with initialization-like nested braces */
struct GTY(()) ConfigData {
    struct GTY(()) sections[3] {
        char * GTY((string)) name;
        struct GTY(()) entries[10] {
            char * GTY((string)) key;
            union GTY(()) value {
                int num;
                char * GTY((string)) str;
                double arr[(3 * sizeof(int))];
            } val;
            void (* GTY((user)) validator)(
                struct entries * GTY((skip)) entry,
                int criteria
            );
        } ents;
        int count;
    } secs;
    
    /* Global callback with nested parameter types */
    void (* GTY((user)) global_hook)(
        struct sections * GTY((skip)) modified_section,
        struct GTY(()) ChangeInfo {
            int type;
            char old_value[100];
            char new_value[100];
            struct GTY(()) { 
                time_t timestamp; 
                int user_id; 
            } meta;
        } change
    );
};

/* Test 10: Extreme nesting of all delimiter types */
struct GTY(()) UltimateNest {
    /* Parentheses within brackets within braces */
    struct GTY(()) level1 {
        int (*level2[2])(
            struct GTY(()) {
                char *level3;
                void (*level4)(
                    int level5[][(10 + sizeof(struct { int a; }))],
                    union GTY(()) {
                        double d;
                        struct GTY(()) {
                            long l;
                            short s[(2 * (3 + sizeof(char)))];
                        } nested;
                    } u
                );
            } *param
        );
    } extreme;
    
    /* Array of function pointers returning arrays of structures */
    struct GTY(()) (*func_table[(sizeof(void*) * 4)])(
        int index,
        char buffer[],
        struct GTY(()) { 
            int size; 
            void (*cleanup)(void); 
        } config
    )[10];
};

/* Main function to make the file compilable (though GTY expands to nothing normally) */
int main(void) {
    return 0;
}
