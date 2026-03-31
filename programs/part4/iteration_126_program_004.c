#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Include standard headers for types */
#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
  int value;
  /* Function pointer with parentheses */
  int (*callback)(int, char*);
  /* Bitfield with parenthesized expression */
  unsigned int bits: (sizeof(int) * 8 - 1);
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
  /* Fixed-size array */
  int fixed_array[10];
  /* Multi-dimensional array */
  double matrix[3][3];
  /* Flexible array member */
  char flexible_array[];
};

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
  int tag;
  /* Nested anonymous union with braces */
  union {
    int int_value;
    float float_value;
    char* string_value;
  } data;
  /* Another nested struct with braces */
  struct {
    short x;
    short y;
  } point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
  /* Function pointer returning pointer to array (parentheses and brackets) */
  int (*(*complex_callback)[5])(void);
  
  /* Array of structs with nested union (brackets and braces) */
  struct {
    int type;
    union {
      long l;
      double d;
    } value;
  } items[8];
  
  /* Pointer to function with array parameter (parentheses and brackets) */
  void (*processor)(int data[], size_t len);
  
  /* Nested struct with bitfield using parenthesized expression */
  struct {
    unsigned int flags: (sizeof(unsigned int) * 8);
    char name[32];
  } metadata;
};

/* Test case 5: Typedef with function pointer type */
typedef int GTY(()) (*comparator_func_t)(const void*, const void*);

/* Test case 6: Struct with all bracket types in one member */
struct GTY(()) UltimateTest {
  /* This has: parentheses for function, brackets for array, braces for struct */
  struct {
    int (*handlers[4])(struct { int a; int b; }*);
  } dispatcher;
};

#endif /* TEST_GTY_H */
