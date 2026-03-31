/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void *ptr_t;

/* Macros to introduce nested delimiters */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define BRACKET_EXPR [ (2 * 3) + (4 / 2) ]
#define COMPLEX_BRACE { .x = (1 + 2), .y = {3, 4} }
#define CALLBACK_MACRO (callback)

/* 1. Test parentheses: struct with GTY length containing nested parentheses */
struct s1 GTY((user)) {
    int count;
    /* Flexible array member with complex length expression */
    int data[] GTY((length("(sizeof(int) * (2 + 3))")));
};

/* Another parentheses test with multiple parenthesized groups */
struct s2 GTY((chain_next("next"), chain_prev("prev"))) {
    struct s2 *next;
    struct s2 *prev;
    int value GTY((skip));
};

/* 2. Test brackets: array types with complex dimension expressions */
struct s3 GTY(()) {
    /* Array with dimension containing parentheses */
    int arr1[ (2 * 3) + (4 / 2) ];
    
    /* Pointer to array with GTY annotation */
    int (*arr2 GTY((length("N"))))[ (sizeof(int) * 2) ];
    
    /* Multi-dimensional array */
    int matrix[ (1 << 2) ][ (1 << 3) ] GTY((tag("MATRIX")));
};

/* 3. Test braces: structure with nested struct definition inside GTY */
struct outer GTY((user)) {
    /* Nested structure definition (contains braces) */
    struct inner GTY((tag("NESTED"))) {
        int x;
        int y;
    } nested;
    
    /* Union with initializer-like syntax in GTY */
    union {
        int i;
        char c;
    } u GTY((desc("(%s ? 0 : 1)", "u.i")));
};

/* 4. Function pointer with complex argument list containing brackets */
typedef void (*complex_func_ptr GTY((callback)))(
    int (*)(int arr[ (sizeof(int) * 2) ]),
    void * GTY((skip))
);

/* 5. Union with array whose size uses macro with parentheses */
union u1 GTY(()) {
    int i;
    char arr[ 10 + (5 * 2) ];
    long buffer[ NESTED_PAREN_EXPR / sizeof(long) ];
};

/* 6. Forward-declared struct with GTY containing nested conditional */
struct forward_decl GTY((user));
struct forward_decl {
    int value;
    struct forward_decl *next GTY((skip));
} GTY((chain_next("next")));

/* 7. Structure with GTY options containing all delimiter types */
struct all_delimiters GTY((user,
    length("(sizeof(struct { int x; int y; }) * (2 + 3))"),
    tag("COMPLEX")
)) {
    /* Array with complex initialization (simulated in GTY) */
    int values[] GTY((length("(1 << 5) * sizeof(int)")));
    
    /* Pointer with cast expression in GTY */
    void *ptr GTY((skip("(void *)((char *)0 + (sizeof(int) * 2))")));
};

/* 8. Typedef with nested GTY annotations */
typedef struct node GTY((user)) {
    int data;
    struct node *children[] GTY((length("(data > 0 ? data : 1)")));
} tree_node;

/* 9. Test with macro expansion containing delimiters */
#define GTY_ARGS (user, tag("MACRO_TEST"), length("(1 + (2 * 3))"))
struct macro_test GTY(GTY_ARGS) {
    int value;
    char name[] GTY((length("(sizeof(char) * (32 + 8))")));
};

/* 10. Complex nested structure with multiple GTY levels */
struct level1 GTY((user)) {
    struct level2 GTY((tag("LEVEL2"))) {
        struct level3 GTY((skip)) {
            int depth;
            int values[ (1 << 3) ][ (1 << 2) ];
        } inner;
        float data;
    } middle;
    char *name GTY((length("strlen(name) + (sizeof(int))")));
};

/* 11. Enum with GTY (though enums don't usually need GC) */
enum my_enum GTY((user)) {
    VALUE1 = (1 << 0),
    VALUE2 = (1 << 1),
    VALUE3 = (1 << 2) | (1 << 3)
};

/* 12. Array of pointers with complex GTY expression */
typedef char *string_array[ (10 + 5) ] GTY((tag("STRING_ARRAY")));

/* 13. Structure with bitfield and GTY */
struct with_bitfields GTY(()) {
    unsigned int flag1:1;
    unsigned int flag2:2;
    unsigned int count: (sizeof(int) * 8 - 3);
    int normal_field GTY((skip));
};

/* 14. Test callback with nested function type */
typedef int (*nested_callback GTY((callback)))(
    void (*)(int, char * GTY((length("(index + 1) * sizeof(char)")))),
    int param
);

/* 15. Final comprehensive test hitting all cases */
struct final_test GTY((
    user,
    tag("FINAL"),
    length("sizeof(struct { int a[(2+3)]; char b; })"),
    skip("(void*)0"),
    callback
)) {
    int simple;
    struct {
        int nested;
        char data[ (sizeof(int) * 4) ];
    } inner GTY((tag("INNER")));
    void (*func)(int, char[]) GTY((callback));
};

#endif /* TEST_PARSE_H */
