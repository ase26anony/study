/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void* my_ptr;

/* Macros to add nesting depth */
#define NESTED_EXPR ( (10) + (20) )
#define COMPLEX_SIZE (sizeof(int) * (2 + 3))
#define ARRAY_DIM ( (5) * (6) )
#define BRACE_INITIALIZER { 1, 2, 3, 4, 5 }
#define CALLBACK_ARG int (*)(int [ (4) ])

/* Forward declarations */
struct forward_decl GTY((user));

/* Case 1: Parentheses - struct with GTY((length("(complex expression)"))) */
struct s1 GTY((length("(2 + 3) * sizeof(int)"))) {
    int data[];
};

/* More complex parentheses nesting */
struct s2 GTY((chain_next("next"), chain_prev("prev"), 
               length("NESTED_EXPR * 2"))) {
    struct s2 *next;
    struct s2 *prev;
    int items[];
};

/* Case 2: Brackets - array types with complex dimension expressions */
struct s3 GTY((length("ARRAY_DIM"))) {
    int matrix[ (2 * 3) ][ (4 + 1) ];
    char buffer[ sizeof(struct { int x; char y; }) ];
};

/* Pointer to array with GTY */
typedef int (*array_ptr GTY((tag("ARRAY_PTR"))))[ (10) + (5) ];

/* Multi-dimensional array with nested brackets */
union u1 GTY(()) {
    int i;
    char arr[ 10 + (5) ][ (3) * (2) ];
    double matrix[ (1 << 2) ][ (1 << 3) ];
};

/* Case 3: Braces - structure with nested struct definition */
struct outer GTY((user)) {
    struct inner GTY((tag("NESTED"))) {
        int x;
        double y;
        char z;
    } nested;
    
    union inner_union GTY((desc("$1.type"))) {
        int ival;
        double dval;
        struct { int a; char b; } sval;
    } u;
};

/* Function pointer with complex signature and GTY((callback)) */
typedef void (*complex_callback GTY((callback)))(
    int (*)(int [ (4) ]), 
    void (*)(struct { int x; }*),
    char (*)[ sizeof(int[ (2) ]) ]
);

/* GTY with skip directive containing parentheses */
struct skip_example GTY((skip(" (skip_me != NULL) "))) {
    void *skip_me;
    int value;
};

/* GTY with param_is directive containing nested parentheses */
struct param_example GTY((param_is(" (typeof(T)) "))) {
    void *data;
    size_t size;
};

/* Array with GTY and variable length using macro expansion */
#define VAR_LEN ( (int)(sizeof(long) * 8) )
struct var_array GTY((length("VAR_LEN"))) {
    unsigned char bits[ VAR_LEN ];
};

/* Nested GTY annotations with all delimiters */
struct comprehensive GTY((chain_next("nxt"), chain_prev("prv"))) {
    struct comprehensive *nxt;
    struct comprehensive *prv;
    
    /* Array with parenthesized size */
    int scores[ (10) + ( (5) * (2) ) ];
    
    /* Nested struct with braces */
    struct {
        int id;
        char name[ (20) + (1) ];
    } info GTY((tag("INFO")));
    
    /* Union with array */
    union {
        int as_int[ (4) ];
        double as_double[ (2) ];
    } data;
};

/* Template-like macro with GTY */
#define DECLARE_GTY_ARRAY(T, N) \
    typedef T GTY((tag(#T))) array_##T[N]

DECLARE_GTY_ARRAY(int, (10));
DECLARE_GTY_ARRAY(double, ( (5) * (2) ));

/* GTY with if_marked directive containing parentheses */
struct marked_example GTY((if_marked(" (mark_flag & 1) "))) {
    int mark_flag;
    void *payload;
};

/* Complex function pointer type with GTY */
typedef int (*(*nested_func_ptr GTY((callback)))(void))(
    int, 
    char (*)[ (10) ],
    struct { int x; double y; }*
);

/* Structure with flexible array member and complex GTY */
struct flex_array GTY((length("(size + 7) & ~7"))) {
    size_t size;
    unsigned char data[];
};

/* Union with GTY and nested type definitions */
union complex_union GTY((desc("$1.type"))) {
    struct type1 { int a; char b[ (8) ]; } t1;
    struct type2 { double x; int y[ (4) ]; } t2;
    struct type3 { 
        struct nested { short s; } n; 
        char buf[ sizeof(struct { int i; }) ];
    } t3;
};

/* GTY with user directive containing type expression */
struct user_example GTY((user(" (user_type *) "))) {
    int type_id;
    void *user_data;
};

/* Array of pointers with GTY */
typedef struct ptr_element {
    int value;
    struct ptr_element *next;
} *ptr_array GTY((length("count")))[];

/* Final test: all delimiters in one GTY annotation */
struct ultimate_test GTY((
    chain_next("next"),
    chain_prev("prev"),
    length("(alloc_size / sizeof(int))"),
    skip(" (skip_test != 0) "),
    param_is(" (param_type) "),
    if_marked(" (mark_bits & MARK_FLAG) ")
)) {
    struct ultimate_test *next;
    struct ultimate_test *prev;
    int skip_test;
    int mark_bits;
    size_t alloc_size;
    int items[];
};

#endif /* TEST_PARSE_H */
