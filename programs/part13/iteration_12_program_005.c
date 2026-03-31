/* complex_gty_test.c - Test file for gengtype delimiter parsing */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter lists */
typedef void GTY(()) (*complex_func_ptr)(
    int (* GTY(()) nested_callback)(char GTY(()) buffer[10]),
    struct GTY(()) { int x; int y; } param
);

/* Test 2: Array with complex dimension expression */
struct GTY(()) ArrayTest {
    int arr1[(10 + sizeof(struct GTY(()) { char c; int i; }))];
    char arr2[5][(2 * sizeof(int))];
};

/* Test 3: Nested structures with multiple delimiter types */
struct GTY(()) OuterStruct {
    /* Function pointer array */
    void (* GTY((chain_next)) fp_array[5])(int GTY(()) matrix[][10]);
    
    /* Nested anonymous struct with bit-field */
    struct GTY(()) {
        unsigned int flags : 3;
        int GTY(()) data[4];
    } inner;
    
    /* Union with nested initializer-like syntax in comments */
    union GTY(()) {
        long long big;
        struct GTY(()) { short a; short b; } parts;
    } u;
};

/* Test 4: Multiple balanced delimiters in single declaration */
typedef int (*(* GTY(()) complex_decl)[5])(char GTY(()) (*arg)[3]);

/* Test 5: Structure with deeply nested initializer pattern */
struct GTY(()) DeepNest {
    /* Multi-dimensional function pointer */
    void (*(* GTY((chain_prev, chain_next)) deep[2][3])[4])(
        struct GTY(()) { 
            int x; 
            struct GTY(()) { 
                char c[2]; 
                union GTY(()) { 
                    float f; 
                    double d; 
                } u; 
            } s; 
        } param
    );
    
    /* Array of structs with array members */
    struct GTY(()) {
        int ids[3];
        char * GTY((length)) names[2];
    } items[5];
};

/* Test 6: Template-like macro expansion with delimiters */
#define GTY_TEMPLATE(type) type GTY(()) *
typedef GTY_TEMPLATE(struct GTY(()) { 
    int value; 
    GTY_TEMPLATE(void) next; 
}) TemplatePtr;

/* Test 7: Attribute lists within GTY annotations */
struct GTY((chain_next, chain_prev, user)) ListNode {
    void * GTY((user)) data;
    struct ListNode * GTY((chain_next)) next;
    struct ListNode * GTY((chain_prev)) prev;
    
    /* Callback with attributes */
    void (* GTY((user)) notify)(
        struct ListNode * GTY((skip)) node,
        int GTY(()) status[2]
    );
};

/* Test 8: Mixed delimiters in type qualifiers */
typedef const volatile struct GTY(()) {
    int (* GTY(()) funcs[3])(void);
    char (* GTY(()) strings)[10];
} CVStruct;

/* Test 9: Structure with all delimiter types combined */
struct GTY(()) AllDelimiters {
    /* Parentheses in function type */
    int (*compare)(const void *, const void *);
    
    /* Brackets in array declaration */
    int matrix[3][(4 + 1)];
    
    /* Braces in nested anonymous struct */
    struct {
        union {
            int x;
            long y;
        } u;
        short s[2];
    } nested;
    
    /* Complex function pointer with all delimiters */
    void (*(* GTY(()) complex)[2])(
        int arg1[],
        struct { int a; char b[3]; } arg2
    );
};

/* Test 10: Recursive structure with function pointer */
struct GTY(()) TreeNode {
    int value;
    struct TreeNode * GTY((user)) left;
    struct TreeNode * GTY((user)) right;
    
    /* Visitor callback */
    void (* GTY((user)) visit)(
        struct TreeNode * GTY((skip)) node,
        void * GTY((user)) context,
        int depth
    );
    
    /* Array of child processors */
    void (*processors[2])(
        int data[],
        struct { int count; char *items[]; } *config
    );
};

/* Test 11: Union with variant records */
union GTY(()) Variant {
    int int_val;
    double double_val;
    struct GTY(()) {
        char * GTY((length)) str;
        int length;
    } string_val;
    
    /* Function pointer variant */
    struct GTY(()) {
        int (* GTY(()) func)(int, char **);
        void * GTY((user)) user_data;
    } func_val;
};

/* Test 12: Structure with designated initializers pattern */
struct GTY(()) Config {
    struct {
        int sizes[3];
        struct {
            char name[20];
            int id;
        } entries[5];
    } sections[2];
    
    /* Callback table */
    struct GTY(()) {
        const char *name;
        void (* GTY(()) handler)(struct Config *, int);
    } callbacks[4];
};

/* Test 13: Type definition with nested parentheses */
typedef int (*CallbackType)(int (*nested)(char[5]), void *context);

struct GTY(()) UsesCallback {
    CallbackType GTY((user)) cb;
    void * GTY((user)) data;
    
    /* Array of callbacks */
    CallbackType callbacks[3];
};

/* Test 14: Complex macro with delimiters */
#define DECLARE_HANDLER(name, ret, ...) \
    ret (* GTY(()) name)(__VA_ARGS__)

DECLARE_HANDLER(global_handler, void,
    int param1,
    char * GTY((length)) param2[],
    struct { int x; double y; } * GTY(()) param3
);

/* Test 15: Final comprehensive test */
struct GTY((user)) Comprehensive {
    /* Nested function pointer with array return */
    int (*(*get_matrix)(void))[10];
    
    /* Anonymous union in struct */
    struct {
        union {
            int i;
            struct { char a; char b; } chars;
        } value;
        int count;
    } state;
    
    /* Multi-level pointer with attributes */
    struct Comprehensive ** GTY((chain_next, chain_prev)) peers;
    
    /* Complex method signature */
    void (* GTY((user)) complex_method)(
        int (*(*arg1)[5])(char[3]),
        struct Comprehensive * GTY((skip)) self,
        ...  /* Variadic arguments */
    );
};

/* Dummy main to make file compilable */
int main(void) {
    return 0;
}
