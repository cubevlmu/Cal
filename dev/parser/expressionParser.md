Structure

```
parseExpr
 └── parseCommaExpr
      └── parseAssignExpr
           └── parseConditionalExpr
                └── parseLogicalOrExpr
                     └── parseLogicalAndExpr
                          └── parseBitwiseOrExpr
                               └── parseBitwiseXorExpr
                                    └── parseBitwiseAndExpr
                                         └── parseEqualityExpr
                                              └── parseRelationalExpr
                                                   └── parseShiftExpr
                                                        └── parseAdditiveExpr
                                                             └── parseMultiplicativeExpr
                                                                  └── parseUnaryExpr
                                                                       └── parsePostfixExpr
                                                                            └── parsePrimaryExpr

```

| Level     | Function Name                 | Main Operators / Forms                                                              | Associativity | Lower Level Call            | Output AST Node                                          | Example                                   |
| :-------- | :---------------------------- | :---------------------------------------------------------------------------------- | :-----------: | :-------------------------- | :------------------------------------------------------- | :---------------------------------------- |
| **1️⃣**    | `parsePrimaryExpr()`          | Basic units:<br>Literals (int/float/string/bool)<br>Identifiers<br>Parenthesized `(expr)` | N/A           | ❌                          | `LiteralExpr`, `IdentifierExpr`, `ParenExpr`             | `42`, `"hi"`, `flag`, `(a + b)`           |
| **2️⃣**    | `parsePostfixExpr()`          | Postfix operations:<br>Function call `()`, member access `.`, subscript `[]`, postfix `++`/`--` | Left          | `parsePrimaryExpr()`        | `CallExpr`, `MemberExpr`, `SubscriptExpr`, `PostfixExpr` | `obj.field`, `arr[i]`, `foo(1, 2)`, `a++` |
| **3️⃣**    | `parseUnaryExpr()`            | Prefix operations: `+`, `-`, `!`, `~`, `++`, `--`,<br>Type cast `(T)expr`, `new`, `delete` | Right         | `parsePostfixExpr()`        | `UnaryExpr`, `CastExpr`, `NewExpr`                       | `-a`, `!ok`, `++x`, `(i32)y`              |
| **4️⃣**    | `parseMultiplicativeExpr()`   | Multiplication/Division/Modulo: `*`, `/`, `%`                                       | Left          | `parseUnaryExpr()`          | `BinaryExpr`                                             | `a * b / c`                               |
| **5️⃣**    | `parseAdditiveExpr()`         | Addition/Subtraction: `+`, `-`                                                      | Left          | `parseMultiplicativeExpr()` | `BinaryExpr`                                             | `a + b - c`                               |
| **6️⃣**    | `parseShiftExpr()`            | Bit shifts: `<<`, `>>`                                                              | Left          | `parseAdditiveExpr()`       | `BinaryExpr`                                             | `a << 2 >> 1`                             |
| **7️⃣**    | `parseRelationalExpr()`       | Relational comparisons: `<`, `<=`, `>`, `>=`, `is`, `as`                            | Left          | `parseShiftExpr()`          | `BinaryExpr`, `TypeCheckExpr`                            | `a < b`, `x is Foo`                       |
| **8️⃣**    | `parseEqualityExpr()`         | Equality comparisons: `==`, `!=`                                                    | Left          | `parseRelationalExpr()`     | `BinaryExpr`                                             | `a == b`, `x != y`                        |
| **9️⃣**    | `parseBitwiseAndExpr()`       | Bitwise AND: `&`                                                                    | Left          | `parseEqualityExpr()`       | `BinaryExpr`                                             | `a & b`                                   |
| **🔟**     | `parseBitwiseXorExpr()`       | Bitwise XOR: `^`                                                                    | Left          | `parseBitwiseAndExpr()`     | `BinaryExpr`                                             | `a ^ b`                                   |
| **1️⃣1️⃣** | `parseBitwiseOrExpr()`        | Bitwise OR: `\|`                                                                     | Left          | `parseBitwiseXorExpr()`     | `BinaryExpr`                                             | `a \| b`                                  |
| **1️⃣2️⃣** | `parseLogicalAndExpr()`       | Logical AND: `&&`                                                                   | Left          | `parseBitwiseOrExpr()`      | `BinaryExpr`                                             | `a && b`                                  |
| **1️⃣3️⃣** | `parseLogicalOrExpr()`        | Logical OR: `\|\|`                                                                   | Left          | `parseLogicalAndExpr()`     | `BinaryExpr`                                             | `a \|\| b`                                |
| **1️⃣4️⃣** | `parseConditionalExpr()`      | Conditional operator: `cond ? trueExpr : falseExpr`                                 | Right         | `parseLogicalOrExpr()`      | `ConditionalExpr`                                        | `a ? b : c`                               |
| **1️⃣5️⃣** | `parseAssignExpr()`           | Assignment & compound assignment: `=`, `+=`, `-=`, `*=`, `/=` etc.                  | Right         | `parseConditionalExpr()`    | `AssignExpr`                                             | `x = 10`, `a += b`                        |
| **1️⃣6️⃣** | `parseCommaExpr()` (Optional) | Comma expression (if supported): `,`                                                | Left          | `parseAssignExpr()`         | `CommaExpr`                                              | `a = 1, b = 2`                            |
| **1️⃣7️⃣** | `parseExpr()`                 | Expression entry point: returns single or multiple expressions (e.g., list initialization, statement entry) | Left          | `parseCommaExpr()`          | `ExprList` / sub-expression                              | `a = b + c`                               |