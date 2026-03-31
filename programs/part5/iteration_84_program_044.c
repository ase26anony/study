/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ( ) ) ) ) )
#define NESTED_BRACKETS [ [ [ [ ] ] ] ]
#define NESTED_BRACES { { { { } } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 1: Balanced token sequences in type definitions */
struct S1 {
    /* Function pointer with deeply nested parentheses */
    int (*fp1)(int (*)(int[2][3]), struct { int a; });
    int (*fp2)(void (*((*)(int)))(int), 
               union { 
                   char c; 
                   struct { 
                       int (*nested_fp)(int (*(*)[5])(int)); 
                   } s; 
               });
};

/* Struct with nested braces and brackets */
struct S2 {
    int arr1[2][(3+4)*2];
    struct {
        int x;
        struct {
            char c;
        } inner;
    } nested;
};

/* Requirement 4: Array declarations with complex dimensions */
int arr1[(2+3)*4][5];
int *ptr1[(sizeof(struct {int x; int y[3];})/sizeof(int))];
int arr2[sizeof(struct { char c; int i; })];
int (*arr3[((sizeof(int*) > 4) ? 2 : 3)])[5];

/* Requirement 3: Attribute specifications */
struct __attribute__((aligned(16), packed)) AttrStruct {
    int x __attribute__((aligned(8)));
    char y;
} __attribute__((deprecated));

/* C++11 style attributes (valid in C23/C2x with -std=c2x) */
#if __STDC_VERSION__ >= 202311L
[[deprecated("Use NewStruct instead")]]
#endif
struct OldStruct {
    int value;
};

/* Requirement 5: Initializer lists with nested braces and parentheses */
int x = { { ( 1 + (2) ) } };
int y = { NESTED_PARENS };
int z[] = NESTED_BRACES;

struct Point {
    int x;
    int y;
    int z[3];
};

struct Point pts[] = { 
    [0] = { .x = (1), .y = {2}, .z = { {1}, {2}, {3} } },
    [1] = { .x = ( (3) ), .y = { {4} }, .z = COMPLEX_MACRO }
};

/* Union with nested initializer */
union U1 {
    int i;
    struct {
        int a;
        int b;
    } s;
} u1 = { .s = { .a = (5), .b = {6} } };

/* Requirement 6: Conditional compilation blocks with balanced tokens */
#ifdef TEST_COMPLEX
    struct ConditionalStruct {
        int (*cond_fp)(int (*arr[((2>1)?3:4)])(int));
        char cond_arr[sizeof(struct { int x; })];
    };
#elif defined(TEST_SIMPLE)
    union ConditionalUnion {
        char c;
        struct { 
            int i; 
            int j[2][2];
        } s;
    };
#else
    /* Default complex definition */
    struct DefaultStruct {
        int (*default_fp)(
            int, 
            struct { 
                int a; 
                int (*nested)(int[][((3+2)*2)]); 
            }
        );
        int arr[ ( { int x = 5; x; } ) ]; /* GCC statement expression */
    };
#endif

/* More complex nested types */
typedef int (*ComplexFuncPtr)(
    int (*)(int[][3], struct { int x; }), 
    union { 
        long l; 
        struct { 
            short s; 
            int i[2][2]; 
        } s; 
    }
);

/* Struct containing all delimiter types */
struct AllDelimiters {
    int (*fp)(int (*)(int[2][3]), struct { int a; });  /* ()[]{} */
    int arr[((2+3)*4)][5];                            /* ()[] */
    struct {                                          /* {} */
        union {                                       /* {} */
            int i;
            char c;
        } u;
    } nested;
    int value __attribute__((aligned((16))));         /* (()) */
};

/* Function pointer array with complex signatures */
int (*(*func_ptr_array[3])())(
    int, 
    void (*)(int (*)(int[2]), struct { char c; })
);

/* Main function that references some types to avoid dead code elimination */
int main(void) {
    struct S1 s1 = {0};
    struct S2 s2 = {0};
    struct AllDelimiters ad = {0};
    
    /* Use variables to prevent optimization */
    if (x + y + z[0] == 0) {
        return pts[0].x + u1.i;
    }
    
    return 0;
}

/* Additional edge cases */
enum E {
    A = (1 + (2 + (3))),
    B = sizeof(struct { int x; int y[2][2]; }),
    C = ( { int tmp = 5; tmp; } )  /* GCC extension */
};

/* Nested type in typedef */
typedef struct {
    int (*method)(
        int param1,
        int (*callback)(
            int, 
            struct { 
                int x; 
                int y[((2*3)+1)]; 
            }
        )
    );
} Object;

/* Array of function pointers with nested types */
void (*signal_handlers[5])(
    int sig, 
    struct {
        int code;
        void (*cleanup)(void);
    } *info
) = {0};

/* Final complex declaration with all requirements combined */
#if defined(__GNUC__)
__attribute__((used, noinline))
#endif
static volatile struct {
    int (*ultimate_fp[2])(
        int (*(*)(int (*[][((2>1)?3:4)])(int)))(int),
        union {
            struct { int a; } s;
            int b;
        }
    );
    char data[sizeof(struct { int x; }) * ((3+2)/2)];
} ultimate_var = {0};
