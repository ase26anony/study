// Create a pointer to int
struct type *int_type = create_type(TYPE_SCALAR);
struct type *ptr_type = create_type(TYPE_POINTER);
ptr_type->u.pointer_to = int_type;

// Create an array of floats
struct type *float_type = create_type(TYPE_SCALAR);
struct type *array_type = create_type(TYPE_ARRAY);
array_type->u.element_type = float_type;
// Would also need array bounds somewhere
