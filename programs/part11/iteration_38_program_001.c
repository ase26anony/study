/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void (*generic_callback)(void);

/* Macros to introduce nested delimiters */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define COMPLEX_BRACKET_EXPR [ (2 * 3) + (4 / 2) ]
#define BRACE_INITIALIZER { .x = 1, .y = {2, 3} }

/* Macro that expands to brace-enclosed code */
#define BRACE_BLOCK do { int temp = (1 + 2); } while(0)

/* ====== Test Case 1: Parentheses in GTY options ====== */

/* Struct with GTY length containing nested parentheses */
struct s1 GTY((user)) {
    int count;
    /* Array with complex length expression in parentheses */
    int* data GTY((length("(sizeof(int) * (2 + 3))")));
};

/* Another struct with multiple parenthesized groups */
struct s2 GTY((chain_next("next"), chain_prev("prev"))) {
    struct s2* next;
    struct s2* prev;
    int value;
    /* Using macro with nested parentheses */
    char buffer GTY((length("NESTED_PAREN_EXPR")))[];
};

/* Function pointer type with complex argument list */
typedef int (*complex_func_ptr GTY((callback)))(
    int (*)(int, char*), 
    void* GTY((skip))
);

/* ====== Test Case 2: Brackets in type declarations ====== */

/* Array type with dimension expression containing parentheses */
struct s3 GTY(()) {
    int arr1 GTY((length("10")))[ (2 * 3) ];
    /* Pointer to array with nested brackets */
    int (*arr2 GTY((length("5"))))[ (4) ][ (2) ];
    /* Multi-dimensional array with complex dimensions */
    char matrix GTY((length("(3 + 2)")))[ (1 + 2) ][ (2 * 2) ];
};

/* Union with array containing bracket expressions */
union u1 GTY((desc("tag"))) {
    int tag;
    /* Array with size from macro containing brackets */
    char data[ 10 + (5) ];
    /* Nested array declaration */
    float matrix[ (2) ][ (3) ];
};

/* Typedef for pointer to array with GTY */
typedef int (*array_ptr GTY((skip)))[ (sizeof(int) == 4) ? 10 : 20 ];

/* ====== Test Case 3: Braces in type definitions ====== */

/* Struct with nested struct definition (contains braces) */
struct outer GTY((user)) {
    int id;
    /* Nested struct with braces, marked with GTY */
    struct inner GTY((tag("NESTED"))) {
        int x;
        int y;
        char name[32];
    } nested;
};

/* Union with anonymous struct containing braces */
union u2 GTY((desc("type"))) {
    int type;
    struct GTY((tag("INT_DATA"))) {
        int values[5];
        char* name;
    } int_data;
    struct GTY((tag("STR_DATA"))) {
        char* str;
        int length;
    } str_data;
};

/* Forward declaration with GTY that will have nested braces later */
struct forward_decl GTY((user));

/* Complete definition with complex initializer-like syntax in comment */
struct forward_decl GTY((user)) {
    int value;
    /* GTY option referencing a macro that expands to braces */
    struct forward_decl* next GTY((skip, reorder("test_reorder")));
};

/* ====== Test Case 4: Mixed delimiters ====== */

/* Struct combining all delimiter types */
struct mixed GTY((chain_next("next"), chain_prev("prev"))) {
    struct mixed* next;
    struct mixed* prev;
    
    /* Parentheses in length expression */
    int* dynamic_array GTY((length("(count * sizeof(int))")));
    
    /* Brackets in array declaration with parenthesized size */
    int fixed_array[ (10 + (2 * 3)) ];
    
    /* Nested struct with braces */
    struct {
        int x;
        int y GTY((length("(2)")))[2];
    } point;
    
    int count;
};

/* Function pointer with complex signature containing all delimiters */
typedef void (*mixed_func_ptr GTY((callback)))(
    int (*handler)(int[ (2) ], struct mixed*),
    void* context GTY((skip))
);

/* ====== Test Case 5: Template-like patterns ====== */

/* Macro that creates GTY annotations with nested delimiters */
#define DEFINE_GTY_STRUCT(name, size_expr) \
    struct name##_t GTY((user)) { \
        int id; \
        char data GTY((length(#size_expr)))[size_expr]; \
    }

/* Instantiate with complex expressions */
DEFINE_GTY_STRUCT(inst1, (10 + (2 * 3)));
DEFINE_GTY_STRUCT(inst2, (sizeof(int) * (1 << 2)));

/* ====== Test Case 6: Edge cases ====== */

/* Empty braces in nested struct */
struct with_empty_braces GTY((user)) {
    struct empty GTY((tag("EMPTY"))) {
        /* intentionally empty */
    } empty_member;
};

/* GTY annotation with string containing delimiters */
struct string_delimiters GTY((user)) {
    char* pattern GTY((length("strlen(\"(test)\")")));
    int flags;
};

/* Conditional compilation with GTY */
#ifdef SPECIAL_FEATURE
struct conditional GTY((user)) {
    int special_data[ (4) + (2) ];
};
#else
struct conditional GTY((user)) {
    int normal_data[ (2) ];
};
#endif

/* ====== Test Case 7: Recursive structures ====== */

/* Self-referential struct with GTY */
struct tree_node GTY((user)) {
    int value;
    struct tree_node* left GTY((skip));
    struct tree_node* right GTY((skip));
    /* Array of pointers with parenthesized size */
    struct tree_node** children GTY((length("(2)")))[2];
};

/* Union with recursive reference */
union recursive_union GTY((desc("type"))) {
    int type;
    struct tree_node* node GTY((skip));
    union recursive_union* next GTY((skip));
};

#endif /* TEST_PARSE_H */
