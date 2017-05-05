#define _GNU_SOURCE
#include "WArray.h"
//#include "Contract.h"
#include <assert.h>	//assert
#include <iso646.h>	//and, or, not
#include <string.h>	//memmove, memset, strdup, strtok_r
#include <stdarg.h>	//va_list
#include <stdio.h>	//printf, fflush
#include <stdlib.h>	//malloc, realloc

//-------------------------------------------------------------------------------
//	Helpers
//-------------------------------------------------------------------------------

static Array*
checkArray( const Array* array ) {
	assert( array->capacity > 0 );
	assert( array->size <= array->capacity );
	assert( array->data );
	assert( array->type );
	assert( array->type->clone );
	assert( array->type->delete );

	return (Array*)array;
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

const ElementType* arrayElement = &(ElementType){
	.clone = (ElementClone*)Array_clone,
	.delete = (ElementDelete*)Array_delete,
	.compare = (ElementCompare*)Array_compare,
	.fromString = (ElementFromString*)Array_fromString,
	.toString = (ElementToString*)Array_toString
};

enum ArrayParameters {
	ArrayDefaultCapacity 	= 100,
	ArrayGrowthRate 		= 2,
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

Array*
Array_new( size_t capacity, const ElementType* type )
{
	assert( not type or type->clone );
	assert( not type or type->delete );

	Array *array = xnew( Array,
		.capacity	= capacity ? capacity : ArrayDefaultCapacity,
		.type	= type ? type : elementPtr,
	);
	array->data = xmalloc( array->capacity * sizeof( void* ));

	assert( array );
	return checkArray( array );
}

Array*
Array_clone( const Array *self )
{
	assert( self );

	Array *copy = xnew( Array,
		.size		= self->size,
		.capacity	= self->capacity,
		.data		= xmalloc( sizeof( void* ) * self->capacity ),
		.type	= self->type
	);

	//TODO: Array_clone() with OpenMP pragma
	for ( size_t i = 0; i < self->size; i++ )
		copy->data[i] = self->type->clone( self->data[i] );

	assert( copy );
	assert( Array_equal( self, copy ));
	return checkArray( copy );
}

void
Array_delete( Array** selfPointer )
{
	if ( not selfPointer or not *selfPointer ) return;

	Array *self = *selfPointer;

	Array_clear( self );
	free( self->data );
	free( self );
	*selfPointer = NULL;
}

Array*
Array_clear( Array *self )
{
	if ( not self ) return self;

	//TODO: Array_clear() with OpenMP pragma
	for ( size_t i = 0; i < self->size; i++ )
		self->type->delete( &self->data[i] );

	self->size = 0;

	assert( self );
	assert( Array_empty( self ));
	return checkArray( self );
}

void
Array_assign( Array** arrayPointer, Array* other )
{
	if ( not arrayPointer ) return;
	Array* array = *arrayPointer;

	assert( array->type == other->type );

	Array_delete( arrayPointer );
	*arrayPointer = other;

	checkArray( array );
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

//Resize the array if necessary, so that it can take the new size.
static void
resize( Array* array, size_t newSize )
{
	if ( newSize <= array->capacity ) return;

	array->capacity = max( newSize, array->capacity * ArrayGrowthRate );
	array->data = xrealloc( array->data, array->capacity * sizeof(void*));
	assert( array->capacity >= newSize );
}

static Array*
put( Array* array, size_t position, const void* element )
{
	assert( array );
	assert( position < array->capacity );

	array->data[position] = array->type->clone( element );
	array->size = max( array->size+1, position+1 );

	return array;
}

Array*
Array_append( Array* array, const void* element )
{
	assert( array );

	resize( array, array->size+1 );

	return checkArray( put( array, array->size, element ));
}

Array*
Array_prepend( Array* array, const void* element )
{
	assert( array );

	resize( array, array->size+1 );
	memmove( &array->data[1], &array->data[0], array->size * sizeof(void*));

	return put( array, 0, element );
}

Array*
Array_set( Array* array, size_t position, const void* element )
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

Array*
Array_insert( Array* array, size_t position, const void* element )
{
	assert( array );

	resize( array, max( array->size+1, position+1 ));

	if ( position < array->size )	//Make room for the new element.
		memmove( &array->data[position+1], &array->data[position], (array->size-position) * sizeof(void*));
	else							//Fill the gap with zeros.
		memset( &array->data[array->size], 0, (position-array->size) * sizeof(void*));

	return put( array, position, element );
}

Array*
Array_insertSorted( Array* array, const void* element )
{
	assert( array );
	assert( array->type->compare );

	ElementCompare* compare = array->type->compare;
	void** data = array->data;
    for ( size_t i = 0; i < array->size; i++ ) {
        if ( compare( element, data[i] ) < 0 )
			return Array_insert( array, i, element );
    }

	return Array_append( array, element );
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

Array*
Array_append_n( Array* array, size_t n, void* const elements[n] )
{
   	assert( array );
	assert( n > 0 );
	assert( elements );

	//TODO: Array_append_n() with OpenMP pragma
	for ( size_t i = 0; i < n; i++ )
		Array_append( array, elements[i] );

	return array;
}

Array*
Array_prepend_n( Array* array, size_t n, void* const elements[n] )
{
   	assert( array );
	assert( n > 0 );
	assert( elements );

	//TODO: Array_prepend_n() with OpenMP pragma
	return Array_insert_n( array, 0, n, elements );
}

Array*
Array_insert_n( Array* array, size_t position, size_t n, void* const elements[n] )
{
   	assert( array );
	assert( n > 0 );
	assert( elements );

 	//TODO: Array_insert_n() with OpenMP pragma
  	for ( size_t i = 0; i < n; i++ )
		Array_insert( array, position+i, elements[i] );

	return array;
}

Array*
Array_set_n( Array* array, size_t position, size_t n, void* const elements[n] )
{
   	assert( array );
	assert( n > 0 );
	assert( elements );

 	//TODO: Array_set_n() with OpenMP pragma
  	for ( size_t i = 0; i < n; i++ )
		Array_set( array, position+i, elements[i] );

	return array;
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

const void*
Array_at( const Array *self, size_t index )
{
	assert( self );
	assert( index < self->size && "Array access out of bounds." );

	return self->data[index];
}

const void*
Array_first( const Array* array )
{
	assert( array );
	assert( Array_nonEmpty( array ));

	return Array_at( array, 0 );
}

const void*
Array_last( const Array* array )
{
	assert( array );
	assert( Array_nonEmpty( array ));

	return Array_at( array, array->size-1 );
}

const void*
Array_sample( const Array* array )
{
	assert( array );
	assert( Array_nonEmpty( array ));

	return Array_at( array, rand() % array->size );
}

void*
Array_cloneAt( const Array* array, size_t position )
{
	assert( array );
	assert( position < array->size );

	return array->type->clone( array->data[position] );
}

void*
Array_cloneFirst( const Array* array )
{
	assert( array );
	assert( Array_nonEmpty( array ));

	return Array_cloneAt( array, 0 );
}

void*
Array_cloneLast( const Array* array )
{
	assert( array );
	assert( Array_nonEmpty( array ));

	return Array_cloneAt( array, array->size-1 );
}

Array*
Array_slice( const Array* array, size_t start, size_t end )
{
	assert( array );
	assert( start <= end );
	assert( end < array->size );

	size_t size = end-start+1;
    Array* slice = Array_new( size, array->type );
	Array_append_n( slice, size, &array->data[start] );

	assert( slice );
	assert( slice->size == end-start+1 );

	return slice;
}

void*
Array_stealAt( Array* array, size_t position )
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
Array_stealFirst( Array* array )
{
	assert( array );
	assert( Array_nonEmpty( array ));

	return Array_stealAt( array, 0 );
}

void*
Array_stealLast( Array* array )
{
	assert( array );
	assert( Array_nonEmpty( array ));

	return Array_stealAt( array, array->size-1 );
}

void*
Array_stealSample( Array* array )
{
	assert( array );
	assert( Array_nonEmpty( array ));

	return Array_stealAt( array, rand() % array->size );
}

Array*
Array_removeAt( Array* array, size_t position )
{
	assert( array );
	assert( position < Array_size( array ));

	array->type->delete( &array->data[position] );

	if ( position < array->size-1 )
		memmove( &array->data[position], &array->data[position+1], (array->size-position-1) * sizeof( void* ));

	array->size--;

	assert( array );
	return array;
}

Array*
Array_removeFirst( Array* array )
{
	assert( array );
	assert( Array_nonEmpty( array ));

	return Array_removeAt( array, 0 );
}

Array*
Array_removeLast( Array* array )
{
	assert( array );
	assert( Array_nonEmpty( array ));

	return Array_removeAt( array, array->size-1 );
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

void
Array_foreach( const Array* array, ElementForeach* foreach, void* foreachData )
{
	assert( array );
	assert( foreach );

	for ( size_t i = 0; i < array->size; i++ )
		foreach( array->data[i], foreachData );
}

void
Array_foreachIndex( const Array* array, ElementForeachIndex* foreach, void* foreachData )
{
	assert( array );
	assert( foreach );

	for ( size_t i = 0; i < array->size; i++ )
		foreach( array->data[i], i, foreachData );
}

Array*
Array_filter( const Array* array, ElementFilter* filter, const void* filterData )
{
	assert( array );
	assert( filter );

	Array* xnewArray = (Array*)Array_new( array->capacity, array->type );

	//TODO: Array_filter() with OpenMP pragma???
    for ( size_t i = 0; i < array->size; i++ ) {
        if ( filter( array->data[i], filterData ))
			xnewArray->data[xnewArray->size++] = array->type->clone( array->data[i] );
    }

	assert( xnewArray );
	assert( Array_size( xnewArray ) <= Array_size( array ));
	return xnewArray;
}

Array*
Array_reject( const Array* array, ElementFilter* reject, const void* rejectData )
{
	assert( array );
	assert( reject );

	Array* xnewArray = (Array*)Array_new( array->capacity, array->type );

    for ( size_t i = 0; i < array->size; i++ ) {
        if ( not reject( array->data[i], rejectData ))
			xnewArray->data[xnewArray->size++] = array->type->clone( array->data[i] );
    }

	assert( xnewArray );
	assert( Array_size( xnewArray ) <= Array_size( array ));
	return xnewArray;
}

Array*
Array_select( Array* array, ElementFilter* filter, const void* filterData )
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

Array*
Array_unselect( Array* array, ElementFilter* filter, const void* filterData )
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

Array*
Array_map( const Array* array, ElementMap* map, const void* mapData, const ElementType* type )
{
	assert( array );
	assert( map );

	if ( not type ) type = elementPtr;
	Array* newArray = Array_new( array->capacity, type );

	//TODO: Array_map() with OpenMP pragma
	for ( size_t i = 0; i < array->size; i++ )
		newArray->data[i] = map( array->data[i], mapData );

    newArray->size = array->size;

	assert( newArray );
	assert( newArray->size == array->size );
	assert( not type or newArray->type == type );
    return newArray;
}

void*
Array_reduce( const Array* array, ElementReduce* reduce, const void* startValue, const ElementType* type )
{
	assert( array );
	assert( reduce );

	if ( not type ) type = elementPtr;

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
Array_count( const Array* array, ElementCondition* condition, const void* conditionData )
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
Array_index( const Array* array, const void* element )
{
	assert( array );
	assert( array->type->compare );

	ElementCompare* compare = array->type->compare;

	for ( size_t i = 0; i < array->size; i++ ) {
		if ( compare( element, array->data[i] ) == 0 )
			return i;
	}

	return -1;
}

ssize_t
Array_rindex( const Array* array, const void* element )
{
	assert( array );
	assert( array->type->compare );

	ElementCompare* compare = array->type->compare;

	for ( size_t i = array->size-1; i < array->size; i-- ) {
		if ( compare( element, array->data[i] ) == 0 )
			return i;
	}

	return -1;
}

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

char*
Array_toString( const Array* array, const char delimiters[] )
{
	assert( array );
	assert( delimiters );
	assert( array->type->toString );

	if ( not array->size ) return strdup( "" );

	ElementToString* toString = array->type->toString;
	char* string = toString( array->data[0] );		//The first element without delimiters

	for ( size_t i = 1; i < array->size; i++ ) {	//Concatenate current string, delimiters and next element.
		char* elementStr = toString( array->data[i] );
		char* tempString = str_printf( "%s%s%s", string, delimiters, elementStr );
		free( string );
		free( elementStr );
		string = tempString;
	}

	assert( string );
	return string;
}

Array*
Array_fromString( const char string[], const char delimiters[] )
{
	assert( string );
	assert( delimiters and delimiters[0] );

	Array* array = Array_new( 0, elementStr );

	char* newString = strdup( string );
	char* context = NULL;
	char* token = strtok_r( newString, delimiters, &context );

	while ( token ) {
		Array_append( array, token );
		token = strtok_r( NULL, delimiters, &context );
	}

	free( newString );

	assert( array );
	return array;
}

int
Array_compare( const Array* array1, const Array* array2 )
{
	assert( array1 );
	assert( array2 );
	assert( array1->type == array2->type && "Can't compare arrays of different type." );
	assert( array1->type->compare != NULL && "Need a comparison function." );

	//Setup some variables to save avoid unnecessary pointer accesses in the loop.
	ElementCompare* compare = array1->type->compare;
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
Array_min( const Array* array )
{
	assert( array );
	assert( Array_nonEmpty( array ));
	assert( array->type->compare );

	ElementCompare* compare = array->type->compare;

    void* minimum = array->data[0];
    for ( size_t i = 1; i < array->size; i++ ) {
		if ( compare( array->data[i], minimum ) < 0 )
			minimum = array->data[i];
    }

	return minimum;
}

const void*
Array_max( const Array* array )
{
	assert( array );
	assert( Array_nonEmpty( array ));
	assert( array->type->compare );

	ElementCompare* compare = array->type->compare;

    void* maximum = array->data[0];
    for ( size_t i = 1; i < array->size; i++ ) {
		if ( compare( array->data[i], maximum ) > 0 )
			maximum = array->data[i];
    }

	return maximum;
}

// Helper struct to get both the bsearch comparison function and the key into our actual comparison function.
typedef struct ElementComparer {
	ElementCompare* compare;
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
Array_bsearch( const Array* array, ElementCompare* compare, const void* key )
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

Array*
Array_reverse( Array* array )
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

Array*
Array_shuffle( Array* array )
{
	assert( array );

	for ( size_t i = 0; i < array->size; i++ ) {
        size_t position = rand() % array->size;
        swapPtr( array->data[i], array->data[position] );
	}

	assert( array );
	return array;
}

Array*
Array_compact( Array* array )
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

Array*
Array_sort( Array* array )
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

Array*
Array_sortBy( Array* array, ElementCompare* compare )
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

Array*
Array_distinct( Array* array)
{
	assert( array );
   	assert( array->type->compare );

	ElementCompare* compare = array->type->compare;

	for ( size_t i = 0; i < array->size; i++ ) {
		for ( size_t j = i+1; j < array->size; j++ ) {
			if ( compare( array->data[i], array->data[j] ) == 0 ) {
				Array_removeAt( array, i );
				i--;
				break;
			}
		}
	}

	assert( array );
	return array;
}

Array*
Array_concat( Array* array1, const Array* array2 )
{
	assert( array1 );
	assert( array2 );
	assert( array1->type == array2->type && "Cannot concatenate arrays with different element types." );

	for ( size_t i = 0; i < array2->size; i++ )
		Array_append( array1, array2->data[i] );

	assert( array1 );
	return array1;
}

//------------------------------------------------------------
//------------------------------------------------------------


Array*
Array_unite( const Array* array1, const Array* array2 )
{
	assert( array1 );
	assert( array2 );
	assert( array1->type == array2->type && "The Elements must have the same type." );
	assert( array1->type->compare && "Need an element comparison method." );
	assert( array2->type->compare && "Need an element comparison method." );

	return Array_distinct( Array_concat( Array_clone( array1 ), array2 ));
}

Array*
Array_intersect( const Array* array1, const Array* array2 )
{
	assert( array1 );
	assert( array2 );
	assert( array1->type == array2->type && "The Elements must have the same type." );
	assert( array1->type->compare && "Need an element comparison method." );
	assert( array2->type->compare && "Need an element comparison method." );

	Array* intersection = Array_new( array1->capacity, array1->type );
	ElementCompare* compare = array1->type->compare;

	for ( size_t i = 0; i < array1->size; i++ ) {
		for ( size_t j = 0; j < array2->size; j++ ) {
			if ( compare( array1->data[i], array2->data[j] ) == 0 ) {
				Array_append( intersection, array1->data[i] );
				break;
			}
		}
	}

	return Array_distinct( intersection );
}

Array*
Array_symDiff( const Array* array1, const Array* array2 )
{
	assert( array1 );
	assert( array2 );
	assert( array1->type == array2->type && "The Elements must have the same type." );
	assert( array1->type->compare && "Need an element comparison method." );
	assert( array2->type->compare && "Need an element comparison method." );

	ElementCompare* compare = array1->type->compare;
	bool equals( const void* element1, const void* element2 ) {
		return compare( element1, element2 ) == 0;
	}

	Array* difference = Array_new( array1->capacity, array1->type );

	for ( size_t i = 0; i < array1->size; i++ ) {
        if ( Array_none( array2, equals, array1->data[i] ))
			Array_append( difference, array1->data[i] );
	}
	for ( size_t i = 0; i < array2->size; i++ ) {
        if ( Array_none( array1, equals, array2->data[i] ))
			Array_append( difference, array2->data[i] );
	}

	return Array_distinct( difference );
}

Array*
Array_addToSet( Array* array, const void* element )
{
	assert( array );

	if ( Array_contains( array, element ))
		return array;

	return Array_append( array, element );
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

bool
Array_all( const Array* array, ElementCondition* condition, const void* conditionData )
{
	assert( array );
	assert( condition );

	//TODO: Array_all() with OpenMP pragma
	for ( size_t i = 0; i < array->size; i++ ) {
        if ( not condition( array->data[i], conditionData ))
			return false;
	}

	return true;
}

bool
Array_any( const Array* array, ElementCondition* condition, const void* conditionData )
{
	assert( array );
	assert( condition );

	//TODO: Array_any() with OpenMP pragma
	for ( size_t i = 0; i < array->size; i++ ) {
        if ( condition( array->data[i], conditionData ))
			return true;
	}

	return false;
}

bool
Array_none( const Array* array, ElementCondition* condition, const void* conditionData )
{
	assert( array );
	assert( condition );

	//TODO: Array_none() with OpenMP pragma
	for ( size_t i = 0; i < array->size; i++ ) {
        if ( condition( array->data[i], conditionData ))
			return false;
	}

	return true;
}

bool
Array_one( const Array* array, ElementCondition* condition, const void* conditionData )
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
	Iterator	base;
	size_t		current;
	int			direction;
}ArrayIterator;

static bool
ArrayIterator_hasNext( const Iterator* iterator )
{
	assert( iterator );

	const ArrayIterator* ai = (ArrayIterator*)iterator;
	const Array* array = ai->base.collection;

	return ai->current < array->size;
}

static const void*
ArrayIterator_next( Iterator* iterator )
{
	assert( iterator );
	assert( Iterator_hasNext( iterator ));

	ArrayIterator* ai = (ArrayIterator*)iterator;
	const Array* array = ai->base.collection;

	size_t current = ai->current;
	ai->current += ai->direction;

	return array->data[current];
}

Iterator*
Array_iterator( const Array* array )
{
	assert( array );

	ArrayIterator* iterator = xmalloc( sizeof(ArrayIterator) );

	iterator->base.collection = array;
	iterator->base.hasNext = ArrayIterator_hasNext;
	iterator->base.next = ArrayIterator_next;
	iterator->current = 0;
	iterator->direction = 1;

	return (Iterator*)iterator;
}

Iterator*
Array_iteratorReverse( const Array* array )
{
	assert( array );

	ArrayIterator* iterator = xmalloc( sizeof(ArrayIterator) );

	iterator->base.collection = array;
	iterator->base.hasNext = ArrayIterator_hasNext;
	iterator->base.next = ArrayIterator_next;
	iterator->current = array->size-1;
	iterator->direction = -1;

	return (Iterator*)iterator;
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

void
Array_print( const Array* array )
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

#if 0
double
Array_dmin( const Array* array )
{
	preconditions {
		assert( array );
		assert( array->type->clone == Element_cloneDouble, "Array must contain double elements." );
	}

	double minimum = DBL_MAX;

	for ( size_t i = 0; i < array->size; i++ ) {
		if ( array->data[i] )
			minimum = min( *(double*)array->data[i], minimum );
	}

	return minimum;
}

double
Array_dmax( const Array* array )
{
	preconditions {
		assert( array );
		assert( array->type->clone == Element_cloneDouble, "Array must contain double elements." );
	}

	double maximum = -DBL_MAX;

	for ( size_t i = 0; i < array->size; i++ ) {
		if ( array->data[i] )
			maximum = max( *(double*)array->data[i], maximum );
	}

	return maximum;
}

double
Array_dsum( const Array* array )
{
	preconditions {
		assert( array );
		assert( array->type->clone == Element_cloneDouble, "Array must contain double elements." );
	}

	double sum = 0;
	for ( size_t i = 0; i < array->size; i++ ) {
		if ( array->data[i] )
			sum += *(double*)array->data[i];
	}

	return sum;
}

double
Array_dmean( const Array* array )
{
	preconditions {
		assert( array );
		assert( array->type->clone == Element_cloneDouble, "Array must contain double elements." );
	}

	double sum = 0;
	for ( size_t i = 0; i < array->size; i++ ) {
		if ( array->data[i] )
			sum += *(double*)array->data[i];
	}

	return sum / array->size;
}

double
Array_dmedian( const Array* array )
{
	return Array_dpercentile( array, 50 );
}

double
Array_ddeviation( const Array* array )
{
	preconditions {
		assert( array );
		assert( array->type->clone == Element_cloneDouble, "Array must contain double elements." );
	}

    double mean = Array_dmean( array );
	double sum = 0;

	for ( size_t i = 0; i < array->size; i++ ) {
		if ( array->data[i] )
			sum += ( *(double*)array->data[i] - mean ) * ( *(double*)array->data[i] - mean );
	}

	return sqrt( sum / array->size-1 );
}

double
Array_dpercentile( const Array* array, size_t n )
{
	preconditions {
		assert( array );
		assert( Array_nonEmpty( array ));
		assert( array->type->clone == Element_cloneDouble, "Array must contain double elements." );
		assert( n <= 100 );
	}

    autoArray* sorted = Array_sort( Array_clone( array ));

	const double epsilon = 0.1;
	double indexDouble = n * array->size / 100.0;
	size_t index = indexDouble;

	if ( indexDouble - (int){ indexDouble } <= epsilon )	//Index is a whole number.
		return ( *(double*)sorted->data[index] + *(double*)sorted->data[index+1] ) / 2;
	else
		return *(double*)sorted->data[index+1];				//Index is not a whole number.
}

Array*
Array_dquartileSummary( const Array* array )
{
	preconditions {
		assert( array );
		assert( Array_nonEmpty( array ));
		assert( array->type->clone == Element_cloneDouble, "Array must contain double elements." );
	}

	Array* summary = Array_new( 5, elementDouble );

	//TODO: Make everything in this function to avoid multiple sorting and looping through the array.
	//Call a function, assign the returned double to a temporary object and pass the object's address to Array_append().
	Array_append( summary, &(double){ Array_dmin( array )});
	Array_append( summary, &(double){ Array_dpercentile( array, 25 )});
	Array_append( summary, &(double){ Array_dpercentile( array, 50 )});
	Array_append( summary, &(double){ Array_dpercentile( array, 75 )});
	Array_append( summary, &(double){ Array_dmax( array )});

	return summary;
}

//------------------------------------------------------------
//------------------------------------------------------------
#endif
