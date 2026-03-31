/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void *ptr_t;

/* Macros to add nesting depth */
#define NESTED_EXPR ( (10) + (20) )
#define COMPLEX_SIZE (sizeof(int) * (2 + 3))
#define ARRAY_DIM ( (5) * (3) )
#define BRACE_INIT { 1, 2, 3, 4, 5 }
#define NESTED_PARENS(x) (((x) + 1) * ((x) - 1))

/* Forward declarations */
struct forward_decl;

/* ========== TEST CASE 1: Parentheses in GTY options ========== */
struct GTY((user)) s1 {
    /* Array with complex length expression containing nested parentheses */
    int * GTY((length("(sizeof(int) * (2 + 3))"))) data1;
    
    /* Multiple parenthesized groups in chain options */
    struct s1 * GTY((chain_next("next"), chain_prev("prev"))) next;
    
    /* Function pointer with nested parentheses in type */
    void (* GTY((callback)) func_ptr)(int (*)(int));
};

/* ========== TEST CASE 2: Brackets in array dimensions ========== */
union GTY(()) u1 {
    /* Array with dimension containing parentheses */
    int arr1[ (2 * 3) ];
    
    /* Multi-dimensional array with complex dimensions */
    char arr2[ 5 ][ (3 + 2) ];
    
    /* Pointer to array with dimension expression */
    int (* GTY((tag("ARR_PTR"))) arr_ptr)[ NESTED_EXPR ];
};

/* ========== TEST CASE 3: Complex nested cases ========== */
typedef struct GTY((for_user)) s2 {
    /* Flexible array member with macro containing parentheses */
    my_int data GTY((length("NESTED_EXPR")))[];
    
    /* Nested structure definition (contains braces) */
    struct GTY((tag("INNER"))) inner {
        int x;
        int y GTY((length("(1 << 5)")))[];
    } nested;
    
    /* Union with array containing bracket expressions */
    union {
        int i;
        char c[ (sizeof(int) + 1) ];
    } GTY((desc("0"))) variant;
} s2_t;

/* ========== TEST CASE 4: Function pointers with complex signatures ========== */
/* Callback with array parameter containing dimension expression */
typedef void (* GTY((callback)) complex_cb_t)(
    int (*processor)(int arr[ (4) ]),
    void * GTY((skip)) context
);

/* Function pointer type with nested parentheses in return type */
typedef int (* GTY((callback)) (*nested_fp_t)(int))[ (2 + 3) ];

/* ========== TEST CASE 5: Structure with all delimiter types ========== */
struct GTY((user, desc("%1.type"))) s3 {
    /* Parentheses in skip expression */
    char * GTY((skip("(void *)0"))) skipped_ptr;
    
    /* Array with multiple bracket levels */
    int (* GTY((length("NESTED_EXPR"))) matrix)[ (3) ][ (4) ];
    
    /* Nested anonymous struct (contains braces) */
    struct {
        int count;
        int values GTY((length("count"))) [];
    } GTY((tag("ANON"))) anonymous;
    
    /* Conditional expression in param_is option */
    int GTY((param_is("(type == 1) ? &param1 : &param2"))) conditional;
};

/* ========== TEST CASE 6: Union with nested initializer-like syntax ========== */
union GTY((desc("%0.union_tag"))) u2 {
    int tag;
    
    /* Structure that looks like it has an initializer */
    struct GTY((tag("1"))) {
        int length;
        /* Array dimension with parentheses */
        char data[ (256) ];
    } string;
    
    /* Another with complex dimension */
    struct GTY((tag("2"))) {
        int count;
        /* Multi-dimensional with expressions */
        float points[ (10) ][ (3) ];
    } cloud;
};

/* ========== TEST CASE 7: Template-like macro usage ========== */
#define DEFINE_GTY_STRUCT(name, size) \
    struct GTY((user)) name { \
        int id; \
        char buffer GTY((length("(" #size ")")))[]; \
    }

/* Instantiate the macro (expands with parentheses) */
DEFINE_GTY_STRUCT(macro_struct, 256);

/* ========== TEST CASE 8: Pointer chains with nested expressions ========== */
struct GTY((chain_next("next"), chain_prev("prev"))) list_node {
    int value;
    /* Array with computed size */
    int extra GTY((length("(value > 0) ? value : 1"))) [];
    struct list_node *next;
    struct list_node *prev;
};

/* ========== TEST CASE 9: Type with deeply nested parentheses ========== */
typedef struct GTY((for_user)) deep_nested {
    /* Very deeply nested parenthetical expression */
    int * GTY((length("((((16) * (32)) + ((8) * (64))) / sizeof(int))"))) deep_array;
    
    /* Function pointer with complex signature */
    void (* GTY((callback)) deep_func)(
        int (*)(int (*)[ (2) ], void *),
        struct deep_nested * GTY((skip))
    );
} deep_nested_t;

/* ========== TEST CASE 10: Edge cases ========== */
/* Empty parentheses */
struct GTY(()) empty_options {
    int x;
};

/* Nested GTY markers */
struct outer GTY((user)) {
    struct inner GTY((tag("INNER"))) {
        int value;
    } nested;
    
    /* Array with empty brackets (flexible array member) */
    int flex_array GTY((length("1"))) [];
};

/* Forward declaration with GTY */
struct GTY((user)) forward_decl;

/* Complete the forward declaration */
struct forward_decl {
    int completed;
    struct forward_decl * GTY((skip)) next;
};

#endif /* TEST_PARSE_H */
