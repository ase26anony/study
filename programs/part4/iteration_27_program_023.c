case TYPE_UNDEFINED:
  nb_undefined++;
  break;
case TYPE_SCALAR:
  nb_scalar++;
  break;
// ... other cases ...
case TYPE_NONE:
  // Should never reach here - invalid type encountered
  gcc_unreachable();
  // In debug builds, you might want to add:
  // assert(0 && "Invalid TYPE_NONE encountered");
  break;
default:
  // Handle any unexpected type values
  log_error("Unexpected type value: %d", type);
  break;
