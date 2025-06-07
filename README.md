# Lurje Programming Language
Lurje is a compiled programming language designed with a slightly more modern syntax. The Lurje compiler is implemented in C and generates assembly code.

## 🚀 Features
Modern and readable syntax inspired by classic languages

Compiled to assembly

Compiler written in C

## 🔧 Status
This project is in early development.

## 📄 Syntax
````
void main() { --| main function
    int i = 0 --| var declaration (integer type)

    if i >= 0 { --| if statement
        i++ --| increment operator
        i++
    }

    for int j = 0; j < i; j++ {
        print('Var value: ${j}') --| outputs j value
    }
}

-|  
this is  
a multi line  
comment...  
-|
````
