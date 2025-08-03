# Beere

Beere is a simple, fast, and low-level programming language, designed for systems programming with modern syntax and manual memory control.

> ⚙️ Inspired by C, Java, Go and C++, but with better readability, more safety, easier to develop projects, and optional object-oriented programming.

## Features (that beere will have in the first release and later)

- Fast native compilation to x86_64 Assembly  
- Manual memory management with safety checks  
- Simple and clean syntax
- Optional object-oriented features (classes, vtables)  
- C interoperability (`extern "C" fn`)  
- Lightweight module system and package manager (WIP)
- Full package manager system
```
beerec install [dependency]
```

## Hello World

```rs
fn main() {
    print("Hello, World!")
}
```

## Building
To compile a Beere file:

```
beerec [entry-point-file] [dot-mod-file]
```
(Compiler will generate raw Assembly and link it automatically)

## Status

> ⚠️ Beere is under active development.

## Development
> Created only by jerious1337.
