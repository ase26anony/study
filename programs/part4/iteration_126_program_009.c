#ifndef TEST_GTY_PARSER_H
#define TEST_GTY_PARSER_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
  int value;
  /* This will trigger the '(' case */
  int (*callback)(int, char*);
  /* More parentheses in bitfield */
  unsigned int bits: (sizeof(int) * 8 - 1);
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
  /* Static array - triggers '[' case */
  int static_array[10];
  /* Flexible array member */
  char flexible_array[];
  /* Multi-dimensional array */
  double matrix[3][4];
};

/* Test case 3: Struct with braces (nested types) */
struct GTY(()) StructWithBraces {
  int id;
  /* Anonymous nested union with braces */
  union {
    int as_int;
    float as_float;
  } GTY((tag("0"))) data;
  
  /* Nested struct definition */
  struct {
    int x;
    int y;
  } GTY((skip)) point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
  /* Function pointer returning pointer to array - has '(' and '[' */
  int (*(*complex_callback)[5])(int, char*);
  
  /* Struct with array of function pointers */
  struct {
    /* Array of function pointers */
    void (*handlers[3])(void);
    /* Bitfield with parentheses */
    unsigned flags: (8 * sizeof(unsigned) - 4);
  } GTY((desc("%1.flags"))) nested;
  
  /* Union with flexible array and nested struct */
  union {
    struct {
      int count;
      /* Variable length array */
      int items[];
    } GTY((length("%h.count"))) var_struct;
    /* Simple array */
    char buffer[256];
  } GTY((tag("1"))) storage;
};

/* Test case 5: Typedef with function pointer type */
typedef int GTY((user)) (*compare_func_t)(const void*, const void*);

struct GTY(()) Container {
  compare_func_t comparator;
  /* Array of pointers with parentheses in type */
  void* GTY((skip)) items[10];
};

/* Test case 6: More nested cases */
struct GTY(()) OuterStruct {
  /* Pointer to array of structs */
  struct Inner {
    int value;
    /* Function pointer member */
    void (*action)(struct Inner*);
  } GTY((ptr)) (*array_ptr)[];
  
  /* Anonymous struct with bitfield */
  struct {
    unsigned a: 3;
    unsigned b: (5 + 2);
  } flags;
};

#endif /* TEST_GTY_PARSER_H */
