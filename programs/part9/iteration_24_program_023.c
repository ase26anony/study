/* Master header file to include all type definitions for gengtype testing */
#ifndef TEST_ALL_TYPES_H
#define TEST_ALL_TYPES_H

#include "test-scalars.h"
#include "test-strings.h"
#include "test-structs-unions.h"
#include "test-pointers.h"
#include "test-arrays.h"
#include "test-callbacks.h"
#include "test-lang-structs.h"
#include "test-undefined.h"
#include "test-complex-nested.h"

/* Additional combined types to ensure coverage */
typedef GTY(()) struct combined {
    my_int scalar_field;
    my_string string_field;
    my_struct *struct_ptr_field;
    my_array array_field;
    callback_t callback_field;
} combined_t;

/* Chainable structure for testing GTY arguments */
typedef struct GTY((chain_next = "next", chain_prev = "prev")) chain_node {
    struct chain_node *GTY((skip)) next;
    struct chain_node *GTY((skip)) prev;
    int data;
} chain_node_t;

#endif /* TEST_ALL_TYPES_H */
