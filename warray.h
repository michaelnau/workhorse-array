/**@file
*/
//TODO: Extract main documentation in separate file.
#ifndef WARRAY_H_INCLUDED
#define WARRAY_H_INCLUDED

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

#include "wcollection.h"
#include <stdbool.h>			//bool
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
	#include <sys/types.h>		//ssize_t on POSIX systems
#else
	#define ssize_t intptr_t	//intptr_t as fallback for other C99 systems
#endif

//------------------------------------------------------------
//	Types and constants
//------------------------------------------------------------

/**	The array type. Access it only through the warray_xyz() functions except
	reading the explicitly public fields.
*/
typedef struct WArray {
	size_t			size;			///<Public read-only, the actual number of elements
	size_t			capacity;		///<Public read-only, the maximum number of elements before the array must grow
	const WType*	type;			//Private, do not directly access it. Pointer to the element methods
	void**			data;			//Private, do not directly access it.
}WArray;

/** Pointer to a struct describing methods for elements that are arrays themselves.
	Can be passed to warray_new().

	- clone = warray_clone,
	- delete = warray_delete,
	- compare = warray_compare,
	- fromString = warray_fromString,
	- toString = warray_toString
*/
extern const WType* wtypeArray;

//------------------------------------------------------------
//	Create and destroy an array.
//------------------------------------------------------------

/**	Create a new empty array with the appropriate methods for the element type.

	@param capacity The initial element capacity. If 0 is given, the initial capacity is set to 100.
	@param type Pointer to a structure with element methods to be used in array functions.
		If NULL is passed, the elements are treated as raw pointers and the methods set in
		\ref wtypePtr are used. If a type is given, but the compare, fromString or toString
		fields are NULL, then the WArray functions using these methods may not be used. The array only
		keeps the type pointer, so the underlying struct must be permanently accessible.
	@return The new array
	@pre If type is given, type->clone may not be NULL.
	@pre If type is given, type->delete may not be NULL.

	Examples:
	\code
	//An array is created treating its elements as raw void* pointers. No extra memory is
	//allocated for the element itself, only for the pointer. The element does not get
	//deallocated when it is removed from the array.
	WArray* a1 = warray_new(		//Create an array
		0,							//with the default capacity (100)
		NULL						//and the default element type (wtypePtr, i.e. raw void*)
	);
	WArray* a2 = warray_new(
		10,							//capacity = 10 elements
		NULL						//type = wtypePtr
	);

	//An array is created treating its elements as char* strings. The elements get copied into
	//the array with strdup(). If the elements are removed, they get freed.
	WArray* a3 = warray_new(
		10,
		wtypeStr					//type = wtypeStr, treat the elements as char* strings.
	);

	//An array is created treating its elements as integer values. The integers are directly
	//stored in the element pointers. The elements can be numerically compared or sorted.
	WArray* a4 = warray_new(
		0,
		wtypeInt					//type = wtypeInt, treat the elements as int values
	);

	//An array is created treating its elements as custom carType object. A clone and deletion
	//function are passed. With them the array can make proper copies and deallocations.
	static const WType carType = {
        .clone = cloneCar,
        .delete = deleteCar
	};
	WArray* a5 = warray_new(
		0,
		&carType
	);
	\endcode
*/
WArray*
warray_new( size_t capacity, const WType* type );

/**	Clone the given array by cloning the elements with the array->type->clone() method.

	@param array
	@return The new array
	@pre array != NULL
*/
WArray*
warray_clone( const WArray* array );

/**	Delete the array and all elements.

	If NULL is passed, this is a no-op.

	@param array Pointer to an array. After the deletion the pointer is set to NULL.
	@return
*/
void
warray_delete( WArray** array );

/**	Delete all elements but leave the array itself intact.

	If NULL is passed, this is a no-op.

	@param array
*/
WArray*
warray_clear( WArray* array );

/**	Delete an array and assign another array to its pointer.

	If NULL is passed, this is a no-op.

	@param array. After the call array points to other.
	@param other
	@pre array->type == other->type
*/
void
warray_assign( WArray** array, WArray *other );

//------------------------------------------------------------
//	Put one element in the array.
//------------------------------------------------------------

/**	Append an element to the array.

	@param array The array to be modified in place.
	@param element The element to be added. NULL elements are allowed. The
		element is copied into the array with the clone() method given in
		the type field at warray_new().
	@return The modified array, allowing the chaining of function calls.
	@pre array != NULL
*/
WArray*
warray_append( WArray* array, const void* element );

