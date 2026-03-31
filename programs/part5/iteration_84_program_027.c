/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ) ) )
#define NESTED_BRACES { { { } } }
#define NESTED_BRACKETS [ [ [ ] ] ]
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
  #define EXTRA_NESTING ( { [ ( ) ] } )
#else
  #define EXTRA_NESTING /* nothing */
#endif

/* Requirement 1: Balanced token sequences in type definitions */

/* Struct with deeply nested function pointer */
struct S1 {
    /* Function pointer with nested parentheses in parameter */
    int (*fp1)(int (*callback)(int[2][3]), struct { int a; });
    
    /* Even more nesting */
    void (*fp2)(int (*(*nested)(int[][(2+3)]))(char), 
                struct { 
                    union { 
                        int x; 
                        char y[(sizeof(int)*2)]; 
                    } u; 
                });
};

/* Union with nested type definitions */
union U1 {
    char c;
    struct {
        int i;
        /* Array of function pointers */
        void (*arr[((2*3)+1)])(int, char);
    } s;
    
    /* Nested anonymous struct */
    struct {
        struct {
            int (*deep_fp)(struct { int x; });  /* Nested struct in parameter */
        } inner;
    };
};

/* Requirement 4: Array declarations with complex dimensions */

/* Multi-dimensional array with parenthesized size expression */
int arr1[(2+3)*4][5];

/* Array with size from nested type definition */
int *ptr1[(sizeof(struct {int x; char y[3];})/sizeof(int))];

/* Complex array dimension with nested everything */
char matrix[ 
    (int){ sizeof(struct { int a; char b[({2+3;})]; }) }  /* Compound literal */
][ 
    ( { int x = 2; x * 3; } )  /* Statement expression (GCC extension) */
];

/* Requirement 3: Attribute specifications with multiple parentheses */

/* Struct with GCC attributes containing parentheses */
struct __attribute__((aligned(16), packed)) S2 {
    int x __attribute__((aligned((8))));
    char y;
} __attribute__((deprecated("use S3 instead")));

/* Variable with attributes */
int global_var __attribute__(( 
    aligned(32), 
    section((".data." #var)),  /* Stringification */
    used 
));

/* Requirement 5: Initializer lists with nested braces and parentheses */

/* Complex initializer */
int x = { { ( 1 + (2) ) } };

/* Struct initializer with designators */
struct Point {
    int x;
    int y;
    int z[2];
};

struct Point pts[] = { 
    [0] = { 
        .x = (1 + (2 * (3))), 
        .y = {2}, 
        .z = { { (5) }, { (6) } } 
    },
    [1] = { 
        .x = ({ int t = 4; t; }),  /* Statement expression */
        .y = 3,
        .z = NESTED_BRACES  /* Using macro */
    }
};

/* More complex nested initializer */
struct Outer {
    struct Inner {
        int a;
        int b[2];
    } inner;
    int c;
};

struct Outer outer = {
    .inner = {
        .a = ( ({ int x = 5; x; }) ),  /* Double parentheses with statement expr */
        .b = { [0] = (1), [1] = {2} }
    },
    .c = { { ( {3} ) } }
};

/* Requirement 1 (more): Typedef with extreme nesting */
typedef int (*ComplexFuncPtr)(
    int (*)(int[][(2+3)], struct { 
        union { 
            int a; 
            char b[({ sizeof(int) })];  /* Statement expr in array size */
        }; 
    }),
    void (*[2])(char, short)
);

/* Enum with complex expressions (GCC extension) */
enum E {
    A = (1 << 2),
    B = ( { int x = sizeof(struct { char c; }); x; } ),  /* Statement expr */
    C = (int){ 3.14 }  /* Compound literal */
};

/* Requirement 4 (more): Variable Length Array with complex size */
void func_with_vla(int n) {
    int vla1[n * (2 + (3))];
    int vla2[ ( { int s = n + 1; s * 2; } ) ];
    
    /* Nested VLA */
    struct {
        int data[ n + ( { int x = 2; x; } ) ];
    } vla_struct;
}

/* Requirement 2 (more): Using macros in type definitions */
int arr2[] = NESTED_BRACES;
int arr3[] = COMPLEX_MACRO;

/* Typedef using macro expansion */
typedef int MacroType EXTRA_NESTING;

/* Requirement 6: More conditional compilation */
#if defined(__GNUC__) && __GNUC__ >= 5
    /* C++11 style attribute in C (GCC extension) */
    struct [[deprecated("old"), gnu::packed]] S3 {
        int x [[gnu::aligned(16)]];
    };
#elif defined(_MSC_VER)
    #pragma pack(push, 1)
    struct S3 {
        int x;
    };
    #pragma pack(pop)
#else
    struct S3 {
        int x;
    } __attribute__((packed));
#endif

/* Function with nested parameter types */
void complex_func(
    int (*param1)(int[][(2+3)]),  /* Parentheses and brackets */
    struct {
        int a;
        union {
            char b;
            int c[( { 2 + 2; } )];  /* Statement expr */
        } u;
    } param2
) {
    /* Local struct with attributes */
    struct __attribute__((aligned(8))) LocalStruct {
        int x;
    } ls = { .x = ( { int y = 5; y; } ) };
    
    /* Array with complex initializer */
    int local_arr[][2] = { 
        { (1), {2} }, 
        { {3}, (4) },
        NESTED_BRACES
    };
}

/* Main function to avoid dead code elimination */
int main(void) {
    /* Reference some variables to prevent optimization */
    struct S1 s1 = {0};
    union U1 u1 = {0};
    struct S2 s2 = {0};
    struct S3 s3 = {0};
    
    /* Use the complex types */
    ComplexFuncPtr cfp = 0;
    
    /* Call function with VLAs */
    func_with_vla(10);
    
    /* Use initialized data */
    int sum = x + pts[0].x + outer.inner.a;
    
    return sum & 0;  /* Always return 0 */
}

/* Final complex type definition wrapping everything */
typedef struct UltimateType {
    struct S1 field1;
    union U1 field2[ (sizeof(struct S2) + ({ int x = 4; x; })) ];
    int (*final_fp)(
        void (*)(int[][ ({ 2 * 2; }) ], 
                struct { 
                    int a[ ( { 3; } ) ]; 
                }),
        [[gnu::nonnull]] char *restrict
    );
} UltimateType __attribute__((aligned(64)));
