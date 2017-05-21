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
#include <assert.h>		//assert()
#include <iso646.h>		//and, or, not
#include <string.h>
#include <stdarg.h>		//va_copy() etc.
#include <stdio.h>
#include <stdlib.h>

//---------------------------------------------------------------------------------
//	Helper functions
//---------------------------------------------------------------------------------

void
__wdie( const char* text )
{
	fputs( text, stderr );
	abort();
}

void*
__wxmalloc( size_t size )
{
	void* ptr = malloc( size );
	if ( not ptr ) __wdie( "Out of memory." );

	return ptr;
}

void*
__wxrealloc( void* pointer, size_t size )
{
	void* ptr = realloc( pointer, size );
	if ( !ptr ) __wdie( "Out of memory." );

	return ptr;
}

//Taken from ccan/asprintf (MIT license), then modified
char*
__wstr_printf( const char* fmt, ... )
{
	va_list ap, ap_copy;
	va_start( ap, fmt );
	va_copy( ap_copy, ap );

	int len = vsnprintf( NULL, 0, fmt, ap_copy );
	if ( len >= 0 ) {
		char* string = __wxmalloc( len+1 );
		if ( vsprintf( string, fmt, ap ) >= 0 ) {
			va_end( ap_copy );
			va_end( ap );
			assert( string );
			return string;
		}
	}

	__wdie( "Out of memory." );
	return NULL;
}

char*
__wstr_dup( const char* string )
{
	if ( not string ) string = "";

	size_t size = strlen( string ) + 1;
	char* copy = __wxmalloc( size );
    memcpy( copy, string, size );

    assert( copy );
    return copy;
}

//---------------------------------------------------------------------------------

static char welementNotFound;
const void* WElementNotFound = &welementNotFound;

//---------------------------------------------------------------------------------
//	Raw void* type
//---------------------------------------------------------------------------------

void* wtypePtr_clone( const void* element ) {
	return (void*)element;
}

void wtypePtr_delete( void** wtypePtr ) {
	(void)wtypePtr;
}

int wtypePtr_compare( const void* e1, const void* e2 ) {
	return (long)e1 - (long)e2;
}

const WType* wtypePtr = &(WType) {
	.clone = wtypePtr_clone,
	.delete = wtypePtr_delete,
	.compare = wtypePtr_compare,
};

//---------------------------------------------------------------------------------
//	int type
//---------------------------------------------------------------------------------

void* wtypeInt_clone( const void* element ) {
	return (void*)element;
}

void wtypeInt_delete( void** wtypePtr ) {
	(void)wtypePtr;
}

int wtypeInt_compare( const void* e1, const void* e2 ) {
	return (long)e1 - (long)e2;
}

void* wtypeInt_fromString( const char* string ) {
	assert( string );

	return (void*)strtol( string, NULL, 0 );
}

char* wtypeInt_toString( const void* element ) {
    assert( element );
    return __wstr_printf( "%ld", (long)element );
}

const WType* wtypeInt = &(WType) {
	.clone = wtypeInt_clone,
	.delete = wtypeInt_delete,
	.compare = wtypeInt_compare,
	.fromString = wtypeInt_fromString,
	.toString = wtypeInt_toString
};

//---------------------------------------------------------------------------------
//	char* type
//---------------------------------------------------------------------------------

void* wtypeStr_clone( const void* element ) {
	return __wstr_dup( element );
}

void wtype_delete( void** wtypePtr ) {
	free( *wtypePtr );
	*wtypePtr = NULL;
}

int wtypeStr_compare( const void* e1, const void* e2 ) {
	if ( e1 and e2 ) return strcmp( e1, e2 );
	if ( not e1 and not e2 ) return 0;
	return e1 ? 1 : -1;
}

void* wtypeStr_fromString( const char* string ) {
	return __wstr_dup( string );
}

char* wtypeStr_toString( const void* element ) {
    assert( element );
	return __wstr_dup( element );
}

const WType* wtypeStr = &(WType) {
	.clone = wtypeStr_clone,
	.delete = wtype_delete,
	.compare = wtypeStr_compare,
	.fromString = wtypeStr_fromString,
	.toString = wtypeStr_toString
};

//---------------------------------------------------------------------------------
//	double type
//---------------------------------------------------------------------------------

void* wtypeDouble_clone( const void* element ) {
	double* clone = __wxmalloc( sizeof( double ));
	*clone = *(double*)element;
	return clone;
}

int wtypeDouble_compare( const void* e1, const void* e2 ) {
	return (e1 and e2) ? *(double*)e1 - *(double*)e2 :
			e1 ? +1 : -1;	//NULL values are considered to be less than every double value
}

void* wtypeDouble_fromString( const char* string ) {
	assert( string );

	double* element = __wxmalloc( sizeof( double ));
	*element = strtod( string, NULL );
	return element;
}

char* wtypeDouble_toString( const void* element ) {
    assert( element );
    return __wstr_printf( "%lf", *(double*)element );
}

const WType* wtypeDouble = &(WType) {
	.clone = wtypeDouble_clone,
	.delete = wtype_delete,
	.compare = wtypeDouble_compare,
	.fromString = wtypeDouble_fromString,
	.toString = wtypeDouble_toString
};

//---------------------------------------------------------------------------------
//	Condition functions
//---------------------------------------------------------------------------------

bool wtypeStr_conditionEquals( const void* e1, const void* e2 ) {
	if ( e1 and e2 ) return strcmp( e1, e2 ) == 0;
	return e1 == e2;
}

bool wtypeStr_conditionEmpty( const void* element, const void* conditionData ) {
	(void)conditionData;
	if ( element ) return ((char*)element)[0] == 0;
	return true;
}

//---------------------------------------------------------------------------------
//	Foreach functions
//---------------------------------------------------------------------------------

void wtypeStr_foreachPrint( const void* element, const void* foreachData ) {
	printf( "%s%s", (char*)element, foreachData ? (char*)foreachData : "" );
}

void wtypeStr_foreachIndexPrint( const void* element, size_t index, const void* foreachData ) {
	printf( "%u/%s%s", index, (char*)element, foreachData ? (char*)foreachData : "" );
}

//---------------------------------------------------------------------------------
