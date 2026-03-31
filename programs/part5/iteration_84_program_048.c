/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ( ) ) ) ) )
#define NESTED_BRACKETS [ [ [ [ ] ] ] ]
#define NESTED_BRACES { { { { } } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 1: Balanced token sequences in type definitions */
struct S1 {
    /* Function pointer with nested parentheses */
    int (*fp1)(int (*)(int[2][3]), struct { int a; });
    
    /* Even more nested function pointer */
    void (**fp2)(int (*(*)(int (*)[(2+3)*4]))(char), 
                 struct { 
                     union { 
                         int x; 
                         double y; 
                     } u; 
                 });
};

/* Struct with all delimiter types nested */
struct S2 {
    /* Array of function pointers */
    int (*arr[5])(int, char);
    
    /* Nested struct with bitfield */
    struct {
        unsigned int flags : (sizeof(int)*8);
        int (*callback)(int (*)(int[][(2+3)]), 
                       struct { 
                           int a; 
                           double b[(sizeof(struct {int x;})/4)]; 
                       });
    } inner;
};

/* Requirement 4: Array declarations with complex dimensions */
int multi_dim[(2+3)*4][5];
int *ptr_array[(sizeof(struct {int x; double y;})/sizeof(int))];

/* Array with nested type in size expression */
struct Container {
    int data;
} container_array[1 + (int)(sizeof(struct { char a; int b; }) / 2)];

/* Requirement 3: Attribute specifications */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int x __attribute__((aligned((8))));
    char y;
} __attribute__((deprecated));

/* GCC-style attributes with parentheses */
int global_var __attribute__((section((".data")), 
                             aligned((32)), 
                             used)) = 42;

/* Requirement 5: Initializer lists with nested braces and parentheses */
int complex_init = { { ( 1 + (2) ) } };

struct Point {
    int x;
    int y[2];
    struct {
        int z;
    } nested;
};

struct Point points[] = { 
    [0] = { .x = (1), .y = {2, {3}} },
    [1] = { .x = { (4) }, .y = {5, 6}, .nested = { .z = (7 + (8)) } }
};

/* Union with nested initializers */
union U {
    int i;
    struct {
        char c;
        float f;
    } s;
} u = { .s = { .c = ('a'), .f = {3.14f} } };

/* Requirement 6: Conditional compilation blocks */
#ifdef TEST_COMPLEX_TYPES
    /* This struct will only be parsed if TEST_COMPLEX_TYPES is defined */
    struct ConditionalStruct {
        int (*func_ptr)(int (*)(int[][(2*3)]), 
                       union { 
                           long l; 
                           struct { 
                               short s; 
                           } inner; 
                       });
    };
#endif

#if defined(__GNUC__) && __GNUC__ >= 5
    /* GCC-specific attribute syntax */
    struct [[deprecated("Use NewStruct instead")]] TaggedStruct {
        int old_field;
    };
#endif

#ifndef SKIP_NESTED
    /* Always include unless SKIP_NESTED is defined */
    typedef struct {
        /* Typedef with function returning pointer to array */
        int (*(*complex_typedef)(void))[(2 + (3 * 4))];
        
        /* Nested anonymous struct */
        struct {
            int (*nested_fp)(struct { 
                int a[(1+(2))]; 
            });
        };
    } VeryComplexType;
#endif

/* More complex examples mixing all requirements */
enum E {
    VAL1 = (1 << (sizeof(int)*8 - 1)),
    VAL2 = { (2) },
    VAL3 = (int){ (3) }
};

/* Function-like macro with balanced delimiters */
#define CALL_WITH_NESTED(func) func( (int[][2]){ {1, (2)}, {3, 4} } )

/* Using the macro in a type context */
typedef void (*CallbackType)(int (*)(int[][(2+3)]));

/* Final struct combining everything */
struct UltimateStruct {
    /* Attribute with parentheses */
    int field1 __attribute__((aligned(( (16) ))));
    
    /* Array with computed size containing nested type */
    char buffer[(sizeof(struct { 
        int a; 
        double b[({2})];  /* GCC statement expression */
    }) + 7) & ~7];
    
    /* Nested anonymous union */
    union {
        /* Function pointer with deeply nested parameters */
        long (*fp)(int (*((*)(int, ...))[])(char), 
                  struct { 
                      int x[([{2}] + 3)];  /* Invalid but tests token balance */
                  });
        void *ptr;
    };
} __attribute__((packed));

/* Main function to avoid dead code elimination */
int main(void) {
    /* Use some variables to prevent optimization */
    struct S1 s1 = {0};
    struct S2 s2 = {0};
    
    /* Reference the arrays */
    multi_dim[0][0] = 1;
    ptr_array[0] = &multi_dim[0][0];
    
    /* Use initialized data */
    points[0].x = complex_init;
    
    /* Conditional compilation in main */
#ifdef TEST_COMPLEX_TYPES
    struct ConditionalStruct cs;
    (void)cs;
#endif
    
#ifndef SKIP_NESTED
    VeryComplexType vct;
    (void)vct;
#endif
    
    return 0;
}

/* Additional edge cases after main */
#if 0
    /* This code won't be compiled but will be parsed */
    struct DisabledStruct {
        /* Extremely nested example */
        int (*(*(*insane)[][2])(int (*(*)(...))[], 
                               struct { 
                                   int a[({})]; 
                               }))[][({})];
    };
#endif

/* Test macro expansion in type definition */
typedef int Matrix_t[][NESTED_BRACKETS 3];
typedef struct { int x; } Struct_t NESTED_BRACES;
