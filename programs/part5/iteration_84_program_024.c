/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ) ) ) )
#define NESTED_BRACKETS [ [ [ [ ] ] ] ]
#define NESTED_BRACES { { { { } } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 1: Balanced token sequences in type definitions */
struct S1 {
    /* Function pointer with nested parentheses */
    int (*fp1)(int (*)(int[2][3]), struct { int a; });
    
    /* Even more nested function pointer */
    void (**fp2)(int (*(*)(int (*)[(2+3)*4]))(double));
};

/* Struct with deeply nested type combinations */
struct S2 {
    /* Array of function pointers */
    int (*arr_fp[5])(int, char);
    
    /* Nested struct with function pointer */
    struct {
        int (*nested_fp)(struct { int x; int y; }*, int[][3]);
    } inner;
    
    /* Union with complex member */
    union {
        int (*ufp)(int (*)(int, int), double);
        char c;
    } u;
};

/* Requirement 4: Array declarations with complex dimensions */
int arr1[(2+3)*4][5];
int *ptr1[sizeof(struct {int x; char y;})/4];
int arr2[1 + (2 * (3 + 4))][NESTED_PARENS ? 5 : 10];

/* Requirement 3: Attribute specifications */
struct S3 {
    int a;
    char b;
} __attribute__((aligned(16), packed));

/* GCC-style attribute with parentheses */
struct S4 {
    int data;
} __attribute__((aligned((sizeof(int)*2)))) __attribute__((packed));

/* C++11 style attribute (valid in C2x as well) */
#if __STDC_VERSION__ >= 202311L
[[deprecated("Use struct S5 instead")]]
#endif
struct S4_alt {
    int old_data;
};

/* Requirement 5: Initializer lists with nested braces and parentheses */
int x = { { ( 1 + (2) ) } };
int y = { NESTED_BRACES };
int z[] = COMPLEX_MACRO;

struct Point {
    int x;
    int y;
    int z[3];
};

struct Point pts[] = { 
    [0] = { .x = (1), .y = {2}, .z = { {1}, {2}, {3} } },
    [1] = { .x = ( (3) ), .y = { (4) }, .z = NESTED_BRACES }
};

/* Complex initializer with nested everything */
struct ComplexInit {
    int (*fp)(int);
    int arr[2][2];
} ci = {
    .fp = NULL,
    .arr = { { (1), {2} }, { {3}, (4) } }
};

/* Requirement 6: Conditional compilation blocks with balanced tokens */
#ifdef TEST
union U1 { 
    char c; 
    struct { 
        int i; 
        int (*fp)(int[][(2+3)]);
    } s; 
};
#endif

#if defined(COMPLEX_TYPES)
struct ConditionalStruct {
    int (*cond_fp)(struct { int a; int b; }[2][(3+4)*2]);
    int cond_arr[ (1 + (2 * 3)) ][4];
};
#elif defined(SIMPLE_TYPES)
struct ConditionalStruct {
    int simple;
};
#else
struct ConditionalStruct {
    int (*default_fp)(int (*)(int), double);
    int default_arr[2][{3}? 4 : 5];  /* Mixed braces in array dimension */
};
#endif

/* Nested conditional compilation */
#if 1
    #if 0
    struct NeverDefined {
        int x;
    };
    #else
    struct DefinedWhenOne {
        int (*fp)(int (*[2])(int, int));
        struct {
            int nested;
        } s;
    };
    #endif
#endif

/* Typedef with complex type */
typedef int (*complex_fp_t)(int (*)(int[2][3]), struct { int a; int b; });

/* More typedef examples */
typedef struct Node {
    int value;
    struct Node *children[(sizeof(int) + 3)/2];
} Node_t;

/* Function pointer typedef with attributes */
typedef void (*callback_t)(int, char) 
#ifdef __GNUC__
    __attribute__((stdcall))
#endif
    ;

/* Enum with complex initializers */
enum E {
    A = (1 + (2 * 3)),
    B = {4},  /* GCC extension: braces in enum */
    C = sizeof(struct { int x; int y; })
};

/* Anonymous struct in parameter */
void func1(int, struct { int x; int y; });

/* Function returning function pointer */
int (*func2(int x))(int, int) {
    return NULL;
}

/* Main function that references some types to avoid dead code elimination */
int main(void) {
    struct S1 s1 = {0};
    struct S2 s2 = {0};
    int sum = arr1[0][0] + arr2[0][0] + x + y + z[0];
    
    /* Use conditional struct to prevent optimization */
#ifdef TEST
    union U1 u1 = {0};
    sum += u1.s.i;
#endif
    
    struct DefinedWhenOne dw = {0};
    sum += dw.s.nested;
    
    return sum - sum;  /* Always return 0 */
}

/* Final complex type definition wrapping everything */
struct UltimateType {
    /* All types of nesting together */
    int (*fp)(struct {
        int a;
        int b[(2+3)*4];
        struct {
            int (*nested_fp)(int, int);
        } inner;
    }*);
    
    /* Array with nested initializer */
    int data[2][2][2];
} ultimate = {
    .fp = NULL,
    .data = { 
        { {1, {2}}, { {3}, 4 } }, 
        { {5, 6}, {7, 8} } 
    }
};
