# Oryn Programming Language
Oryn is a fast, modern programming language, with beautiful syntax and functionality at heart.

## Syntax
### Variables
Variables are defined using a type and name.  
`int32 count;`  
A value can be defined to them on definition.  
`int32 count = 10;`  
Variables can also be 

auto test = "hi!";

// Normal functions
int32 calculate(int32 value) {
  return value * count;
}

// Flat functions
// Flat functions have no 
int32 calculatedouble(int32 value) return value * count * 2;

// Also allowed:
// int32 calculatedouble(int32 value)
//   return value * count * 2;
//

// Pure, overloaded function
// Overloaded functions require the 'overload' keyword. The original function does not.
// Pure functions use an '=>' directly after the declaration to be defined. They can be flat or normal.
overload float64 calculate(int32 value, int32 value2) => value * value2 * 1.04f64;


```
