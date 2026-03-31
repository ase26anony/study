/* complex_gty_test.c - Test file for gengtype delimiter parsing */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter lists and attributes */
typedef void (*GTY((user)) complex_func_ptr)(
    int (*GTY((skip)) nested_callback)(char GTY((length("10"))) buffer[10]),
    struct GTY(()) nested_struct { 
        int x; 
        void (*GTY((chain_next)) another_fn)(int[][5]);
    } param
);

/* Test 2: Array declarations with complex dimensions containing parentheses */
struct GTY(()) ArrayTest {
    /* Array size with expression containing parentheses */
    int arr1[GTY((user)) (10 + sizeof(struct GTY(()) Dummy { int y; }))];
    
    /* Multi-dimensional array with function pointer type */
    void (*arr2[3][2])(int (*)(char[][10]), struct { int a; double b; });
    
    /* Array of pointers to arrays */
    int (*arr3[5])[10];
};

/* Test 3: Deeply nested structure with all delimiter types */
union GTY(()) DeeplyNested {
    struct GTY(()) Level1 {
        int x;
        struct GTY(()) Level2 {
            char c;
            union GTY(()) Level3 {
                double d;
                struct GTY(()) Level4 {
                    /* Mix of all delimiters in one member */
                    void (*complex_member)(
                        int arg1,
                        char arg2[][(sizeof(struct GTY(()) S { int z; }) + 5)],
                        struct GTY(()) { 
                            int a; 
                            int b[10]; 
                        } arg3
                    );
                } l4;
            } l3;
        } l2;
    } l1;
    
    /* Initializer with nested braces */
    struct GTY(()) InitExample {
        int values[5];
        struct GTY(()) Point {
            int x, y;
        } points[3];
    } init;
};

/* Test 4: Template-like pattern using nested parentheses and brackets */
#define GTY_TEMPLATE(name, type) \
    struct GTY(()) name##_container { \
        type GTY((tag("0"))) data; \
        struct name##_container* GTY((skip)) next; \
    }

/* Instantiate with complex type */
GTY_TEMPLATE(MyTemplate, 
    struct GTY(()) {
        int (*compute[5])(int matrix[10][10], 
                         void (*callback)(struct { int x; int y; }));
        union GTY(()) {
            char str[100];
            int* ptr;
        } data;
    }
);

/* Test 5: Multiple balanced delimiters in sequence */
struct GTY(()) MultiDelimiter {
    /* Contains: (* []) (int [][]) { } */
    void (*fn_array[5])(
        int param1[][10],
        struct GTY(()) { 
            int tag; 
            union GTY(()) { 
                int i; 
                char* GTY((length("len"))) s; 
            } value; 
        } param2
    );
    
    /* Complex initializer with designators */
    struct GTY(()) InitStruct {
        int a;
        int b[3];
        struct GTY(()) { int x, y; } point;
    } init GTY((user)) = {
        .a = 1,
        .b = {[(2 + 3) % 4] = 5, [1] = 10, [0] = 20},
        .point = {.x = 100, .y = 200}
    };
};

/* Test 6: Chain of pointers with nested attributes */
struct GTY(()) ListNode {
    int GTY((user)) data;
    struct ListNode* GTY((chain_next("next"), chain_prev("prev"))) next;
    struct ListNode* GTY((chain_next("next"), chain_prev("prev"))) prev;
    
    /* Nested function pointer in list node */
    void (*GTY((skip)) process)(
        struct ListNode* list,
        int (*GTY((user)) compare)(const void*, const void*)
    );
};

/* Test 7: Union with anonymous structs containing arrays of function pointers */
union GTY(()) ComplexUnion {
    struct GTY(()) {
        int (*funcs[3])(
            char* GTY((length("len"))) str,
            int arr[][(sizeof(int*) * 2)]
        );
        union GTY(()) {
            long l;
            double d;
        } value;
    } s;
    
    struct GTY(()) {
        void (*vtable[5])(void);
        struct GTY(()) {
            int x, y;
        } coords[10];
    } t;
};

/* Test 8: Typedef with deeply nested parentheses */
typedef int (*(*(*GTY((user)) nested_func_factory)(void))[5])(
    struct GTY(()) { 
        int id; 
        char name[50]; 
    } config,
    int (*handlers[])(int, char**)
);

/* Test 9: Structure with bit-fields and nested arrays */
struct GTY(()) BitFieldTest {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int : 0;  /* Force alignment */
    
    /* Array of structures containing function pointers */
    struct GTY(()) Handler {
        int (*process)(int, char*[]);
        void (*cleanup)(struct GTY(()) { int count; void* data; } ctx);
    } handlers[4];
    
    /* Anonymous union with array */
    union {
        int nums[10];
        struct GTY(()) {
            char* str;
            int len;
        } strings[5];
    } data;
};

/* Test 10: Recursive structure with complex function pointer */
struct GTY(()) TreeNode {
    int value;
    struct TreeNode* GTY((user)) left;
    struct TreeNode* GTY((user)) right;
    
    /* Visitor function with nested parameter */
    void (*GTY((skip)) visit)(
        struct TreeNode* node,
        void (*GTY((user)) action)(
            int depth,
            struct { int path[100]; } context
        )
    );
};

/* Main function to make the file compilable (though GTY expands to nothing normally) */
int main(void) {
    return 0;
}
