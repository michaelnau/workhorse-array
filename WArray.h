/**@file
	A dynamic growing array of arbitrary elements.

	Features
	--------
	- The array can only contain elements of the same type.
	- Has many functions to add elements: Array_prepend(), Array_append(), Array_insert(),
	  Array_update().
	- Peek at elements with Array_first(), Array_last() and Array_at().
	- Take elements out of the array with Array_stealFirst(), Array_stealLast() and Array_stealAt().
	- Iterate through all elements with Array_filter(), Array_map() and Array_reduce().
	- The array copies the given elements and leaves the originals untouched.
	- Elements get deleted automatically when necessary.
	- The array takes full ownership of the elements. They can only be manipulated through functions.
	- Supports sparse arrays. The gaps are filled with NULL.

	API design
	----------
	- Once the elements are added to the array, they belong to the array and may only be
		accessed through the array functions. Don't try to read or write elements directly
		from the Array.data field, the behaviour in this case is undefined.
    - All functions except Array_new() take an array pointer as first argument. This is the array
		queried or manipulated by the functions. It may never be NULL.
    - Functions taking a const argument, be it an array or some other data guarantee not to modify it.
	- Functions taking a non-const argument probably will modify it.
	- Functions manipulating one or more array elements in general do it in place, except where it is not
		possible or does not make sense. The latter functions take a const Array* argument.
	- The elements may be of any type, as long as they can be passed as void*. NULL as element is allowed.
	- The functions never return NULL except when a NULL element is returned.
	- When a function returns a pointer to const it means that the result is read-only for the calling
		code. It might be changed by the next array function. So clone it with e.g. array->cloneElement( result )
		if you need it later.
	- When a function returns a pointer to a non-const result, it most certainly will be allocated or
		constructed in some way. The calling code is responsible to free or destroy it properly. This
		can be done with array->deleteElement( result );
    - The calling code can pass two functions to the array, one for copying and one for deleting elements.
		When an element is added to the array, it is copied there with the clone function. Whenever an
		element must be deleted by an array function, it is done with the delete function. There are default
		functions that simply copy the pointer (clone) resp. do nothing (delete). The calling code must
		ensure that either the clone function makes a deep copy of the passed element. Otherwise the array
		holds a reference to the given element. Then the caller must ensure that the reference is valid as
		long as the array is alive.
	- The array is a non-opaque, but read only structure. Don't access the actual elements directly.
	- The fields capacity, size, cloneElement and deleteElement can be read.
	- If an array function has a precondition it is described in the API documentation. The functions check if
		the preconditions are met and abort the program with an error message if they are not met
		(Design by Contract). A failed precondition is a bug in the calling code and must be corrected.
		The behaviour cannot be deactivated.

	Ownership
	---------
	- Elements get copied to the array. The original data stays untouched --> const void*
	- Can only be manipulated through array functions. --> const void*
	- Elements get automatically copied or deleted if necessary.
	- Read-only access with Array_at() --> returned as const void*
	- Ownership transfer with Array_steal() --> void*

	Create and delete an array
	--------------------------

	Put elements in the array
	-------------------------

	Read and remove elements from the array
	---------------------------------------

	|                     | Read element  | Remove element |
	|---------------------|:-------------:|:--------------:|
	| Array_at()          | x             |                |
	| Array_first()       | x             |                |
	| Array_last()        | x             |                |
	| Array_sample()      | x             |                |
	| Array_min()         | x             |                |
	| Array_max()         | x             |                |
	| Array_stealAt()     | x             | x              |
	| Array_stealFirst()  | x             | x              |
	| Array_stealLast()   | x             | x              |
	| Array_stealSample() | x             | x              |
	| Array_removeAt()    |               | x              |
	| Array_removeFirst() |               | x              |
	| Array_removeLast()  |               | x              |

	Manipulate array elements
	-------------------------

	Iterate over the array elements
	-------------------------------

	Search the array for elements
	-----------------------------

	Convert an array to and from a string
	-------------------------------------

	Check properties of the elements
	--------------------------------

    Set operations
    --------------

	Statistics operations for double elements
    -----------------------------------------
*/
#ifndef ARRAY_H_INCLUDED
#define ARRAY_H_INCLUDED

#include "WCollection.h"
#include <stdbool.h>	//bool
#include <sys/types.h>	//ssize_t

//------------------------------------------------------------
//	Types and constants
//------------------------------------------------------------