/**	Prepend an element to the array.

	@param array The array to be modified in place.
	@param element The element to be added. NULL elements are allowed. The
		element is copied into the array with the clone() method given in
		the type field at warray_new().
	@return The modified array, allowing the chaining of function calls.
	@pre array != NULL
*/
WArray*
warray_prepend( WArray* array, const void* element );

/**	Set or update the element at the given position.

	@param array The array to be modified in place.
	@param position May be greater than the current size. A possible gap
		between the last current and the new element is filled with
		NULL elements.
	@param element The element to be added. NULL elements are allowed. The
		element is copied into the array with the clone() method given in
		the type field at warray_new().
	@return The modified array, allowing the chaining of function calls.
	@pre array != NULL
*/
WArray*
warray_set( WArray* array, size_t position, const void* element );

/**	Insert an element into the array.

	@param array The array to be modified in place.
	@param position May be greater than the current size. A possible gap
		between the last current and the new element is filled with
		NULL elements.
	@param element The element to be added. NULL elements are allowed. The
		element is copied into the array with the clone() method given in
		the type field at warray_new().
	@return The modified array, allowing the chaining of function calls.
	@pre array != NULL
*/
WArray*
warray_insert( WArray* array, size_t position, const void* element );

/**	Insert an element into the array, so that it keeps the ascending element order
	according to the array->type->compare() method.

	@param array The array to be modified in place.
	@param element The element to be added. NULL elements are allowed. The
		element is copied into the array with the clone() method given in
		the type field at warray_new().
	@return The modified array, allowing the chaining of function calls.
	@pre array != NULL
	@pre array->type->compare != NULL
*/
WArray*
warray_insertSorted( WArray* array, const void* element );

//------------------------------------------------------------
//	Put several elements in the array.
//------------------------------------------------------------

/**	Append one or several elements to the array.

	@param array The array to be modified in place.
	@param n The number of elements to be added. Must match with the actual
		number of elements in the elements array.
	@param elements A list of n elements to be added. NULL elements are allowed.
		The element is copied into the array with the clone() method given in
		the type field at warray_new().
	@return The modified array, allowing the chaining of function calls.
	@pre array != NULL
	@pre n > 0
	@pre elements != NULL

*/
WArray*
warray_append_n( WArray* array, size_t n, void* const elements[n] );

/**	Prepend one or several elements to the array.

	@param array The array to be modified in place.
	@param n The number of elements to be added. Must match with the actual
		number of elements in the elements array.
	@param elements A list of n elements to be added. NULL elements are allowed.
		The element is copied into the array with the clone() method given in
		the type field at warray_new().
	@return The modified array, allowing the chaining of function calls.
	@pre array != NULL
	@pre n > 0
	@pre elements != NULL
*/
WArray*
warray_prepend_n( WArray* array, size_t n, void* const elements[n] );

/**	Set or update one or several elements in the array.

	@param array The array to be modified in place.
	@param position May be greater than the current size. A possible gap
		between the last current and the first new element is filled with
		NULL elements.
	@param n The number of elements to be added. Must match with the actual
		number of elements in the elements array.
	@param elements A list of n elements to be added. NULL elements are allowed.
		The element is copied into the array with the clone() method given in
		the type field at warray_new().
	@return The modified array, allowing the chaining of function calls.
	@pre array != NULL
	@pre n > 0
	@pre elements != NULL
*/
WArray*
warray_set_n( WArray* array, size_t position, size_t n, void* const elements[n] );

/**	Insert one or several elements to the array.

	@param array The array to be modified in place.
	@param position May be greater than the current size. A possible gap
		between the last current and the first new element is filled with
		NULL elements.
	@param n The number of elements to be added. Must match with the actual
		number of elements in the elements array.
	@param elements A list of n elements to be added. NULL elements are allowed.
		The element is copied into the array with the clone() method given in
		the type field at warray_new().
	@return The modified array, allowing the chaining of function calls.
	@pre array != NULL
	@pre n > 0
	@pre elements != NULL
*/
WArray*
warray_insert_n( WArray* array, size_t position, size_t n, void* const elements[n] );

