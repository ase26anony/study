#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
  int (*callback)(int, char*);  /* Parentheses for function pointer */
  unsigned int bits: (sizeof(int)*8);  /* Parentheses in bitfield */
  void (*complex_func)(int (*)(float));  /* Nested parentheses */
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
  int fixed_array[10];  /* Fixed-size array */
  char variable_array[];  /* Flexible array member */
  int (*array_of_ptrs)[5];  /* Pointer to array */
  int multi_dim[3][4];  /* Multi-dimensional array */
};

/* Test case 3: Struct with braces (nested types) */
struct GTY(()) StructWithBraces {
  int id;
  union {  /* Anonymous union with braces */
    int as_int;
    float as_float;
    struct {  /* Nested anonymous struct */
      char c;
      short s;
    } nested;
  } value;
  struct inner {  /* Named nested struct */
    int x;
    int y;
  } GTY(()) point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
  /* Function pointer returning pointer to array - uses () and [] */
  int (*(*complex_callback)(int))[10];
  
  /* Struct with bitfield using parentheses */
  struct {
    unsigned int flags: (8 * sizeof(char));
    int data;
  } GTY(()) header;
  
  /* Array of structs with function pointers */
  struct GTY(()) {
    char* name;
    void (*action)(void);
  } actions[5];
  
  /* Union with array member */
  union {
    int ints[4];
    struct GTY(()) {
      int a;
      int b;
    } pair;
  } container;
};

/* Test case 5: Pointer to function with array parameter */
typedef int (*Comparator)(int array[], size_t size);

struct GTY(()) StructWithTypedef {
  Comparator cmp;  /* Uses typedef with [] in its definition */
  void (*initialize)(struct ComplexType*);  /* Forward reference with () */
};

/* Test case 6: Multiple levels of nesting */
struct GTY(()) DeeplyNested {
  struct GTY(()) level1 {
    union GTY(()) level2 {
      struct GTY(()) level3 {
        int (*func_ptr)(int[][5]);  /* [] inside () */
        struct { int x; } anonymous;
      } deepest;
      char array[sizeof(struct level3*)];
    } mid;
  } top;
};

#endif /* TEST_GTY_H */