/**	The array type. Access it only through the Array_xyz() functions except
	reading the explicitly public fields.
*/
typedef struct WArray {
	size_t				size;			///<Public read-only, the actual number of elements
	size_t				capacity;		///<Public read-only, the maximum number of elements before the array must grow
	const ElementType*	type;			///<Public read only, pointer to the element methods
	void**				data;			//Private, do not directly access it.
}WArray;

/** Pointer to a struct describing methods for elements that are arrays themselves.
	Can be passed to Array_new().

	- clone = Array_clone,
	- delete = Array_delete,
	- compare = Array_compare,
	- fromString = Array_fromString,
	- toString = Array_toString
*/
extern const ElementType* arrayElement;

/**	When declared as autoArray, an array gets automatically destroyed when
	leaving scope.
*/
#define autoWArray __attribute__(( cleanup( Array_delete ))) WArray

//------------------------------------------------------------
//	Array namespace
//------------------------------------------------------------

/**	Structure containing pointers to all Array functions. Can be used to simulate
	a module namespace.

	Example:
	\code
    ArrayNamespace a = arrayNamespace;

    ...

    Array* array = Array_new( 15, elementStr );
    a.append( array, "cat" );
    a.prepend( array, "dog" );
    printf( "%s", a.toString( array ));		//->"dog", "cat"
	\endcode
*/
typedef struct WArrayNamespace {
	WArray* 	(*new)		(size_t capacity, const ElementType* type);
    WArray* 	(*clone)	(const WArray* array);
	void		(*delete)	(WArray** array);
	WArray*		(*clear)	(WArray* array);

	WArray* 	(*append)	(WArray* array, const void* element);
	WArray* 	(*prepend)	(WArray* array, const void* element);
	WArray* 	(*set)		(WArray* array, size_t, const void* element);
	WArray* 	(*insert)	(WArray* array, size_t, const void* element);
	WArray*		(*insertSorted)(WArray* array, const void* element);
	WArray*		(*addToSet)	(WArray* array, const void* element);

	WArray*		(*append_n)	(WArray* array, size_t n, void* const elements[n]);
	WArray*		(*prepend_n)(WArray* array, size_t n, void* const elements[n]);
	WArray*		(*set_n)	(WArray* array, size_t position, size_t n, void* const elements[n]);
	WArray*		(*insert_n)	(WArray* array, size_t position, size_t n, void* const elements[n]);

	const void* (*at)		(const WArray* array, size_t position);
	const void* (*first)	(const WArray* array);
	const void* (*last)		(const WArray* array);

	void* 		(*stealAt)	(WArray* array, size_t position);
	void* 		(*stealFirst)(WArray* array);
	void* 		(*stealLast)(WArray* array);
	void* 		(*stealSample)(WArray* array);

	WArray*		(*removeAt)	(WArray* array, size_t position);
	WArray*		(*removeFirst)(WArray* array);
	WArray*		(*removeLast)(WArray* array);

	size_t		(*size)		(const WArray* array);
	bool		(*empty)	(const WArray* array);
	bool		(*nonEmpty)	(const WArray* array);

	bool		(*all)		(const WArray* array, ElementCondition* condition, const void* conditionData);
	bool		(*any)		(const WArray* array, ElementCondition* condition, const void* conditionData);
	bool		(*none)		(const WArray* array, ElementCondition* condition, const void* conditionData);
	bool		(*one)		(const WArray* array, ElementCondition* condition, const void* conditionData);

	WArray* 	(*filter)	(const WArray* array, ElementFilter* filter, const void* filterData );
	WArray* 	(*reject)	(const WArray* array, ElementFilter* filter, const void* filterData );
	WArray* 	(*map)		(const WArray* array, ElementMap*, const void*, const ElementType* type );
	void*		(*reduce)	(const WArray* array, ElementReduce*, const void*, const ElementType* type );

	void		(*foreach)	(const WArray* array, ElementForeach* foreach, void* foreachData);
	void		(*foreachIndex)(const WArray* array, ElementForeachIndex* foreach, void* foreachData);

//TODO: a.min() and a.max() disturbed by min() and max() macros
//	const void*	(*min)		(const Array*);
//	const void*	(*max)		(const Array*);
	ssize_t		(*index)	(const WArray* array, const void* element);
	ssize_t		(*rindex)	(const WArray* array, const void* element);
	ssize_t		(*bsearch)	(const WArray* array, ElementCompare* compare, const void* key);
	bool		(*contains)	(const WArray* array, const void* element);
	size_t		(*count)	(const WArray* array, ElementCondition*, const void* conditionData);

	WArray*		(*reverse)	(WArray* array);
	WArray*		(*compact)	(WArray* array);
	WArray*		(*sort)		(WArray* array);
	WArray*		(*sortBy)	(WArray* array, ElementCompare* compare);
	WArray*		(*distinct)	(WArray* array);
	WArray*		(*shuffle)	(WArray* array);

	char*		(*toString)	(const WArray* array, const char delimiters[]);
	WArray*		(*fromString)(const char string[], const char delimiters[]);
	int			(*compare) 	(const WArray* array1, const WArray* array2);
	bool		(*equal)	(const WArray* array1, const WArray* array2);

	WArray*		(*unite)	(const WArray* array1, const WArray* array2);
	WArray*		(*intersect)(const WArray* array1, const WArray* array2);
	WArray*		(*symDiff)	(const WArray* array1, const WArray* array2);

	Iterator*	(*iterator)	(const WArray* array);
	Iterator*	(*iteratorReverse)(const WArray* array);
}WArrayNamespace;

