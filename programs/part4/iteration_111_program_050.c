## Important Constraint:

The compiler must **re-evaluate `cond > 0` on every iteration** because `cond` changes inside the loop. This prevents the compiler from making the optimization that would transform the code to:
