#include "warray.h"		//warray_xyz()
#include <assert.h>		//assert()
#include <stdio.h>		//fread(), fwrite(), fprintf(), fclose()
#include <stdlib.h>		//free(), exit()
#include <string.h>		//strcpy(), strrchr()
#include <sys/stat.h>	//POSIX stat() used for readFile()

//---------------------------------------------------------------------------------
//	The necessary prototypes
//---------------------------------------------------------------------------------

char*
ini2json( const char* ini );

char*
readFile( const char* filePath );

int
writeFile( const char* filename, const char* string );

void
error( const char* text );

//---------------------------------------------------------------------------------
//	The simple main logic
//---------------------------------------------------------------------------------

int
main( int argc, char* argv[argc+1] )
{
	//Read the given ini file.
	const char* inifilename = argv[1];
	if ( !inifilename || !inifilename[0] )
		error( "Need a ini filename." );
	char* ini = readFile( inifilename );
	if ( !ini )
		error( "Error in reading ini file." );

	//Convert the ini string to a json string.
	char* json = ini2json( ini );
	free( ini );
	if ( !json )
		error( "Error in converting ini file." );

	//Write json to a file.
	char* jsonfilename = __wstr_printf( "%s.json", inifilename );
	int result = writeFile( jsonfilename, json );

	//And clean up.
	free( json );
	free( jsonfilename );
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
reducekeyValuePairs( const void* element, const void* intermediate )
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
	WArray* keyValuePairs = warray_map( lines, line2keyValuePair, NULL, wtypeArray );

	//"{
	//	"key1": "value1",
	//	"key2": "value2",
	//	...,
	//"
	char* json = warray_reduce( keyValuePairs, reducekeyValuePairs, "{\n", wtypeStr );

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

//Helper function to read a file into an allocated char* buffer.
char*
readFile( const char* filePath )
{
	assert( filePath );

	//Open the file.
	FILE* file = fopen( filePath, "r" );
	if ( !file ) return NULL;

	//Get the file size.
	struct stat fileData;
	if ( stat( filePath, &fileData ) < 0 ) { fclose( file ); return NULL; }
	size_t size = fileData.st_size;

	//Allocate and read into the buffer.
	char* buffer = __wxmalloc( size + 1 );
	if ( fread( buffer, 1, size, file ) != size ) { fclose( file ); free( buffer ); return NULL; }
	buffer[ size ] = 0;

	fclose( file );

	assert( buffer );
	return buffer;
}

//Helper function to write a file from a char* buffer.
int
writeFile( const char* filename, const char* string )
{
	FILE* file = fopen( filename, "w" );
	if ( !file )
		return -1;

	if ( fputs( string, file ) < 0 ) {
		if ( fclose( file ) < 0 )
			return -3;

		return -2;
	}

	if ( fclose( file ) < 0 )
		return -4;

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
