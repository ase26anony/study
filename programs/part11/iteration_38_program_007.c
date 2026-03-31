/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void (*generic_callback)(void);

/* Macros to add nesting depth */
#define NESTED_EXPR ( (10) + (20) )
#define COMPLEX_SIZE (sizeof(int) * (2 + 3))
#define ARRAY_DIM ( (5) * ( (2) + (1) ) )
#define BRACE_INITIALIZER { .x = 1, .y = 2 }
#define NESTED_PARENS(x) (((x) + 1) * ((x) - 1))

/* Case 1: Parentheses - struct with GTY length containing nested parentheses */
struct s1 GTY((user)) {
    int count;
    /* Flexible array member with complex length expression */
    int data GTY((length("(sizeof(int) * (2 + 3))"))) [];
};

/* More parentheses - multiple parenthesized groups in GTY options */
struct list_node GTY((chain_next("next"), chain_prev("prev"))) {
    int value;
    struct list_node *next;
    struct list_node *prev;
};

/* Case 2: Brackets - array types with complex dimension expressions */
struct s2 GTY(()) {
    /* Array with dimension containing parentheses */
    int arr1 GTY((length("N"))) [ (2 * 3) ];
    
    /* Pointer to array with nested brackets */
    int (*arr2 GTY((length("10")))) [ (4 + 1) ];
    
    /* Multi-dimensional array */
    int matrix GTY((length("ROWS"), param_is("COLS"))) [ (3) ][ (2 + 2) ];
};

/* Typedef with array in function pointer */
typedef int (*array_func_ptr GTY((callback)))(
    int (*)(int [ (sizeof(int) > 4 ? 8 : 4) ])
);

/* Case 3: Braces - structure with nested struct definition */
struct outer GTY((user)) {
    int id;
    
    /* Nested structure definition (contains braces) */
    struct inner GTY((tag("NESTED"))) {
        int x;
        int y;
    } nested;
    
    /* Union with array */
    union {
        int i;
        char str[ (10) + (5) ];
    } data GTY((desc("$1.i > 0")));
};

/* More complex: GTY annotation with macro expanding to nested expressions */
struct s3 GTY((length("NESTED_EXPR"))) {
    /* Using macro that expands to parenthesized expression */
    int items GTY((length("COMPLEX_SIZE"))) [];
};

/* Function pointer type with complex signature */
typedef void (*complex_callback GTY((callback)))(
    int,
    int (*handler)(int [ARRAY_DIM], struct s1*),
    char (*names[])[ (20) + (10) ]
);

/* Union with GTY and array */
union u1 GTY((desc("$1.type"))) {
    int type;
    float f;
    /* Array with size from macro */
    char buffer[ ARRAY_DIM ];
    
    struct {
        int a;
        int b GTY((length("(a + b) * 2"))) [];
    } nested;
};

/* Forward declaration with GTY annotation containing conditional expression */
struct forward_decl GTY((user, if("(defined(FLAG1) && defined(FLAG2)) || FLAG3"))) ;

/* Now define it */
struct forward_decl {
    int value;
    struct forward_decl *next GTY((skip));
};

/* Structure with bitfield and GTY */
struct with_bitfields GTY(()) {
    unsigned int flag:1;
    unsigned int count: (sizeof(int) * 8 - 1);
    
    /* Array pointer with GTY length containing ternary operator */
    int *dynamic_array GTY((length("flag ? (10) : (20)")));
};

/* Typedef for pointer to array of function pointers */
typedef int (*(*complex_array_ptr GTY((callback)))
    [ (2) + (3) ])(int, int);

/* Structure using all three delimiters in one GTY annotation */
struct all_delimiters GTY((
    user,
    param_is("(int [ (sizeof(char) + 1) ])"),
    length("({ int x = 5; x * 2; })")  /* GCC statement expression */
)) {
    /* This tests the parser's ability to handle:
       - Parentheses: in param_is
       - Brackets: in array type
       - Braces: in statement expression (GCC extension) */
    int count;
};

/* Test nested GTY annotations (though not standard, might trigger edge cases) */
struct doubly_nested {
    struct inner1 GTY((chain_next("next"))) {
        int val1;
        struct inner1 *next;
        
        struct inner2 {
            int val2;
            int arr[ (3) * ( (2) + (1) ) ];
        } nested GTY((tag("INNER2")));
    } *level1 GTY((length("(depth + 1)")));
};

/* Array of structures with GTY */
typedef struct array_element GTY((user)) {
    int id;
    char name[ (32) + ( (8) * (2) ) ];
    struct array_element *next;
} array_element_t[ (100) + ( (50) * (2) ) ];

/* Final test: extremely nested expressions */
struct extreme_nesting GTY((
    length("((((((1) + (2)) * ((3) - (4))) / ((5) % (6))) + (((7) | (8)) & ((9) ^ (10)))) * sizeof(int))")
)) {
    int ultra_nested_array[
        ( (1) << ( (2) + ( (3) * ( (4) / (5) ) ) ) )
    ];
};

#endif /* TEST_PARSE_H */