/**	Append the elements of an array to another array.

    @param array1 The array being modified
    @param array2 The array that gets appended to array1
    @return The concatenated array
	@pre array1 != NULL
	@pre array2 != NULL
	@pre array1->type == array2->type
*/
WArray*
warray_concat( WArray* array1, const WArray* array2 );

//------------------------------------------------------------
//	Read and delete elements from the array.
//------------------------------------------------------------

/**	Return the element at the given position.

	@pre array != NULL
	@pre position < array->size
*/
const void*
warray_at( const WArray* array, size_t position );

/**	Return the first element.

	@pre array != NULL
	@pre warray_nonEmpty( array )
*/
const void*
warray_first( const WArray* array );

/**	Return the last element.

	@pre array != NULL
	@pre warray_nonEmpty( array )
*/
const void*
warray_last( const WArray* array );

/**	Return a random element.

	@pre array != NULL
	@pre warray_nonEmpty( array )
*/
const void*
warray_sample( const WArray* array );

/**	Clone the element at the given position.

	@param array
	@param position
	@return The element as it was copied into the array with the given ElementClone* function.
		The calling code must ensure to deallocate it properly if needed.
*/
void*
warray_cloneAt( const WArray* array, size_t position );

/**	Clone the first element.

	@param array
	@return The element as it was copied into the array with the given ElementClone* function.
		The calling code must ensure to deallocate it properly if needed.
*/
void*
warray_cloneFirst( const WArray* array );

/**	Clone the last element.

	@param array
	@return The element as it was copied into the array with the given ElementClone* function.
		The calling code must ensure to deallocate it properly if needed.
*/
void*
warray_cloneLast( const WArray* array );

/**	Remove the element at the array position and return it.

	The order of the other elements remains intact.

	@param array
	@param position
	@return The element as it was copied into the array with the given ElementClone* function.
		The calling code must ensure to deallocate it properly if needed.
	@pre array != NULL
	@pre position < array->size
*/
void*
warray_stealAt( WArray* array, size_t position );

/**	Remove the first element and return it.

	The order of the other elements remains intact.

	@param array
	@return The element as it was copied into the array with the given ElementClone* function.
		The calling code must ensure to deallocate it properly if needed.
	@pre array != NULL
	@pre warray_nonEmpty( array )
*/
void*
warray_stealFirst( WArray* array );

/**	Remove the last element and return it.

	The order of the other elements remains intact.

	@param array
	@return The element as it was copied into the array with the given ElementClone* function.
		The calling code must ensure to deallocate it properly if needed.
	@pre array != NULL
	@pre warray_nonEmpty( array )
*/
void*
warray_stealLast( WArray* array );

/**	Remove a random element and return it.

	The order of the other elements remains intact.

	@param array
	@return The element as it was copied into the array with the given ElementClone* function.
		The calling code must ensure to deallocate it properly if needed.
	@pre array != NULL
	@pre warray_nonEmpty( array )
*/
void*
warray_stealSample( WArray* array );

/**	Remove and delete the element at the given position.

	The order of the other elements remains intact.

	@param array
	@param position
	@return The modified array
	@pre array != NULL
	@pre position < warray_size( array )
*/
WArray*
warray_removeAt( WArray* array, size_t position );

/**	Remove and delete the first element.

	The order of the other elements remains intact.

	@param array
	@return The modified array
	@pre array != NULL
	@pre warray_nonEmpty( array )
*/
WArray*
warray_removeFirst( WArray* array );

/**	Remove and delete the last element.

	The order of the other elements remains intact.

	@param array
	@return The modified array
	@pre array != NULL
	@pre warray_nonEmpty( array )
*/
WArray*
warray_removeLast( WArray* array );

/**	Return several cloned elements at the given position.

	@param array
	@param start
	@param end
	@return
	@pre array != NULL
	@pre start <= end
	@pre end < array->size
	@post returnValue != NULL
	@post returnValue->size == end-start+1
*/
WArray*
warray_slice( const WArray* array, size_t start, size_t end );

//------------------------------------------------------------
//	Search the array
//------------------------------------------------------------

/**	Return the minimum element.

	@param array
	@return The minimum element
	@pre array != NULL
	@pre warray_nonEmpty( array )
	@pre array->type->compare != NULL
*/
const void*
warray_min( const WArray* array );

/**	Return the maximum element.

	@param array
	@return The maximum element
	@pre array != NULL
	@pre warray_nonEmpty( array )
	@pre array->type->compare != NULL
*/
const void*
warray_max( const WArray* array );

