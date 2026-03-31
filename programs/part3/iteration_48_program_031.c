loop:
  i = 0
  if (some_condition(i)):
    val_A = 1
  else:
    val_B = 0
  val_phi = φ(val_A, val_B)  ← Phi node merges both paths
  if (val_phi == 0):
    // do work
  i = i + 1
  if (i < n) goto loop
