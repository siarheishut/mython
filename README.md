# Mython Interpreter

## Overview
This project is an implementation of a Mython interpreter, which supports a subset of Python-like features, including:

1. **Basic data types**: integers, strings, booleans (`True`, `False`), and `None`.
2. **Class system**: including fields and methods, with inheritance.
3. **Dynamic typing**: variables can reference values of any type.
4. **Arithmetic and string operations**.
5. **Control flow**: `if` statements and logical operators (`and`, `or`, `not`).
6. **Printing and string conversion**: via `print` and `str` functions.

The interpreter is built in C++ using modular components for lexing, parsing, and runtime execution. It includes an LL(1) parser for Mython's syntax and an evaluator to execute Mython programs.

## Features

### Data Types
- **Integers**: Support for arithmetic operations like `+`, `-`, `*`, `//` (integer division).
- **Strings**: Support for concatenation using `+`, comparison operators, and immutable values.
- **Booleans**: Logical constants `True` and `False`.
- **None**: Represents null values.

### Classes and Objects
- Classes are defined using `class` keyword and include fields and methods.
- Special methods: `__init__` (constructor), `__str__` (string representation).
- Support for inheritance.

### Operations
- Arithmetic operations for integers.
- String concatenation and comparison.
- Logical operations: `and`, `or`, `not`.
- Type conversion using `str`.

### Control Flow
- `if` statements with optional `else`.
- Logical conditions support numbers, strings, booleans, objects, and `None`.

### Print Function
- Outputs values to standard output with space separation.
- Converts values to strings using `str`.

### Dynamic Typing
- Variables are dynamically typed and can reference values of any type.
- Assignments use reference semantics.

## Components

### Lexer
The lexer tokenizes Mython source code into meaningful tokens, such as:
- **Keywords**: `class`, `def`, `return`, `if`, `else`, `print`, `None`, `True`, `False`.
- **Symbols**: `(`, `)`, `:`, `,`, `.`.
- **Identifiers**: Names for variables, classes, and methods.
- **Literals**: Integers and strings.
- **Special tokens**: Indentation changes (`Indent`, `Dedent`) and `Eof`.

### Parser
The parser converts the token stream into an Abstract Syntax Tree (AST) based on Mython's grammar. The AST nodes include:
- **Statements**: Expressions, assignments, print statements, `if` blocks, etc.
- **Class and method definitions**.

### Executor
The executor evaluates the AST:
- Handles variable bindings via `Runtime::Closure`.
- Evaluates expressions and executes statements.
- Manages object lifecycles and method calls.

### Object Model
- Provides classes for integers, strings, booleans, and user-defined objects.
- Includes the `ObjectHolder` class for managing Mython objects.

## File Structure
- `lexer.h/cpp`: Lexer implementation.
- `parser.h/cpp`: Parser implementation.
- `runtime/`: Contains runtime components like `object.h` and `object_holder.h`.
- `statement.h`: AST and statement execution logic.

## Future Enhancements
- Add support for more data types (e.g., floats, dictionaries).
- Extend control flow with loops.
- Implement more built-in functions.
