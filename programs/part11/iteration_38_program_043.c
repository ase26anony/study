/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void (*generic_callback)(void);

/* Macros to add nesting depth for delimiters */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define COMPLEX_BRACKET_EXPR [ (10) + (20) ]
#define BRACE_INITIALIZER { .x = 1, .y = {2, 3} }
#define NESTED_MACRO(x) ((x) * ((x) + 1))

/* Case 1: Parentheses - struct with GTY length containing nested parentheses */
struct s1 GTY((user)) {
    int count;
    /* Flexible array member with complex length expression in parentheses */
    int data GTY((length("(NESTED_PAREN_EXPR)")))[];
};

/* Case 2: Parentheses - GTY with multiple parenthesized groups */
struct list_node GTY((chain_next("next"), chain_prev("prev"))) {
    int value;
    struct list_node *next;
    struct list_node *prev;
};

/* Case 3: Parentheses - function pointer typedef with nested parentheses */
typedef int (*complex_func_ptr GTY((callback)))(
    int (*)(int, int (*)(int)),
    void (*)(char (*)[(2 + 3)])
);

/* Case 4: Brackets - array type with complex dimension expressions */
struct s2 GTY(()) {
    /* Array with dimension containing parentheses */
    int arr1 GTY((length("10")))[ (2 * 3) + (4 / 2) ];
    
    /* Pointer to array with nested brackets */
    int (*arr_ptr GTY((skip)))[ (5) ][ (6) ];
    
    /* Multi-dimensional array */
    int matrix GTY((length("N * M")))[ (3 + 2) ][ (4 * 2) ];
};

/* Case 5: Brackets - typedef with array declarator containing brackets */
typedef struct s3 GTY((tag("TAG1"))) {
    char buffer[ 10 + (sizeof(int) * 2) ];
    int (*func_array[ (1 << 3) ])(void);
} s3_t;

/* Case 6: Braces - union with nested structure definition (contains braces) */
union u1 GTY((desc("0"))) {
    int i;
    struct inner GTY((tag("INNER_TAG"))) {
        int x;
        int y GTY((length("2")))[2];
    } inner_struct;  /* This struct definition contains braces */
    
    /* Array with brace-enclosed initializer in comment (triggers tokenization) */
    /* Note: actual initializers can't be in GTY, but we can have nested structs */
};

/* Case 7: Braces - struct containing another GTY-marked struct with braces */
struct outer GTY((user)) {
    int id;
    /* Nested struct definition with braces */
    struct nested GTY((tag("NESTED"))) {
        int a;
        int b;
        char c GTY((length("(5 + 3)")))[];
    } nested_data;  /* This will be parsed and contains braces */
};

/* Case 8: Complex combination - all delimiters in one type */
typedef struct all_delimiters GTY((user)) {
    /* Parentheses in function pointer */
    void (*callback GTY((skip)))(int (*)(int[ (2) ]));
    
    /* Brackets in array with parenthesized expression */
    int values GTY((length("(count + 1)")))[ (sizeof(int) > 4) ? 8 : 4 ];
    
    /* Nested struct with braces */
    struct {
        int x GTY((length("(2 * (3 + 1))")))[];
        int y;
    } nested;
} all_delimiters_t;

/* Case 9: GTY with macro expansion containing parentheses */
#define GTY_SPECIAL ((user), (tag("SPECIAL")))
struct special_struct GTY(GTY_SPECIAL) {
    int data;
};

/* Case 10: Forward declaration with GTY containing parenthesized expression */
struct forward_decl GTY((user, desc("(1 << 5)")));

/* Case 11: Enum with GTY (contains braces) */
enum my_enum GTY((tag("ENUM_TAG"))) {
    ENUM_VAL1,
    ENUM_VAL2 = (1 << 3),
    ENUM_VAL3
};

/* Case 12: Typedef for pointer to array with complex brackets */
typedef int (*complex_array_ptr GTY((skip)))[ (2 + 3) ][ (4 * 2) ];

/* Case 13: Structure with bitfield containing parenthesized expression */
struct bitfield_struct GTY(()) {
    unsigned int flags : (sizeof(int) * 8 - 1);
    int data GTY((length("(flags & 0xFF)")))[];
};

/* Case 14: Union with array containing nested bracket expressions */
union final_union GTY((desc("2"))) {
    long long big;
    int parts GTY((length("sizeof(long long)/sizeof(int)")))[ 
        (sizeof(long long) / sizeof(int)) 
    ];
};

#endif /* TEST_PARSE_H */
