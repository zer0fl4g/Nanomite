#Code Convention for Nanomite

## Introduction
If you want to contribute to this project then please follow this guide. It will only
cover the most important things and may be updated from time to time. This should help
other developers to read our code and make things easier for us.

##General Naming
- english language
- only alphanumeric names e.g "isEnabled1"
- camelCase not PascalCase
- names must be readable and easy to remember

##Files
- ".h" ending for header files
- ".cpp" ending for code files
- if possible use the same name for header and code files
- use header file guards to prevent multiple inclusion

##Variables
- do not use hungarian naming
- use "p" prefix for pointers
- initialize variables as soon as possible
- deklare a variable as near as possible to her first use
- choose logical names with relation to their content

##Functions
- if a functions purpose is to return a value then name it like this
- order parameters first input then output
- after each parameter there has to be a space
- do not return large objects, use references or pointers instead
- choose describing function names

##Classes
- if possible use initialization list for member variables
- use "m_" prefix for private member variables
- take advantage of encapsulation when declaring public member variables - use geter and seter
- public: before private:
- variables before methods