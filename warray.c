/*  Copyright (c) 2017 Michael Nau
    This file is part of Workhorse Array.

    Workhorse Array is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Workhorse Array is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Workhorse Array.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "warray.h"
#include <assert.h>	//assert
#include <iso646.h>	//and, or, not
#include <string.h>	//memmove, memset
#include <stdarg.h>	//va_list
#include <stdlib.h>	//free, rand, bsearch, qsort

//-------------------------------------------------------------------------------
//	Invariants check, performed after every public function
//-------------------------------------------------------------------------------

static WArray*
checkArray( const WArray* array ) {
	assert( array->capacity > 0 );
	assert( array->size <= array->capacity );
	assert( array->data );
	assert( array->type );
	assert( array->type->clone );
	assert( array->type->delete );

	return (WArray*)array;
}

//-------------------------------------------------------------------------------
//	Helpers
//-------------------------------------------------------------------------------

//Based on http://groups.google.com/group/comp.lang.c/msg/2ab1ecbb86646684 (Public Domain) via Stackoverflow.com
static char*
__wstr_sep_r( char string[], const char delimiters[], char** next )
{
	assert( string or next );
	assert( delimiters and delimiters[0] );

	if ( not string )
        string = *next;

    if ( not *string )
        return NULL;

    char* token = string;

    string = strstr( string, delimiters );
    if ( string ) {
		*string = '\0';
		*next = string + strlen( delimiters );
	}
	else {
		*next = "";
	}

	assert( token );
    return token;
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

const WType* wtypeArray = &(WType){
	.clone = (WElementClone*)warray_clone,
	.delete = (WElementDelete*)warray_delete,
	.compare = (WElementCompare*)warray_compare,
	.fromString = (WElementFromString*)warray_fromString,
	.toString = (WElementToString*)warray_toString
};

enum ArrayParameters {
	ArrayDefaultCapacity 	= 100,
	ArrayGrowthRate 		= 2,
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

WArray*
warray_new( size_t capacity, const WType* type )
{
	assert( not type or type->clone );
	assert( not type or type->delete );

	WArray* array = __wxnew( WArray,
		.capacity	= capacity ? capacity : ArrayDefaultCapacity,
		.type		= type ? type : wtypePtr,
	);
	array->data = __wxmalloc( array->capacity * sizeof(void*) );

	assert( array );
	return checkArray( array );
}

WArray*
warray_clone( const WArray* array )
{
	assert( array );

	WArray *copy = __wxnew( WArray,
		.size		= array->size,
		.capacity	= array->capacity,
		.data		= __wxmalloc( sizeof( void* ) * array->capacity ),
		.type		= array->type
	);

	for ( size_t i = 0; i < array->size; i++ ) {
		if ( array->data[i] )
			copy->data[i] = array->type->clone( array->data[i] );
		else
			copy->data[i] = NULL;
	}

	assert( copy );
	assert( not array->type->compare or warray_equal( array, copy ));
	return checkArray( copy );
}

void
warray_delete( WArray** arrayPtr )
{
	if ( not arrayPtr ) return;

	WArray* array = *arrayPtr;

	warray_clear( array );
	free( array->data );
	free( array );
	*arrayPtr = NULL;
}

WArray*
warray_clear( WArray* array )
{
	if ( not array ) return array;

	for ( size_t i = 0; i < array->size; i++ ) {
		assert( &array->data[i] );
		array->type->delete( &array->data[i] );
	}

	array->size = 0;

	assert( array );
	assert( warray_empty( array ));
	return checkArray( array );
}

void
warray_assign( WArray** arrayPointer, WArray* other )
{
	if ( not arrayPointer ) return;
	WArray* array = *arrayPointer;

	assert( array->type == other->type && "Can only assign arrays of the same type." );

	warray_delete( arrayPointer );
	*arrayPointer = other;

	checkArray( *arrayPointer );
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

//Resize the array if necessary, so that it can take the new size.
static void
resize( WArray* array, size_t newSize )
{
	if ( newSize <= array->capacity ) return;

	array->capacity = __wmax( newSize, array->capacity * ArrayGrowthRate );
	array->data = __wxrealloc( array->data, array->capacity * sizeof(void*));
	assert( array->capacity >= newSize );
	checkArray( array );
}

static WArray*
put( WArray* array, size_t position, const void* element )
{
	assert( array );
	assert( position < array->capacity );

	if ( element )
		array->data[position] = array->type->clone( element );
	else
		array->data[position] = NULL;
	array->size = __wmax( array->size+1, position+1 );

	assert( array );
	assert( array->size > position );
	assert( array->capacity > position );
	return checkArray( array );
}

WArray*
warray_append( WArray* array, const void* element )
{
	assert( array );

	resize( array, array->size+1 );

	return checkArray( put( array, array->size, element ));
}

WArray*
warray_prepend( WArray* array, const void* element )
{
	assert( array );

	resize( array, array->size+1 );
	memmove( &array->data[1], &array->data[0], array->size * sizeof(void*));

	return checkArray( put( array, 0, element ));
}

WArray*
warray_set( WArray* array, size_t position, const void* element )
{
	assert( array );

	resize( array, __wmax( array->size, position+1 ));

	if ( position < array->size ) {	//Delete the old element.
		assert( &array->data[position] );
		array->type->delete( &array->data[position] );
	}
	else {							//Fill the gap with zeroes.
		memset( &array->data[array->size], 0, (position-array->size) * sizeof(void*));
		array->size = position+1;
	}

	if ( element )
		array->data[position] = array->type->clone( element );
	else
		array->data[position] = NULL;

	assert( array );
	return checkArray( array );
}

WArray*
warray_insert( WArray* array, size_t position, const void* element )
{
	assert( array );

	resize( array, __wmax( array->size+1, position+1 ));

	if ( position < array->size )	//Make room for the new element.
		memmove( &array->data[position+1], &array->data[position], (array->size-position) * sizeof(void*));
	else							//Fill the gap with zeros.
		memset( &array->data[array->size], 0, (position-array->size) * sizeof(void*));

	return checkArray( put( array, position, element ));
}

WArray*
warray_insertSorted( WArray* array, const void* element )
{
	assert( array );
	assert( array->type->compare );

	WElementCompare* compare = array->type->compare;
	void** data = array->data;
    for ( size_t i = 0; i < array->size; i++ ) {
        if ( compare( element, data[i] ) < 0 )
			return warray_insert( array, i, element );
    }

	return warray_append( array, element );
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

WArray*
warray_append_n( WArray* array, size_t n, void* const elements[n] )
{
   	assert( array );
	assert( n > 0 );
	assert( elements );

	for ( size_t i = 0; i < n; i++ )
		warray_append( array, elements[i] );

	assert( array );
	return checkArray( array );
}

WArray*
warray_prepend_n( WArray* array, size_t n, void* const elements[n] )
{
   	assert( array );
	assert( n > 0 );
	assert( elements );

	assert( array );
	return warray_insert_n( array, 0, n, elements );
}

WArray*
warray_insert_n( WArray* array, size_t position, size_t n, void* const elements[n] )
{
   	assert( array );
	assert( n > 0 );
	assert( elements );

  	for ( size_t i = 0; i < n; i++ )
		warray_insert( array, position+i, elements[i] );

	assert( array );
	return checkArray( array );
}

WArray*
warray_set_n( WArray* array, size_t position, size_t n, void* const elements[n] )
{
   	assert( array );
	assert( n > 0 );
	assert( elements );

  	for ( size_t i = 0; i < n; i++ )
		warray_set( array, position+i, elements[i] );

	assert( array );
	return checkArray( array );
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

const void*
warray_at( const WArray* array, size_t index )
{
	assert( array );
	assert( index < array->size && "Array access out of bounds." );

	return array->data[index];
}

const void*
warray_first( const WArray* array )
{
	assert( array );
	assert( warray_nonEmpty( array ));

	return warray_at( array, 0 );
}

const void*
warray_last( const WArray* array )
{
	assert( array );
	assert( warray_nonEmpty( array ));

	return warray_at( array, array->size-1 );
}

const void*
warray_sample( const WArray* array )
{
	assert( array );
	assert( warray_nonEmpty( array ));

	return warray_at( array, rand() % array->size );
}

void*
warray_cloneAt( const WArray* array, size_t position )
{
	assert( array );
	assert( array->type );
	assert( array->type->clone );
	assert( position < array->size );

	if ( array->data[position] )
		return array->type->clone( array->data[position] );
	else
		return NULL;
}

void*
warray_cloneFirst( const WArray* array )
{
	assert( array );
	assert( warray_nonEmpty( array ));

	return warray_cloneAt( array, 0 );
}

void*
warray_cloneLast( const WArray* array )
{
	assert( array );
	assert( warray_nonEmpty( array ));

	return warray_cloneAt( array, array->size-1 );
}

WArray*
warray_slice( const WArray* array, size_t start, size_t end )
{
	assert( array );
	assert( start <= end );
	assert( end < array->size );

	size_t size = end-start+1;
    WArray* slice = warray_new( size, array->type );
	warray_append_n( slice, size, &array->data[start] );

	assert( slice );
	assert( slice->size == end-start+1 );
	return checkArray( slice );
}

void*
warray_stealAt( WArray* array, size_t position )
{
	assert( array );
	assert( position < array->size && "Array access out of bounds." );

	void* value = array->data[position];

	if ( position < array->size-1 )
		memmove( &array->data[position], &array->data[position+1], (array->size-position-1) * sizeof(void*));

	array->size--;

	return value;
}

void*
warray_stealFirst( WArray* array )
{
	assert( array );
	assert( warray_nonEmpty( array ));

	return warray_stealAt( array, 0 );
}

void*
warray_stealLast( WArray* array )
{
	assert( array );
	assert( warray_nonEmpty( array ));

	return warray_stealAt( array, array->size-1 );
}

void*
warray_stealSample( WArray* array )
{
	assert( array );
	assert( warray_nonEmpty( array ));

	return warray_stealAt( array, rand() % array->size );
}

WArray*
warray_removeAt( WArray* array, size_t position )
{
	assert( array );
	assert( position < warray_size( array ));

	assert( &array->data[position] );
	array->type->delete( &array->data[position] );

	if ( position < array->size-1 )
		memmove( &array->data[position], &array->data[position+1], (array->size-position-1) * sizeof( void* ));

	array->size--;

	assert( array );
	return checkArray( array );
}

WArray*
warray_removeFirst( WArray* array )
{
	assert( array );
	assert( warray_nonEmpty( array ));

	return warray_removeAt( array, 0 );
}

WArray*
warray_removeLast( WArray* array )
{
	assert( array );
	assert( warray_nonEmpty( array ));

	return warray_removeAt( array, array->size-1 );
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

void
warray_foreach( const WArray* array, WElementForeach* foreach, void* foreachData )
{
	assert( array );
	assert( foreach );

	for ( size_t i = 0; i < array->size; i++ )
		foreach( array->data[i], foreachData );
}

void
warray_foreachIndex( const WArray* array, WElementForeachIndex* foreach, void* foreachData )
{
	assert( array );
	assert( foreach );

	for ( size_t i = 0; i < array->size; i++ )
		foreach( array->data[i], i, foreachData );
}

WArray*
warray_filter( const WArray* array, WElementCondition* filter, const void* filterData )
{
	assert( array );
	assert( filter );

	WArray* newArray = (WArray*)warray_new( array->capacity, array->type );

    for ( size_t i = 0; i < array->size; i++ ) {
        if ( filter( array->data[i], filterData )) {
			if ( array->data[i] )
				newArray->data[newArray->size++] = array->type->clone( array->data[i] );
			else
				newArray->data[newArray->size++] = NULL;
		}
    }

	assert( newArray );
	assert( warray_size( newArray ) <= warray_size( array ));
	assert( warray_all( newArray, filter, filterData ));
	return checkArray( newArray );
}

WArray*
warray_reject( const WArray* array, WElementCondition* reject, const void* rejectData )
{
	assert( array );
	assert( reject );

	WArray* newArray = (WArray*)warray_new( array->capacity, array->type );

    for ( size_t i = 0; i < array->size; i++ ) {
        if ( not reject( array->data[i], rejectData )) {
			if ( array->data[i] )
				newArray->data[newArray->size++] = array->type->clone( array->data[i] );
			else
				newArray->data[newArray->size++] = NULL;
		}
    }

	assert( newArray );
	assert( warray_size( newArray ) <= warray_size( array ));
	assert( warray_none( newArray, reject, rejectData ));
	return checkArray( newArray );
}

WArray*
warray_select( WArray* array, WElementCondition* filter, const void* filterData )
{
	assert( array );
	assert( filter );

	size_t to = 0;
    for ( size_t from = 0; from < array->size; from++ ) {
        if ( filter( array->data[from], filterData ))
			array->data[to++] = array->data[from];
		else {
			assert( &array->data[from] );
			array->type->delete( &array->data[from] );
		}
    }

    array->size = to;

	assert( array );
	assert( warray_all( array, filter, filterData ));
	return checkArray( array );
}

WArray*
warray_unselect( WArray* array, WElementCondition* filter, const void* filterData )
{
	assert( array );
	assert( filter );

	size_t to = 0;
    for ( size_t from = 0; from < array->size; from++ ) {
        if ( !filter( array->data[from], filterData ))
			array->data[to++] = array->data[from];
		else {
			assert( &array->data[from] );
			array->type->delete( &array->data[from] );
		}
	}

    array->size = to;

	assert( array );
	assert( warray_none( array, filter, filterData ));
	return checkArray( array );
}

WArray*
warray_map( const WArray* array, WElementMap* map, const void* mapData, const WType* type )
{
	assert( array );
	assert( map );

	if ( not type ) type = wtypePtr;
	WArray* newArray = warray_new( array->capacity, type );

	for ( size_t i = 0; i < array->size; i++ )
		newArray->data[i] = map( array->data[i], mapData );

    newArray->size = array->size;

	assert( newArray );
	assert( newArray->size == array->size );
	assert( not type or newArray->type == type );
    return checkArray( newArray );
}

void*
warray_reduce( const WArray* array, WElementReduce* reduce, const void* startValue, const WType* type )
{
	assert( array );
	assert( reduce );

	if ( not type ) type = wtypePtr;

	if ( array->size == 0 ) {
		if ( startValue )
			return type->clone( startValue );
		else
			return NULL;
	}

	void* reduction = reduce( array->data[0], startValue );

    for ( size_t i = 1; i < array->size; i++ ) {
		void* newReduction = reduce( array->data[i], reduction );
		assert( &reduction );
		type->delete( &reduction );
		reduction = newReduction;
	}

	return reduction;
}

size_t
warray_count( const WArray* array, WElementCondition* condition, const void* conditionData )
{
	assert( array );
	assert( condition );

	size_t count = 0;

    for ( size_t i = 0; i < array->size; i++ ) {
		if ( condition( array->data[i], conditionData ))
			count++;
    }

	assert( count <= array->size );
	return count;
}

ssize_t
warray_index( const WArray* array, const void* element )
{
	assert( array );
	assert( array->type->compare );

	WElementCompare* compare = array->type->compare;

	for ( size_t i = 0; i < array->size; i++ ) {
		if ( compare( element, array->data[i] ) == 0 ) {
			assert( i <= array->size );
			return i;
		}
	}

	return -1;
}

ssize_t
warray_rindex( const WArray* array, const void* element )
{
	assert( array );
	assert( array->type->compare );

	WElementCompare* compare = array->type->compare;

	for ( size_t i = array->size-1; i < array->size; i-- ) {
		if ( compare( element, array->data[i] ) == 0 ) {
			assert( i <= array->size );
			return i;
		}
	}

	return -1;
}

//Helper for warray_toString()
static char*
str_cat3( const char* str1, const char* str2, const char* str3 )
{
	assert( str1 );
	assert( str2 );
	assert( str3 );

    size_t size1 = strlen( str1 );
    size_t size2 = strlen( str2 );
    size_t size3 = strlen( str3 );

	char* newStr = __wxmalloc( size1 + size2 + size3 + 1 );

	memcpy( newStr,				str1, size1 );
	memcpy( newStr+size1,		str2, size2 );
	memcpy( newStr+size1+size2, str3, size3 );

	newStr[size1+size2+size3] = 0;
	assert( newStr );
	return newStr;
}

char*
warray_toString( const WArray* array, const char delimiters[] )
{
	assert( array );
	assert( delimiters );
	assert( array->type->toString );

	if ( not array->size ) return __wstr_dup( "" );

	WElementToString* toString = array->type->toString;
	char* string = array->data[0] ? toString( array->data[0] ) : __wstr_dup( "NULL" );	//The first element without delimiters
	assert( string );

	for ( size_t i = 1; i < array->size; i++ ) {	//Concatenate current string, delimiters and next element.
		char* elementStr = array->data[i] ? toString( array->data[i] ) : __wstr_dup( "NULL" );
		assert( elementStr );
		char* temp = str_cat3( string, delimiters, elementStr );
		free( string );
		free( elementStr );
		string = temp;
	}

	assert( string );
	return string;
}

WArray*
warray_fromString( const char string[], const char delimiters[], const WType* targetType )
{
	assert( string );
	assert( delimiters and delimiters[0] );
	assert( targetType );
	assert( targetType->clone and targetType->delete and targetType->fromString );

	WArray* array = warray_new( 0, targetType );

	char* newString = __wstr_dup( string );
	char* context = NULL;
	char* token = __wstr_sep_r( newString, delimiters, &context );

	while ( token ) {
		void* element;
		if ( strcmp( token, "NULL" ) != 0 ) {
			element = targetType->fromString( token );
			assert( element );
			warray_append( array, element );
			targetType->delete( &element );
		}
		else
			warray_append( array, NULL );
		token = __wstr_sep_r( NULL, delimiters, &context );
	}

	free( newString );

	assert( array );
	return checkArray( array );
}

int
warray_compare( const WArray* array1, const WArray* array2 )
{
	assert( array1 );
	assert( array2 );
	assert( array1->type == array2->type && "Can't compare arrays of different type." );
	assert( array1->type->compare != NULL && "Need a comparison function." );

	//Setup some variables to save avoid unnecessary pointer accesses in the loop.
	WElementCompare* compare = array1->type->compare;
	void** data1 = array1->data;
	void** data2 = array2->data;

	//Compare all elements with each other until one array ends or a difference is found.
	for ( size_t i = 0; i < array1->size and i < array2->size; i++ ) {
		int result = compare( data1[i], data2[i] );
		if ( result ) return result;
	}

	//The compared elements were equal, now compare the sizes.
	return (ssize_t)array1->size - (ssize_t)array2->size;
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

const void*
warray_min( const WArray* array )
{
	assert( array );
	assert( warray_nonEmpty( array ));
	assert( array->type->compare && "Need a comparison method!" );

	WElementCompare* compare = array->type->compare;

    void* minimum = array->data[0];
    for ( size_t i = 1; i < array->size; i++ ) {
		if ( compare( array->data[i], minimum ) < 0 )
			minimum = array->data[i];
    }

	return minimum;
}

const void*
warray_max( const WArray* array )
{
	assert( array );
	assert( warray_nonEmpty( array ));
	assert( array->type->compare && "Need a comparison method!" );

	WElementCompare* compare = array->type->compare;

    void* maximum = array->data[0];
    for ( size_t i = 1; i < array->size; i++ ) {
		if ( compare( array->data[i], maximum ) > 0 )
			maximum = array->data[i];
    }

	return maximum;
}

ssize_t
warray_search( const WArray* array, WElementCompare* compare, const void* key )
{
	assert( array );
	assert( compare && "Need a comparison function!" );

	for ( size_t i = 0; i < array->size; i++ ) {
		if ( compare( key, array->data[i] ) == 0 )
			return i;
	}

	return -1;
}

// Helper struct to get both the bsearch comparison function and the key into our actual comparison function.
typedef struct ElementComparer {
	WElementCompare* compare;
	const void*		element;
}ElementComparer;

static inline int
compareKeyWithElement( const void* key, const void* element )
{
	//Extract the comparison function and the key.
	const ElementComparer* keyComparer = key;
	//bsearch gives us a pointer to the element, we deliver the element itself to the client comparison function.
	return keyComparer->compare( keyComparer->element, *(void**)element );
}

ssize_t
warray_bsearch( const WArray* array, WElementCompare* compare, const void* key )
{
	assert( array );
	assert( compare && "Need a comparison function!" );

	if ( not array->size ) return -1;

	ElementComparer keyComparer = { .compare = compare, .element = key };
	void** element = bsearch( &keyComparer, array->data, array->size, sizeof(void*), compareKeyWithElement );
	if ( not element ) return -1;

	assert( element >= array->data );
	assert( element < &array->data[array->size] );

	size_t position = element - array->data;	//This gives the real position, not the byte difference!
	assert( position <= array->size );
	assert( compare( key, warray_at( array, position )) == 0 );
	return position;
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

WArray*
warray_reverse( WArray* array )
{
	assert( array );

	ssize_t front = 0;
	ssize_t back = array->size-1;

	while ( front < back ) {
		__wswapPtr( array->data[front], array->data[back] );
		front++;
		back--;
	}

	assert( array );
	return checkArray( array );
}

WArray*
warray_shuffle( WArray* array )
{
	assert( array );

	for ( size_t i = 0; i < array->size; i++ ) {
        size_t position = rand() % array->size;
        __wswapPtr( array->data[i], array->data[position] );
	}

	assert( array );
	return checkArray( array );
}

WArray*
warray_compact( WArray* array )
{
	assert( array );

	size_t write = 0;
    for ( size_t read = 0; read < array->size; read++ ) {
		void* value = array->data[read];
		if ( value ) array->data[write++] = value;
    }

    array->size = write;

	assert( array );
	assert( not warray_contains( array, NULL ));
	return checkArray( array );
}

static bool
isSorted( const WArray* array )
{
	if ( not array->size ) return true;

	WElementCompare* compare = array->type->compare;
	for ( size_t j = 0; j < array->size-1; j++ ) {
		if ( compare( warray_at( array, j ), warray_at( array, j+1 )) == 1 )
			return false;
	}

	return true;
}

WArray*
warray_sort( WArray* array )
{
   	assert( array );
   	assert( array->type->compare );

	return warray_sortBy( array, array->type->compare );
}

/*	Here we define a helper function converting the void** pointers from qsort to void* elements.
	This helps keeping the API uniform, we always provide void* elements to the client
	callback functions. The downside: we get a dependency from C11 to remain thread-safe.
*/
_Thread_local WElementCompare* sortCompare;
static int compareTwoElements( const void* element1, const void* element2 ) {	//qsort delivers void**
	return sortCompare( *(void**)element1, *(void**)element2 );					//we deliver void* instead
}

