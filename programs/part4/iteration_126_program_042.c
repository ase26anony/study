#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
  int value;
  /* This will trigger case '(': consume_balanced('(', ')') */
  int (*callback)(int, char*);
  /* More parentheses in bitfield */
  unsigned int bits: (sizeof(int) * 8 - 1);
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
  /* This will trigger case '[': consume_balanced('[', ']') */
  int fixed_array[10];
  char variable_array[];
  /* Multi-dimensional array */
  double matrix[5][5];
};

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
  int tag;
  /* This will trigger case '{': consume_balanced('{', '}') */
  union {
    int int_val;
    float float_val;
    char* ptr_val;
  } data;
  /* Another nested struct */
  struct {
    short x;
    short y;
  } point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
  /* Parentheses in function pointer */
  void (*init)(struct ComplexType*);
  
  /* Brackets in array declaration */
  struct StructWithParens* items[100];
  
  /* Braces for nested anonymous struct */
  struct {
    /* Nested array with parentheses in size expression */
    char buffer[(sizeof(void*) * 2)];
    
    /* Function pointer array */
    int (*handlers[5])(void);
    
    /* Union inside struct */
    union {
      long long_val;
      double double_val;
    } value;
  } state;
  
  /* Flexible array member at the end */
  unsigned char extra_data[];
};

/* Test case 5: Pointer to array of function pointers */
typedef int GTY(()) (*func_ptr_t)(int);
struct GTY(()) Container {
  /* Complex type: pointer to array of function pointers */
  func_ptr_t (*get_func_table(void))[10];
  
  /* Nested with all brackets */
  struct {
    int (*compare)(const void*, const void*);
    void* data[20];
  } GTY((skip)) utils;  /* GTY marker with parentheses */
};

#endif /* TEST_GTY_H */
