/**	@file
	Types and functions common for Collection modules
*/
#ifndef COLLECTION_H_INCLUDED
#define COLLECTION_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>

//---------------------------------------------------------------------------------

/**	Return value of search functions if no element could be found.
*/
#define ElementNotFound ((void*)&__elementNotFound)
extern char __elementNotFound;

//---------------------------------------------------------------------------------
//	Function prototypes for the element methods
//---------------------------------------------------------------------------------

/**	Function prototype for cloning an element.
*/
typedef void*	ElementClone(const void* element);

/**	Function prototype for deleting an element.
*/
typedef void	ElementDelete(void** element);

/**	Function prototype for comparing to elements with another.
*/
typedef int		ElementCompare(const void* element1, const void* element2);

/**	Function prototype for getting an element from a string.
*/
typedef void*	ElementFromString(const char* elementStr);

/**	Function prototype for converting an element to a string.
*/
typedef char*	ElementToString(const void* element);

/**	Function prototype for doing read-only stuff to an element.
*/
typedef void	ElementForeach(const void* element, void* foreachData );

/**	Function prototype for doing read-only stuff to an element and its index.
*/
typedef void	ElementForeachIndex(const void* element, size_t index, void* foreachData );

/**	Function prototype returning true for an element to be included in the filter.
*/
typedef	bool	ElementFilter(const void* element, const void* filterData);

/**	Function prototype for mapping an input element to an output element.
*/
typedef void*	ElementMap(const void* element, const void* mapData);

/**	Function prototype for combining the element and the intermediate result to a new internediate result.
*/
typedef	void*	ElementReduce(const void* element, const void* intermediate);

/**	Function prototype returning true if the element meets a certaion condition.
*/
typedef bool	ElementCondition(const void* element, const void* conditionData);

/**	Defines the essential behaviour and thus the type of a collection element.
	It is passed to functions like warray_new().
*/
typedef struct ElementType {
	ElementClone*		clone;		///<Method to copy an element into the collection. Mandatory.
	ElementDelete*		delete;		///<Method to destroy an element, e.g. if it is removed from the collection. Mandatory.
	ElementCompare*		compare;	///<Method to compare two elements with each other. Mandatory only for some collection functions.
	ElementFromString*	fromString;	///<Method to parse an element from a string. Mandatory only for some collection functions.
	ElementToString*	toString;	///<Method to convert an element to a string. Mandatory only for some collection functions.
}ElementType;

//---------------------------------------------------------------------------------
//	Predefined element types
//---------------------------------------------------------------------------------

/** Defines a collection ElementType for raw void* pointers. Can be passed to functions like warray_new().

	- clone = Element_clonePtr()
	- delete = Element_deletePtr()
	- compare = Element_compareUndefined()
	- fromString = Element_fromStringUndefined()
	- toString = Element_toStringUndefined()
*/
extern const ElementType* elementPtr;

/** Defines a collection ElementType for int values. Can be passed to functions like warray_new().

	- clone = Element_cloneInt()
	- delete = Element_deleteInt()
	- compare = Element_compareInt()
	- fromString = Element_fromStringInt()
	- toString = Element_toStringInt()
*/
extern const ElementType* elementInt;

/** Defines a collection ElementType for char* values. Can be passed to functions like warray_new().

	- clone = Element_cloneStr()
	- delete = Element_delete()
	- compare = Element_compareStr()
	- fromString = Element_fromStringStr()
	- toString = Element_toStringStr()
*/
extern const ElementType* elementStr;

/** Defines a collection ElementType for double values. Can be passed to functions like warray_new().

	- clone = Element_cloneDouble()
	- delete = Element_delete()
	- compare = Element_compareDouble()
	- fromString = Element_fromStringDouble()
	- toString = Element_toStringDouble()
*/
extern const ElementType* elementDouble;

//---------------------------------------------------------------------------------
//	Raw void* pointer element methods
//---------------------------------------------------------------------------------

/**	1:1 copy from input value to output value

	@param element
	@return
*/
void*
Element_clonePtr( const void* element );

/**	Does nothing and especially does not free the element.

	@param elementPtr
*/
void
Element_deletePtr( void** elementPtr );


//---------------------------------------------------------------------------------
//	int element methods
//---------------------------------------------------------------------------------

/**	Simply return the element pointer.

	Gets used e.g. for elements of type char and int.

	@param element
	@return
*/
void*
Element_cloneInt( const void* element );

/**	Do nothing with the element.

	Gets used e.g. for elements of type char and int.

	@param elementPtr The pointer to the element
*/
void
Element_deleteInt( void** elementPtr );

/**	Compare the pointer values directly.
*/
int
Element_compareInt( const void* element1, const void* element2 );

