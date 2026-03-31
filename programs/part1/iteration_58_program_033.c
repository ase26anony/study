case TYPE_NONE:
  // Should never happen - internal error
  gcc_unreachable ();
  break;
default:
  // Handle unexpected type values
  gcc_unreachable (); // or log an error
  break;
