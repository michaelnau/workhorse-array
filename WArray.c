#define _POSIX_C_SOURCE 200809L
#include "WArray.h"
#include <assert.h>	//assert
#include <iso646.h>	//and, or, not
#include <string.h>	//memmove, memset, strdup, strtok_r
#include <stdarg.h>	//va_list
#include <stdio.h>	//printf, fflush
#include <stdlib.h>	//malloc, realloc

//TODO: Remove dependency from _POSIX_C_SOURCE.
//TODO: Remove GNU C nested functions.

//-------------------------------------------------------------------------------
//	Helpers
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

static void*
xmalloc( size_t size )
{
	void* ptr = malloc( size );
	if ( !ptr ) {
		fputs( "Out of memory.", stderr );
		exit( EXIT_FAILURE );
	}

	return ptr;
}

static void*
xrealloc( void* pointer, size_t size )
{
	void* ptr = realloc( pointer, size );
	if ( !ptr ) {
		fputs( "Out of memory.", stderr );
		exit( EXIT_FAILURE );
	}

	return ptr;
}

#define xnew( type, ... )							\
	memcpy( xmalloc( sizeof( type )), &(type){ __VA_ARGS__ }, sizeof(type) )

#define max( x, y )	((x) > (y) ? (x) : (y))

#define swapPtr( var1, var2 )	\
do {							\
	void* __temp = var2;		\
	var2 = var1;				\
	var1 = __temp;				\
}while(0)

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

	WArray *array = xnew( WArray,
		.capacity	= capacity ? capacity : ArrayDefaultCapacity,
		.type	= type ? type : wtypePtr,
	);
	array->data = xmalloc( array->capacity * sizeof( void* ));

	assert( array );
	return checkArray( array );
}

WArray*
warray_clone( const WArray *self )
{
	assert( self );

	WArray *copy = xnew( WArray,
		.size		= self->size,
		.capacity	= self->capacity,
		.data		= xmalloc( sizeof( void* ) * self->capacity ),
		.type	= self->type
	);

	//TODO: warray_clone() with OpenMP pragma
	for ( size_t i = 0; i < self->size; i++ )
		copy->data[i] = self->type->clone( self->data[i] );

	assert( copy );
	assert( warray_equal( self, copy ));
	return checkArray( copy );
}

void
warray_delete( WArray** selfPointer )
{
	if ( not selfPointer or not *selfPointer ) return;

	WArray *self = *selfPointer;

	warray_clear( self );
	free( self->data );
	free( self );
	*selfPointer = NULL;
}

WArray*
warray_clear( WArray *self )
{
	if ( not self ) return self;

	//TODO: warray_clear() with OpenMP pragma
	for ( size_t i = 0; i < self->size; i++ )
		self->type->delete( &self->data[i] );

	self->size = 0;

	assert( self );
	assert( warray_empty( self ));
	return checkArray( self );
}

void
warray_assign( WArray** arrayPointer, WArray* other )
{
	if ( not arrayPointer ) return;
	WArray* array = *arrayPointer;

	assert( array->type == other->type );

	warray_delete( arrayPointer );
	*arrayPointer = other;

	checkArray( array );
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

//Resize the array if necessary, so that it can take the new size.
static void
resize( WArray* array, size_t newSize )
{
	if ( newSize <= array->capacity ) return;

	array->capacity = max( newSize, array->capacity * ArrayGrowthRate );
	array->data = xrealloc( array->data, array->capacity * sizeof(void*));
	assert( array->capacity >= newSize );
}

static WArray*
put( WArray* array, size_t position, const void* element )
{
	assert( array );
	assert( position < array->capacity );

	array->data[position] = array->type->clone( element );
	array->size = max( array->size+1, position+1 );

	return array;
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

	return put( array, 0, element );
}

WArray*
warray_set( WArray* array, size_t position, const void* element )
{
	assert( array );

	resize( array, max( array->size, position+1 ));

	if ( position < array->size )	//Delete the old element.
		array->type->delete( &array->data[position] );
	else {							//Fill the gap with zeroes.
		memset( &array->data[array->size], 0, (position-array->size) * sizeof(void*));
		array->size = position+1;
	}

	array->data[position] = array->type->clone( element );

	return array;
}

WArray*
warray_insert( WArray* array, size_t position, const void* element )
{
	assert( array );

	resize( array, max( array->size+1, position+1 ));

	if ( position < array->size )	//Make room for the new element.
		memmove( &array->data[position+1], &array->data[position], (array->size-position) * sizeof(void*));
	else							//Fill the gap with zeros.
		memset( &array->data[array->size], 0, (position-array->size) * sizeof(void*));

	return put( array, position, element );
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

	//TODO: warray_append_n() with OpenMP pragma
	for ( size_t i = 0; i < n; i++ )
		warray_append( array, elements[i] );

	return array;
}

WArray*
warray_prepend_n( WArray* array, size_t n, void* const elements[n] )
{
   	assert( array );
	assert( n > 0 );
	assert( elements );

	//TODO: warray_prepend_n() with OpenMP pragma
	return warray_insert_n( array, 0, n, elements );
}

WArray*
warray_insert_n( WArray* array, size_t position, size_t n, void* const elements[n] )
{
   	assert( array );
	assert( n > 0 );
	assert( elements );

 	//TODO: warray_insert_n() with OpenMP pragma
  	for ( size_t i = 0; i < n; i++ )
		warray_insert( array, position+i, elements[i] );

	return array;
}

WArray*
warray_set_n( WArray* array, size_t position, size_t n, void* const elements[n] )
{
   	assert( array );
	assert( n > 0 );
	assert( elements );

 	//TODO: warray_set_n() with OpenMP pragma
  	for ( size_t i = 0; i < n; i++ )
		warray_set( array, position+i, elements[i] );

	return array;
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

const void*
warray_at( const WArray *self, size_t index )
{
	assert( self );
	assert( index < self->size && "Array access out of bounds." );

	return self->data[index];
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
	assert( position < array->size );

	return array->type->clone( array->data[position] );
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

	return slice;
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

	array->type->delete( &array->data[position] );

	if ( position < array->size-1 )
		memmove( &array->data[position], &array->data[position+1], (array->size-position-1) * sizeof( void* ));

	array->size--;

	assert( array );
	return array;
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

	WArray* xnewArray = (WArray*)warray_new( array->capacity, array->type );

	//TODO: warray_filter() with OpenMP pragma???
    for ( size_t i = 0; i < array->size; i++ ) {
        if ( filter( array->data[i], filterData ))
			xnewArray->data[xnewArray->size++] = array->type->clone( array->data[i] );
    }

	assert( xnewArray );
	assert( warray_size( xnewArray ) <= warray_size( array ));
	return xnewArray;
}

WArray*
warray_reject( const WArray* array, WElementCondition* reject, const void* rejectData )
{
	assert( array );
	assert( reject );

	WArray* xnewArray = (WArray*)warray_new( array->capacity, array->type );

    for ( size_t i = 0; i < array->size; i++ ) {
        if ( not reject( array->data[i], rejectData ))
			xnewArray->data[xnewArray->size++] = array->type->clone( array->data[i] );
    }

	assert( xnewArray );
	assert( warray_size( xnewArray ) <= warray_size( array ));
	return xnewArray;
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
		else
			array->type->delete( array->data[from] );
    }

    array->size = to;

	assert( array );
	return array;
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
		else
			array->type->delete( array->data[from] );
	}

    array->size = to;

	assert( array );
	return array;
}

WArray*
warray_map( const WArray* array, WElementMap* map, const void* mapData, const WType* type )
{
	assert( array );
	assert( map );

	if ( not type ) type = wtypePtr;
	WArray* newArray = warray_new( array->capacity, type );

	//TODO: warray_map() with OpenMP pragma
	for ( size_t i = 0; i < array->size; i++ )
		newArray->data[i] = map( array->data[i], mapData );

    newArray->size = array->size;

	assert( newArray );
	assert( newArray->size == array->size );
	assert( not type or newArray->type == type );
    return newArray;
}

