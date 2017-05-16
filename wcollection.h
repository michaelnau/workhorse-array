/**	@file
	Types and functions common for workhorse collections
*/
#ifndef WCOLLECTION_H_INCLUDED
#define WCOLLECTION_H_INCLUDED

/* Copyright (c) 2017 Michael Nau
Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdbool.h>
#include <stddef.h>

//---------------------------------------------------------------------------------

/**	Return value of search functions if no element could be found. A unique pointer
	value not matching any other pointer.
*/
extern const void* WElementNotFound;

//---------------------------------------------------------------------------------
//	Function prototypes for the element methods
//---------------------------------------------------------------------------------

//TODO: Never pass NULL elements to WElementClone and WElementDelete etc.?

/**	Function prototype for cloning an element.

	@param element The element to be cloned. May be NULL.
	@return Output element of the target collection, must be allocated according to the behaviour
		of the target collection clone() method. May be NULL.
*/
typedef void*	WElementClone(const void* element);

/**	Function prototype for deleting an element.

    The function must deallocate, free or close all resources connected with the element.

	@param element Pointer to the element to be deleted. element may never be NULL. *element
		may be NULL.
*/
typedef void	WElementDelete(void** element);

/**	Function prototype for comparing to elements with another.

	@param element1 1st input element to be compared. May be NULL.
	@param element2 2nd input element to be compared. May be NULL.
	@return < 0, == 0 or > 0 if element1 is less than, equal to or greater than element2
*/
typedef int		WElementCompare(const void* element1, const void* element2);

//TODO: Where do we actually use WElementFromString?
/**	Function prototype for getting an element from a string.

	@param string Input string to be converted to a collection element. May not be NULL.
	@return The target collection element. May be NULL.
*/
typedef void*	WElementFromString(const char* string);

/**	Function prototype for converting an element to a string.

	@param element Input element of the source collection. May be NULL.
	@return The stringified element. May not be NULL.
*/
typedef char*	WElementToString(const void* element);

/**	Function prototype for doing read-only stuff to an element.

	@param element Input element of the source collection. May be NULL.
	@param foreachData Optional data passed to the function. May be NULL.
*/
typedef void	WElementForeach(const void* element, void* foreachData );

/**	Function prototype for doing read-only stuff to an element and its index.

	@param element Input element of the source collection. May be NULL.
	@param index The element's position in the array.
	@param foreachData Optional data passed to the function. May be NULL.
*/
typedef void	WElementForeachIndex(const void* element, size_t index, void* foreachData );

/**	Function prototype for mapping an input element to an output element.

	@param element Input element of the source collection. May be NULL.
	@param mapData Optional data passed to the function. May be NULL.
	@return Output element of the target collection, must be allocated according to the behaviour
		of the target collection clone() method. May be NULL.
*/
typedef void*	WElementMap(const void* element, const void* mapData);

//TODO: Optional data argument for WElementReduce
/**	Function prototype for combining the element and the intermediate result to a new internediate result.

	@param element Input element of the source collection. May be NULL.
	@param intermediate May be NULL.
	@return The combined value of the input element and the intermediate value. Is fed back as intermediate
		result into the next iteration. Must be allocated according to the behaviour of the target type
		clone() method. May be NULL.
*/
typedef	void*	WElementReduce(const void* element, const void* intermediate);

/**	Function prototype returning true if the element meets a certaion condition.

	@param element Input element of the source collection. May be NULL.
	@param conditionData May be NULL.
	@return true if the condition is met.
*/
typedef bool	WElementCondition(const void* element, const void* conditionData);

//---------------------------------------------------------------------------------
//	The WType struct
//---------------------------------------------------------------------------------

/**	Defines the essential behaviour and thus the type of a collection element.
	It is passed to functions like warray_new().
*/
typedef struct WType {
	WElementClone*		clone;		///<Method to copy an element into the collection. Mandatory.
	WElementDelete*		delete;		///<Method to destroy an element, e.g. if it is removed from the collection. Mandatory.
	WElementCompare*	compare;	///<Method to compare two elements with each other. Mandatory only for some collection functions.
	WElementFromString*	fromString;	///<Method to parse an element from a string. Mandatory only for some collection functions.
	WElementToString*	toString;	///<Method to convert an element to a string. Mandatory only for some collection functions.
}WType;

//---------------------------------------------------------------------------------
//	Predefined element types
//---------------------------------------------------------------------------------

/** Defines a collection type for raw void* pointers. Can be passed to functions like warray_new().

	- clone = Element_clonePtr()
	- delete = Element_deletePtr()
	- compare = Element_comparePtr()
	- fromString = NULL
	- toString = NULL
*/
extern const WType* wtypePtr;

/** Defines a collection type for int values. Can be passed to functions like warray_new().

	- clone = Element_cloneInt()
	- delete = Element_deleteInt()
	- compare = Element_compareInt()
	- fromString = Element_fromStringInt()
	- toString = Element_toStringInt()
*/
extern const WType* wtypeInt;

