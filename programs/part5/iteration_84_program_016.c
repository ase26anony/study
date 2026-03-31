/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ( ) ) ) ) )
#define NESTED_BRACKETS [ [ [ [ ] ] ] ]
#define NESTED_BRACES { { { { } } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 1: Balanced token sequences in type definitions */

/* Struct with deeply nested function pointer */
struct DeepNested {
    /* Function pointer with nested parentheses in parameters */
    void (*callback1)(int (*helper)(int[2][(3+4)*2], 
                                   struct { 
                                       int x; 
                                       char (*fn)(double, 
                                                 float (*)(int, 
                                                          void (*)() 
                                                         ) 
                                                ); 
                                   } 
                                 ),
                     union {
                         int a;
                         struct { 
                             long b; 
                             int (*fp)(int, 
                                      char (*)(float, 
                                              double[sizeof(struct {int i;})] 
                                             ) 
                                     ); 
                         } s;
                     } u
                    );
    
    /* Array of function pointers with complex signatures */
    int (*arr_fp[((sizeof(int)*2)/4)])(char (*)(int, 
                                               struct { 
                                                   int x; 
                                                   int y[3][(2+1)]; 
                                               } 
                                              ), 
                                      double
                                     );
};

/* Union with nested type definitions */
union ComplexUnion {
    struct {
        /* Nested struct with attributes */
        struct Inner {
            int data;
            void (*method)(int, 
                          char (*callback)(struct { 
                                          int a; 
                                          int b[2][(3+4)]; 
                                         } 
                                        )
                         );
        } inner;
        
        /* Pointer to array with computed size */
        int (*ptr_to_arr)[sizeof(struct Inner)/sizeof(int)];
    } s;
    
    /* Function pointer with multiple nested levels */
    long (*complex_fp)(int (*a)(int, 
                               char (*b)(float, 
                                        double (*c)(long, 
                                                   short (*d)() 
                                                  ) 
                                       ) 
                              ), 
                      union { 
                          int x; 
                          struct { int y; } s; 
                      } u
                     );
};

/* Requirement 3: Attribute specifications with multiple parentheses */

/* Struct with GCC attributes containing parentheses */
struct __attribute__((aligned((16)), 
                     packed, 
                     deprecated("Use NewStruct instead"))) 
AttributedStruct {
    int value __attribute__((aligned((8+8)), 
                           warn_unused_result));
    char data[10] __attribute__((aligned((sizeof(int)*2))));
};

/* Variable with attributes */
int global_var __attribute__((aligned((32)), 
                            section((".data" ".special"))));

/* C++11 style attributes (if parsed as C++) */
#ifdef __cplusplus
[[deprecated("Old type"), 
  gnu::aligned((64))]] 
struct CppStruct {
    [[maybe_unused]] int member;
};
#endif

/* Requirement 4: Array declarations with complex dimensions */

/* Multi-dimensional array with parenthesized size expressions */
int complex_array[(2+3)*(4+1)][sizeof(struct DeepNested)/sizeof(int)+1];

/* Array where size uses nested type definitions */
void* pointer_array[sizeof(union { 
                          struct { 
                              int x; 
                              double y[((2)*(3))]; 
                          } s; 
                          long z; 
                      })];

/* Array with function pointer type */
char (*func_ptr_array[((int)(3.14*2))])(int, 
                                       struct { 
                                           int a; 
                                           int b[2][(1+2)]; 
                                       }
                                      );

/* Requirement 5: Initializer lists with nested braces and parentheses */

/* Complex initializer */
struct Point {
    int x;
    int y;
    int z[2][3];
};

/* Initializer with deeply nested braces and parentheses */
struct Point points[] = {
    [0] = { 
        .x = (1 + (2 * (3 + (4)))),
        .y = { { ( (2) ) } },
        .z = { { 1, 2, 3 }, { 4, 5, (6+7) } }
    },
    [1] = { 
        .x = NESTED_PARENS + 1,
        .y = 2,
        .z = { { 0 } }
    }
};

/* Variable with nested initializer */
int nested_init = { { ( 1 + (2 + (3 + (4)))) } };

/* Using macro in initializer */
int macro_init[] = COMPLEX_MACRO;

/* Requirement 6: Conditional compilation blocks with balanced tokens */

#ifdef TEST_COMPLEX
/* Type definition inside conditional block */
struct ConditionalStruct {
    int (*cond_fp)(int (*)(int[2][3], 
                          struct { 
                              int a; 
                              union { 
                                  char c; 
                                  int i; 
                              } u; 
                          } 
                         ), 
                  double
                 );
    char data[((2)*(3))];
};
#endif

#if defined(USE_NESTED) || 1
/* Always included but with conditional syntax */
union NestedUnion {
    struct {
        int depth;
        void (*nested_callback)(int, 
                               struct { 
                                   int level; 
                                   int data[2][(3+4)]; 
                               } 
                              );
    } inner;
    
#ifdef EXTRA_FEATURE
    /* Extra nested type when feature enabled */
    struct Extra {
        int value __attribute__((aligned((16))));
    } extra;
#endif
};
#endif

#ifndef SKIP_TYPEDEF
/* Typedef with complex type */
typedef int (*ComplexFuncPtr)(int (*)(int, 
                                     char (*)(float, 
                                             double[2][(3+1)] 
                                            ) 
                                    ), 
                             struct { 
                                 int tag; 
                                 union { 
                                     int i; 
                                     float f; 
                                 } value; 
                             } 
                            );
#endif

/* Main function to avoid dead code elimination */
int main(void) {
    /* Reference some variables to prevent optimization */
    complex_array[0][0] = sizeof(struct DeepNested);
    points[0].x = nested_init;
    
#ifdef TEST_COMPLEX
    struct ConditionalStruct cs;
    cs.data[0] = 1;
#endif
    
    return 0;
}

/* Additional edge cases */

/* Enum with complex expressions */
enum ComplexEnum {
    VALUE1 = (1 + (2 * (3))),
    VALUE2 = sizeof(struct { int x; int y[2][(3)]; }),
    VALUE3 = VALUE1 * ((2)+(3))
};

/* Anonymous struct in parameter */
void process_data(int (*processor)(struct { 
                                   int id; 
                                   int data[((2)*(2))]; 
                                 }, 
                                 void*
                                )
                 ) {
    /* Function body */
}

/* Nested type in type definition */
typedef struct Outer {
    struct {
        struct InnerMost {
            int value;
            int (*compute)(int, 
                          int (*)(int, 
                                 struct { 
                                     int coeff[2][(1+2)]; 
                                 } 
                                ) 
                         );
        } inner;
    } container;
} OuterType;
