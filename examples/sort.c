/*	Test program sorting a text file given on stdin by line and outputting it to stdout.
	It demonstrates the use of some warray_xyz() functions and the autoWArray* and WArrayNamespace features.

	Compile e.g. with gcc -gnu=c11 sort.c warray.c wcollection.c -o ./sort
	and test with cat unsorted.txt | ./sort > sorted.txt
*/
#include "warray_sugar.h"	//autoWArray, WArrayNamespace
#include <assert.h>			//assert()
#include <stdio.h>			//fread(), fprintf(), fclose()
#include <stdlib.h>			//free(), exit()
#include <string.h>			//strcpy(), strrchr()

//---------------------------------------------------------------------------------
//	The necessary prototypes
//---------------------------------------------------------------------------------

char*
sort( const char text[] );

char*
readStdin();

int
writeStdout( const char string[] );

void
error( const char text[] );

//---------------------------------------------------------------------------------
//	The simple main logic
//---------------------------------------------------------------------------------

//Create a namespace for shorter function calls.
WArrayNamespace a = warrayNamespace;

int main()
{
	//Read a text file from stdin.
	char* unsorted = readStdin();
	if ( !unsorted )
		error( "Error in reading text file from stdin." );

	//Sort by line.
	char* sorted = sort( unsorted );
	free( unsorted );

	//Write it to stdout.
	int result = writeStdout( sorted );

	//And clean up.
	free( sorted );
	if ( result < 0 )
		error( "Error in writing sorted text file." );
}

//---------------------------------------------------------------------------------
//	Here is where the WArray functions do the actual work.
//---------------------------------------------------------------------------------

/*
*/
char*
sort( const char text[] )
{
	assert( text );

	//Split the string at line ends and create an array of char* elements. Mark the lines array
	//to be autodestroyed when leaving scope. We use the namespace variable "a" defined above.
	//It is equivalent to writing warray_fromString( text, "\n", wtypeStr ).
	autoWArray* lines = a.fromString( text, "\n", wtypeStr );

	//Sort it and convert it back to a string.
	a.sort( lines );
	char* sorted = a.toString( lines, "\n" );

	assert( sorted );
	return sorted;
}

//---------------------------------------------------------------------------------
//	Some needed helper functions.
//---------------------------------------------------------------------------------

char*
readStdin()
{
	enum { bufferSize = 256 };
	char buffer[bufferSize];

	size_t stringCapacity = 2 * bufferSize;
	size_t stringIndex = 0;
	char* string = __wxmalloc( stringCapacity );

	while ( true ) {
		size_t size = fread( buffer, 1, bufferSize, stdin );
		if ( !size ) break;

		memcpy( &string[stringIndex], buffer, size );
		stringIndex += size;

		if ( stringIndex >= stringCapacity ) {
			stringCapacity *= 2;
			string = __wxrealloc( string, stringCapacity );
		}
	}

	string[stringIndex] = 0;

	assert( string );
	return string;
}

int
writeStdout( const char string[] )
{
	assert( string );

	if ( fputs( string, stdout ) < 0 )
		return -1;

	return 0;
}

//Helper function to exit with an error message.
void
error( const char* text )
{
	fprintf( stderr, "%s\n", text );
	exit( EXIT_FAILURE );
}

//---------------------------------------------------------------------------------
