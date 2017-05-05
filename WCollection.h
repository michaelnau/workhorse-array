/**	@file
	Types and functions common for collection modules
*/
#ifndef COLLECTION_H_INCLUDED
#define COLLECTION_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>

//---------------------------------------------------------------------------------

/**	Return value of search functions if no element could be found.
*/
#define WElementNotFound ((void*)&__welementNotFound)
extern char __welementNotFound;

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

/**	Defines the essential behaviour and thus the type of a collection element.
	It is passed to functions like warray_new().
*/
typedef struct WType {
	WElementClone*		clone;		///<Method to copy an element into the collection. Mandatory.
	WElementDelete*		delete;		///<Method to destroy an element, e.g. if it is removed from the collection. Mandatory.
	WElementCompare*		compare;	///<Method to compare two elements with each other. Mandatory only for some collection functions.
	WElementFromString*	fromString;	///<Method to parse an element from a string. Mandatory only for some collection functions.
	WElementToString*	toString;	///<Method to convert an element to a string. Mandatory only for some collection functions.
}WType;

//---------------------------------------------------------------------------------
//	Predefined element types
//---------------------------------------------------------------------------------

/** Defines a collection WType for raw void* pointers. Can be passed to functions like warray_new().

	- clone = Element_clonePtr()
	- delete = Element_deletePtr()
	- compare = Element_compareUndefined()
	- fromString = Element_fromStringUndefined()
	- toString = Element_toStringUndefined()
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
WElement_clonePtr( const void* element );

/**	Does nothing and especially does not free the element.

	@param elementPtr
*/
void
WElement_deletePtr( void** wtypePtr );


//---------------------------------------------------------------------------------
//	int element methods
//---------------------------------------------------------------------------------

/**	Simply return the element pointer.

	Gets used e.g. for elements of type char and int.

	@param element
	@return
*/
void*
WElement_cloneInt( const void* element );

/**	Do nothing with the element.

	Gets used e.g. for elements of type char and int.

	@param elementPtr The pointer to the element
*/
void
WElement_deleteInt( void** element );

/**	Compare the pointer values directly.
*/
int
WElement_compareInt( const void* element1, const void* element2 );

/**	Convert the string with strtol() to a number.
*/
void*
WElement_fromStringInt( const char* str );

/**	Convert the pointer value to a string.
*/
char*
WElement_toStringInt( const void* element );

//---------------------------------------------------------------------------------
//	char* element methods
//---------------------------------------------------------------------------------

/**	Clones a string element.

	@param element The string to be cloned
	@return The cloned string.
*/
void*
WElement_cloneStr( const void* element );

/**	Free the string and set the pointer to NULL.

	@param elementPointer The pointer to the element
*/
#define WElement_deleteStr WElement_delete

int
WElement_compareStr( const void* element1, const void* element2 );

void*
WElement_fromStringStr( const char* str );

char*
WElement_toStringStr( const void* element );

//---------------------------------------------------------------------------------
//	double element methods
//---------------------------------------------------------------------------------

/*	Clone a double element.

	@param element The double to be cloned given as (double*)
	@return The cloned double.
*/
void*
WElement_cloneDouble( const void *element );

#define WElement_deleteDouble( ... ) WElement_delete( __VA_ARGS__ )

int
WElement_compareDouble( const void* element1, const void* element2 );

void*
WElement_fromStringDouble( const char* str );

char*
WElement_toStringDouble( const void* element );

//---------------------------------------------------------------------------------
//	Other element methods
//---------------------------------------------------------------------------------

/**	Generic delete method, freeing the element and NULLing the pointer
*/
void
WElement_delete( void **wtypePtr );

#if 0
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
Element_deleteUndefined( void** wtypePtr );

/**	Method stub for elements not implementing a compare() method

	If called it aborts the program with an error message.
*/
int
Element_compareUndefined( const void* element1, const void* element2 );

/**	Method stub for elements not implementing a fromString() method

	If called it aborts the program with an error message.
*/
void*
Element_fromStringUndefined( const char* wtypeStr );

/**	Method stub for elements not implementing a toString() method

	If called it aborts the program with an error message.
*/
char*
Element_toStringUndefined( const void* element );
#endif // 0

//---------------------------------------------------------------------------------
//	Condition functions
//---------------------------------------------------------------------------------

bool
WElement_conditionStrEquals( const void* e1, const void* e2 );

bool
WElement_conditionStrEmpty( const void* element, const void* conditionData );

//---------------------------------------------------------------------------------
//	Foreach functions
//---------------------------------------------------------------------------------

void
WElement_foreachStrPrint( const void* element, const void* foreachData );

void
WElement_foreachIndexStrPrint( const void* element, size_t index, const void* foreachData );

//---------------------------------------------------------------------------------
//	Iterators
//---------------------------------------------------------------------------------

/**	Iterator type for collection modules. To be used only through the iterator
	functions.
*/
typedef struct WIterator {
	const void* collection;										//Private, do not use.
	bool		(*hasNext)(const struct WIterator* iterator);	//Private, do not use.
	const void*	(*next)(struct WIterator* iterator);			//Private, do not use.
}WIterator;

/**	When declared as autoIterator, an iterator gets automatically destroyed when
	leaving scope.
*/
#define autoWIterator __attribute__(( cleanup( WIterator_delete ))) WIterator

//---------------------------------------------------------------------------------

/**
*/
typedef struct WIteratorNamespace {
	void		(*delete)	(WIterator** iterator);
	bool		(*hasNext)	(const WIterator* iterator);
	const void*	(*next)		(WIterator* iterator);
}WIteratorNamespace;

/**
*/
#define witeratorNamespace {			\
	.delete = WIterator_delete,		\
	.hasNext = WIterator_hasNext,	\
	.next = WIterator_next,			\
}

/**	Delete the iterator and free its memory.

	@param iterator
*/
void
WIterator_delete( WIterator** iterator );

/**	Check if there are more elements in the iterator.

	@param iterator
	@return
*/
static inline bool
WIterator_hasNext( const WIterator* iterator ) { return iterator->hasNext( iterator ); }

/**	Return the next element in the iterator.

	@param iterator
	@return The next element. It is meant to be read-only. The pointer is only
		guaranteed to be  valid as long as no functions from the underlying collection
		module are called. If the element is to be used long-term, please make a
		clone of it.
*/
static inline const void*
WIterator_next( WIterator* iterator ) { return iterator->next( iterator ); }

//---------------------------------------------------------------------------------

#endif // COLLECTION_H_INCLUDED
