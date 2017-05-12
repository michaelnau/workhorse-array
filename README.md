# workhorse-array

A dynamic growing array of void* elements

Features
--------
- Lots of common functions like append, prepend, insert, filter, map, reduce, search & sort
- Automatic memory management for the contained elements
- Elements have type information, so the array knows how to clone, delete or compare them.
- Optional API with kind of a namespace to allow shorter function names

When to use
-----------
- You need a powerful array library in C providing similar functionality like the core classes in Ruby or Python.
- You'd like to minimize the need for explicit memory management of elements. Once an element is put in the array,
  the array takes care of it. There is less risk of dangling pointers, double frees or memory leaks.
- You don't want to integrate a full-blown framework, but rather have minimum dependencies.

When not to use
---------------
- You are looking for the fastest array implementation on earth. Then please chose some other library, probably
  something macro based. workhorse-array makes heavy use of pointers, function pointers and indirection, which is
  not the optimum for cache utilization.

Dependencies
------------
- C11 (in work, there is one GNU extension in use still to get rid of)
- C standard library

License
-------
MIT

