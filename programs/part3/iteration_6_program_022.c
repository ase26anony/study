int val;
if (cond) val = 1; else val = 2;
if (val == 1) // This `val` may come from a PHI node at the merge point.