void*
warray_reduce( const WArray* array, WElementReduce* reduce, const void* startValue, const WType* type )
{
	assert( array );
	assert( reduce );

	if ( not type ) type = wtypePtr;

	if ( array->size == 0 )
		return type->clone( startValue );

	void* reduction = reduce( array->data[0], startValue );

    for ( size_t i = 1; i < array->size; i++ ) {
		void* newReduction = reduce( array->data[i], reduction );
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

	return count;
}

ssize_t
warray_index( const WArray* array, const void* element )
{
	assert( array );
	assert( array->type->compare );

	WElementCompare* compare = array->type->compare;

	for ( size_t i = 0; i < array->size; i++ ) {
		if ( compare( element, array->data[i] ) == 0 )
			return i;
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
		if ( compare( element, array->data[i] ) == 0 )
			return i;
	}

	return -1;
}

#if 0
static char*
str_printf( const char* format, ... )
{
	assert( format );

	char* string;

    va_list args;
    va_start( args, format );
	if ( vasprintf( &string, format, args ) < 0 ) {
		fputs( "Error in vasprintf().", stderr );
		abort();
	}

	va_end( args );

	assert( string );
	return string;
}
#endif // 0

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

	char* newStr = xmalloc( size1 + size2 + size3 + 1 );

	memcpy( newStr,				str1, size1 );
	memcpy( newStr+size1,		str2, size2 );
	memcpy( newStr+size1+size2, str3, size3 );

	newStr[size1+size2+size3] = 0;
	return newStr;
}

