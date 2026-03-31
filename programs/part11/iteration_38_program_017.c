/* test_parse.h - Complex GTY annotations to test delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void *my_ptr;

/* Macros to add nesting depth */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define NESTED_BRACKET_EXPR [ (2 * 3) + (4 / 2) ]
#define BRACE_INITIALIZER { .x = 1, .y = {2, 3} }
#define COMPLEX_MACRO(x) ((x) * (x + 1))

/* Forward declarations */
struct forward_decl GTY(());

/* ========== TEST CASE 1: Parentheses in GTY options ========== */
struct s1 GTY((user)) {
    int data GTY((length("(sizeof(int) * (2 + 3))")))[];
    struct s1 *next GTY((chain_next("next"), chain_prev("prev")));
};

/* Complex parentheses nesting */
typedef struct GTY((tag("TAG1"))) {
    int (*callback GTY((callback)))(int, int (*)(int));
    void (*func_ptr GTY((skip)))(int (*(*)(int[ (2) ]))(void));
} complex_struct;

/* ========== TEST CASE 2: Brackets in array dimensions ========== */
struct s2 GTY(()) {
    /* Array with complex dimension expression */
    int arr1 GTY((length("N")))[ (2 * 3) + sizeof(int) ];
    
    /* Pointer to array with nested brackets */
    int (*arr_ptr GTY((skip)))[ (sizeof(int) > 4) ? 8 : 4 ];
    
    /* Multi-dimensional array */
    int matrix GTY((length("ROWS*COLS")))[ (1 << 2) ][ (1 << 3) ];
};

/* Array type with GTY on typedef */
typedef int array_type GTY((length("(1 << 5)")))[ (1 << 5) ];

/* ========== TEST CASE 3: Braces in nested structures ========== */
union u1 GTY((desc("(%1.u_tag ? 1 : 0)"))) {
    int i;
    
    /* Nested structure definition with braces */
    struct GTY((tag("INNER"))) {
        int x GTY((default("0")));
        int y GTY((default("0")));
    } nested;
    
    /* Array with initializer-like syntax in GTY */
    char arr[ 10 + (5) ];
};

/* Structure containing another structure definition */
struct s3 GTY((user)) {
    struct inner GTY((tag("LANG"))) {
        int x;
        struct inner *next;
    } nested_struct;
    
    /* Union with brace-enclosed initializer in macro */
    union {
        int a;
        double b;
    } u GTY((default(BRACE_INITIALIZER)));
};

/* ========== TEST CASE 4: Mixed delimiters ========== */
typedef void (*complex_func_ptr GTY((callback)))(
    int (*)(int[ (sizeof(int)) ], 
            struct { int x; int y; } GTY(()) *),
    void (*)(int, ...)
);

/* Structure with all delimiter types */
struct s4 GTY((chain_next("next"))) {
    /* Parentheses in length expression */
    int *dynamic_array GTY((length("(count + offset)")));
    
    /* Brackets in array declaration */
    int fixed_array[ (sizeof(double) * 2) ];
    
    /* Nested structure with braces */
    struct {
        int depth;
        struct s4 *parent;
    } GTY((skip)) hierarchy;
    
    /* Function pointer with complex signature */
    int (*processor GTY((callback)))(int (*[ (2) ])(void));
};

/* ========== TEST CASE 5: Forward declarations with GTY ========== */
struct forward_decl GTY((user)) {
    int value;
    struct forward_decl *next;
    
    /* Conditional expression in array dimension */
    char buffer[ (sizeof(void*) == 8) ? 64 : 32 ];
};

/* ========== TEST CASE 6: Template-like patterns ========== */
/* These simulate C++ template patterns that gengtype might encounter */
#define GTY_TEMPLATE(name) struct name GTY((user))

GTY_TEMPLATE(template_struct) {
    int data;
    GTY_TEMPLATE(template_struct) *next;
};

/* Complex macro expansion with delimiters */
#define DEFINE_ARRAY_TYPE(name, size) \
    typedef int name GTY((length(#size)))[ (size) + (sizeof(int)) ]

DEFINE_ARRAY_TYPE(my_array, 16);

/* ========== TEST CASE 7: Edge cases ========== */
/* Empty braces */
struct empty_braces GTY((user)) {
    struct {} empty;  /* Nested empty struct */
};

/* Multiple nested parentheses */
typedef int (*func_nesting GTY((callback)))(
    int (*)(int (*)(int (*)(int)))
);

/* Array of function pointers */
typedef int (*func_array GTY((skip))[ (4) ])(int, int);

#endif /* TEST_PARSE_H */
