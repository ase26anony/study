/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void (*generic_callback)(void);

/* Macros to introduce nested delimiter layers */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define COMPLEX_BRACKET_EXPR [ (10) + (20) ]
#define BRACE_INITIALIZER { .x = 1, .y = 2 }
#define NESTED_MACRO(x) ( (x) * ((x) + 1) )

/* Case 1: Parentheses - struct with GTY length containing nested parentheses */
struct s1 GTY((user)) {
    int count;
    /* Flexible array member with complex length expression */
    int data GTY((length("(sizeof(int) * (2 + 3))"))) [];
};

/* Additional parentheses case with multiple parenthesized groups */
struct s2 GTY((chain_next("next"), chain_prev("prev"))) {
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Case 2: Brackets - array types with complex dimension expressions */
struct s3 GTY((user)) {
    /* Array with dimension containing parentheses */
    int arr1 GTY((tag("ARR1")))[ (2 * 3) + 4 ];
    
    /* Pointer to array with nested brackets */
    int (*arr2 GTY((length("N"))))[ (5) ];
    
    /* Multi-dimensional array */
    int matrix GTY((user))[ 3 ][ (2 + 1) ];
};

/* Typedef with array declarator containing brackets */
typedef struct s4 GTY((user)) {
    my_int values[ NESTED_MACRO(5) ];
} s4_t;

/* Case 3: Braces - structure with nested struct definition */
struct outer GTY((user)) {
    /* Nested structure definition (contains braces) */
    struct inner GTY((tag("INNER"))) {
        int x;
        int y;
    } nested;
    
    /* Union with initializer-like syntax in GTY */
    union u GTY((desc("((%1.type == 0) ? 0 : 1)"))) {
        int i;
        double d;
    } value;
};

/* Function pointer type with complex argument list containing brackets */
typedef void (*complex_func_ptr GTY((callback)))(
    int (*)(int arr[ (sizeof(int) > 4) ? 8 : 4 ]),
    void (*)(struct s3 *s[ (2) ])
);

/* Union with array containing parenthesized size */
union u1 GTY((user)) {
    int i;
    char arr[ 10 + (5 * 2) ];
    long long big_num;
};

/* Forward-declared struct with GTY annotation containing nested expression */
struct forward_decl GTY((user, desc("%1.tag == 0 ? 0 : 1"))) {
    int tag;
    union {
        int ival;
        double dval;
    } data;
};

/* Structure with skip field containing complex expression */
struct skip_example GTY((user)) {
    void *ptr GTY((skip(("(char *)%1 - (char *)&%1"))));
    int length;
};

/* Array of pointers with nested GTY options */
struct array_of_ptrs GTY((user)) {
    /* Length expression with function call-like syntax */
    struct s1 **items GTY((length("(int)(get_count() * sizeof(void*))"))) ;
    int count;
};

/* Nested structure with deep parenthesis nesting */
struct deeply_nested GTY((user)) {
    /* Use macro that expands to deeply nested parentheses */
    int value GTY((if("(NESTED_MACRO(10) > (50 + (20 * 2)))")));
    
    /* Chain with complex conditional */
    struct deeply_nested *next GTY((chain_next(
        "(%1.value > (10 + (5 * 2))) ? %1.next : NULL"
    )));
};

/* Test case with all three delimiters in one type */
struct all_delimiters GTY((user)) {
    /* Parentheses in length expression */
    int *array GTY((length("(sizeof(int) * ((2) + (3)))"))) ;
    
    /* Brackets in array declaration */
    char buffer[ (16) + (8) ];
    
    /* Nested structure (braces) */
    struct {
        int x;
        int y;
    } point GTY((tag("POINT")));
};

/* Enum with GTY marker (rare but valid) */
enum my_enum GTY((user)) {
    VAL1 = (1 << 0),
    VAL2 = (1 << 1),
    VAL3 = (1 << (1 + 1))
};

/* Typedef with function type containing complex parameter list */
typedef int (*comparator_fn GTY((callback)))(
    const void *a,
    const void *b,
    int (*helper)(int matrix[3][ (2) ])
);

/* Structure with conditional expressions in GTY options */
struct conditional_expr GTY((user)) {
    void *data GTY((if("(condition && (subcondition || fallback))")));
    int type GTY((desc("(%1.type == (TYPE_A | TYPE_B)) ? 0 : 1")));
};

#endif /* TEST_PARSE_H */
