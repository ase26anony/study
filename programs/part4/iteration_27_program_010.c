switch (type) {
case TYPE_UNDEFINED:
    nb_undefined++;
    break;
case TYPE_SCALAR:
    nb_scalar++;
    break;
// ... other cases ...
case TYPE_NONE:
    // Should never reach here - TYPE_NONE indicates invalid/uninitialized type
    gcc_unreachable();
    break;
default:
    // Handle any unexpected type values
    gcc_unreachable(); // or log an error
    break;
}
