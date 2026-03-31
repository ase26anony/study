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
#define ARRAY_DIM ( (5) * ( (3) + (2) ) )
#define BRACE_BLOCK { int x = 0; }
#define CALLBACK_FN ((void (*)(void))0)

/* Case 1: Parentheses - struct with complex length expression */
struct s1 GTY((length("(2 + 3) * sizeof(int)"))) {
    int data[];
};

/* Case 2: More nested parentheses in GTY options */
struct s2 GTY((chain_next("next"), chain_prev("prev"), user)) {
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Case 3: Parentheses with macro expansion */
struct s3 GTY((length("NESTED_EXPR"))) {
    char buffer[];
};

/* Case 4: Brackets - array with complex dimension */
struct s4 GTY((user)) {
    int arr GTY((length("N")))[ (2 * 3) ];
    int N;
};

/* Case 5: Nested brackets in array declarator */
typedef int (*array_ptr_t GTY((user)))[ (4) ][ (5) ];

/* Case 6: Brackets with parenthesized expressions */
struct s5 {
    int matrix GTY((length("rows * cols")))[ ( (3) + (2) ) ][ ( (4) * (1) ) ];
    int rows;
    int cols;
};

/* Case 7: Function pointer with brackets in parameter */
typedef void (*complex_fn_ptr GTY((callback)))(int (*)(int [ (4) ]));

/* Case 8: Braces - union with nested structure definition */
union u1 GTY((user)) {
    struct inner1 {
        int x;
        int y;
    } GTY((tag("STRUCT"))) nested;
    int i;
    char arr[ 10 + (5) ];
};

/* Case 9: Structure with nested anonymous struct (braces) */
struct s6 GTY((user)) {
    struct {
        int a;
        int b;
    } GTY((tag("ANON"))) data;
    int extra;
};

/* Case 10: Multiple delimiters combined */
struct s7 GTY((length("(sizeof(struct s7) + (100))"))) {
    struct inner2 {
        int values[ (2) ][ (3) ];
        char *name;
    } GTY((tag("INNER"))) nested_data;
    int count;
};

/* Case 11: Typedef with complex GTY annotation containing all delimiters */
typedef struct {
    int (*callback GTY((callback)))(int, char *);
    int array[ ( (2) + ( (3) * (1) ) ) ];
    union {
        int i;
        float f;
    } value;
} GTY((user)) complex_type_t;

/* Case 12: Forward declaration with GTY annotation containing parentheses */
struct s8 GTY((user, chain_next("next")));
struct s8 {
    struct s8 *next;
    int data[ ( (1) << (2) ) ];
};

/* Case 13: Enum with GTY marker (though enums don't usually need GC) */
enum my_enum GTY((user)) {
    VALUE1 = (1),
    VALUE2 = ( (2) + (3) ),
    VALUE3 = ( (4) * ( (2) + (1) ) )
};

/* Case 14: Pointer to array with nested dimension calculation */
typedef int (*ptr_to_array_t GTY((user)))[ (sizeof(int) == 4 ? 10 : 20) ];

/* Case 15: Structure with flexible array member and complex GTY */
struct s9 GTY((length("((count + 7) & ~7)"))) {
    int count;
    double items[];
};

/* Case 16: Union containing array with macro-based size */
union u2 GTY((user)) {
    int numbers[ ARRAY_DIM ];
    char string[ ( (10) + (5) ) ];
};

/* Case 17: Nested structure definitions with GTY on inner struct */
struct outer GTY((user)) {
    struct inner3 GTY((tag("LEVEL1"))) {
        struct inner_inner GTY((tag("LEVEL2"))) {
            int deep_value;
        } deeper;
        int mid_value;
    } middle;
    int outer_value;
};

/* Case 18: Function pointer type with complex signature */
typedef int (*signal_handler_t GTY((callback)))
    (int signum, void (*old_handler)(int), char context[ (32) ]);

/* Case 19: Structure with bitfield and GTY */
struct s10 GTY((user)) {
    unsigned int flags : ( (3) );
    int data;
};

/* Case 20: Final test with deeply nested expressions */
struct s11 GTY((length("( ( (2) + (3) ) * ( (4) - (1) ) )"))) {
    int deep_array[ ( ( (1) << ( (2) + (1) ) ) ) ];
    struct {
        int x[ ( (2) * ( (1) + (1) ) ) ];
        int y;
    } GTY((tag("NESTED"))) nested;
};

#endif /* TEST_PARSE_H */
