// Original code might look like:
if (x == 0)  // cmp_stmt compares x (SSA_NAME) against 0
  // ...
// where x is defined by a PHI node from different paths