/** Defines a collection type for char* values. Can be passed to functions like warray_new().

	- clone = Element_cloneStr()
	- delete = Element_delete()
	- compare = Element_compareStr()
	- fromString = Element_fromStringStr()
	- toString = Element_toStringStr()
*/
extern const WType* wtypeStr;

/** Defines a collection type for double values. Can be passed to functions like warray_new().

	- clone = Element_cloneDouble()
	- delete = Element_delete()
	- compare = Element_compareDouble()
	- fromString = Element_fromStringDouble()
	- toString = Element_toStringDouble()
*/
extern const WType* wtypeDouble;

//---------------------------------------------------------------------------------
//	Raw void* pointer element methods
//---------------------------------------------------------------------------------

/**	1:1 copy from input value to output value

	@param element
	@return
*/
void*
wtypePtr_clone( const void* element );

/**	Does nothing and especially does not free the element.

	@param elementPtr
*/
void
wtypePtr_delete( void** element );

/**
*/
int
wtypePtr_compare( const void* element1, const void* element2 );

//---------------------------------------------------------------------------------
//	int element methods
//---------------------------------------------------------------------------------

/**	Simply return the element pointer.

	Gets used e.g. for elements of type char and int.

	@param element
	@return
*/
void*
wtypeInt_clone( const void* element );

/**	Do nothing with the element.

	Gets used e.g. for elements of type char and int.

	@param elementPtr The pointer to the element
*/
void
wtypeInt_delete( void** element );

/**	Compare the pointer values directly.
*/
int
wtypeInt_compare( const void* element1, const void* element2 );

/**	Convert the string with strtol() to a number.
*/
void*
wtypeInt_fromString( const char* str );

/**	Convert the pointer value to a string.
*/
char*
wtypeInt_toString( const void* element );

//---------------------------------------------------------------------------------
//	char* element methods
//---------------------------------------------------------------------------------

/**	Clone a char* element.

	@param element The string to be cloned
	@return The cloned string.
*/
void*
wtypeStr_clone( const void* element );

/**	Free the string and set the pointer to NULL.

	@param elementPointer The pointer to the element
*/
#define wtypeStr_delete wtype_delete

int
wtypeStr_compare( const void* element1, const void* element2 );

void*
wtypeStr_fromString( const char* str );

char*
wtypeStr_toString( const void* element );

//---------------------------------------------------------------------------------
//	double element methods
//---------------------------------------------------------------------------------

/*	Clone a double element.

	@param element The double to be cloned given as (double*)
	@return The cloned double.
*/
void*
wtypeDouble_clone( const void* element );

#define wtypeDouble_delete( ... ) wtype_delete( __VA_ARGS__ )

int
wtypeDouble_compare( const void* element1, const void* element2 );

void*
wtypeDouble_fromString( const char* str );

char*
wtypeDouble_toString( const void* element );

//---------------------------------------------------------------------------------
//	Other element methods
//---------------------------------------------------------------------------------

/**	Generic delete method, freeing the element and NULLing the pointer
*/
void
wtype_delete( void** element );

//---------------------------------------------------------------------------------
//	Condition functions
//---------------------------------------------------------------------------------

bool
wtypeStr_conditionEquals( const void* element1, const void* element2 );

bool
wtypeStr_conditionEmpty( const void* element, const void* conditionData );

//---------------------------------------------------------------------------------
//	Foreach functions
//---------------------------------------------------------------------------------

void
wtypeStr_foreachPrint( const void* element, const void* foreachData );

void
wtypeStr_foreachIndexPrint( const void* element, size_t index, const void* foreachData );

//---------------------------------------------------------------------------------
//	Common helpers
//---------------------------------------------------------------------------------

//Not part of the public API, do not use: abort program.
void
__wdie( const char* text );

//Not part of the public API, do not use: malloc wrapper
void*
__wxmalloc( size_t size );

//Not part of the public API, do not use: realloc wrapper
void*
__wxrealloc( void* pointer, size_t size );

//Not part of the public API, do not use: convenient object allocation
#define __wxnew( type, ... )	\
	memcpy( __wxmalloc( sizeof( type )), &(type){ __VA_ARGS__ }, sizeof(type) )

//Not part of the public API, do not use: formatted string creation
char*
__wstr_printf( const char* fmt, ... );

//Not part of the public API, do not use: strdup() replacement
char*
__wstr_dup( const char* string );

//Not part of the public API, do not use: max() macro
#define __wmax( x, y )	((x) > (y) ? (x) : (y))

//Not part of the public API, do not use: swap() macro
#define __wswapPtr( var1, var2 )\
do {							\
	void* __temp = var2;		\
	var2 = var1;				\
	var1 = __temp;				\
}while(0)

//---------------------------------------------------------------------------------

#endif // WCOLLECTION_H_INCLUDED