/**	Predefined value for ArrayNamespace variables
*/
#define warrayNamespace					\
{										\
	.new = Array_new,					\
	.clone = Array_clone,				\
	.delete = Array_delete,				\
	.clear = Array_clear,				\
\
	.append = Array_append,				\
	.prepend = Array_prepend,			\
	.set = Array_set,					\
	.insert = Array_insert,				\
	.insertSorted = Array_insertSorted,	\
	.addToSet = Array_addToSet,			\
\
	.append_n = Array_append_n,			\
	.prepend_n = Array_prepend_n,		\
	.set_n = Array_set_n,				\
	.insert_n = Array_insert_n,			\
\
	.at = Array_at,						\
	.first = Array_first,				\
	.last = Array_last,					\
\
	.stealAt = Array_stealAt,			\
	.stealFirst = Array_stealFirst,		\
	.stealLast = Array_stealLast,		\
	.stealSample = Array_stealSample,	\
\
	.removeAt = Array_removeAt,			\
	.removeFirst = Array_removeFirst,	\
	.removeLast = Array_removeLast,		\
\
	.size = Array_size,					\
	.empty = Array_empty,				\
	.nonEmpty = Array_nonEmpty,			\
\
	.all = Array_all,					\
	.any = Array_any,					\
	.none = Array_none,					\
	.one = Array_one,					\
\
	.filter = Array_filter,				\
	.reject = Array_reject,				\
	.map = Array_map,					\
	.reduce = Array_reduce,				\
\
	.foreach = Array_foreach,			\
	.foreachIndex = Array_foreachIndex,	\
\
	.index = Array_index,				\
	.rindex = Array_rindex,				\
	.bsearch = Array_bsearch,			\
	.contains = Array_contains,			\
	.count = Array_count,				\
\
	.reverse = Array_reverse,			\
	.compact = Array_compact,			\
	.sort = Array_sort,					\
	.sortBy = Array_sortBy,				\
	.distinct = Array_distinct,			\
	.shuffle = Array_shuffle,			\
\
	.toString = Array_toString,			\
	.fromString = Array_fromString,		\
	.compare = Array_compare,			\
	.equal = Array_equal,				\
\
	.unite = Array_unite,				\
	.intersect = Array_intersect,		\
	.symDiff = Array_symDiff,			\
\
	.iterator = Array_iterator,			\
	.iteratorReverse = Array_iteratorReverse,	\
}

//------------------------------------------------------------
//	Create and destroy an array.
//------------------------------------------------------------

/**	Create a new empty array with the appropriate methods for the element type.

	@param capacity The initial element capacity. If 0 is given, the initial capacity is set to 100.
	@param type Pointer to a structure with element methods to be used in array functions.
		If NULL is passed, the elements are treated as raw pointers and the methods set in
		\ref elementPtr are used. If a type is given, but the compare, fromString or toString
		fields are NULL, then the Array functions using these methods may not be used. The array only
		keeps the type pointer, so the underlying struct must be permanently accessible.
	@return The new array
	@pre If type is given, type->clone may not be NULL.
	@pre If type is given, type->delete may not be NULL.

	Examples:
	\code
	Array* a1 = Array_new( 0, NULL );
	Array* a2 = Array_new( 10, NULL );			//The predefined elementPtr for raw void* pointers is selected.
	Array* a3 = Array_new( 10, elementStr );	//elementStr is predefined for char* strings.
	Array* a4 = Array_new( 0, elementInt );		//elementInt is predefined for int values.

	static const ElementType carType = {
        .clone = cloneCar,
        .delete = deleteCar
	};
	Array* a5 = Array_new( 0, &carType );
	\endcode
*/
WArray*
Array_new( size_t capacity, const ElementType* type );

