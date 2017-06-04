/*	Small demo addressbook program demonstrating the use of a user-defined element WType. This
	is very basic and especially lacks proper error handling here and there.

	The user can pass the filename of a text addressbook file. Then he can give the command
	"add", "remove", "find" and "save". The command syntax is very basic and strict. A sampl
	transcript follows:

	>add, Michael, Nau, Highway, 18, 12345, Sampletown
	>save
	>add, Luise, Johnson, Lowstreet, 4, 54321, Examplecity
	>save
	>find, Nau
	Michael, Nau, Highway, 18, 12345, Sampletown
	>find, Luise
	Luise, Johnson, Lowstreet, 4, 54321, Examplecity
	>exit

	Compile e.g. with gcc -std=c11 addressbook.c warray.c wcollection.c -o addressbook
	and test with ./addressbook ./addressbook.txt.
*/
#include "warray_sugar.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

//---------------------------------------------------------------------------------
//	Prototypes
//---------------------------------------------------------------------------------

void
addressbookAdd( WArray* addressbook, const char* command );

void
addressbookRemove( WArray* addressbook, const char* command );

void
addressbookFind( WArray* addressbook, const char* command );

WArray*
addressbookLoad( const char* filename );

void
addressbookSave( WArray* addressbook, const char* filename );

void
error( const char* text );

char*
readFile( const char* filePath );

int
writeFile( const char* filename, const char* string );

char*
prompt( const char* text );

//---------------------------------------------------------------------------------
//
//---------------------------------------------------------------------------------

WArrayNamespace a = warrayNamespace;

//---------------------------------------------------------------------------------
//	Person struct, workhorse type and its functions
//---------------------------------------------------------------------------------

//This struct is the central data structure to be stored in workhorse arrays.
typedef struct Person {
	char*		firstName;
	char*		name;
	char*		street;
	unsigned	number;
	unsigned	zipCode;
	char*		city;
}Person;

//Now we need some functions to clone, delete, stringify and destringify a person.
Person*
personClone( const Person* person ) {
	return __wxnew( Person,
		.firstName	= __wstr_dup( person->firstName ),
		.name		= __wstr_dup( person->name ),
		.street		= __wstr_dup( person->street ),
		.number		= person->number,
		.zipCode	= person->zipCode,
		.city		= __wstr_dup( person->city ),
	);
}

void
personDelete( Person** personPtr ) {
	if ( !personPtr && !*personPtr ) return;
	Person* person = *personPtr;

	free( person->firstName );
	free( person->name );
	free( person->street );
	free( person->city );

	free( person );
	*personPtr = NULL;
}

char*
personToString( const Person* person ) {
	return __wstr_printf( "%s, %s, %s, %u, %u, %s",
		person->firstName,
		person->name,
		person->street,
		person->number,
		person->zipCode,
		person->city
	);
}

Person*
personFromString( const char* str ) {
	assert( str );
	autoWArray* tokens = a.fromString( str, ", ", wtypeStr );
	assert( a.size( tokens ) == 6 );

	return __wxnew( Person,
		.firstName	= a.stealFirst( tokens ),
		.name 		= a.stealFirst( tokens ),
		.street		= a.stealFirst( tokens ),
		.number		= atoi( a.stealFirst( tokens )),
		.zipCode	= atoi( a.stealFirst( tokens )),
		.city		= a.stealFirst( tokens ),
	);
}

static int
personMatches( const char* key, const Person* person ) {
	if ( strstr( person->firstName, key ) != NULL ||
		strstr( person->name, key ) != NULL ||
		strstr( person->street, key ) != NULL ||
		strstr( person->city, key ) != NULL ||
		person->number == atoi( key ) ||
		person->zipCode == atoi( key ))
		return 0;

	return -1;
}

//Here we define our element WType enabling the workhorse arrays to correctly
//handle our Person elements.
WType typePerson = {
	.clone = (WElementClone*)personClone,
	.delete = (WElementDelete*)personDelete,
	.toString = (WElementToString*)personToString,
	.fromString = (WElementFromString*)personFromString
};

