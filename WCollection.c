#define _GNU_SOURCE
#include "WCollection.h"

#include <assert.h>		//assert()
#include <iso646.h>		//and, or, not
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//---------------------------------------------------------------------------------
//	Helper functions
//---------------------------------------------------------------------------------

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

void* welement_clonePtr( const void* element ) {
	return (void*)element;
}

void welement_deletePtr( void** wtypePtr ) {
	(void)wtypePtr;
}

int welement_comparePtr( const void* e1, const void* e2 ) {
	return (long)e1 - (long)e2;
}

const WType* wtypePtr = &(WType) {
	.clone = welement_clonePtr,
	.delete = welement_deletePtr,
	.compare = welement_comparePtr,
};

//---------------------------------------------------------------------------------
//	int type
//---------------------------------------------------------------------------------

void* welement_cloneInt( const void* element ) {
	return (void*)element;
}

void welement_deleteInt( void** wtypePtr ) {
	(void)wtypePtr;
}

int welement_compareInt( const void* e1, const void* e2 ) {
	return (long)e1 - (long)e2;
}

void* welement_fromStringInt( const char* string ) {
	assert( string );

	return (void*)strtol( string, NULL, 0 );
}

char* welement_toStringInt( const void* element ) {
	char* string = NULL;
    return asprintf( &string, "%ld", (long)element ) >= 0 ? string : strdup( "" );
}

const WType* wtypeInt = &(WType) {
	.clone = welement_cloneInt,
	.delete = welement_deleteInt,
	.compare = welement_compareInt,
	.fromString = welement_fromStringInt,
	.toString = welement_toStringInt
};

//---------------------------------------------------------------------------------
//	char* type
//---------------------------------------------------------------------------------

void* welement_cloneStr( const void* element ) {
	return element ? strdup( element ) : NULL;
}

void welement_delete( void** wtypePtr ) {
	if ( not wtypePtr ) return;

	free( *wtypePtr );
	*wtypePtr = NULL;
}

int welement_compareStr( const void* e1, const void* e2 ) {
	if ( e1 and e2 ) return strcmp( e1, e2 );
	if ( not e1 and not e2 ) return 0;
	return e1 ? 1 : -1;
}

void* welement_fromStringStr( const char* string ) {
	return string ? strdup( string ) : strdup( "" );
}

char* welement_toStringStr( const void* element ) {
	return element ? strdup( element ) : strdup( "" );
}

const WType* wtypeStr = &(WType) {
	.clone = welement_cloneStr,
	.delete = welement_delete,
	.compare = welement_compareStr,
	.fromString = welement_fromStringStr,
	.toString = welement_toStringStr
};

//---------------------------------------------------------------------------------
//	double type
//---------------------------------------------------------------------------------

void* welement_cloneDouble( const void* element ) {
	double* clone = xmalloc( sizeof( double ));
	*clone = *(double*)element;
	return clone;
}

int welement_compareDouble( const void* e1, const void* e2 ) {
	return (e1 and e2) ? *(double*)e1 - *(double*)e2 :
			e1 ? +1 : -1;	//NULL values are considered to be less than every double value
}

void* welement_fromStringDouble( const char* string ) {
	assert( string );

	double* element = xmalloc( sizeof( double ));
	*element = strtod( string, NULL );
	return element;
}

char* welement_toStringDouble( const void* element ) {
	char* string = NULL;
    return not element ?									strdup("") :	//NULL element
		asprintf( &string, "%lf", *(double*)element ) < 0 ?	strdup("") :	//Problems in stringifying double
															string;			//Actual double
}

const WType* wtypeDouble = &(WType) {
	.clone = welement_cloneDouble,
	.delete = welement_delete,
	.compare = welement_compareDouble,
	.fromString = welement_fromStringDouble,
	.toString = welement_toStringDouble
};

#if 0

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
#endif // 0

//---------------------------------------------------------------------------------
//	Condition functions
//---------------------------------------------------------------------------------

bool welement_conditionStrEquals( const void* e1, const void* e2 ) {
	if ( e1 and e2 ) return strcmp( e1, e2 ) == 0;
	return e1 == e2;
}

bool welement_conditionStrEmpty( const void* element, const void* conditionData ) {
	(void)conditionData;
	if ( element ) return ((char*)element)[0] == 0;
	return true;
}

//---------------------------------------------------------------------------------
//	Foreach functions
//---------------------------------------------------------------------------------

void welement_foreachStrPrint( const void* element, const void* foreachData ) {
	printf( "%s%s", (char*)element, foreachData ? (char*)foreachData : "" );
}

void welement_foreachIndexStrPrint( const void* element, size_t index, const void* foreachData ) {
	printf( "%u/%s%s", index, (char*)element, foreachData ? (char*)foreachData : "" );
}

//---------------------------------------------------------------------------------
//	Iterator functions
//---------------------------------------------------------------------------------

void
witerator_delete( WIterator** iterator )
{
	if ( not iterator ) return;

	free( *iterator );
	*iterator = NULL;
}

//---------------------------------------------------------------------------------
