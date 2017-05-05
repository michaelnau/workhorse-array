#define _GNU_SOURCE
#include "WCollection.h"

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

static char welementNotFound;
const void* WElementNotFound = &welementNotFound;

//---------------------------------------------------------------------------------
//	Raw void* type
//---------------------------------------------------------------------------------

void* WElement_clonePtr( const void* element ) {
	return (void*)element;
}

void WElement_deletePtr( void** wtypePtr ) {
	(void)wtypePtr;
}

int WElement_comparePtr( const void* e1, const void* e2 ) {
	return (long)e1 - (long)e2;
}

const WType* wtypePtr = &(WType) {
	.clone = WElement_clonePtr,
	.delete = WElement_deletePtr,
	.compare = WElement_comparePtr,
};

//---------------------------------------------------------------------------------
//	int type
//---------------------------------------------------------------------------------

void* WElement_cloneInt( const void* element ) {
	return (void*)element;
}

void WElement_deleteInt( void** wtypePtr ) {
	(void)wtypePtr;
}

int WElement_compareInt( const void* e1, const void* e2 ) {
	return (long)e1 - (long)e2;
}

void* WElement_fromStringInt( const char* string ) {
	assert( string );

	return (void*)strtol( string, NULL, 0 );
}

char* WElement_toStringInt( const void* element ) {
	char* string = NULL;
    return asprintf( &string, "%ld", (long)element ) >= 0 ? string : strdup( "" );
}

const WType* wtypeInt = &(WType) {
	.clone = WElement_cloneInt,
	.delete = WElement_deleteInt,
	.compare = WElement_compareInt,
	.fromString = WElement_fromStringInt,
	.toString = WElement_toStringInt
};

//---------------------------------------------------------------------------------
//	char* type
//---------------------------------------------------------------------------------

void* WElement_cloneStr( const void* element ) {
	return element ? strdup( element ) : NULL;
}

void WElement_delete( void** wtypePtr ) {
	if ( not wtypePtr ) return;

	free( *wtypePtr );
	*wtypePtr = NULL;
}

int WElement_compareStr( const void* e1, const void* e2 ) {
	if ( e1 and e2 ) return strcmp( e1, e2 );
	if ( not e1 and not e2 ) return 0;
	return e1 ? 1 : -1;
}

void* WElement_fromStringStr( const char* string ) {
	return string ? strdup( string ) : strdup( "" );
}

char* WElement_toStringStr( const void* element ) {
	return element ? strdup( element ) : strdup( "" );
}

const WType* wtypeStr = &(WType) {
	.clone = WElement_cloneStr,
	.delete = WElement_delete,
	.compare = WElement_compareStr,
	.fromString = WElement_fromStringStr,
	.toString = WElement_toStringStr
};

//---------------------------------------------------------------------------------
//	double type
//---------------------------------------------------------------------------------

void* WElement_cloneDouble( const void* element ) {
	double* clone = xmalloc( sizeof( double ));
	*clone = *(double*)element;
	return clone;
}

int WElement_compareDouble( const void* e1, const void* e2 ) {
	return (e1 and e2) ? *(double*)e1 - *(double*)e2 :
			e1 ? +1 : -1;	//NULL values are considered to be less than every double value
}

void* WElement_fromStringDouble( const char* string ) {
	assert( string );

	double* element = xmalloc( sizeof( double ));
	*element = strtod( string, NULL );
	return element;
}

char* WElement_toStringDouble( const void* element ) {
	char* string = NULL;
    return not element ?									strdup("") :	//NULL element
		asprintf( &string, "%lf", *(double*)element ) < 0 ?	strdup("") :	//Problems in stringifying double
															string;			//Actual double
}

const WType* wtypeDouble = &(WType) {
	.clone = WElement_cloneDouble,
	.delete = WElement_delete,
	.compare = WElement_compareDouble,
	.fromString = WElement_fromStringDouble,
	.toString = WElement_toStringDouble
};

//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
//	Stubs for elements not implementing methods
//---------------------------------------------------------------------------------

void* Element_cloneUndefined( const void* element ) {
	(void)element;
	die( "The clone() method is undefined for this element type." );
	return NULL;
}

void Element_deleteUndefined( void** wtypePtr ) {
	(void)wtypePtr;
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

bool WElement_conditionStrEquals( const void* e1, const void* e2 ) {
	if ( e1 and e2 ) return strcmp( e1, e2 ) == 0;
	return e1 == e2;
}

bool WElement_conditionStrEmpty( const void* element, const void* conditionData ) {
	(void)conditionData;
	if ( element ) return ((char*)element)[0] == 0;
	return true;
}

//---------------------------------------------------------------------------------
//	Foreach functions
//---------------------------------------------------------------------------------

void WElement_foreachStrPrint( const void* element, const void* foreachData ) {
	printf( "%s%s", (char*)element, foreachData ? (char*)foreachData : "" );
}

void WElement_foreachIndexStrPrint( const void* element, size_t index, const void* foreachData ) {
	printf( "%u/%s%s", index, (char*)element, foreachData ? (char*)foreachData : "" );
}

//---------------------------------------------------------------------------------
//	Iterator functions
//---------------------------------------------------------------------------------

void
WIterator_delete( WIterator** iterator )
{
	if ( not iterator ) return;

	free( *iterator );
	*iterator = NULL;
}

//---------------------------------------------------------------------------------
