#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Struct triggering case '(': function pointer with parentheses */
struct GTY(()) StructWithParens {
  int (*callback)(int);
  unsigned int bits: (sizeof(int)*8); /* parentheses in bitfield */
};

/* Union triggering case '[': various array declarations */
union GTY(()) UnionWithBrackets {
  int fixed_array[10];
  char variable_array[];
  double *ptr_array[5];
};

/* Struct triggering case '{': nested anonymous union */
struct GTY(()) StructWithBraces {
  int tag;
  union {
    int as_int;
    float as_float;
  } GTY((tag("tag"))) value;
};

/* Complex type combining all three bracket types */
struct GTY(()) ComplexType {
  /* Parentheses in function pointer returning pointer to array */
  int (*(*complex_callback)(int))[10];
  
  /* Brackets in flexible array member */
  struct GTY(()) InnerStruct {
    int x;
  } inner_array[];
  
  /* Braces for nested anonymous struct */
  struct {
    int a;
    char b;
  } nested;
};

#endif /* TEST_GTY_H */
