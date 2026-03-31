/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef unsigned int size_t;

/* Macros to add nesting depth */
#define NESTED_PAREN_EXPR ( (10) + (20) * ( (5) - (2) ) )
#define ARRAY_DIM_EXPR [ (2 * 3) + (4 / 2) ]
#define BRACE_INITIALIZER { .x = (1), .y = (2) }

/* Macro that expands to brace-enclosed code (tricky but possible in GTY context) */
#define GTY_SPECIAL_MARKER ((user))

/* Case 1: Parentheses - struct with GTY((length("complex expression"))) */
struct s1 GTY((length("(sizeof(my_int) * " "(2 + 3)" ")" ))) {
    int data[];
};

/* More parentheses: multiple GTY options with nested parens */
struct s2 GTY((chain_next("next"), chain_prev("prev"), 
               user, skip)) {
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Case 2: Brackets - array types with complex dimension expressions */
struct s3 GTY((length("N"))) {
    int arr[ (2 * 3) + sizeof(int) ];
};

/* Pointer to array with nested brackets */
typedef int (*complex_array_ptr GTY((tag("ARRAY_PTR"))))[ (10) ][ (20) ];

/* Multi-dimensional array with GTY */
struct s4 GTY(()) {
    int matrix[ (1 << 2) ][ (1 << 3) ][ (1 << 4) ];
};

/* Case 3: Braces - structure with nested struct definition inside GTY */
struct s5 GTY((user)) {
    struct inner GTY((tag("NESTED"))) {
        int x;
        int y;
    } nested;
};

/* Union with array and GTY marker containing brace-like syntax */
union u1 GTY((desc("(%d)"))) {
    int i;
    char arr[ 10 + (5) ];
    struct {
        float f;
        double d;
    } s;
};

/* Function pointer type with GTY((callback)) and complex argument list */
typedef void (*callback_func GTY((callback)))(int (*)(int [ (4) ][ (8) ]), 
                                              struct s1 *);

/* Forward declared struct with GTY containing parenthesized conditional */
struct s6 GTY((if("(CONDITION && (SUB_CONDITION || DEFAULT))"))) {
    int value;
};

/* GTY with deeply nested parentheses in length expression */
struct s7 GTY((length("((((1 + 2) * (3 - 4)) / (5 % 6)) + 7)"))) {
    unsigned char bytes[];
};

/* Array of pointers with GTY and nested brackets */
typedef struct s2* (array_of_ptrs GTY((length("COUNT"))))[ (16) ];

/* Structure with GTY option containing macro that expands to parentheses */
struct s8 GTY((length( "NESTED_PAREN_EXPR" ))) {
    long data[];
};

/* Complex GTY combination - testing all delimiters in one */
struct s9 GTY((chain_next("((next))"), 
               chain_prev("prev"),
               user,
               skip,
               length("(sizeof(struct s1) + (alignof(struct s2) * 2))"))) {
    struct s9 *next;
    struct s9 *prev;
    int values[ (10) + (20) ];
    union {
        int i;
        float f;
    } u;
};

/* Template-like macro usage with GTY (C++ style comment for gengtype) */
#define DEFINE_GTY_STRUCT(name, size) \
    struct name##_t GTY((length(#size))) { \
        int data[size]; \
    }

/* Actually define one using the macro */
struct macro_struct GTY((length("(100)"))) {
    int items[ (50) * (2) ];
};

/* Enum with GTY - though enums don't typically need GC, test parentheses */
enum my_enum GTY((tag("ENUM"))) {
    VALUE1 = (1 << 0),
    VALUE2 = (1 << 1),
    VALUE3 = (1 << 2) | (1 << 3)
};

/* Typedef with GTY and function type containing nested parentheses */
typedef int (*complex_func_ptr GTY((callback)))(
    int (*)(int, int), 
    void (*)(char *[(10)])
);

/* Structure with bitfield and GTY */
struct s10 GTY((user)) {
    unsigned int flag1 : (1);
    unsigned int flag2 : (2);
    unsigned int flag3 : (3);
    int regular_field;
};

/* Test case for error recovery - malformed but recoverable */
struct s11 GTY((length("unclosed /* comment */"), user)) {
    int x;
};

/* Another complex case: pointer to function returning pointer to array */
typedef int (*(*func_returning_array_ptr GTY((callback)))(void))[ (10) + (20) ];

#endif /* TEST_PARSE_H */