//---------------------------------------------------------------------------------
//	Main logic
//---------------------------------------------------------------------------------

int main( int argc, char* argv[] )
{
	if ( argc == 1 )
		error( "No addressbook file given." );

	WArray* addressbook = addressbookLoad( argv[1] );

	char* command;
	while (1) {
		command = prompt( ">" );
		if ( strstr( command, "add" ) == command )
			addressbookAdd( addressbook, command );
		else
		if ( strstr( command, "remove" ) == command )
			addressbookRemove( addressbook, command );
		else
		if ( strstr( command, "find" ) == command )
			addressbookFind( addressbook, command );
		else
		if ( strstr( command, "save" ) == command )
			addressbookSave( addressbook, argv[1] );
		else
		if ( strstr( command, "exit" ) == command )
			break;

		free( command );
	}

	addressbookSave( addressbook, argv[1] );
	free( command );
	a.delete( &addressbook );
}

//---------------------------------------------------------------------------------
//	Addressbook functions
//---------------------------------------------------------------------------------

void
addressbookAdd( WArray* addressbook, const char* command )
{
	assert( addressbook );
	assert( command );

	autoWArray* tokens = a.fromString( command, ", ", wtypeStr );
	assert( a.size( tokens ) == 7 );

	Person person = {
		.firstName	= (char*)a.at( tokens, 1 ),
		.name		= (char*)a.at( tokens, 2 ),
		.street		= (char*)a.at( tokens, 3 ),
		.number		= atoi( a.at( tokens, 4 )),
		.zipCode	= atoi( a.at( tokens, 5 )),
		.city		= (char*)a.at( tokens, 6 ),
	};

	a.append( addressbook, &person );
}

void
addressbookRemove( WArray* addressbook, const char* command )
{
	assert( addressbook );
	assert( command );

	autoWArray* tokens = a.fromString( command, ", ", wtypeStr );
	assert( a.size( tokens ) == 2 );

	ssize_t position = a.search( addressbook, (WElementCompare*)personMatches, a.at( tokens, 1 ));
	if ( position >= 0 )
		a.removeAt( addressbook, position );
}

void
addressbookFind( WArray* addressbook, const char* command )
{
	assert( addressbook );
	assert( command );

	autoWArray* tokens = a.fromString( command, ", ", wtypeStr );
	assert( a.size( tokens ) == 2 );

	ssize_t position = a.search( addressbook, (WElementCompare*)personMatches, a.at( tokens, 1 ));
	if ( position >= 0 )
		puts( personToString( a.at( addressbook, position )));
	else
		puts( "Person not found." );
}

WArray*
addressbookLoad( const char* filename )
{
	assert( filename );

	char* addresses = readFile( filename );
	if ( !addresses )
		error( "Could not read address book." );

	WArray* addressbook = a.fromString( addresses, "\n", &typePerson );
	if ( !addressbook )
		error( "Invalid data in address book." );

	free( addresses );
	return addressbook;
}

void
addressbookSave( WArray* addressbook, const char* filename )
{
	assert( addressbook );
	assert( filename );

	char* addresses = a.toString( addressbook, "\n" );
	if ( !addresses )
		error( "Invalid data in address book." );

	int result = writeFile( filename, addresses );
	if ( result < 0 )
		error( "Could not write address book." );

	free( addresses );
}

//---------------------------------------------------------------------------------
//	Helper functions
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

//Helper function to show a command prompt and read in a command.
char*
prompt( const char* displayText )
{
	enum { bufferLen = 1000 };
	char buffer[bufferLen];

	fputs( displayText, stdout );
	char* string = fgets( buffer, bufferLen, stdin );
	string[strlen( string )-1] = 0;						//Remove trailing newline
	if ( string )
		return __wstr_dup( string );
	else
		return NULL;
}

//---------------------------------------------------------------------------------

