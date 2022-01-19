# Lox Grammar

Grammar are split into "Statements" and "Expressions". Each expression should return a value, while statement are used to represent a series of expressions.
 
## Program

```javascript
program         => declaration * EOF;
```

## Declarations

```javascript
declaration     => varDecl
                |  funcDecl
                |  classDecl
                |  statement ;
// allow: var a,b=1,c="hello",...;
varDecl         => "var" IDENTIFIER ("=" expression )?
                   (, IDENTIFIER ("=" expression )?)* ";" ;
funcDecl        => "func" function ;
classDecl       => "class" IDENTIFIER ( ">" IDENTIFIER )? "{" function* "}" ;
```

## Statements

```javascript
statement       => exprStmt
                |  ifStmt
                |  whileStmt
                |  forStmt
                |  breakStmt
                |  continueStmt
                |  returnStmt
                |  importStmt
                |  block
exprStmt        => expression ";" ;
ifStmt          => "if" "(" expression ")" statement
                ( "else" statement )? ;
whileStmt       => "while" "(" expression ")" statement ;
forStmt         => "for" "(" (varDecl | exprStmt | ";")
                expression? ";"
                expression? ")" statement ;
breakStmt       => "break" ";" ;
continueStmt    => "continue" ";" ;
returnStmt      => "return" expression? ";" ;
importStmt      => "import" "{" ("*" | (IDENTIFIER "as" IDENTIFIER)+) "}" "from" STRING ";" ;
block           => "{" declaration* "}" ;
```

## Expressions

```javascript
expression      => comma ;
comma           => assignment (, assignment)* ;
assignment      => (call ".") IDENTIFIER ("=" | "+=" | "-=" | "*=" | "/=") assignment  
                |  ternary ;
ternary         => logic_or ( "?" expression ":" ternary)? ;
logic_or        => logic_and ("or" logic_and)* ;
logic_and       => equality ("and" equality)* ;
equality        => comparison ( ( "!=" | "==" ) comparison )* ;
comparison      => term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term            => factor ( ( "-" | "+" ) factor )* ;
factor          => unary ( ( "/" | "*" ) unary )* ;
unary           => ( "!" | "-" ) unary | prefix ;
prefix          => ("++" | "--") call | postfix ;
postfix         => call ("++" | "--")? ;
call            => primary ( "(" arguments? ")" | "." IDENTIFIER | "[" logic_or "]")* ;
primary         => "true" | "false" | "nil" | "this" | "super" "." IDENTIFIER
                | NUMBER | STRING | IDENTIFIER | "(" expression ")"
                | lambda | list
lambda          => "func" "(" parameters? ")" block;
list            => "[" arguments "]" ;
```

## Utility

This part extracts some useful syntaxes, while they're not Node on the AST

```javascript
arguments       => ternary ( "," ternary )* ;
function        => IDENTIFIER "(" parameters? ")" block ;
// parameters could have default value, but they must sit in the back
parameters      => IDENTIFIER (= ternary)? ( "," IDENTIFIER (= ternary)?)* ;
```

## Last Update

2022-01-19 12:47:50