/**	Convert the string with strtol() to a number.
*/
void*
Element_fromStringInt( const char* elementStr );

/**	Convert the pointer value to a string.
*/
char*
Element_toStringInt( const void* element );

//---------------------------------------------------------------------------------
//	char* element methods
//---------------------------------------------------------------------------------

/**	Clones a string element.

	@param element The string to be cloned
	@return The cloned string.
*/
void*
Element_cloneStr( const void *element );

/**	Free the string and set the pointer to NULL.

	@param elementPointer The pointer to the element
*/
#define Element_deleteString Element_delete

int
Element_compareStr( const void* element1, const void* element2 );

void*
Element_fromStringStr( const char* string );

char*
Element_toStringStr( const void* element );

//---------------------------------------------------------------------------------
//	double element methods
//---------------------------------------------------------------------------------

/*	Clone a double element.

	@param element The double to be cloned given as (double*)
	@return The cloned double.
*/
void*
Element_cloneDouble( const void *element );

#define Element_deleteDouble( ... ) Element_delete( __VA_ARGS__ )

int
Element_compareDouble( const void* element1, const void* element2 );

void*
Element_fromStringDouble( const char* string );

char*
Element_toStringDouble( const void* element );

//---------------------------------------------------------------------------------
//	Other element methods
//---------------------------------------------------------------------------------

/**	Generic delete method, freeing the element and NULLing the pointer
*/
void
Element_delete( void **elementPtr );

//---------------------------------------------------------------------------------
//	Aborting element methods
//---------------------------------------------------------------------------------

/**	Method stub for elements not implementing a clone() method

	If called it aborts the program with an error message.
*/
void*
Element_cloneUndefined( const void* elememt );

/**	Method stub for elements not implementing a delete() method

	If called it aborts the program with an error message.
*/
void
Element_deleteUndefined( void** elementPtr );

/**	Method stub for elements not implementing a compare() method

	If called it aborts the program with an error message.
*/
int
Element_compareUndefined( const void* element1, const void* element2 );

/**	Method stub for elements not implementing a fromString() method

	If called it aborts the program with an error message.
*/
void*
Element_fromStringUndefined( const char* elementStr );

/**	Method stub for elements not implementing a toString() method

	If called it aborts the program with an error message.
*/
char*
Element_toStringUndefined( const void* element );

//---------------------------------------------------------------------------------
//	Condition functions
//---------------------------------------------------------------------------------

bool
Element_conditionStrEquals( const void* e1, const void* e2 );

bool
Element_conditionStrEmpty( const void* element, const void* conditionData );

//---------------------------------------------------------------------------------
//	Foreach functions
//---------------------------------------------------------------------------------

void
Element_foreachStrPrint( const void* element, const void* foreachData );

void
Element_foreachIndexStrPrint( const void* element, size_t index, const void* foreachData );

//---------------------------------------------------------------------------------
//	Iterators
//---------------------------------------------------------------------------------

/**	Iterator type for collection modules. To be used only through the iterator
	functions.
*/
typedef struct Iterator {
	const void* collection;										//Private, do not use.
	bool		(*hasNext)(const struct Iterator* iterator);	//Private, do not use.
	const void*	(*next)(struct Iterator* iterator);				//Private, do not use.
}Iterator;

/**	When declared as autoIterator, an iterator gets automatically destroyed when
	leaving scope.
*/
#define autoIterator __attribute__(( cleanup( Iterator_delete ))) Iterator

//---------------------------------------------------------------------------------

/**
*/
typedef struct IteratorNamespace {
	void		(*delete)	(Iterator** iterator);
	bool		(*hasNext)	(const Iterator* iterator);
	const void*	(*next)		(Iterator* iterator);
}IteratorNamespace;

/**
*/
#define iteratorNameSpace {			\
	.delete = Iterator_delete,		\
	.hasNext = Iterator_hasNext,	\
	.next = Iterator_next,			\
}

/**	Delete the iterator and free its memory.

	@param iterator
*/
void
Iterator_delete( Iterator** iterator );

/**	Check if there are more elements in the iterator.

	@param iterator
	@return
*/
static inline bool
Iterator_hasNext( const Iterator* iterator ) { return iterator->hasNext( iterator ); }

/**	Return the next element in the iterator.

	@param iterator
	@return The next element. It is meant to be read-only. The pointer is only
		guaranteed to be  valid as long as no functions from the underlying collection
		module are called. If the element is to be used long-term, please make a
		clone of it.
*/
static inline const void*
Iterator_next( Iterator* iterator ) { return iterator->next( iterator ); }

//---------------------------------------------------------------------------------

#endif // COLLECTION_H_INCLUDED
