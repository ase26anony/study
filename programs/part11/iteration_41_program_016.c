val₀ = 0
for i = 0 to n-1:
  valᵢ = Φ(val₀/valᵢ₋₁, 1, 0)  // Phi node merging values
  if (valᵢ == 1):
    // do work
