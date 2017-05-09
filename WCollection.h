/**	@file
	Types and functions common for collection modules
*/
#ifndef WCOLLECTION_H_INCLUDED
#define WCOLLECTION_H_INCLUDED

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

/**	Function prototype for cloning an element.
*/
typedef void*	WElementClone(const void* element);

/**	Function prototype for deleting an element.
*/
typedef void	WElementDelete(void** element);

/**	Function prototype for comparing to elements with another.
*/
typedef int		WElementCompare(const void* element1, const void* element2);

/**	Function prototype for getting an element from a string.
*/
typedef void*	WElementFromString(const char* wtypeStr);

/**	Function prototype for converting an element to a string.

	@param element
	@return The returned string. May not be NULL!
*/
typedef char*	WElementToString(const void* element);

/**	Function prototype for doing read-only stuff to an element.
*/
typedef void	WElementForeach(const void* element, void* foreachData );

/**	Function prototype for doing read-only stuff to an element and its index.
*/
typedef void	WElementForeachIndex(const void* element, size_t index, void* foreachData );

/**	Function prototype for mapping an input element to an output element.
*/
typedef void*	WElementMap(const void* element, const void* mapData);

/**	Function prototype for combining the element and the intermediate result to a new internediate result.
*/
typedef	void*	WElementReduce(const void* element, const void* intermediate);

/**	Function prototype returning true if the element meets a certaion condition.
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

/** Defines a collection WType for raw void* pointers. Can be passed to functions like warray_new().

	- clone = Element_clonePtr()
	- delete = Element_deletePtr()
	- compare = Element_comparePtr()
	- fromString = NULL
	- toString = NULL
*/
extern const WType* wtypePtr;

/** Defines a collection WType for int values. Can be passed to functions like warray_new().

	- clone = Element_cloneInt()
	- delete = Element_deleteInt()
	- compare = Element_compareInt()
	- fromString = Element_fromStringInt()
	- toString = Element_toStringInt()
*/
extern const WType* wtypeInt;

/** Defines a collection WType for char* values. Can be passed to functions like warray_new().

	- clone = Element_cloneStr()
	- delete = Element_delete()
	- compare = Element_compareStr()
	- fromString = Element_fromStringStr()
	- toString = Element_toStringStr()
*/
extern const WType* wtypeStr;

/** Defines a collection WType for double values. Can be passed to functions like warray_new().

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
welement_clonePtr( const void* element );

/**	Does nothing and especially does not free the element.

	@param elementPtr
*/
void
welement_deletePtr( void** element );

/**
*/
int
welement_comparePtr( const void* element1, const void* element2 );

//---------------------------------------------------------------------------------
//	int element methods
//---------------------------------------------------------------------------------

/**	Simply return the element pointer.

	Gets used e.g. for elements of type char and int.

	@param element
	@return
*/
void*
welement_cloneInt( const void* element );

/**	Do nothing with the element.

	Gets used e.g. for elements of type char and int.

	@param elementPtr The pointer to the element
*/
void
welement_deleteInt( void** element );

/**	Compare the pointer values directly.
*/
int
welement_compareInt( const void* element1, const void* element2 );

/**	Convert the string with strtol() to a number.
*/
void*
welement_fromStringInt( const char* str );

/**	Convert the pointer value to a string.
*/
char*
welement_toStringInt( const void* element );

//---------------------------------------------------------------------------------
//	char* element methods
//---------------------------------------------------------------------------------

/**	Clone a char* element.

	@param element The string to be cloned
	@return The cloned string.
*/
void*
welement_cloneStr( const void* element );

/**	Free the string and set the pointer to NULL.

	@param elementPointer The pointer to the element
*/
#define welement_deleteStr welement_delete

int
welement_compareStr( const void* element1, const void* element2 );

void*
welement_fromStringStr( const char* str );

char*
welement_toStringStr( const void* element );

//---------------------------------------------------------------------------------
//	double element methods
//---------------------------------------------------------------------------------

/*	Clone a double element.

	@param element The double to be cloned given as (double*)
	@return The cloned double.
*/
void*
welement_cloneDouble( const void* element );

#define welement_deleteDouble( ... ) welement_delete( __VA_ARGS__ )

int
welement_compareDouble( const void* element1, const void* element2 );

void*
welement_fromStringDouble( const char* str );

char*
welement_toStringDouble( const void* element );

//---------------------------------------------------------------------------------
//	Other element methods
//---------------------------------------------------------------------------------

/**	Generic delete method, freeing the element and NULLing the pointer
*/
void
welement_delete( void** element );

//---------------------------------------------------------------------------------
//	Condition functions
//---------------------------------------------------------------------------------

bool
welement_conditionStrEquals( const void* element1, const void* element2 );

bool
welement_conditionStrEmpty( const void* element, const void* conditionData );

//---------------------------------------------------------------------------------
//	Foreach functions
//---------------------------------------------------------------------------------

void
welement_foreachStrPrint( const void* element, const void* foreachData );

void
welement_foreachIndexStrPrint( const void* element, size_t index, const void* foreachData );

//---------------------------------------------------------------------------------

#endif // WCOLLECTION_H_INCLUDED