/**	Search the array for the given element. The given element is compared
	with the array elements using the array compare() method.

	@param array
	@param element
	@return The position of the found element or -1 if no element was found.
	@pre array != NULL
	@pre array->type->compare != NULL
*/
ssize_t
warray_index( const WArray* array, const void* element );

/**	Search the array backwards for the given element. The given element is compared
	with the array elements using the array compare() method.

	@param array
	@param element
	@return The position of the found element or -1 if no element was found.
	@pre array != NULL
	@pre array->type->compare != NULL
*/
ssize_t
warray_rindex( const WArray* array, const void* element );

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
warray_bsearch( const WArray* array, WElementCompare* compare, const void* key );

/**	Search the array for the given element. The given element is compared
	with the array elements using the array compare() method.

	@param array
	@param element
	@return true if the element was found, false otherwise
	@pre array != NULL
	@pre array->type->compare != NULL
*/
static inline bool
warray_contains( const WArray* array, const void* element ) { return warray_index( array, element ) >= 0; }

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
warray_compare( const WArray* array1, const WArray* array2 );

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
warray_equal( const WArray* array1, const WArray* array2 ){ return warray_compare( array1, array2 ) == 0; }

//------------------------------------------------------------
//	WArray --> string --> WArray
//------------------------------------------------------------

/**	Join the elements to a string.

	Each element is stringified with the array->toString() method given at warray_new().

	@param array
	@param delimiters Separator string inserted between the strings of two elements
	@return Allocated string, either of stringified elements or "", if the array is empty
	@pre array != NULL
	@pre delimiters != NULL
	@pre array->type->toString != NULL
*/
char*
warray_toString( const WArray* array, const char delimiters[] );

//TODO: Pass a WType to warray_fromString() to create an array with real elements
/**	Split the string in elements separated by one of the given delimiters and create an array
	with the string elements.

	@param string
	@param delimiters
	@return array of type wtypeStr
	@pre string != NULL
	@pre delimiters != NULL and delimiters[0] != 0
*/
WArray*
warray_fromString( const char string[], const char delimiters[] );

//------------------------------------------------------------
//	Query basic array data.
//------------------------------------------------------------

/**	Return the size of the array.

	@param array
	@return size
*/
inline static size_t
warray_size( const WArray* array ) { return array->size; }

/**	Return true if the array has no elements.

	@param array (const *WArray)
	@return (bool)
*/
inline static bool
warray_empty( const WArray* array ) { return array->size == 0; }

/**	Return true if the array has elements.

	@param array (const *WArray)
	@return (bool)
*/
inline static bool
warray_nonEmpty( const WArray* array ) { return array->size > 0; }

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
warray_foreach( const WArray* array, WElementForeach* foreach, void* foreachData  );

/**	Apply a read-only function to all elements and their indexes.

	@pre array != NULL
	@pre foreach != NULL
*/
void
warray_foreachIndex( const WArray* array, WElementForeachIndex* foreach, void* foreachData );

/**	Take all elements meeting a filter criterion and put them in a new array.

	Function iterating through all array elements and calling a client condition function. If
	the condition is true the current element is added to the output array.

	@param array
	@param condition Function taking the current element in the iteration and the optional
		conditionData argument. If it returns true the element is added to the output array,
		otherwise not.
	@param conditionData Passed to the condition function. May be NULL.
	@return The output array. Is never NULL.
	@pre array != NULL
	@pre condition != NULL
*/
WArray*
warray_filter( const WArray* array, WElementCondition* condition, const void* conditionData );

/**	Reject all elements meeting a criterion and put the rest in a new array.

	@param array
	@param condition Function taking the current element in the iteration and the optional
		conditionData argument. If it returns false the element is added to the output array,
		otherwise not.
	@param conditionData Passed to the condition function. May be NULL.
	@return The output array. Is never NULL.
	@pre array != NULL
	@pre reject != NULL
*/
WArray*
warray_reject( const WArray* array, WElementCondition* condition, const void* conditionData );

/**	Keep all elements meeting a filter criterion and delete the rest.

	@param array
	@param condition
	@param conditionData
	@return The array. Is never NULL.
	@pre array != NULL
	@pre filter != NULL
*/
WArray*
warray_select( WArray* array, WElementCondition* condition, const void* conditionData );

