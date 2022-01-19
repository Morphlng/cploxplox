# cploxplox

> Since C++ is Cplusplus, I used Cploxplox and Cxx :)

A Lox Interpreter implemented using Modern C++

[国内仓库](https://gitee.com/Morphlng/cploxplox)

## Installation

> This code is based on C++17, so you'll need g++-8 or higher version to compile.
> 
> For Windows User, I suggest using [TDM-GCC](https://jmeubank.github.io/tdm-gcc/). Just be aware that you'll have to manually rename "mingw32-make" to "make".

1. Run `make` under the root folder
2. The executable `cploxplox` will be generated in `output` folder
3. `make clean` can delete all .o files
4. `make clean` can delete the executable as well

## Syntax

> To see full Grammar, please goto [grammar.md](./grammar.md)

### Declarations

#### Variable

```javascript
lox > var a = 1;
lox > var b;
```

You **have to** declare a variable before using it. Use `var` to declare, with or without an init value.

#### Function

```javascript
lox > func div(a,b=1) {
...     return a/b;        
...   }
```

Unlike the original Lox, I used `func` as the keyword for function. You can declare a function with name, or a lambda expression without name. See expression for Lambda.

#### Class

```javascript
lox > class A {
...     init() {
...       this.name = "A";
...     }
...     
...     hi() {
...       print(this.name); // you have to use "this" for class-member
...     }
...   }
...
lox > class B > A {
...     hi() {
...       super.hi();
...       print("B");
...     }
...   }
...
lox > var b = B();
lox > b.hi();
"A"
"B"
```

I used `>` instead to represent inheritance, so be careful when running original lox code.

### Statements

#### Block

Block is the representation of a scope, each section surrounded by `{}` will create a new Scope, and Lox is a Lexical Scoping language. 

In most cases, Block is the accessory of other statement like `for-loop`, but you could also use it alone:

```javascript
lox > var a = "global";
...   {
...     func showA() {
...       print(a);
...     }
...     
...     showA();
...     var a = "block";
...     showA();
...     print(a);
...   }
"global"
"global"
"block"
```

> This example shows you what is Lexical Scoping.

#### ifStmt

Lox only have if and else, and it's enough.

```javascript
lox > func Range(age) {
...     if (age < 18) {
...       return "Teen";
...     }
...     else if (age < 35) {
...       return "Programmer";
...     }
...     else {
...       return "Retired";
...     }
...   }
...
lox > Range(16);
...
Teen
lox > Range(19);
...
Programmer
lox > Range(87);
...
Retired
```

> Lox is not Python, you can't leave out the "()"

#### ForStmt

We have while and for loop, they all follow the syntax of C++.

```javascript
lox > for(var i = 0; i < 3; i+=1){
...   	print(i);
...   }
...
0
1
2
```

#### WhileStmt

```javascript
lox > var i = 0;
lox > while(i < 3){
...     print(i++);	
...   }
...
0
1
2
```

> As you can see，cploxplox support ++、--、+=、-=、*=、/=

#### break/continue Stmt

break/continue can only be used inside loop statement.

```javascript
lox > for(var i = 0; i < 10; i++){
...		if(i > 5) break;
...		if(i == 3) continue;
...   	print(i);
...   }
...
0
1
2
4
5
```

#### ReturnStmt

You can return nothing, e.g. `return;`, or return a value. Return can only be used inside a function.

> Note: you can't return any value except nil in a class init function.

####  ImportStmt

最新版本的cploxplox加入了模块概念，就像python的import、c++的include，你可以引用另一个lox文件中定义的函数、类、与全局变量：

In the latest update, I introduced `import`. It's similar to Python or TypeScript, you can import **global** functions, class, and variables from another lox script.

```typescript
lox > import { add as a, sub as s } from "math.lox"
lox > a(1,2);
3
// or
lox > import { * } from "math"
lox > add(1,2);
3 
```

> Note: 
> 1. After `from` should be a filepath. If it's an absolute path, then the Interpreter will check if the file exists. Otherwise, the Interpreter will search the Environment for "LOXLIB", and go through each folder to find the script. If you haven't define "LOXLIB", then it will just look for the script under current running folder.
> 2. The filepath could leave out the extension, as shown in the example. The Interpreter will automatically add suffix ".lox".

### Expression

Expression is mainly for all sort of arithmetic computation

#### Assignment

1. You can assign value to a **declared** variable.
2. Or add new field to a Instance.
3. Reassign list element.

```javascript
lox > var a = 1;
lox > a += 10;
11
lox > class Person{}
lox > var me = Person();
lox > me.name = "Dave"
lox > list = [1,2,3];
lox > list[0] = "Hello";
```

#### Lambda

When declaring function, if there is no name after keyword `func`, then it will be a lambda function.

```javascript
lox > func (a,b){return a+b;}(1,2);
3
```

> Lambda is an expression，so it will return a value. You can assign it to a variable, pass it to another function, all sorts of things.

## Built-in functions and classes

Go see [scripts/doc/Built_in_Expansion_Guide.md](./scripts/doc/Built_in_Expansion_Guide.md)

PS: May need update.

## Command-line arguments

```bash
$ ./cploxplox -h
Welcome to cploxplox!
Usage: ./cploxplox [options...]

Options:
        -f,--file : Execute Lox script from given file_path [default: none]
               -i : A flag to toggle interactive mode [implicit: "true", default: false]
     -v,--verbose : A flag to toggle verbose [implicit: "true", default: false]
       -D,--Debug : A flag to toggle debug mode [implicit: "true", default: false]
        -h,--help : print help [implicit: "true", default: false]
```

By default, cploxplox will run in REPL mode.

> See [Argparse](https://github.com/morrisfranken/argparse)，for further extension.

## Credits

|             [munificent](https://github.com/munificent)              |                  [TobiPristupin](https://github.com/TobiPristupin)                   |                 [chanryu](https://github.com/chanryu)                 |
| :--------------------------------------------------------------------------: | :---------------------------------------------------------------------------: | :--------------------------------------------------------------------------: |
| <img width="60" src="https://avatars.githubusercontent.com/u/46275?v=4"/> | <img width="60" src="https://avatars.githubusercontent.com/u/22137035?v=4" /> | <img width="60" src="https://avatars.githubusercontent.com/u/1925108?v=4" /> |