/**	Clone the given array by cloning the elements with the array->type->clone() method.

	@param array
	@return The new array
	@pre array != NULL
*/
WArray*
Array_clone( const WArray* array );

/**	Delete the array and all elements.

	If NULL is passed, this is a no-op.

	@param array Pointer to an array. After the deletion the pointer is set to NULL.
	@return
*/
void
Array_delete( WArray** array );

/**	Delete all elements but leave the array itself intact.

	If NULL is passed, this is a no-op.

	@param array
*/
WArray*
Array_clear( WArray* array );

/**	Delete an array and assign another array to its pointer.

	If NULL is passed, this is a no-op.

	@param array. After the call array points to other.
	@param other
	@pre array->type == other->type
*/
void
Array_assign( WArray** array, WArray *other );

//------------------------------------------------------------
//	Put one element in the array.
//------------------------------------------------------------

/**	Append an element to the array.

	@param array The array to be modified in place.
	@param element The element to be added. NULL elements are allowed. The
		element is copied into the array with the clone() method given in
		the type field at Array_new().
	@return The modified array, allowing the chaining of function calls.
	@pre array != NULL
*/
WArray*
Array_append( WArray* array, const void* element );

/**	Prepend an element to the array.

	@param array The array to be modified in place.
	@param element The element to be added. NULL elements are allowed. The
		element is copied into the array with the clone() method given in
		the type field at Array_new().
	@return The modified array, allowing the chaining of function calls.
	@pre array != NULL
*/
WArray*
Array_prepend( WArray* array, const void* element );

/**	Set or update the element at the given position.

	@param array The array to be modified in place.
	@param position May be greater than the current size. A possible gap
		between the last current and the new element is filled with
		NULL elements.
	@param element The element to be added. NULL elements are allowed. The
		element is copied into the array with the clone() method given in
		the type field at Array_new().
	@return The modified array, allowing the chaining of function calls.
	@pre array != NULL
*/
WArray*
Array_set( WArray* array, size_t position, const void* element );

/**	Insert an element into the array.

	@param array The array to be modified in place.
	@param position May be greater than the current size. A possible gap
		between the last current and the new element is filled with
		NULL elements.
	@param element The element to be added. NULL elements are allowed. The
		element is copied into the array with the clone() method given in
		the type field at Array_new().
	@return The modified array, allowing the chaining of function calls.
	@pre array != NULL
*/
WArray*
Array_insert( WArray* array, size_t position, const void* element );

/**	Insert an element into the array, so that it keeps the ascending element order
	according to the array->type->compare() method.

	@param array The array to be modified in place.
	@param element The element to be added. NULL elements are allowed. The
		element is copied into the array with the clone() method given in
		the type field at Array_new().
	@return The modified array, allowing the chaining of function calls.
	@pre array != NULL
	@pre array->type->compare != NULL
*/
WArray*
Array_insertSorted( WArray* array, const void* element );

//------------------------------------------------------------
//	Put several elements in the array.
//------------------------------------------------------------

/**	Append one or several elements to the array.

	@param array The array to be modified in place.
	@param n The number of elements to be added. Must match with the actual
		number of elements in the elements array.
	@param elements A list of n elements to be added. NULL elements are allowed.
		The element is copied into the array with the clone() method given in
		the type field at Array_new().
	@return The modified array, allowing the chaining of function calls.
	@pre array != NULL
	@pre n > 0
	@pre elements != NULL

*/
WArray*
Array_append_n( WArray* array, size_t n, void* const elements[n] );

/**	Prepend one or several elements to the array.

	@param array The array to be modified in place.
	@param n The number of elements to be added. Must match with the actual
		number of elements in the elements array.
	@param elements A list of n elements to be added. NULL elements are allowed.
		The element is copied into the array with the clone() method given in
		the type field at Array_new().
	@return The modified array, allowing the chaining of function calls.
	@pre array != NULL
	@pre n > 0
	@pre elements != NULL
*/
WArray*
Array_prepend_n( WArray* array, size_t n, void* const elements[n] );

