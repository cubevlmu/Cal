Structure

```
parseExpr
  └── parseAssignExpr
         └── parseLogicalOrExpr
                └── parseLogicalAndExpr
                       └── parseEqualityExpr
                              └── parseRelationalExpr
                                     └── parseAdditiveExpr
                                            └── parseMultiplicativeExpr
                                                   └── parseUnaryExpr
                                                          └── parsePrimaryExpr
```

| Level | Function Name | Main Operators / Functions | Associativity | Lower Level Call | Output AST Node | Example |
|-------|---------------|----------------------------|---------------|------------------|-----------------|---------|
| **1️⃣** | `parsePrimaryExpr()` | Parses most basic units:<br>Literals (int/float/string/bool/char)<br>Identifiers (variable/function/class)<br>Grouped expressions `(expr)` | N/A | ❌ | `LiteralExpr`, `IdentifierExpr`, `CallExpr`, `MemberAccessExpr`, ... | `42`, `"str"`, `x`, `(a + b)`, `foo()`, `obj.member` |
| **2️⃣** | `parseUnaryExpr()` | Prefix operations: `+`, `-`, `!`, `~`, `++`, `--`, `*` (dereference), `&` (address-of),<br>and type casting `(T)a` | Right | `parsePrimaryExpr()` | `UnaryExpr`, `CastExpr` | `-a`, `!flag`, `++i`, `(i32)val` |
| **3️⃣** | `parseMultiplicativeExpr()` | Multiplicative: `*`, `/`, `%` | Left | `parseUnaryExpr()` | `BinaryExpr` | `a * b / c % d` |
| **4️⃣** | `parseAdditiveExpr()` | Additive: `+`, `-` | Left | `parseMultiplicativeExpr()` | `BinaryExpr` | `a + b - c` |
| **5️⃣** | `parseShiftExpr()` | Bit shift: `<<`, `>>` | Left | `parseAdditiveExpr()` | `BinaryExpr` | `a << 2 >> 1` |
| **6️⃣** | `parseRelationalExpr()` | Comparison: `<`, `<=`, `>`, `>=` | Left | `parseShiftExpr()` | `BinaryExpr` | `a < b`, `x >= y` |
| **7️⃣** | `parseEqualityExpr()` | Equality: `==`, `!=` | Left | `parseRelationalExpr()` | `BinaryExpr` | `a == b`, `x != y` |
| **8️⃣** | `parseBitwiseAndExpr()` | Bitwise AND: `&` | Left | `parseEqualityExpr()` | `BinaryExpr` | `a & b` |
| **9️⃣** | `parseBitwiseXorExpr()` | Bitwise XOR: `^` | Left | `parseBitwiseAndExpr()` | `BinaryExpr` | `a ^ b` |
| **🔟** | `parseBitwiseOrExpr()` | Bitwise OR: `\|` | Left | `parseBitwiseXorExpr()` | `BinaryExpr` | `a \| b` |
| **11️⃣** | `parseLogicalAndExpr()` | Logical AND: `&&` | Left | `parseBitwiseOrExpr()` | `BinaryExpr` | `a && b` |
| **12️⃣** | `parseLogicalOrExpr()` | Logical OR: `\|\|` | Left | `parseLogicalAndExpr()` | `BinaryExpr` | `a \|\| b` |
| **13️⃣** | `parseAssignExpr()` | Assignment & compound assignment: `=`, `+=`, `-=`, `*=`, `/=`, `%=`, etc. | Right | `parseLogicalOrExpr()` | `AssignExpr` | `a = b`, `x += 1` |
| **14️⃣** | `parseExpr()` | Expression entry point, may handle multiple expression combinations, comma expressions, etc. | Left | `parseAssignExpr()` | `ExprList`, `CommaExpr`, or directly returns sub-expression | `a = 1, b = 2` |