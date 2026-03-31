#ifndef TEST_GTY_H
#define TEST_GTY_H

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
    int int_val;
    float float_val;
    char* ptr_val;
  } data;
  /* Another nested struct with braces */
  struct {
    int x;
    int y;
  } point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
  /* Function pointer returning pointer to array (parentheses and brackets) */
  int (*(*complex_callback)(int))[10];
  
  /* Struct with all bracket types nested */
  struct GTY(()) {
    /* Array of function pointers */
    void (*handlers[5])(void);
    /* Bitfield with parenthesized size */
    unsigned int flags: (sizeof(unsigned int) * 8);
  } nested;
  
  /* Flexible array of structs */
  struct GTY(()) {
    int id;
    char name[20];
  } items[];
};

/* Test case 5: Pointer to struct with parenthesized declarator */
typedef struct GTY(()) BaseType {
  int data;
} BaseType;

struct GTY(()) Container {
  /* Pointer with parenthesized declarator */
  BaseType *(*get_base)(void);
  /* Array of pointers to functions */
  void (*operations[3])(BaseType*);
};

#endif /* TEST_GTY_H */