/*	Set or update one or several elements in the array.

	@param array The array to be modified in place.
	@param position May be greater than the current size. A possible gap
		between the last current and the first new element is filled with
		NULL elements.
	@param n The number of elements to be added. Must match with the actual
		number of elements in the elements array.
	@param elements A list of n elements to be added. NULL elements are allowed.
		The element is copied into the array with the clone() method given in
		the type field at Array_new().
	@return The modified array, allowing the chaining of function calls.
	@pre array != NULL
	@pre n > 0
	@pre elements != NULL
*/
WArray*
Array_set_n( WArray* array, size_t position, size_t n, void* const elements[n] );

/**	Insert one or several elements to the array.

	@param array The array to be modified in place.
	@param position May be greater than the current size. A possible gap
		between the last current and the first new element is filled with
		NULL elements.
	@param n The number of elements to be added. Must match with the actual
		number of elements in the elements array.
	@param elements A list of n elements to be added. NULL elements are allowed.
		The element is copied into the array with the clone() method given in
		the type field at Array_new().
	@return The modified array, allowing the chaining of function calls.
	@pre array != NULL
	@pre n > 0
	@pre elements != NULL
*/
WArray*
Array_insert_n( WArray* array, size_t position, size_t n, void* const elements[n] );

//------------------------------------------------------------
//	Read and delete elements from the array.
//------------------------------------------------------------

/**	Return the element at the given position.

	@pre array != NULL
	@pre position < array->size
*/
const void*
Array_at( const WArray* array, size_t position );

/**	Return the first element.

	@pre array != NULL
	@pre Array_nonEmpty( array )
*/
const void*
Array_first( const WArray* array );

/**	Return the last element.

	@pre array != NULL
	@pre Array_nonEmpty( array )
*/
const void*
Array_last( const WArray* array );

/**	Return a random element.

	@pre array != NULL
	@pre Array_nonEmpty( array )
*/
const void*
Array_sample( const WArray* array );

/*	Clone the element at the given position.
*/
void*
Array_cloneAt( const WArray* array, size_t position );

/*	Clone the first element.
*/
void*
Array_cloneFirst( const WArray* array );

/*	Clone the last element.
*/
void*
Array_cloneLast( const WArray* array );

/**	Remove the element at the array position and return it.

	@param array
	@param position
	@return The element as it was copied into the array with the given ElementClone* function.
	@pre array != NULL
	@pre position < array->size
*/
void*
Array_stealAt( WArray* array, size_t position );

/**	Remove the first element and return it.

	@param array
	@return The element as it was copied into the array with the given ElementClone* function.
	@pre array != NULL
	@pre Array_nonEmpty( array )
*/
void*
Array_stealFirst( WArray* array );

/**	Remove the last element and return it.

	@param array
	@return The element as it was copied into the array with the given ElementClone* function.
	@pre array != NULL
	@pre Array_nonEmpty( array )
*/
void*
Array_stealLast( WArray* array );

/**	Remove a random element and return it.

	@param array
	@return The element as it was copied into the array with the given ElementClone* function.
	@pre array != NULL
	@pre Array_nonEmpty( array )
*/
void*
Array_stealSample( WArray* array );

/**	Remove and delete the element at the given position.

	@param array
	@param position
	@return The modified array
	@pre array != NULL
	@pre position < Array_size( array )
*/
WArray*
Array_removeAt( WArray* array, size_t position );

/**	Remove and delete the first element.

	@param array
	@return The modified array
	@pre array != NULL
	@pre Array_nonEmpty( array )
*/
WArray*
Array_removeFirst( WArray* array );

/**	Remove and delete the last element.

	@param array
	@return The modified array
	@pre array != NULL
	@pre Array_nonEmpty( array )
*/
WArray*
Array_removeLast( WArray* array );

/*	Return several cloned elements at the given position.

	@param array
	@param start
	@param end
	@pre array != NULL
	@pre start <= end
	@pre end < array->size
	@post returnValue != NULL
	@post returnValue->size == end-start+1
*/
WArray*
Array_slice( const WArray* array, size_t start, size_t end );

//------------------------------------------------------------
//	Search the array
//------------------------------------------------------------

/**	Return the minimum element.

	@param array
	@return The minimum element
	@pre array != NULL
	@pre Array_nonEmpty( array )
	@pre array->type->compare != NULL
*/
const void*
Array_min( const WArray* array );

