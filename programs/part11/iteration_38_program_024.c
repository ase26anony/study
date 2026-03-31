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
#define FUNC_PROTO(x) void (*x)(int, char)

/* Case 1: Parentheses - struct with GTY((length("(complex expression)"))) */
struct s1 GTY((user)) {
    int data GTY((length("(sizeof(int) * (2 + 3))")))[];
    struct s1 *next GTY((skip));
};

/* Case 2: More parentheses - multiple parenthesized groups */
struct s2 GTY((chain_next("next"), chain_prev("prev"), 
               length("( (1 << 5) + (sizeof(char) * 8) )"))) {
    int value;
    struct s2 *next;
    struct s2 *prev;
    char buffer GTY((length("NESTED_EXPR")))[];
};

/* Case 3: Brackets - array with complex dimension expression */
struct s3 GTY(()) {
    int arr1 GTY((length("5")))[ (2 * 3) ];
    int arr2 GTY((length("ARRAY_DIM")))[ ( (5) * ( (2) + (1) ) ) ];
    int (*matrix GTY((skip)))[ (4) ][ (3) ];
};

/* Case 4: Nested brackets - pointer to array of arrays */
typedef int (*complex_array_ptr GTY((skip)))
    [ (2) + (3) ]
    [ ( (4) * (1) ) ];

/* Case 5: Braces - structure with nested struct definition inside GTY */
struct outer GTY((user)) {
    struct inner GTY((tag("LANG"))) {
        int x;
        int y;
    } nested;
    int value;
};

/* Case 6: Union with array and initializer-like syntax in comment */
union u1 GTY((desc("0"))) {
    int i;
    char arr[ 10 + (5) ];
    struct {
        int a;
        int b;
    } s;
};

/* Case 7: Function pointer with complex argument list containing brackets */
typedef void (*complex_func_ptr GTY((callback)))
    (int (*)(int [ (4) ][ (3) ]), 
     char *(*)(void *[ (2) + (1) ]));

/* Case 8: Structure with conditional in array dimension */
struct s4 GTY((length("(flag ? 10 : 20)"))) {
    int items[];
};

/* Case 9: Multiple nested parentheses in callback specification */
struct s5 GTY((callback("COMPARE", 
    "(const void *, const void *)"))) {
    int key;
    char *value;
};

/* Case 10: Macro expansion with parentheses */
#define GTY_SPECIAL(x) GTY((user, desc("((" #x ") + 1)")))
struct s6 GTY_SPECIAL(5) {
    int id;
    char name[32];
};

/* Case 11: Structure containing all delimiter types */
struct s7 GTY((chain_next("next"), 
               length("(sizeof(struct s7) + (offsetof(struct s7, data)))"))) {
    int count;
    /* Array with bracketed dimension containing parenthesized expression */
    int matrix GTY((length("count")))[ (3) * (2) ][ (4) + (1) ];
    /* Pointer to function with parenthesized argument list */
    void (*handler GTY((skip)))(int, char);
    struct s7 *next;
    /* Flexible array member with complex size expression */
    char data GTY((length("(count * sizeof(int))")))[];
};

/* Case 12: Typedef with function type containing nested parentheses */
typedef int (*(*complex_fp GTY((skip)))
    (int, 
     void (*)(int [ (2) ])))(char *[ (3) ]);

/* Case 13: Forward declaration with GTY annotation containing parentheses */
struct s8 GTY((user, desc("(struct s8 *)"))) ;
struct s8 {
    int value;
    struct s8 *link;
};

/* Case 14: Enum with GTY marker (though enums don't usually need GC) */
enum e1 GTY((tag("ENUM"))) {
    VALUE1 = (1 << 0),
    VALUE2 = (1 << 1),
    VALUE3 = (1 << 2)
};

/* Case 15: Structure with bitfield containing parenthesized expression */
struct s9 GTY(()) {
    unsigned int flags : (8);
    unsigned int status : ( (3) + (1) );
    int array GTY((length("(flags & 0x0F)")))[];
};

#endif /* TEST_PARSE_H */
