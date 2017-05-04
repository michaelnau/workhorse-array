#define _GNU_SOURCE
#include "Collection.h"

#include <assert.h>		//assert()
#include <iso646.h>		//and, or, not
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//---------------------------------------------------------------------------------

static void
die( const char* text )
{
	fprintf( stderr, "Failed: %s", text );
	abort();
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

//---------------------------------------------------------------------------------

char __elementNotFound;

//---------------------------------------------------------------------------------
//	Default methods for unspecified pointer types
//---------------------------------------------------------------------------------

const ElementType* elementPtr = &(ElementType) {
	.clone = Element_clonePtr,
	.delete = Element_deletePtr,
	.compare = Element_compareUndefined,
	.fromString = Element_fromStringUndefined,
	.toString = Element_toStringUndefined
};

void* Element_clonePtr( const void* element ) {
	return (void*)element;
}

void Element_deletePtr( void** elementPtr ) {
	(void)elementPtr;
}

//---------------------------------------------------------------------------------
//	int methods
//---------------------------------------------------------------------------------

const ElementType* elementInt = &(ElementType) {
	.clone = Element_cloneInt,
	.delete = Element_deleteInt,
	.compare = Element_compareInt,
	.fromString = Element_fromStringInt,
	.toString = Element_toStringInt
};

void* Element_cloneInt( const void* element ) {
	return (void*)element;
}

void Element_deleteInt( void** elementPtr ) {
	(void)elementPtr;
}

int Element_compareInt( const void* e1, const void* e2 ) {
	return (long)e1 - (long)e2;
}

void* Element_fromStringInt( const char* string ) {
	assert( string );

	return (void*)strtol( string, NULL, 0 );
}

char* Element_toStringInt( const void* element ) {
	char* string = NULL;
    return asprintf( &string, "%ld", (long)element ) >= 0 ? string : strdup( "" );
}

//---------------------------------------------------------------------------------
//	char* methods
//---------------------------------------------------------------------------------

const ElementType* elementStr = &(ElementType) {
	.clone = Element_cloneStr,
	.delete = Element_delete,
	.compare = Element_compareStr,
	.fromString = Element_fromStringStr,
	.toString = Element_toStringStr
};

void* Element_cloneStr( const void* element ) {
	return element ? strdup( element ) : NULL;
}

int Element_compareStr( const void* e1, const void* e2 ) {
	if ( e1 and e2 ) return strcmp( e1, e2 );
	if ( not e1 and not e2 ) return 0;
	return e1 ? 1 : -1;
}

void* Element_fromStringStr( const char* string ) {
	return string ? strdup( string ) : strdup( "" );
}

char* Element_toStringStr( const void* element ) {
	return element ? strdup( element ) : strdup( "" );
}

//---------------------------------------------------------------------------------
//	double methods
//---------------------------------------------------------------------------------

const ElementType* elementDouble = &(ElementType) {
	.clone = Element_cloneDouble,
	.delete = Element_delete,
	.compare = Element_compareDouble,
	.fromString = Element_fromStringDouble,
	.toString = Element_toStringDouble
};

void* Element_cloneDouble( const void* element ) {
	double* clone = xmalloc( sizeof( double ));
	*clone = *(double*)element;
	return clone;
}

int Element_compareDouble( const void* e1, const void* e2 ) {
	return (e1 and e2) ? *(double*)e1 - *(double*)e2 :
			e1 ? +1 : -1;	//NULL values are considered to be less than every double value
}

void* Element_fromStringDouble( const char* string ) {
	assert( string );

	double* element = xmalloc( sizeof( double ));
	*element = strtod( string, NULL );
	return element;
}

char* Element_toStringDouble( const void* element ) {
	char* string = NULL;
    return not element ?									strdup("") :	//NULL element
		asprintf( &string, "%lf", *(double*)element ) < 0 ?	strdup("") :	//Problems in stringifying double
															string;			//Actual double
}

//---------------------------------------------------------------------------------

void
Element_delete( void** elementPtr )
{
	if ( not elementPtr or not *elementPtr ) return;

	free( *elementPtr );
	*elementPtr = NULL;
}

//---------------------------------------------------------------------------------
//	Stubs for elements not implementing methods
//---------------------------------------------------------------------------------

void* Element_cloneUndefined( const void* element ) {
	(void)element;
	die( "The clone() method is undefined for this element type." );
	return NULL;
}

void Element_deleteUndefined( void** elementPtr ) {
	(void)elementPtr;
	die( "The delete() method is undefined for this element type." );
}

int Element_compareUndefined( const void* e1, const void* e2 ) {
	(void)e1, (void)e2;
	die( "The compare() method is undefined for this element type." );
	return -1;
}

void* Element_fromStringUndefined( const char* string ) {
	(void)string;
	die( "The fromString() method is undefined for this element type." );
	return NULL;
}

char* Element_toStringUndefined( const void* element ) {
	(void)element;
	die( "The toString() method is undefined for this element type." );
	return NULL;
}

//---------------------------------------------------------------------------------
//	Condition functions
//---------------------------------------------------------------------------------

bool Element_conditionStrEquals( const void* e1, const void* e2 ) {
	if ( e1 and e2 ) return strcmp( e1, e2 ) == 0;
	return e1 == e2;
}

bool Element_conditionStrEmpty( const void* element, const void* conditionData ) {
	(void)conditionData;
	if ( element ) return ((char*)element)[0] == 0;
	return true;
}

//---------------------------------------------------------------------------------
//	Foreach functions
//---------------------------------------------------------------------------------

void Element_foreachStrPrint( const void* element, const void* foreachData ) {
	printf( "%s%s", (char*)element, foreachData ? (char*)foreachData : "" );
}

void Element_foreachIndexStrPrint( const void* element, size_t index, const void* foreachData ) {
	printf( "%u/%s%s", index, (char*)element, foreachData ? (char*)foreachData : "" );
}

//---------------------------------------------------------------------------------

void
Iterator_delete( Iterator** iterator )
{
	if ( not iterator ) return;

	free( *iterator );
	*iterator = NULL;
}

//---------------------------------------------------------------------------------