/**	Return the maximum element.

	@param array
	@return The maximum element
	@pre array != NULL
	@pre Array_nonEmpty( array )
	@pre array->type->compare != NULL
*/
const void*
Array_max( const WArray* array );

/**	Search the array for the given element. The given element is compared
	with the array elements using the array compare() method.

	@param array
	@param element
	@return The position of the found element or -1 if no element was found.
	@pre array != NULL
	@pre array->type->compare != NULL
*/
ssize_t
Array_index( const WArray* array, const void* element );

/**	Search the array backwards for the given element. The given element is compared
	with the array elements using the array compare() method.

	@param array
	@param element
	@return The position of the found element or -1 if no element was found.
	@pre array != NULL
	@pre array->type->compare != NULL
*/
ssize_t
Array_rindex( const WArray* array, const void* element );

/**	Search an element in a sorted array.

	@param array
	@param compare Function comparing the the key (1st argument) and the current
		element in the array (2nd argument)
	@param key The key being passed to the compare function
	@return The position of the found element or -1 if no element was found.
	@pre array != NULL
	@pre compare != NULL
*/
ssize_t
Array_bsearch( const WArray* array, ElementCompare* compare, const void* key );

/**	Search the array for the given element. The given element is compared
	with the array elements using the array compare() method.

	@param array
	@param element
	@return true if the element was found, false otherwise
	@pre array != NULL
	@pre array->type->compare != NULL
*/
static inline bool
Array_contains( const WArray* array, const void* element ) { return Array_index( array, element ) >= 0; }

/**	Count how many elements meet a condition.

	@param array
	@param condition Function checking if the elements meet the condition
	@param conditionData Optional data passed to the condition function
	@return The count how many elements meet the condition
	@pre array != NULL
	@pre condition != NULL
*/
size_t
Array_count( const WArray* array, ElementCondition* condition, const void* conditionData );

//------------------------------------------------------------
//	Comparing arrays
//------------------------------------------------------------

/**	Compare two arrays.

	Compare the elements pairwise until one element is less than the other, as compared with the
	array->type->compare method. If all elements are equal the smaller array is seen as less than
	the other.

	@param array1
	@param array2
	@return The result of the comparison like in strcmp(): < 0, == 0, > 0
	@pre array1 =! NULL
	@pre array2 =! NULL
	@pre array1->type == array2->type
	@pre array1->type->compare != NULL
*/
int
Array_compare( const WArray* array1, const WArray* array2 );

/**	Check if two arrays are equal.

	Compare the elements pairwise until one element is less than the other, as compared with the
	array->type->compare method. If all elements are equal and the size is equal, they are considered
	equal.

	@param array1
	@param array2
	@return true if all elements are equal.
	@pre array1 =! NULL
	@pre array2 =! NULL
	@pre array1->type == array2->type
	@pre array1->type->compare != NULL
*/
static inline bool
Array_equal( const WArray* array1, const WArray* array2 ){ return Array_compare( array1, array2 ) == 0; }

//------------------------------------------------------------
//	Array --> string --> Array
//------------------------------------------------------------

/**	Join the elements to a string.

	Each element is stringified with the array->toString() method given at Array_new().

	@param array
	@param delimiters Separator string inserted between the strings of two elements
	@return Allocated string, either of stringified elements or "", if the array is empty
	@pre array != NULL
	@pre delimiters != NULL
	@pre array->type->toString != NULL
*/
char*
Array_toString( const WArray* array, const char delimiters[] );

/**	Split the string in elements separated by one of the given delimiters and create an array
	with the string elements.

	@param string
	@param delimiters
	@return
	@pre string != NULL
	@pre delimiters != NULL and delimiters[0] != 0
*/
WArray*
Array_fromString( const char string[], const char delimiters[] );

//------------------------------------------------------------
//	Query basic array data.
//------------------------------------------------------------

/**	Return the size of the array.

	@param array
	@return size
*/
inline static size_t
Array_size( const WArray* array ) { return array->size; }

/**	Return true if the array has no elements.

	@param array (const *Array)
	@return (bool)
*/
inline static bool
Array_empty( const WArray* array ) { return array->size == 0; }

/**	Return true if the array has elements.

	@param array (const *Array)
	@return (bool)
*/
inline static bool
Array_nonEmpty( const WArray* array ) { return array->size > 0; }

//------------------------------------------------------------
//	Functions iterating the array elements
//------------------------------------------------------------

