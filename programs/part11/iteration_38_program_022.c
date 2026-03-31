/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs needed by gengtype */
typedef int my_int;
typedef unsigned int size_t;
typedef void* ptr_t;

/* Macros to introduce nested delimiters */
#define NESTED_PAREN_EXPR ((sizeof(int) * (2 + 3)) / (4 - 1))
#define ARRAY_DIM_EXPR [ (2 * 3) + (4 / 2) ]
#define BRACE_INITIALIZER { .x = 1, .y = {2, 3} }
#define COMPLEX_MACRO(x) ((x) + ( (x) * (2) ))

/* Forward declarations */
struct forward_decl GTY((user));

/* Test case 1: Parentheses in GTY options - triggers '(' case */
struct s1 GTY((length("(sizeof(int) * (2 + 3))")))
{
    int data[];
};

/* Test case 2: Multiple parenthesized groups - triggers '(' case multiple times */
struct s2 GTY((chain_next("next"), chain_prev("prev"), user))
{
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Test case 3: Brackets in array declarations - triggers '[' case */
struct s3 GTY((length("N")))
{
    int arr[ (2 * 3) + (4 / 2) ];
};

/* Test case 4: Nested brackets in pointer to array - triggers '[' case */
typedef int (*complex_array_ptr GTY((user)))[ (sizeof(int) > 4) ? 8 : 16 ];

/* Test case 5: Function pointer with nested parentheses - triggers '(' case */
typedef void (*callback_fn GTY((callback)))(int (*)(int [ (4) ]));

/* Test case 6: Union with complex array - triggers '[' and '(' cases */
union u1 GTY((user))
{
    int i;
    char arr[ 10 + (5) ];
    long long big[ (sizeof(long long) == 8) ? 10 : 20 ];
};

/* Test case 7: Structure with nested structure definition (contains braces) */
struct outer GTY((user))
{
    struct inner GTY((tag("LANG")))
    {
        int x;
        int y[ (2) ];
    } nested;
    
    /* This should trigger '{' case when parsing the nested struct definition */
    struct another_inner
    {
        int a;
        int b;
    } GTY((user)) another;
};

/* Test case 8: Using macro expansions with delimiters */
struct s4 GTY((length("NESTED_PAREN_EXPR")))
{
    double values[];
};

/* Test case 9: Complex conditional in array dimension */
struct s5
{
    int matrix GTY((user))[ (sizeof(void*) == 8) ? 64 : 32 ][ 16 ];
};

/* Test case 10: Multiple nested delimiters in single declaration */
struct s6 GTY((chain_next("n"), chain_prev("p")))
{
    struct s6 *n;
    struct s6 *p;
    void (*func GTY((callback)))(int, char*);
    int data[ (1 << 5) + (3 * 2) ];
};

/* Test case 11: Forward declared struct with complex GTY annotation */
struct forward_decl GTY((user))
{
    int value;
    struct forward_decl *next;
};

/* Test case 12: Typedef with function type containing nested parentheses */
typedef int (complex_func GTY((callback)))(int (*callback)(int, int), 
                                          char *array[ (10) ]);

/* Test case 13: Structure with bitfield containing parenthesized expression */
struct s7 GTY((user))
{
    unsigned int flags : (sizeof(int) * 8 - 1);
    int normal_field;
};

/* Test case 14: Array of pointers with GTY on the pointer */
struct s8
{
    void * GTY((user)) pointers[ (4) + (2) ];
};

/* Test case 15: Nested structure with array of structures */
struct container GTY((user))
{
    struct element GTY((tag("1")))
    {
        int id;
        char name[ (32) ];
        struct element *next;
    } elements[ (100) ];
    
    int count;
};

/* Test case 16: Union containing anonymous struct with braces */
union u2 GTY((user))
{
    struct
    {
        int x;
        int y;
    } point;
    
    long coordinates[ (2) ];
};

/* Test case 17: Macro used in GTY argument with parentheses */
#define CHAIN_NAME "next_ptr"
struct s9 GTY((chain_next(CHAIN_NAME)))
{
    struct s9 *next_ptr;
    int data;
};

/* Test case 18: Conditional expression in GTY option */
struct s10 GTY((maybe_undef))
{
    int value;
};

/* Test case 19: Skip parameter with nested parentheses */
struct s11 GTY((skip(("skip_me"))))
{
    int skip_me;
    int keep_me;
};

/* Test case 20: Deeply nested parentheses */
struct s12 GTY((length("((((1) + (2)) * ((3) - (4))) / (5))")))
{
    int deep_array[];
};

#endif /* TEST_PARSE_H */
