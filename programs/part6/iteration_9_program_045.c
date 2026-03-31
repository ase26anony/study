   if (GET_CODE(x) == ZERO_EXTRACT || GET_CODE(x) == STRICT_LOW_PART)
     mark_referenced_resources(x, res, false);
