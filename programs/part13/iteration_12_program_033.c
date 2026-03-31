/* complex-gty-test.c - Test file for exercising gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter lists and attributes */
typedef void (*GTY((user)) complex_func_ptr)(
    int (*GTY((skip)) nested_callback)(char GTY((length("10"))) buffer[10]),
    struct GTY((tag("1"))) { int x; int y; } nested_struct
);

/* Test 2: Array with complex dimension expression */
struct GTY(()) ArrayTest {
    int GTY((length("(10 + sizeof(struct ArrayTest))"))) 
        variable_length_array[(10 + sizeof(struct ArrayTest))];
    
    /* Nested array of function pointers */
    complex_func_ptr (*GTY((skip)) func_array[5])(int[][10]);
};

/* Test 3: Deeply nested structure with all delimiter types */
struct GTY((chain_next("next"), chain_prev("prev"))) OuterStruct {
    struct GTY((for_user)) InnerStruct {
        /* Function pointer returning pointer to array */
        int (*(*GTY((skip)) complex_member)(
            void (*)(int, char), 
            struct { int a; double b; }
        ))[10];
        
        /* Nested union with bitfields */
        union GTY((desc("%1.type"))) {
            int GTY((default)) i;
            struct GTY((tag("2"))) {
                unsigned int : 4;
                unsigned int field1: 8;
                unsigned int field2: 8;
            } GTY((skip)) bits;
            char GTY((length("sizeof(int)"))) bytes[sizeof(int)];
        } GTY((skip)) data;
    } GTY((skip)) inner;
    
    /* Array of structures with initializer-like designators */
    struct GTY((user)) {
        int x;
        double y;
        char z[20];
    } GTY((skip)) struct_array[3];
    
    struct OuterStruct *GTY((chain_next)) next;
    struct OuterStruct *GTY((chain_prev)) prev;
};

/* Test 4: Template-like macro expansion with nested delimiters */
#define DECLARE_VECTOR_TYPE(TYPE, SIZE) \
    struct GTY(()) Vector_##TYPE { \
        TYPE GTY((length("SIZE"))) data[SIZE]; \
        int (*(*GTY((skip)) operations[SIZE]))(TYPE (*)[SIZE]); \
    }

DECLARE_VECTOR_TYPE(int, 10);
DECLARE_VECTOR_TYPE(double, (5 * 2));

/* Test 5: Complex typedef with multiple nested levels */
typedef struct GTY((user)) {
    /* Pointer to array of function pointers */
    void (*(*GTY((skip)) level1[3])(
        int (*level2)(char[][5], struct { int x; }),
        double level3
    ))[];
    
    /* Nested anonymous struct with bitfield array */
    struct {
        unsigned int flags[2];
        struct GTY((tag("3"))) {
            int : 16;
            int value: 16;
        } packed;
    } GTY((skip)) container;
} UltraComplexType;

/* Test 6: Structure with complex initializer (for brace handling) */
static struct GTY(()) WithInitializer {
    int matrix[2][3];
    struct GTY((user)) {
        char *name;
        int id;
    } entries[4];
} GTY((user)) initialized_var = {
    .matrix = { {1, 2, 3}, {4, 5, 6} },
    .entries = {
        [0] = { .name = "first", .id = 1 },
        [2] = { .name = "third", .id = 3 },
        { .name = "fourth", .id = 4 },
        { .name = "fifth", .id = 5 }
    }
};

/* Test 7: Union containing all delimiter types */
union GTY((desc("%1.utype"))) AllDelimiters {
    /* Parentheses case */
    int (*func_ptr)(int (*)(char[]), double);
    
    /* Brackets case */
    struct {
        int multi_dim[2][(sizeof(int) * 2)][3];
        char *GTY((length("10"))) strings[10];
    } arrays;
    
    /* Braces case */
    struct GTY((tag("4"))) {
        struct GTY((user)) {
            int x;
            struct { int a; int b; } point;
        } nested;
        union {
            int i;
            float f;
        } value;
    } structures;
};

/* Test 8: Recursive structure with function pointers */
struct GTY((chain_next("link"))) TreeNode {
    char *GTY((length("depth * 2 + 1"))) data;
    int depth;
    
    /* Array of child nodes with function pointers as comparators */
    struct TreeNode *GTY((length("child_count"))) children[];
    
    /* Comparator function pointer with complex signature */
    int (*GTY((skip)) compare)(
        struct TreeNode *,
        struct TreeNode *,
        int (*)(const char *, const char *)
    );
    
    struct TreeNode *GTY((chain_next)) link;
};

/* Test 9: Mixed attributes with nested parenthesized expressions */
struct GTY((
    user,
    desc("%0.is_union ? UNION : STRUCT"),
    tag("5")
)) MixedAttributes {
    int is_union;
    
    /* Conditional array size based on expression */
    char buffer[sizeof(struct MixedAttributes) > 16 ? 32 : 16];
    
    /* Function pointer with attribute in parameter */
    void (*GTY((skip)) handler)(
        int GTY((user)) param1,
        struct GTY((tag("6"))) { 
            int x; 
            int y; 
        } param2
    );
};

/* Test 10: Multiple levels of nested GTY annotations */
typedef struct GTY((user)) Level1 {
    struct GTY((for_user)) Level2 {
        struct GTY((skip)) Level3 {
            /* Triple pointer with array dimensions */
            int (***GTY((skip)) triple_ptr)[10][20];
            
            /* Nested function pointer array */
            void (*GTY((skip)) func_table[5])(
                int,
                char *GTY((length("len"))) [],
                struct Level1 *
            );
        } *level3_ptr;
    } level2;
} *Level1Ptr;

/* Main function to make the file compilable */
int main(void) {
    /* Dummy usage to avoid compiler warnings */
    struct ArrayTest at = {0};
    struct OuterStruct os = {0};
    UltraComplexType uct = {0};
    
    (void)at;
    (void)os;
    (void)uct;
    
    return 0;
}