/**	Apply a read-only function to all elements.

	@param array
	@param foreach The function to be applied to each element
	@param foreachData Optional data to be passed to the foreach function.
	@pre array != NULL
	@pre foreach != NULL
*/
void
Array_foreach( const WArray* array, ElementForeach* foreach, void* foreachData  );

/*	Apply a read-only function to all elements and their indexes.

	@pre array != NULL
	@pre foreach != NULL
*/
void
Array_foreachIndex( const WArray* array, ElementForeachIndex* foreach, void* foreachData );

/**	Take all elements meeting a filter criterion and put them in a new array.

	@pre array != NULL
	@pre filter != NULL
*/
WArray*
Array_filter( const WArray* array, ElementFilter* filter, const void* filterData );

/**	Reject all elements meeting a criterion and put the rest in a new array.

	@pre array != NULL
	@pre reject != NULL
*/
WArray*
Array_reject( const WArray* array, ElementFilter* reject, const void* rejectData );

/**	Map each element to a new element of a specified type and put them all in a new array.

	@param array The array serving as base for the mapped array. Left untouched.
	@param map A function mapping the current element plus the optional mapData argument
		to a new element of the mapped array. The functions's return value must be of the
		type specified by the type argument. It also must be allocated the same way the
		type->clone() method would do it.
	@param mapData Optional argument passed to the map function. Can be set to NULL if
		not used.
	@param type Structure describing the type of the mapped elements. If type == NULL, it
		is set by default to elementPtr.
	@return A new array of the same size as the given array with the mapped elements and
		of the type as specified in the type argument.
	@pre array != NULL
	@pre map != NULL
	@post returnValue != NULL
	@post returnValue->size == array->size
	@post If the given type != NULL, then returnValue->type == type

	Example:
	\code
	//This map function takes a char* element, converts it to int and adds the mapData
	//argument. Error handling for strtol() is omitted for clarity.
	void* toIntPlusX( const void* element, const void* mapData )
	{
        const char* string = element;
        int number = (int)mapData;
		return strtol( string, NULL, 0 ) + number;
	}

	...

	//Make an array with stringified numbers.
	Array* array = Array_new( 0, elementStr );
	Array_append_n( array, 3, "237", "11", "-42" );

	//Map it to an array with numbers. ->238, 12, -41
	Array* numbers = Array_map( array, toIntPlusX, (void*)1, elementInt );
	\endcode
*/
WArray*
Array_map( const WArray* array, ElementMap* map, const void* mapData, const ElementType* type );

/**	Reduce all elements to a single return value of an arbitrary type.

	@param array
	@param reduce
	@param startValue
	@param type
	@return
	@pre array != NULL
	@pre reduce != NULL
*/
void*
Array_reduce( const WArray* array, ElementReduce* reduce, const void* startValue, const ElementType* type );

/**	Keep all elements meeting a filter criterion and delete the rest.

	@param array
	@param filter
	@param filterData
	@return
	@pre array != NULL
	@pre filter != NULL
*/
WArray*
Array_select( WArray* array, ElementFilter* filter, const void* filterData );

/**	Delete all elements meeting a filter criterion and keep the rest.

	@param array
	@param filter
	@param filterData
	@return
	@pre array != NULL
	@pre filter != NULL
*/
WArray*
Array_unselect( WArray* array, ElementFilter* filter, const void* filterData );

//------------------------------------------------------------
//	Do stuff with the elements.
//------------------------------------------------------------

/**	Reverse the element order of the array.

	@param array
	@return The modified array
	@pre array != NULL
*/
WArray*
Array_reverse( WArray* array );

/*	Return the array with the elements in random order.

	@pre array != NULL
*/
WArray*
Array_shuffle( WArray* array );

/**	Remove all NULL elements.

	@param array
	@return The modified array
	@pre array != NULL
*/
WArray*
Array_compact( WArray* array );

/**	Sort the array elements in place using qsort() with the array comparison function.

	@param array
	@return The sorted array
	@pre array != NULL
	@pre array->type->compare != NULL
*/
WArray*
Array_sort( WArray* array );

/**	Sort the array elements in place using qsort() with the given comparison function.

	@param array
	@param compare
	@return The sorted array
	@pre array != NULL
	@pre compare != NULL
*/
WArray*
Array_sortBy( WArray* array, ElementCompare* compare );