WArray*
warray_sortBy( WArray* array, WElementCompare* compare )
{
	assert( array );
	assert( compare );

	sortCompare = compare;
	qsort( array->data, array->size, sizeof( void* ), compareTwoElements );

	assert( array );
	assert( isSorted( array ));
	return checkArray( array );
}

WArray*
warray_distinct( WArray* array)
{
	assert( array );
   	assert( array->type->compare );

	WElementCompare* compare = array->type->compare;

	for ( size_t i = 0; i < array->size; i++ ) {
		for ( size_t j = i+1; j < array->size; j++ ) {
			if ( compare( array->data[i], array->data[j] ) == 0 ) {
				warray_removeAt( array, i );
				i--;
				break;
			}
		}
	}

	assert( array );
	return checkArray( array );
}

WArray*
warray_concat( WArray* array1, const WArray* array2 )
{
	assert( array1 );
	assert( array2 );
	assert( array1->type == array2->type && "Arrays to be concatenated must have the same element types." );

	for ( size_t i = 0; i < array2->size; i++ )
		warray_append( array1, array2->data[i] );

	assert( array1 );
	return checkArray( array1 );
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

bool
warray_all( const WArray* array, WElementCondition* condition, const void* conditionData )
{
	assert( array );
	assert( condition && "Need a condition to check for." );

	for ( size_t i = 0; i < array->size; i++ ) {
        if ( not condition( array->data[i], conditionData ))
			return false;
	}

	return true;
}

bool
warray_any( const WArray* array, WElementCondition* condition, const void* conditionData )
{
	assert( array );
	assert( condition && "Need a condition to check for." );

	for ( size_t i = 0; i < array->size; i++ ) {
        if ( condition( array->data[i], conditionData ))
			return true;
	}

	return false;
}

bool
warray_none( const WArray* array, WElementCondition* condition, const void* conditionData )
{
	assert( array );
	assert( condition && "Need a condition to check for." );

	for ( size_t i = 0; i < array->size; i++ ) {
        if ( condition( array->data[i], conditionData ))
			return false;
	}

	return true;
}

bool
warray_one( const WArray* array, WElementCondition* condition, const void* conditionData )
{
	assert( array );
	assert( condition && "Need a condition to check for." );

	size_t counter = 0;
	for ( size_t i = 0; i < array->size; i++ ) {
        if ( condition( array->data[i], conditionData ))
			counter++;
	}

	return counter == 1;
}

//-------------------------------------------------------------------------------
