/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ( ) ) ) ) )
#define NESTED_BRACKETS [ [ [ [ ] ] ] ]
#define NESTED_BRACES { { { { } } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
/* Requirement 1: Balanced token sequences in type definitions */
struct FunctionPointerStruct {
    /* Nested function pointer with complex parameter list */
    int (*fp1)(int (*callback)(int[2][3]), struct { int a; double b; } param);
    
    /* Even more nesting */
    void (**fp2)(char (*)(int (*[][4])(), union { long x; short y; }));
};

/* Union with deeply nested type */
union NestedUnion {
    struct {
        int (*arr_ptr)[(2+3)*4];
        struct Inner {
            char *(*func)(int, ...);
        } inner;
    } data;
    long long value;
};
#endif /* TEST_COMPLEX */

/* Requirement 3: Attribute specifications with multiple parentheses */
struct __attribute__((aligned(16), packed)) AlignedStruct {
    int x __attribute__((deprecated("use y instead")));
    double y;
    char z;
} __attribute__((visibility("hidden")));

/* Another struct with GCC attributes containing parentheses */
struct __attribute__((designated_init)) ComplexAttr {
    int a;
    int b;
} __attribute__((aligned((sizeof(int)*4))));

/* Requirement 4: Array declarations with complex dimensions */
int multi_dim_array[(2+3)*4][5];
int *pointer_array[sizeof(struct {int x; char y[3];})/4];

/* Array with nested type in dimension */
extern int complex_array[sizeof(union {
    struct { int a; char b[10]; } s;
    long l;
})];

/* Requirement 1 (more): Struct with nested parentheses in bitfield */
struct BitfieldStruct {
    unsigned int a : (1 + (2 * (3)));
    signed int b : ((sizeof(int)*8) - 1);
    int c : ( ( ( 5 ) ) );
};

/* Requirement 5: Initializer lists with nested braces and parentheses */
int initialized_var = { { ( 1 + (2) ) } };

struct Point {
    int x;
    int y;
    int z[3];
};

/* Complex initializer with designated initializers */
struct Point points[] = {
    [0] = { .x = (1), .y = {2}, .z = { (3), {4}, 5 } },
    [1] = { .x = { ( (6) ) }, .y = 7, .z = COMPLEX_MACRO },
    [2] = { NESTED_BRACES, .y = 8, .z = NESTED_BRACKETS }
};

/* Nested struct initializer */
struct Outer {
    struct Inner {
        int a;
        struct Deeper {
            char c;
        } deeper;
    } inner;
    float f;
} outer_instance = {
    .inner = {
        .a = (10 + (20 * (30))),
        .deeper = {
            .c = 'X'
        }
    },
    .f = { 3.14 }
};

/* Requirement 4 (more): Variable length array in struct */
struct VLAHolder {
    int size;
    int data[];  /* Flexible array member */
};

/* Function with complex return type and attributes */
__attribute__((noinline, cold))
int (*complex_function(int param))[]
    __attribute__((warn_unused_result)) {
    static int arr[] = {1, 2, 3, NESTED_PARENS, 5};
    return &arr;
}

/* Typedef with nested parentheses */
typedef int (*FuncPtrTypedef)(int (*)(int[][(2+3)]), ...);

/* Enum with complex expressions (C++ only, but valid in some C contexts) */
enum ComplexEnum {
    VALUE_A = (1 << (sizeof(int)*8 - 1)),
    VALUE_B = ( ( ( 100 ) ) ),
    VALUE_C = sizeof(struct { char a[(2+3)]; })
};

/* Requirement 2: Using macros in type definitions */
int macro_array[] = NESTED_BRACES;
struct MacroStruct {
    int x NESTED_BRACKETS;
} macro_instance = COMPLEX_MACRO;

/* Main function that references our types to avoid dead code elimination */
int main(void) {
    /* Reference variables to prevent optimization */
    struct FunctionPointerStruct fps;
    (void)fps;
    
    struct AlignedStruct as;
    (void)as;
    
    struct BitfieldStruct bfs;
    (void)bfs;
    
    struct VLAHolder *vlap = NULL;
    (void)vlap;
    
    /* Use the initialized variables */
    if (initialized_var > 0) {
        points[0].x = 1;
    }
    
    outer_instance.inner.a = 42;
    
    /* Reference array */
    int sum = 0;
    for (int i = 0; i < sizeof(multi_dim_array)/sizeof(multi_dim_array[0]); i++) {
        sum += multi_dim_array[i][0];
    }
    
    /* Reference macro-based variables */
    if (macro_array[0] == 0) {
        macro_instance.x = 1;
    }
    
    return sum > 0 ? 0 : 1;
}

/* Requirement 6: More conditional compilation */
#if defined(EXTRA_FEATURES)
/* Additional complex type when EXTRA_FEATURES is defined */
struct ExtraStruct {
    int (*(*nested_fp)[5])(int (*)(), struct { 
        union { 
            int a; 
            char b[({ int x = 5; x; })]; /* GCC statement expression */
        } u; 
    });
} __attribute__((packed));
#endif

/* Final check: Ensure all three delimiter types appear in complex nested form */
int final_check = { 
    [0] = ( { 
        struct { 
            int x[({ 
                int y = 0; 
                y; 
            })]; 
        } s; 
        0; 
    } ) 
};
