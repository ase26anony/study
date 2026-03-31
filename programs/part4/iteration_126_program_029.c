#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
  int value;
  /* This will trigger case '(': */
  int (*callback)(int, char*);
  /* More parentheses in bitfield */
  unsigned int bits: (sizeof(int)*8 - 1);
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
  /* This will trigger case '[': */
  int fixed_array[10];
  char variable_array[];
  /* Multi-dimensional array */
  double matrix[5][5];
};

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
  int tag;
  /* This will trigger case '{': with nested union */
  union {
    int as_int;
    float as_float;
    char* as_string;
  } data;
  /* Another nested struct */
  struct {
    int x;
    int y;
  } point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
  /* Parentheses in function pointer type */
  void (*init_func)(struct ComplexType*);
  
  /* Brackets in array declaration */
  struct StructWithParens* items[100];
  
  /* Braces for nested anonymous struct */
  struct {
    int counter;
    /* Nested array with parentheses in size expression */
    char buffer[(sizeof(int) * 256)];
  } state;
  
  /* Function pointer returning pointer to array */
  int (*(*complex_callback)[10])(void);
};

/* Test case 5: Another variation with all brackets deeply nested */
typedef struct GTY(()) DeeplyNested {
  /* Array of function pointers */
  int (*callbacks[5])(int, char*);
  
  /* Union containing struct with array */
  union {
    struct {
      int values[20];
      /* Bitfield with parentheses */
      unsigned int flags: (8 * sizeof(int));
    } data;
    /* Pointer to array of pointers */
    struct DeeplyNested** (*get_children)(void);
  } u;
} DeeplyNestedType;

#endif /* TEST_GTY_H */