/** Remove all identical elements using the comparison function from the
	element type. It is undefined which of the duplicate elements remains afterwards.
	If the array contains several NULL elements they get reduced to one NULL value too.

	@param array
	@return The array without duplicate elements
	@pre array != NULL
	@pre array->type->compare != NULL
*/
WArray*
Array_distinct( WArray* array );

/**	Append the elements of an array to another array.

    @param array1 The array being modified
    @param array2 The array that gets appended to array1
    @return The concatenated array
	@pre array1 != NULL
	@pre array2 != NULL
	@pre array1->type == array2->type
*/
WArray*
Array_concat( WArray* array1, const WArray* array2 );

//------------------------------------------------------------
//	Check properties of the elements
//------------------------------------------------------------

/**	Return true if all elements meet a condition.

	@param array
	@param condition
	@param conditionData
	@return true if the array is empty or all elements meet the condition
	@pre array != NULL
	@pre condition != NULL
*/
bool
Array_all( const WArray* array, ElementCondition* condition, const void* conditionData );

/**	Return true if at least one element meets a condition.

	@param array
	@param condition
	@param conditionData
	@pre array != NULL
	@pre condition != NULL
*/
bool
Array_any( const WArray* array, ElementCondition* condition, const void* conditionData );

/**	Return true if no element meets a condition.

	@param array
	@param condition
	@param conditionData
	@pre array != NULL
	@pre condition != NULL
*/
bool
Array_none( const WArray* array, ElementCondition* condition, const void* conditionData );

/**	Return true if exactly one element meets a condition.

	@param array
	@param condition
	@param conditionData
	@pre array != NULL
	@pre condition != NULL
*/
bool
Array_one( const WArray* array, ElementCondition* condition, const void* conditionData );

//------------------------------------------------------------
//	Common set operations
//------------------------------------------------------------

/**	Return an array with elements that are in one or both of the given arrays without duplicates.

	@param array1
	@param array2
	@pre array1 != NULL
	@pre array2 != NULL
	@pre array1->type == array2->type
	@pre array1->type->compare != NULL
	@pre array2->type->compare != NULL
*/
WArray*
Array_unite( const WArray* array1, const WArray* array2 );

/**	Return an array with elements that are both in array1 and array2 without duplicates.

	@param array1
	@param array2
	@pre array1 != NULL
	@pre array2 != NULL
	@pre array1->type == array2->type
	@pre array1->type->compare != NULL
	@pre array2->type->compare != NULL
*/
WArray*
Array_intersect( const WArray* array1, const WArray* array2 );

/**	Return an array with elements that are either in array1 or array2 without duplicates.

	The result contains only unique elements.

	@param array1
	@param array2
	@pre array1 != NULL
	@pre array2 != NULL
	@pre array1->type == array2->type
	@pre array1->type->compare != NULL
	@pre array2->type->compare != NULL
*/
WArray*
Array_symDiff( const WArray* array1, const WArray* array2 );

WArray*
Array_diff( const WArray* array1, const WArray* array2 );

/**	Append the element to the array, if it is not there already.

	@param array
	@param element
	@return
	@pre array != NULL
	@pre array->type->compare != NULL
*/
WArray*
Array_addToSet( WArray* array, const void* element );

#if 0
//------------------------------------------------------------
//	TODO: Extract statistical functions in separate module
//------------------------------------------------------------

/*	Calculate the minimum element.
*/
double
Array_dmin( const WArray* array );

/*	Calculate the maximum element.
*/
double
Array_dmax( const WArray* array );

/*	Calculate the sum of all elements.
*/
double
Array_dsum( const WArray* array );

/*	Calculate the mean of the elements.
*/
double
Array_dmean( const WArray* array );

/*	Calculate the median.
*/
double
Array_dmedian( const WArray* array );

double
Array_dvariance( const WArray* array );

double
Array_dcovariance( const WArray* array );

/*	Calculate the standard deviation of the elements.
*/
double
Array_ddeviation( const WArray* array );

/*	Return the nth percentile.
*/
double
Array_dpercentile( const WArray* array, unsigned n );
#endif // 0

//------------------------------------------------------------
//	Iterator functions
//------------------------------------------------------------

/** Return an iterator capable of traversing the array in normal direction.

	@param array
	@return
*/
Iterator*
Array_iterator( const WArray* array );

/** Return an iterator capable of traversing the array in reverse direction.

	@param array
	@return
*/
Iterator*
Array_iteratorReverse( const WArray* array );

//------------------------------------------------------------

void
Array_print( const WArray* array );

//------------------------------------------------------------

#endif
