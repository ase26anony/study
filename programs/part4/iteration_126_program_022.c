#ifndef TEST_GTY_H
#define TEST_GTY_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
  int value;
  /* This will trigger case '(': */
  int (*callback)(int, char*);
  /* More parentheses in bitfield */
  unsigned int bits: (sizeof(int) * 8 - 1);
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
  /* This will trigger case '[': */
  int fixed_array[10];
  char variable_array[];
  /* Nested array in struct */
  struct {
    double matrix[3][3];
  } nested;
};

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
  int tag;
  /* This will trigger case '{': */
  union {
    int as_int;
    float as_float;
    void* as_ptr;
  } data;
  /* Another nested struct with braces */
  struct {
    short x, y;
  } point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
  /* Parentheses: function pointer */
  void (*init)(struct ComplexType*);
  
  /* Brackets: multiple arrays */
  int (*get_values)(int array[10], size_t count);
  char name[50];
  
  /* Braces: anonymous struct */
  struct {
    /* Nested parentheses in function pointer array */
    int (*handlers[5])(void);
    
    /* Nested brackets in multi-dimensional array */
    float coordinates[4][2];
    
    /* Nested braces in union */
    union {
      long big;
      short small[4];
    } variant;
  } internal;
  
  /* Flexible array member with parentheses in size expression */
  unsigned char extra_data[];
};

/* Test case 5: Pointer to array of function pointers */
typedef int GTY(()) (*func_ptr_t)(void);
struct GTY(()) Container {
  /* Complex type: pointer to array of function pointers */
  func_ptr_t (*get_func_array(void))[10];
  
  /* Array of structs with bitfields using parentheses */
  struct {
    unsigned int flags: (8 * sizeof(unsigned int) - 4);
    char code;
  } entries[20];
};

#endif /* TEST_GTY_H */
