## Primay expression
- work status until 2025/11/03

| Category           | Implemented |
| ------------------ | ----------- |
| Identifier         | ✅          |
| Character Literal   | ✅          |
| String Literal      | ✅          |
| Integer/Float Literal | ✅          |
| Boolean            | ✅          |
| null/this/super    | ✅          |
| new                | ✅          |
| fun/lambda         | ✅          |
| cast               | ✅          |
| Array Literal      | ✅          |
| sizeof/typeof      | ⛔️         |
| Parenthesized Expression | ✅          |


##### todo -- sizeof/typeof
```c++
else if (check(TokenType::kSizeof) || check(TokenType::kTypeof))
{
    auto op = current().type;
    advance(); // skip 'sizeof' or 'typeof'

    if (!check(TokenType::kLParen))
        return Result::failure("expected '(' after sizeof/typeof", ERRR());
    advance(); // skip '('

    Expected<ASTExpr*> inner;
    if (check(TokenType::kIdentifier) || check(TokenType::kLParen))
    {
        inner = parseExpr();
    }
    else
    {
        auto type = parseType();
        if (type.hasError())
            return Result::failure("invalid argument in sizeof/typeof", ERRR());
        inner = type.value();
    }
    CHECK_ERROR(inner);

    if (!check(TokenType::kRParen))
        return Result::failure("expected ')' after sizeof/typeof argument", ERRR());
    advance();

    return new BuiltinUnaryExpr(op, inner.value());
}

```