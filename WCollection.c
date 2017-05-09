#define _POSIX_C_SOURCE 200809L
#include "WCollection.h"

#include <assert.h>		//assert()
#include <iso646.h>		//and, or, not
#include <string.h>
#include <stdarg.h>		//va_copy() etc.
#include <stdio.h>
#include <stdlib.h>

//TODO: Remove dependency from _strdup()

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

//Taken from ccan/asprintf, then modified
static int
vasprintf( char **strp, const char *fmt, va_list ap )
{
	int len;
	va_list ap_copy;

	va_copy( ap_copy, ap );
	len = vsnprintf( NULL, 0, fmt, ap_copy );
	va_end( ap_copy );

	if ( len < 0 )
		return -1;

	*strp = malloc( len+1 );
	if ( !*strp )
		return -1;

	return vsprintf( *strp, fmt, ap );
}

//Taken from ccan/asprintf, then modified
static char*
str_printf( const char *fmt, ... )
{
	va_list ap;
	char* ptr;

	va_start( ap, fmt );
	if ( vasprintf( &ptr, fmt, ap ) < 0 )
		abort();
	va_end( ap );

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
    return str_printf( "%ld", (long)element );
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
    return element ?
		str_printf( "%lf", *(double*)element ) :
		strdup("");
}

const WType* wtypeDouble = &(WType) {
	.clone = welement_cloneDouble,
	.delete = welement_delete,
	.compare = welement_compareDouble,
	.fromString = welement_fromStringDouble,
	.toString = welement_toStringDouble
};

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
