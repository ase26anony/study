/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ) ) )
#define NESTED_BRACKETS [ [ [ ] ] ]
#define NESTED_BRACES { { { } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
#define EXTRA_NESTING ([[{(([]))}]])
#endif

/* Requirement 1: Balanced token sequences in type definitions */
struct S1 {
    /* Function pointer with nested parentheses */
    int (*fp1)(int (*)(int[2][3]), struct { int a; });
    
    /* Even more nested */
    void (*fp2)(int (*(*)(int (*)[(2+3)])[5])(char));
};

/* Union with nested type definitions */
union U1 {
    char c;
    struct {
        int i;
        /* Nested array in anonymous struct */
        float arr[(sizeof(int) + 2) * 3];
    } s;
};

/* Requirement 4: Array declarations with complex dimensions */
int arr1[(2+3)*4][5];
int *ptr1[(sizeof(struct {int x; char y[(1+2)*3];})/4)];

/* Using macros in array dimensions */
int arr2[] = NESTED_BRACES;
char arr3[][NESTED_BRACKETS[0] ? 10 : 20];

/* Requirement 3: Attribute specifications */
struct S2 {
    int a;
    double b;
} __attribute__((aligned(16), packed, 
                 deprecated("Use S3 instead")));

/* Struct with GCC attributes containing parentheses */
struct __attribute__((aligned((16)), 
                     mode(__byte__))) S3 {
    unsigned char data[64];
};

/* Requirement 5: Initializer lists with nested braces/parentheses */
int x1 = { { ( 1 + (2) ) } };
int x2 = { NESTED_PARENS, { COMPLEX_MACRO } };

struct Point {
    int x;
    int y[2];
    struct {
        int z;
    } nested;
};

struct Point pts[] = { 
    [0] = { 
        .x = (1 + (2 * (3))), 
        .y = {2, {3}}, 
        .nested = { .z = ({4}) }
    },
    [1] = NESTED_BRACES
};

/* More complex type definitions */
typedef int (*complex_func_t)(
    int (*callback)(int, char**, 
                    struct { 
                        int depth; 
                        int *ptr[(2+3)]; 
                    }),
    void *data[sizeof(struct {int a; double b;})]
);

/* Enum with complex expressions */
enum E {
    VAL1 = (1 << (sizeof(int)*8 - 1)),
    VAL2 = (int){ (2 + (3 * 4)) },
    VAL3 = sizeof(int[(1+2)*3])
};

/* Requirement 6: Conditional compilation blocks */
#ifdef TEST_NESTED
/* Struct only defined if TEST_NESTED is set */
struct ConditionalStruct {
    int (*method)(int param[(sizeof(struct {int x;})/4)]);
    union {
        char c;
        struct { 
            int i; 
        } s;
    } u;
};
#endif

#if defined(USE_ATTRIBUTES) && USE_ATTRIBUTES > 0
/* Variable with attribute containing parentheses */
int global_var __attribute__((used, 
                              section((".data" ".special")),
                              aligned((16))));
#endif

/* Function with complex parameter list */
void process(
    int matrix[][(2*3)+1],
    void (*handler)(int, 
                    struct { 
                        int count; 
                        int items[({ int x = 5; x; })]; 
                    })
) {
    /* Local struct with nested braces */
    struct Local {
        int a;
        int b[({ int y = 10; y; })];
    } local = { .a = 1, .b = {2, 3} };
    
    /* Array with complex initializer */
    int nested_init[2][2] = { 
        { (1), {2} }, 
        { {3}, (4) } 
    };
}

/* Main function that references defined types to avoid dead code elimination */
int main(void) {
    struct S1 s1 = {0};
    struct S2 s2 = {0};
    struct S3 s3 = {0};
    
    /* Use arrays to prevent optimization */
    arr1[0][0] = 1;
    ptr1[0] = &arr1[0][0];
    
    /* Reference initialized variables */
    x1 += x2;
    pts[0].x = pts[1].y[0];
    
    /* Reference conditional struct if defined */
#ifdef TEST_NESTED
    struct ConditionalStruct cs = {0};
    (void)cs;
#endif
    
    return 0;
}

/* Additional complex typedefs at file scope */
typedef struct {
    int (*vtable[((sizeof(void*) * 8) / 4)])(void*, 
                                             int[][({ int dim = 3; dim; })]);
} Object;

/* Final macro with all delimiter types */
#define ULTIMATE_NEST { ( [ { ( [ { } ] ) } ] ) }
