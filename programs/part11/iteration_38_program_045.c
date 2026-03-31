/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef unsigned int size_t;

/* Macros to add nesting depth */
#define NESTED_EXPR ( (10) + (20) )
#define COMPLEX_SIZE (sizeof(int) * (2 + 3))
#define ARRAY_DIM ( (5) * (3) )
#define BRACE_INITIALIZER { 1, 2, 3, 4, 5 }
#define FUNC_MACRO(x) ( (x) * (x + 1) )

/* Case 1: Parentheses - struct with GTY length containing nested parentheses */
struct s1 GTY((user)) {
    int count;
    /* Flexible array member with complex length expression */
    int data GTY((length("(sizeof(int) * (2 + 3))")))[];
};

/* Case 2: More parentheses - chain operations with multiple parenthesized groups */
struct node GTY((chain_next("next"), chain_prev("prev"), user)) {
    struct node *next;
    struct node *prev;
    int value;
    /* Array with macro containing parentheses */
    char buffer GTY((length("NESTED_EXPR")))[];
};

/* Case 3: Brackets - array types with complex dimension expressions */
struct array_struct GTY((user)) {
    /* Direct array with bracketed dimension */
    int arr1 GTY((length("N")))[ (2 * 3) ];
    
    /* Pointer to array with nested brackets */
    int (*arr2 GTY((length("M"))))[ (4) + (2) ];
    
    /* Multi-dimensional array */
    int matrix GTY((length("ROWS"), length("COLS")))[ 2 + (3) ][ (5) * 2 ];
};

/* Case 4: Function pointer with complex argument list containing brackets */
typedef void (*complex_callback GTY((callback)))(
    int (*processor)(int data[ (sizeof(int) * 2) ], 
                     char buffer[ 10 + (5) ]),
    struct array_struct *ctx
);

/* Case 5: Union with array containing bracketed size expression */
union u1 GTY((user)) {
    int i;
    /* Array with size from macro containing parentheses */
    char arr[ 10 + (5) ];
    double values[ ARRAY_DIM ];
};

/* Case 6: Nested structure definition with braces */
struct outer GTY((user)) {
    int id;
    /* Nested struct definition (contains braces) */
    struct inner GTY((tag("LANG"))) {
        int x;
        int y;
        char name[32];
    } nested;
    
    /* Another nested anonymous struct */
    struct GTY((user)) {
        float f;
        double d;
    } anonymous;
};

/* Case 7: GTY with skip option containing parenthesized expression */
struct skip_example GTY((skip(" (offsetof(struct skip_example, data) > 0) "))) {
    int header;
    char data[100];
};

/* Case 8: Variable length array in parameter (more brackets) */
typedef struct vla_example GTY((user)) {
    int len;
    /* Note: VLA in struct requires C99/C11, gengtype should handle it */
    int items[];
} vla_example_t;

/* Case 9: Complex conditional in GTY options */
struct conditional_example GTY((
    maybe_undef,
    tag("(DEBUG_LEVEL > 0) ? \"DEBUG\" : \"RELEASE\"")
)) {
    int value;
    char *message;
};

/* Case 10: Multiple nested delimiters in one declaration */
struct complex_nesting GTY((user)) {
    /* Function pointer array with complex signatures */
    void (*callbacks[ (2) + (3) ])(
        int param1,
        char param2[ (10) * sizeof(char) ],
        struct { int a; double b; } *param3
    );
    
    /* Nested array of pointers to arrays */
    int *(*(nested_arr GTY((length("N"))))[ (5) ])[ (3) ];
};

/* Case 11: Enum with GTY marker (contains braces) */
enum color GTY((user)) {
    RED,
    GREEN = (1 << 2),
    BLUE = (3 + (2 * 2))
};

/* Case 12: Typedef with function type containing complex parameters */
typedef int (*math_func GTY((callback)))(
    int a[ (sizeof(int)) ],
    int b,
    int (*callback)(int result[ (10) ], void *ctx)
);

/* Case 13: Structure with embedded union containing array */
struct with_union GTY((user)) {
    int type;
    union {
        int int_val;
        char str_val[ 20 + (10) ];
        double arr_val[ (3) * (2) ];
    } data;
};

/* Forward declaration with GTY annotation containing parentheses */
struct forward_decl GTY((user));
struct forward_decl {
    int value;
    struct forward_decl *next;
};

#endif /* TEST_PARSE_H */
