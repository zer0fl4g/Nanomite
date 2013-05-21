#Code Convention for Nanomite

## Introduction
If you want to contribute to this project then please follow this guide. It will only
cover the most important things and may be updated from time to time. This should help
other developers to read our code and make things easier for us.

##General Naming
- english language
- only alphanumeric names e.g "IsEnabled1"
- names must be readable and easy to remember 

##Files
- ".h" for header files
- ".cpp" for code files
- "cls" prefix for files that will contain a class
- "qt" prefix for qt gui related stuff
- if possible use the same name for header and code files
- use header file guards to prevent multiple inclusion

##Variables
- do not use hungarian naming
- use "p" prefix for pointers
- initialize variables as soon as possible
- deklare a variable as near as possible to her first use
- choose logical names whith relation to their content

##Functions
- if a functions purpose is to return a value then name it like this
- order parameters first input then output
- do not return large objects, use references or pointers instead
- choose describing function names

##Classes
- if possible use initialization list for member variables
- use "m_" prefix for member variables
- public: before private:
- variables before methods