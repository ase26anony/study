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
#define NESTED_PARENS(x) (((x) + 1) * ((x) - 1))

/* Macro that expands to brace-enclosed content */
#define STRUCT_BODY { int x; double y; char z; }

/* ====== Test Case 1: Parentheses in GTY options ====== */

/* Struct with GTY annotation containing nested parentheses in length expression */
struct s1 GTY((length("(sizeof(int) * (2 + 3))"))) {
    int data[];
};

/* Another struct with multiple parenthesized groups */
struct s2 GTY((chain_next("next"), chain_prev("prev"))) {
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Struct with deeply nested parentheses */
struct s3 GTY((length("NESTED_EXPR"))) {
    char buffer[];
};

/* ====== Test Case 2: Brackets in type declarations ====== */

/* Array type with complex dimension expression inside GTY */
struct array_struct {
    int arr GTY((length("N")))[ (2 * 3) ];
    int N;
};

/* Pointer to array with nested brackets */
typedef int (*complex_array_ptr GTY((user)))[ (sizeof(int) > 4) ? 8 : 16 ];

/* Multi-dimensional array with bracketed expressions */
struct md_array {
    int matrix GTY((length("rows"), param_is("struct md_array *")))[ (5) ][ (3) ];
    int rows;
};

/* ====== Test Case 3: Braces in type definitions ====== */

/* Union with GTY and array containing brace initializer in size expression */
union u1 GTY(()) {
    int i;
    char arr[ 10 + (5) ];
    long data[ sizeof(int) ];
};

/* Struct with nested struct definition (contains braces) inside GTY context */
struct outer GTY((user)) {
    struct inner {
        int x;
        double y;
    } GTY((tag("LANG"))) nested;
    int count;
};

/* Another approach: use macro that expands to braces */
struct macro_brace GTY((user)) {
    int values[5];
    /* Note: Can't directly put braces in GTY, but the struct definition itself has braces */
};

/* ====== Test Case 4: Function pointers with complex signatures ====== */

/* Typedef for function pointer with GTY callback and complex argument list */
typedef void (*complex_callback GTY((callback)))(int (*)(int [ (4) ]), 
                                                 char *(*[ (2) ])(void));

/* Function pointer type with nested parentheses in parameter types */
typedef int (*fnptr_with_parens GTY((user)))(
    int (*(*)(int [NESTED_EXPR]))(void),
    char (*)[ sizeof(int) + (2) ]
);

/* ====== Test Case 5: Mixed delimiters ====== */

/* Struct combining all delimiter types */
struct mixed_delimiters GTY((length("(sizeof(struct { int x; }) + 5)"))) {
    union {
        int a;
        char b[ (2) + (3) ];
    } data;
    void (*func)(int [ (1) ][ (2) ]);
};

/* ====== Test Case 6: Forward declarations with GTY ====== */

/* Forward declared struct with GTY annotation containing parenthesized expression */
struct forward_decl GTY((user, chain_next("next")));
struct forward_decl {
    struct forward_decl *next;
    int value GTY((length("(value > 0 ? value : 1)")));
};

/* ====== Test Case 7: Conditional expressions in GTY options ====== */

struct conditional_gt GTY((length("(sizeof(int) > 4) ? 8 : 4"))) {
    char data[];
};

/* ====== Test Case 8: Nested structures with GTY markers ====== */

/* Outer struct with GTY, inner struct with its own GTY */
struct outer_nested GTY((user)) {
    struct inner_nested GTY((tag("INNER"))) {
        int x;
        int y;
        int arr[ (2) * (3) ];
    } inner;
    int count;
};

/* ====== Test Case 9: Array of pointers with GTY ====== */

typedef struct element {
    int value;
    struct element *next;
} element_t;

/* Array of pointers with GTY chain */
element_t * GTY((length("count"), chain_next("next"))) ptr_array[ (10) ];

/* ====== Test Case 10: Complex typedef with all delimiters ====== */

typedef struct {
    int (*processor GTY((callback)))(
        int matrix[ (2) ][ (3) ],
        void (*callback)(int, char)
    );
    union {
        int i;
        char str[ (20) + (5) ];
    } data;
} complex_type GTY((user));

#endif /* TEST_PARSE_H */