/**	Delete all elements meeting a filter criterion and keep the rest.

	@param array
	@param condition
	@param conditionData
	@return The array. Is never NULL.
	@pre array != NULL
	@pre filter != NULL
*/
WArray*
warray_unselect( WArray* array, WElementCondition* condition, const void* conditionData );

/**	Map each element to a new element of a specified type and put them all in a new array.

	@param array The array serving as base for the mapped array. Left untouched.
	@param map A function mapping the current element plus the optional mapData argument
		to a new element of the mapped array. The functions's return value must be of the
		type specified by the type argument. It also must be allocated the same way the
		type->clone() method would do it.
	@param mapData Optional argument passed to the map function. Can be set to NULL if
		not used.
	@param targetType Structure describing the type of the mapped elements. If type == NULL, it
		is set by default to wtypePtr.
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
	WArray* array = warray_new( 0, wtypeStr );
	warray_append_n( array, 3, "237", "11", "-42" );

	//Map it to an array with numbers. ->238, 12, -41
	WArray* numbers = warray_map( array, toIntPlusX, (void*)1, wtypeInt );
	\endcode
*/
WArray*
warray_map( const WArray* array, WElementMap* map, const void* mapData, const WType* targetType );

/**	Reduce all elements to a single return value of an arbitrary type.

	@param array
	@param reduce
	@param startValue
	@param targetType
	@return
	@pre array != NULL
	@pre reduce != NULL
*/
void*
warray_reduce( const WArray* array, WElementReduce* reduce, const void* startValue, const WType* targetType );

//------------------------------------------------------------
//	Do stuff with the elements.
//------------------------------------------------------------

/**	Reverse the element order of the array.

	@param array
	@return The modified array
	@pre array != NULL
*/
WArray*
warray_reverse( WArray* array );

/**	Return the array with the elements in random order.

	@pre array != NULL
*/
WArray*
warray_shuffle( WArray* array );

/**	Remove all NULL elements.

	@param array
	@return The modified array
	@pre array != NULL
*/
WArray*
warray_compact( WArray* array );

/**	Sort the array elements in place using qsort() with the array comparison function.

	@param array
	@return The sorted array
	@pre array != NULL
	@pre array->type->compare != NULL
*/
WArray*
warray_sort( WArray* array );

/**	Sort the array elements in place using qsort() with the given comparison function.

	@param array
	@param compare
	@return The sorted array
	@pre array != NULL
	@pre compare != NULL
*/
WArray*
warray_sortBy( WArray* array, WElementCompare* compare );

/** Remove all identical elements using the comparison function from the
	element type. It is undefined which of the duplicate elements remains afterwards.
	If the array contains several NULL elements they get reduced to one NULL value too.

	@param array
	@return The array without duplicate elements
	@pre array != NULL
	@pre array->type->compare != NULL
*/
WArray*
warray_distinct( WArray* array );

//------------------------------------------------------------
//	Check properties of the elements
//------------------------------------------------------------

/**	Count how many elements meet a condition.

	@param array
	@param condition Function checking if the elements meet the condition
	@param conditionData Optional data passed to the condition function
	@return The count how many elements meet the condition
	@pre array != NULL
	@pre condition != NULL
*/
size_t
warray_count( const WArray* array, WElementCondition* condition, const void* conditionData );

/**	Return true if all elements meet a condition.

	@param array
	@param condition
	@param conditionData
	@return true if the array is empty or all elements meet the condition
	@pre array != NULL
	@pre condition != NULL
*/
bool
warray_all( const WArray* array, WElementCondition* condition, const void* conditionData );

/**	Return true if at least one element meets a condition.

	@param array
	@param condition
	@param conditionData
	@pre array != NULL
	@pre condition != NULL
*/
bool
warray_any( const WArray* array, WElementCondition* condition, const void* conditionData );

/**	Return true if no element meets a condition.

	@param array
	@param condition
	@param conditionData
	@pre array != NULL
	@pre condition != NULL
*/
bool
warray_none( const WArray* array, WElementCondition* condition, const void* conditionData );

/**	Return true if exactly one element meets a condition.

	@param array
	@param condition
	@param conditionData
	@pre array != NULL
	@pre condition != NULL
*/
bool
warray_one( const WArray* array, WElementCondition* condition, const void* conditionData );

//------------------------------------------------------------

#endif
