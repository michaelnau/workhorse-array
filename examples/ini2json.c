/*	Test program converting a key-value ini file from stdin to a corresponding JSON file at stdout.
	It demonstrates the use of the warray_fromString(), warray_map() and warray_reduce() functions.

	Compile e.g. with gcc -std=c11 ini2json.c warray.c wcollection.c -o ./ini2json
	and test with cat test.ini | ./ini2json > test.json
*/
#include "warray.h"		//warray_xyz()
#include <assert.h>		//assert()
#include <stdio.h>		//fread(), fprintf(), fclose()
#include <stdlib.h>		//free(), exit()
#include <string.h>		//strcpy(), strrchr()

//---------------------------------------------------------------------------------
//	The necessary prototypes
//---------------------------------------------------------------------------------

char*
ini2json( const char* ini );

char*
readStdin();

int
writeStdout( const char* string );

void
error( const char* text );

//---------------------------------------------------------------------------------
//	The simple main logic
//---------------------------------------------------------------------------------

int main()
{
	//Read an ini file from stdin.
	char* ini = readStdin();
	if ( !ini )
		error( "Error in reading ini file from stdin." );

	//Convert the ini string to a json string.
	char* json = ini2json( ini );
	free( ini );
	if ( !json )
		error( "Error in converting ini file." );

	//Write the json file to stdout.
	int result = writeStdout( json );

	//And clean up.
	free( json );
	if ( result < 0 )
		error( "Error in writing json file." );
}

//---------------------------------------------------------------------------------
//	Here is where the WArray functions do the actual work.
//---------------------------------------------------------------------------------

/*	Split a key value pair in an array of two elements.
	"key=value" ==> ["key","value"]
*/
static void*
line2keyValuePair( const void* element, const void* unused )
{
	(void)unused;
	assert( element );
	return warray_fromString( element, "=", wtypeStr );
}

/*	Append the key and value from an array to the intermediate json string.
		intermediate=
		"{
			"key1": "value1",
		"
	+	element = ["key2", "value2"]
	==>	"{
			"key1": "value1",
			"key2": "value2",
		"


*/
static void*
appendKeyValuePairToJson( const void* element, const void* intermediate )
{
	const WArray* pair = element;
	return __wstr_printf( "%s\t\"%s\": \"%s\",\n", intermediate, warray_at( pair, 0 ), warray_at( pair, 1 ));
}

/*	Convert a line-oriented ini string to a json string:

		key1=value1
		key2=value2
		key3=value3
		...
	==>
		{
			"key1": "value1",
			"key2": "value2",
			"key3": "value3",
			...
		}

	by splitting the string in an array and using warray_map() and warray_reduce() calls.
*/
char*
ini2json( const char ini[] )
{
	assert( ini );

	//Split the string at line ends and create an array of char* elements: ["key1=value1", "key2=value2",... ]
	WArray* lines = warray_fromString( ini, "\n", wtypeStr );

	//Map it to an array with elements like this: [[key1, value1], [key2, values], ...]
	WArray* keyValuePairs = warray_map(	//Map
		lines,							//the array with key-value string elements
		line2keyValuePair,				//applying this map function to every array element
		NULL,							//without additional data to the callback function
		wtypeArray						//and the hint on the result type (a WArray of WArrays).
	);

	/*"	{
			"key1": "value1",
			"key2": "value2",
			...,
		"
	*/
	char* json = warray_reduce(		//Reduce
		keyValuePairs,				//the array with key-value pairs
		appendKeyValuePairToJson,	//applying this function to every key-value pair
		"{\n",						//with this as start for the json string
		wtypeStr					//and this hint about the intermediate and result type (char*).
	);

	//Now we only have to replace the last ",\n" with a "\n}" and we are done.
	char* jsonEnd = strrchr( json, ',' );
	if ( jsonEnd ) strcpy( jsonEnd, "\n}" );

	warray_delete( &lines );
	warray_delete( &keyValuePairs );

	assert( json );
	return json;
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
	return string;
}

int
writeStdout( const char* string )
{
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
