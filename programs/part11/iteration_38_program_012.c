/* test_parse.h - Complex GTY annotations to test delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void* my_ptr;

/* Macros to introduce nested delimiters */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define COMPLEX_BRACKET_EXPR [ (10) + (20) ]
#define BRACE_INITIALIZER { .x = 1, .y = {2, 3} }
#define CALLBACK_CAST (void (*)(int (*)(int [ (4) ])))

/* Case 1: Parentheses - struct with GTY containing nested parentheses */
struct s1 GTY((length("(sizeof(int) * (2 + 3))")))
{
    int data[];
};

/* More parentheses with multiple GTY options */
struct s2 GTY((chain_next("next"), chain_prev("prev"), user))
{
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Case 2: Brackets - array types with complex dimension expressions */
struct s3 GTY((length("N")))
{
    int arr[ (2 * 3) ];
    int matrix[ (1 << 2) ][ (1 << 3) ];
};

/* Pointer to array with GTY */
typedef int (*array_ptr GTY((tag("ARRAY"))))[ (sizeof(int) == 4) ? 10 : 20 ];

/* Case 3: Braces - structure with nested struct definition inside GTY */
struct outer GTY((user))
{
    struct inner GTY((tag("NESTED")))
    {
        int x;
        int y;
    } nested;
    
    union inner_union GTY(())
    {
        int i;
        char c[ (4) ];
    } u;
};

/* Union with array and GTY containing brace-like constructs */
union u1 GTY((desc("%0.type")))
{
    int type;
    struct 
    {
        int length;
        char data[ (256) ];
    } GTY((tag("1"))) buffer;
};

/* Function pointer type with complex parentheses */
typedef void (*complex_func_ptr GTY((callback)))(int (*)(int [ (4) ]));

/* Forward declared struct with GTY containing parenthesized expression */
struct forward_decl GTY((user, length("(cond ? 10 : 20)")));

/* Now define it */
struct forward_decl
{
    int data[ (10) ];
    struct forward_decl *next;
};

/* GTY with skip option containing parentheses */
struct skipped GTY((skip(("skip_field"))))
{
    int skip_field;
    int keep_field GTY((length("(5 + 3)")));
};

/* Nested GTY annotations with all delimiters */
struct mega_struct GTY((user))
{
    /* Parentheses in array dimension */
    int arr1[ (NESTED_PAREN_EXPR) ];
    
    /* Pointer to function with parentheses */
    void (*func_ptr GTY((callback)))(int, char);
    
    /* Nested struct with braces */
    struct 
    {
        int x GTY((length("(sizeof(x) * 2)")));
        int y[ (2) ][ (3) ];
    } GTY((tag("INNER"))) inner;
    
    /* Union initializer-like syntax in comment/string */
    char *desc GTY((length("strlen(\"(test)\")")));
};

/* Template-like macro expansion to stress parser */
#define GTY_ARRAY(type, size) type GTY((length(#size))) [ (size) ]

struct with_macro_array
{
    GTY_ARRAY(int, (10 + 5));
    GTY_ARRAY(char, (20 * 2));
};

/* Multiple nested delimiters in single declaration */
struct extreme GTY((chain_next("next"), chain_prev("prev"), 
                    length("(int)((struct extreme *)0)->next ? 1 : 0")))
{
    struct extreme *next;
    struct extreme *prev;
    int values[ ( (sizeof(int*) == 8) ? 64 : 32 ) ];
    
    struct 
    {
        int depth;
        char path[ (256) ];
    } GTY((tag("PATH"))) metadata;
};

/* Test case specifically for bracket consumption */
typedef struct bracket_test
{
    /* Array with nested brackets in dimension */
    int (*complex_array GTY((length("dim"))))[ (3) ][ (4) ];
    
    /* Pointer to array of pointers */
    int *(*(*nested_ptr GTY((skip)))[ (2) ])[ (3) ];
    
} bracket_test_t;

/* Union with switch expression in GTY */
union switch_union GTY((desc("(int)(%0.kind)")))
{
    enum { KIND_A, KIND_B } kind;
    struct 
    {
        int count;
        int items[ (10) ];
    } GTY((tag("KIND_A"))) a;
    struct 
    {
        float value;
        char name[ (32) ];
    } GTY((tag("KIND_B"))) b;
};

/* Final test: all delimiters mixed */
struct all_delimiters GTY((user,
                          length("(int)(sizeof(struct all_delimiters) / (2))"),
                          skip(("skip_me"))))
{
    int skip_me;
    int array[ ( (1 << 3) + (1 << 2) ) ];
    
    struct 
    {
        int x[ (2) ];
        int y[ (3) ];
    } GTY((tag("COORDS"))) coords;
    
    void (*callback GTY((callback)))(struct all_delimiters *);
};

#endif /* TEST_PARSE_H */