char*
warray_toString( const WArray* array, const char delimiters[] )
{
	assert( array );
	assert( delimiters );
	assert( array->type->toString );

	if ( not array->size ) return strdup( "" );

	WElementToString* toString = array->type->toString;
	char* string = toString( array->data[0] );		//The first element without delimiters
	assert( string );

	for ( size_t i = 1; i < array->size; i++ ) {	//Concatenate current string, delimiters and next element.
		char* elementStr = toString( array->data[i] );
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
warray_fromString( const char string[], const char delimiters[] )
{
	assert( string );
	assert( delimiters and delimiters[0] );

	WArray* array = warray_new( 0, wtypeStr );

	char* newString = strdup( string );
	char* context = NULL;
	char* token = strtok_r( newString, delimiters, &context );

	while ( token ) {
		warray_append( array, token );
		token = strtok_r( NULL, delimiters, &context );
	}

	free( newString );

	assert( array );
	return array;
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
	assert( array->type->compare );

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
	assert( array->type->compare );

	WElementCompare* compare = array->type->compare;

    void* maximum = array->data[0];
    for ( size_t i = 1; i < array->size; i++ ) {
		if ( compare( array->data[i], maximum ) > 0 )
			maximum = array->data[i];
    }

	return maximum;
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
	assert( compare );

	if ( not array->size ) return -1;

	ElementComparer keyComparer = { .compare = compare, .element = key };
	void** element = bsearch( &keyComparer, array->data, array->size, sizeof(void*), compareKeyWithElement );
	if ( not element ) return -1;

	assert( element >= array->data );
	assert( element < &array->data[array->size] );

	return element - array->data;	//This gives the real position, not the byte difference!
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
		swapPtr( array->data[front], array->data[back] );
		front++;
		back--;
	}

	assert( array );
	return array;
}

WArray*
warray_shuffle( WArray* array )
{
	assert( array );

	for ( size_t i = 0; i < array->size; i++ ) {
        size_t position = rand() % array->size;
        swapPtr( array->data[i], array->data[position] );
	}

	assert( array );
	return array;
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
	return array;
}

static inline int
compareTwoElements( const void* element1, const void* element2 )
{
	//Extract the comparison function and the key.
	const ElementComparer* comparer = element1;
	//bsearch gives us a pointer to the elements, we deliver the elements itself to the client comparison function.
	return comparer->compare( *(void**)comparer->element, *(void**)element2 );
}

WArray*
warray_sort( WArray* array )
{
   	assert( array );
   	assert( array->type->compare );

	int wrappedCompare( const void* element1, const void* element2 ) {			//qsort delivers void**
        return array->type->compare( *(void**)element1, *(void**)element2 );	//we deliver void* instead
	}

    qsort( array->data, array->size, sizeof( void* ), wrappedCompare );

	assert( array );
	return array;
}

WArray*
warray_sortBy( WArray* array, WElementCompare* compare )
{
	assert( array );
	assert( compare );

	int wrappedCompare( const void* element1, const void* element2 ) {	//qsort delivers void**
        return compare( *(void**)element1, *(void**)element2 );			//we deliver void* instead
	}

    qsort( array->data, array->size, sizeof( void* ), wrappedCompare );

	assert( array );
	return array;
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
	return array;
}

WArray*
warray_concat( WArray* array1, const WArray* array2 )
{
	assert( array1 );
	assert( array2 );
	assert( array1->type == array2->type && "Cannot concatenate arrays with different element types." );

	for ( size_t i = 0; i < array2->size; i++ )
		warray_append( array1, array2->data[i] );

	assert( array1 );
	return array1;
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

bool
warray_all( const WArray* array, WElementCondition* condition, const void* conditionData )
{
	assert( array );
	assert( condition );

	//TODO: warray_all() with OpenMP pragma
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
	assert( condition );

	//TODO: warray_any() with OpenMP pragma
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
	assert( condition );

	//TODO: warray_none() with OpenMP pragma
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
	assert( condition );

	size_t counter = 0;
	for ( size_t i = 0; i < array->size; i++ ) {
        if ( condition( array->data[i], conditionData ))
			counter++;
	}

	return counter == 1;
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

typedef struct ArrayIterator {
	WIterator	base;
	size_t		current;
	int			direction;
}ArrayIterator;

static bool
ArrayIterator_hasNext( const WIterator* iterator )
{
	assert( iterator );

	const ArrayIterator* ai = (ArrayIterator*)iterator;
	const WArray* array = ai->base.collection;

	return ai->current < array->size;
}

static const void*
ArrayIterator_next( WIterator* iterator )
{
	assert( iterator );
	assert( witerator_hasNext( iterator ));

	ArrayIterator* ai = (ArrayIterator*)iterator;
	const WArray* array = ai->base.collection;

	size_t current = ai->current;
	ai->current += ai->direction;

	return array->data[current];
}

WIterator*
warray_iterator( const WArray* array )
{
	assert( array );

	ArrayIterator* iterator = xmalloc( sizeof(ArrayIterator) );

	iterator->base.collection = array;
	iterator->base.hasNext = ArrayIterator_hasNext;
	iterator->base.next = ArrayIterator_next;
	iterator->current = 0;
	iterator->direction = 1;

	return (WIterator*)iterator;
}

WIterator*
warray_iteratorReverse( const WArray* array )
{
	assert( array );

	ArrayIterator* iterator = xmalloc( sizeof(ArrayIterator) );

	iterator->base.collection = array;
	iterator->base.hasNext = ArrayIterator_hasNext;
	iterator->base.next = ArrayIterator_next;
	iterator->current = array->size-1;
	iterator->direction = -1;

	return (WIterator*)iterator;
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

void
warray_print( const WArray* array )
{
	assert( array );

    printf("\n");
    printf("size     = %u\n", array->size);
    printf("capacity = %u\n", array->capacity);
    printf("type     = %p\n", (void*)array->type);
    printf("data     = %p\n", (void*)array->data);
    printf("-->\n");

    for ( size_t i = 0; i < array->size; i++ ) {
//		printf("    %p --> %s\n", array->data[i], array->type->toString ? array->type->toString( array->data[i] ) : "-" );
		printf("    %p\n", array->data[i] );
    }

    printf("\n");
	fflush( stdout );
}

//-------------------------------------------------------------------------------
