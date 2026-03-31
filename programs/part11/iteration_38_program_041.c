/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void (*generic_callback)(void);

/* Macros to introduce nested delimiters */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define COMPLEX_BRACKET_EXPR [ (10) + (20) ]
#define BRACE_INITIALIZER { .x = 1, .y = {2, 3} }
#define NESTED_MACRO(x) ( (x) * ((x) + 1) )

/* ====== Test Case 1: Parentheses in GTY options ====== */
struct s1 GTY((user)) {
    /* Length with nested parentheses */
    int* data GTY((length("(sizeof(int) * (2 + 3))")));
    
    /* Multiple parenthesized groups */
    struct s1* next GTY((chain_next("next"), chain_prev("prev")));
    
    /* Deeply nested parentheses */
    int* deep_nested GTY((length("(((1 + 2) * (3 + 4)) / (5 - 1))")));
};

/* Function pointer with complex parentheses */
typedef int (*complex_func_ptr GTY((callback)))(
    int (*)(int, char*), 
    void* GTY((skip))
);

/* ====== Test Case 2: Brackets in array dimensions ====== */
struct s2 GTY(()) {
    /* Array with parenthesized dimension expression */
    int arr1 GTY((length("N")))[ (2 * 3) ];
    
    /* Multi-dimensional array with nested brackets */
    int arr2 GTY((length("M")))[ 2 ][ (3 + 4) ];
    
    /* Pointer to array with complex dimension */
    int (*ptr_to_arr) GTY((length("P")))[ (sizeof(int) == 4 ? 10 : 20) ];
};

/* Array type with GTY and nested brackets */
typedef int array_type GTY((length("(1 << 5)")))[ (1 << 5) ];

/* ====== Test Case 3: Braces in nested structures ====== */
struct outer GTY((user)) {
    /* Nested struct definition with braces */
    struct inner GTY((tag("LANG"))) {
        int x;
        int y[2];
    } nested;
    
    /* Union with brace-enclosed initializer in comment/string */
    union {
        int i;
        char str[10];
    } u GTY((desc("$1"), param_is("union { int i; char str[10]; }")));
};

/* Structure with bitfield containing parenthesized expression */
struct bitfield_struct GTY(()) {
    unsigned int flags: (sizeof(int) * 8 - 1);
    unsigned int value GTY((length("(flags & 1) ? 10 : 20")))[];
};

/* ====== Test Case 4: Mixed delimiters ====== */
struct mixed_delimiters GTY((user)) {
    /* All three delimiters in different contexts */
    
    /* 1. Parentheses in callback specification */
    void (*callback GTY((callback)))(
        int arg1[ (sizeof(int)) ], 
        struct mixed_delimiters* GTY((skip))
    );
    
    /* 2. Brackets in array with parenthesized size */
    char buffer GTY((length("(BUF_SIZE + 1)")))[ (256 + 128) ];
    
    /* 3. Nested structure with braces */
    struct {
        int* data GTY((length("(count * sizeof(int))")));
        int count;
    } container;
};

/* ====== Test Case 5: Template-like macros with delimiters ====== */
#define DEFINE_GTY_STRUCT(name, size_expr) \
    struct name##_t GTY((user)) { \
        int data GTY((length(#size_expr)))[ size_expr ]; \
        struct name##_t* next; \
    }

/* Instantiate with complex expressions */
DEFINE_GTY_STRUCT(complex1, (10 * (2 + 3)));
DEFINE_GTY_STRUCT(complex2, ((1 << 4) - 1));

/* ====== Test Case 6: Forward declarations with GTY ====== */
struct forward_decl GTY((user));

struct uses_forward GTY(()) {
    struct forward_decl* ptr GTY((tag("1")));
    int* array GTY((length("(ptr ? ((int*)ptr)[0] : 0)")))[];
};

struct forward_decl {
    int data[ (sizeof(int) * 2) ];
    struct uses_forward* backref;
};

/* ====== Test Case 7: Union with variant delimiters ====== */
union variant_union GTY((desc("$1"))) {
    /* Case with parentheses */
    int (*func_ptr)(int, int);
    
    /* Case with brackets */
    int array[ (10) ];
    
    /* Case with nested structure braces */
    struct {
        char* str GTY((length("(len + 1)")));
        int len;
    } str_info;
};

/* ====== Test Case 8: GTY options with string literals containing delimiters ====== */
struct string_delimiters GTY((
    tag("LANG"),
    param_is("struct { int x; char s[(10)]; }"),
    user
)) {
    int x;
    char s[ (10) ];
};

/* ====== Test Case 9: Multiple GTY annotations on same type ====== */
typedef struct multi_gty GTY((user)) {
    /* Chain of pointers with skip parameter containing parentheses */
    struct multi_gty* chain GTY((chain_next("chain"), skip(("next"))));
    
    /* Array with skip_if containing parenthesized condition */
    void* data GTY((
        length("size"),
        skip_if("(*(int*)data == -1)")
    ))[];
    int size;
} multi_gty_t;

/* ====== Test Case 10: Edge cases ====== */
/* Empty parentheses */
struct empty_parens GTY(( )) {
    int x;
};

/* Nested brackets in typedef */
typedef int (*nested_bracket_func GTY((callback)))(
    int matrix[][ (3 * 4) ]
);

/* Structure with all delimiter types in one member */
struct all_in_one GTY((user)) {
    /* Function returning pointer to array, taking array parameter */
    int (*(*complex_member GTY((callback)))(
        int arg[ (2) ][ (3) ]
    ))[ (4) ];
};

#endif /* TEST_PARSE_H */
