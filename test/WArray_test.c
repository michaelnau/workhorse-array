#define _GNU_SOURCE
#include "WArray_sugar.h"
#define TEST_IMPLEMENTATION
#include "Testing.h"
#include <assert.h>
#include <iso646.h>
#include <stdarg.h>
#include <stdlib.h>

//--------------------------------------------------------------------------------

#define autoChar __attribute__(( cleanup( str_delete ))) char

static char*
str_printf( const char* format, ... )
{
	char* string;

    va_list args;
    va_start( args, format );
	vasprintf( &string, format, args );
	va_end( args );

	return string;
}

static void
str_delete( char** string )
{
	if ( not string ) return;

	free( *string );
	*string = NULL;
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

#define xnew( type, ... )							\
	memcpy( xmalloc( sizeof( type )), &(type){ __VA_ARGS__ }, sizeof(type) )

//--------------------------------------------------------------------------------

WArrayNamespace a = warrayNamespace;
WIteratorNamespace it = witeratorNamespace;

//--------------------------------------------------------------------------------
typedef struct Person {
	char*	name;
	char*	firstname;
	size_t	age;
}Person;

Person*
clonePerson( const Person* person ) {
	assert( person );

	return xnew( Person,
		.name = strdup( person->name ),
		.firstname = strdup( person->firstname ),
		.age = person->age,
	);
}

void
deletePerson( void** element ) {
	if ( not element ) return;

    Person* person = *element;
	free( person->name );
	free( person->firstname );
	free( person );
	*element = NULL;
}

WType* personType = &(WType){
    .clone = (WElementClone*)clonePerson,
    .delete = (WElementDelete*)deletePerson
};

//--------------------------------------------------------------------------------

void
Test_warray_clone_ints()
{
	autoWArray *array = warray_new( 0, wtypeInt );
	autoWArray *clone1 = warray_clone( array );
	assert_true( warray_empty( clone1 ));

	warray_append( array, (void*)1 );
	autoWArray *clone2 = warray_clone( array );
	assert_equal( warray_at( clone2, 0 ), 1 );
	assert_true( warray_nonEmpty( clone2 ));
	assert_equal( warray_size( clone2 ), 1 );

	warray_append( array, (void*)2 );
	warray_append( array, (void*)3 );
	warray_append( array, (void*)4 );
	warray_append( array, (void*)5 );
	autoWArray *clone3 = warray_clone( array );
	assert_equal( warray_at( clone3, 0 ), 1 );
	assert_equal( warray_at( clone3, 1 ), 2 );
	assert_equal( warray_at( clone3, 2 ), 3 );
	assert_equal( warray_at( clone3, 3 ), 4 );
	assert_equal( warray_at( clone3, 4 ), 5 );
	assert_equal( warray_size( clone3 ), 5 );
}
void
Test_warray_clone_strings()
{
	autoWArray *array = warray_new( 0, wtypeStr );
	autoWArray *clone1 = warray_clone( array );
	assert_true( warray_empty( clone1 ));

	warray_append( array, "cat" );
	autoWArray *clone2 = warray_clone( array );
	assert_strequal( warray_at( clone2, 0 ), "cat" );
	assert_equal( warray_size( clone2 ), 1 );

	warray_append( array, "dog" );
	warray_append( array, "bird" );
	warray_append( array, "hawk" );
	autoWArray *clone3 = warray_clone( array );
	assert_strequal( warray_at( clone3, 0 ), "cat" );
	assert_strequal( warray_at( clone3, 1 ), "dog" );
	assert_strequal( warray_at( clone3, 2 ), "bird" );
	assert_strequal( warray_at( clone3, 3 ), "hawk" );
	assert_equal( warray_size( clone3 ), 4 );
}

//--------------------------------------------------------------------------------

void
Test_warray_append_ints()
{
	autoWArray *array = warray_new( 0, wtypeInt );
	assert_equal( warray_size( array ), 0 );

	warray_append( array, (void*)1 );
	assert_equal( warray_at( array, 0 ), 1 );
	assert_equal( warray_size( array ), 1 );

	warray_append( array, (void*)2 );
	assert_equal( warray_at( array, 0 ), 1 );
	assert_equal( warray_at( array, 1 ), 2 );
	assert_equal( warray_size( array ), 2 );

	warray_append( array, (void*)3 );
	warray_append( array, (void*)4 );

	warray_append( array, (void*)5 );
	assert_equal( warray_at( array, 0 ), 1 );
	assert_equal( warray_at( array, 1 ), 2 );
	assert_equal( warray_at( array, 2 ), 3 );
	assert_equal( warray_at( array, 3 ), 4 );
	assert_equal( warray_at( array, 4 ), 5 );
	assert_equal( warray_size( array ), 5 );
}
void
Test_warray_append_strings()
{
	autoWArray *array = a.new( 0, wtypeStr );

	warray_append( array, "cat" );
	const char* string1 = warray_at( array, 0 );
	assert_strequal( string1, "cat" );
	assert_equal( warray_size( array ), 1 );

	warray_append( array, "cow" );
	warray_append( array, "dog" );
	warray_append( array, "horse" );
	const char* string2 = warray_at( array, 0 );
	assert_strequal( string2, "cat" );
	const char* string3 = warray_at( array, 1 );
	assert_strequal( string3, "cow" );
	const char* string4 = warray_at( array, 2 );
	assert_strequal( string4, "dog" );
	const char* string5 = warray_at( array, 3 );
	assert_strequal( string5, "horse" );
	assert_equal( warray_size( array ), 4 );
}
void
Test_warray_prepend_strings()
{
	autoWArray *array = a.new( 3, wtypeStr );

	warray_prepend( array, "cat" );
	const char*string1 = warray_at( array, 0 );
	assert_strequal( string1, "cat" );
	assert_equal( warray_size( array ), 1 );

	warray_prepend( array, "cow" );
	warray_prepend( array, "dog" );
	warray_prepend( array, "horse" );
	assert_strequal( warray_at( array, 0 ), "horse" );
	assert_strequal( warray_at( array, 1 ), "dog" );
	assert_strequal( warray_at( array, 2 ), "cow" );
	assert_strequal( warray_at( array, 3 ), "cat" );
	assert_equal( warray_size( array ), 4 );
}
void
Test_warray_insert_strings()
{
	autoWArray *array = a.new( 3, wtypeStr );

	//Insert 1st element
	warray_insert( array, 0, "cat" );
	assert_strequal( warray_at( array, 0 ), "cat" );
	assert_equal( warray_size( array ), 1 );

	//Insert before all other elements
	warray_insert( array, 0, "cow" );
	assert_strequal( warray_at( array, 0 ), "cow" );
	assert_equal( warray_size( array ), 2 );

//	//Insert as last element
	warray_insert( array, 2, "fish" );
	assert_strequal( warray_at( array, 2 ), "fish" );
	assert_equal( warray_size( array ), 3 );

//	//Insert after all elements leaving a gap
	warray_insert( array, 4, "dog" );
	warray_insert( array, 5, "wolf" );
	warray_insert( array, 8, "bird" );
	assert_strequal( warray_at( array, 0 ), "cow" );
	assert_strequal( warray_at( array, 1 ), "cat" );
	assert_strequal( warray_at( array, 2 ), "fish" );
	assert_null( warray_at( array, 3 ));
	assert_strequal( warray_at( array, 4 ), "dog" );
	assert_strequal( warray_at( array, 5 ), "wolf" );
	assert_null( warray_at( array, 6 ));
	assert_null( warray_at( array, 7 ));
	assert_strequal( warray_at( array, 8 ), "bird" );
	assert_equal( warray_size( array ), 9 );
}
void
Test_warray_insertSorted()
{
	autoWArray* array = warray_new( 0, wtypeStr );

    warray_insertSorted( array, "lion" );
    assert_equal( array->size, 1 );
	assert_strequal( a.at( array, 0 ), "lion" );

    a.insertSorted( array, "dog" );
    assert_equal( array->size, 2 );
	assert_strequal( a.at( array, 0 ), "dog" );
	assert_strequal( a.at( array, 1 ), "lion" );

    a.insertSorted( array, "elephant" );
    assert_equal( array->size, 3 );
	assert_strequal( a.at( array, 0 ), "dog" );
	assert_strequal( a.at( array, 1 ), "elephant" );
	assert_strequal( a.at( array, 2 ), "lion" );

    a.insertSorted( array, "zebra" );
    assert_equal( array->size, 4 );
	assert_strequal( a.at( array, 0 ), "dog" );
	assert_strequal( a.at( array, 1 ), "elephant" );
	assert_strequal( a.at( array, 2 ), "lion" );
	assert_strequal( a.at( array, 3 ), "zebra" );

    a.insertSorted( array, "" );
    assert_equal( array->size, 5 );
	assert_strequal( a.at( array, 0 ), "" );
	assert_strequal( a.at( array, 1 ), "dog" );
	assert_strequal( a.at( array, 2 ), "elephant" );
	assert_strequal( a.at( array, 3 ), "lion" );
	assert_strequal( a.at( array, 4 ), "zebra" );

    a.insertSorted( array, NULL );
    assert_equal( array->size, 6 );
	assert_equal( a.at( array, 0 ), NULL );
	assert_strequal( a.at( array, 1 ), "" );
	assert_strequal( a.at( array, 2 ), "dog" );
	assert_strequal( a.at( array, 3 ), "elephant" );
	assert_strequal( a.at( array, 4 ), "lion" );
	assert_strequal( a.at( array, 5 ), "zebra" );
}
void
Test_warray_set()
{
	autoWArray *array = a.new( 0, wtypeStr );

	a.set( array, 0, "cat" );
	assert_equal( a.size( array ), 1 );
	assert_strequal( a.at( array, 0 ), "cat" );

	warray_append( array, "dog" );
	warray_append( array, "bird" );

	a.set( array, 0, "tiger" );
	assert_strequal( warray_at( array, 0 ), "tiger" );
	assert_strequal( warray_at( array, 1 ), "dog" );
	assert_strequal( warray_at( array, 2 ), "bird" );
	assert_equal( warray_size( array ), 3 );

	warray_set( array, 2, "hawk" );
	assert_equal( warray_size( array ), 3 );

	warray_set( array, 4, "mongoose" );
	assert_strequal( warray_at( array, 0 ), "tiger" );
	assert_strequal( warray_at( array, 1 ), "dog" );
	assert_strequal( warray_at( array, 2 ), "hawk" );
	assert_null( warray_at( array, 3 ));
	assert_strequal( warray_at( array, 4 ), "mongoose" );
	assert_equal( warray_size( array ), 5 );
}

//--------------------------------------------------------------------------------

void
Test_warray_append_n()
{
	autoWArray* array = a.new( 0, wtypeStr );

	warray_append_n( array, 1, (void*[]){ "cat" });
	assert_strequal( a.at( array, 0 ), "cat" );
	assert_equal( a.size( array ), 1 );

	a.append_n( array, 2, (void*[]){ "dog", "bird" });
	assert_strequal( a.at( array, 0 ), "cat" );
	assert_strequal( a.at( array, 1 ), "dog" );
	assert_strequal( a.at( array, 2 ), "bird" );
	assert_equal( a.size( array ), 3 );

	a.append_n( array, 2, (void*[]){ "", NULL });
	assert_strequal( a.at( array, 0 ), "cat" );
	assert_strequal( a.at( array, 1 ), "dog" );
	assert_strequal( a.at( array, 2 ), "bird" );
	assert_strequal( a.at( array, 3 ), "" );
	assert_equal( a.at( array, 4 ), NULL );
	assert_equal( a.size( array ), 5 );
}
void
Test_warray_prepend_n()
{
	autoWArray* array = a.new( 0, wtypeStr );

	warray_prepend_n( array, 1, (void*[]){ "cat" });
	assert_strequal( a.at( array, 0 ), "cat" );
	assert_equal( a.size( array ), 1 );

	a.prepend_n( array, 2, (void*[]){ "dog", "bird" });
	assert_strequal( a.at( array, 0 ), "dog" );
	assert_strequal( a.at( array, 1 ), "bird" );
	assert_strequal( a.at( array, 2 ), "cat" );
	assert_equal( a.size( array ), 3 );

	a.prepend_n( array, 2, (void*[]){ "", NULL });
	assert_strequal( a.at( array, 0 ), "" );
	assert_equal( a.at( array, 1 ), NULL );
	assert_strequal( a.at( array, 2 ), "dog" );
	assert_strequal( a.at( array, 3 ), "bird" );
	assert_strequal( a.at( array, 4 ), "cat" );
	assert_equal( a.size( array ), 5 );
}

//--------------------------------------------------------------------------------

void
Test_warray_firstLastEmptyNonEmpty()
{
	autoWArray *array = a.new( 0, wtypeStr );
	assert_true( warray_empty( array ));
	assert_false( warray_nonEmpty( array ));

	warray_append( array, "cat" );
	assert_false( warray_empty( array ));
	assert_true( warray_nonEmpty( array ));
	assert_strequal( warray_first( array ), "cat" );
	assert_strequal( warray_last( array ), "cat" );

	warray_append( array, "dog" );
	assert_false( warray_empty( array ));
	assert_true( warray_nonEmpty( array ));
	assert_strequal( warray_first( array ), "cat" );
	assert_strequal( warray_last( array ), "dog" );

	warray_insert( array, 15, "tiger" );
	assert_false( warray_empty( array ));
	assert_true( warray_nonEmpty( array ));
	assert_strequal( warray_first( array ), "cat" );
	assert_strequal( warray_last( array ), "tiger" );
}
void
Test_warray_steal()
{
	autoWArray *array = a.new( 0, wtypeStr );

	//Steal the only element.
	warray_append( array, "cat" );
	autoChar* cat = warray_stealAt( array, 0 );
	assert_strequal( cat, "cat" );
	assert_equal( array->size, 0 );

	//Steal the last element.
	warray_append( array, "dog" );
	warray_append( array, "tiger" );
	autoChar* tiger = warray_stealAt( array, 1 );
	assert_strequal( tiger, "tiger" );
	assert_strequal( warray_at( array, 0 ), "dog" );
	assert_equal( array->size, 1 );

	//Steal the middle element.
	warray_append( array, "bird" );
	warray_append( array, "bear" );
	autoChar* bird = warray_stealAt( array, 1 );
	assert_strequal( bird, "bird" );
	assert_strequal( warray_at( array, 0 ), "dog" );
	assert_strequal( warray_at( array, 1 ), "bear" );
	assert_equal( array->size, 2 );

	//Steal a sample element.
	warray_append( array, "hawk" );
	autoChar* sample1 = warray_stealSample( array );
	autoChar* sample2 = warray_stealSample( array );
	autoChar* sample3 = warray_stealSample( array );
	assert_true( sample1 != sample2 and sample2 != sample3 );
	assert_equal( array->size, 0 );
}
void
Test_warray_stealFirstLast()
{
	autoWArray* array = a.new( 0, wtypeStr );

	a.append( array, NULL );
	assert_equal( a.stealFirst( array ), NULL );
	a.append( array, NULL );
	assert_equal( a.stealLast( array ), NULL );

	a.append( array, "cat" );
	autoChar* str1 = a.stealFirst( array );
	assert_strequal( str1, "cat" );
	a.append( array, "cat" );
	autoChar* str2 = a.stealLast( array );
	assert_strequal( str2, "cat" );

	a.append( array, "cat" );
	a.append( array, "dog" );
	a.append( array, "snake" );
	a.append( array, "bird" );
	autoChar* str3 = a.stealFirst( array );
	assert_strequal( str3, "cat" );
	autoChar* str4 = a.stealLast( array );
	assert_strequal( str4, "bird" );
}

void
Test_warray_removeAt()
{
	autoWArray* array = a.new( 0, wtypeStr );

	//Remove the only element.
	a.append( array, "cat" );
	warray_removeAt( array, 0 );
	assert_equal( array->size, 0 );

	//Remove the last element.
	a.append( array, "dog" );
	a.append( array, "tiger" );
	a.removeAt( array, 1 );
	assert_strequal( a.at( array, 0 ), "dog" );
	assert_equal( array->size, 1 );

	//Remove the middle element.
	a.append( array, "bird" );
	a.append( array, "bear" );
	a.removeAt( array, 1 );
	assert_strequal( a.at( array, 0 ), "dog" );
	assert_strequal( a.at( array, 1 ), "bear" );
	assert_equal( array->size, 2 );

	//Remove the first element.
	a.append( array, "cat" );
	warray_removeAt( array, 0 );
	assert_strequal( a.at( array, 0 ), "bear" );
	assert_strequal( a.at( array, 1 ), "cat" );
	assert_equal( array->size, 2 );

}
void
Test_warray_removeFirst()
{
	autoWArray* array = a.new( 0, wtypeStr );

	a.append( array, "cat" );
	warray_removeFirst( array );
	assert_equal( array->size, 0 );

	a.append( array, "dog" );
	a.append( array, "tiger" );
	a.removeFirst( array );
	assert_strequal( a.at( array, 0 ), "tiger" );
	assert_equal( array->size, 1 );

	//Remove the middle element.
	a.append( array, "bird" );
	a.append( array, "bear" );
	a.removeFirst( array );
	assert_strequal( a.at( array, 0 ), "bird" );
	assert_strequal( a.at( array, 1 ), "bear" );
	assert_equal( array->size, 2 );
}
void
Test_warray_removeLast()
{
	autoWArray* array = a.new( 0, wtypeStr );

	a.append( array, "cat" );
	a.removeLast( array );
	assert_equal( array->size, 0 );

	a.append( array, "dog" );
	a.append( array, "tiger" );
	a.removeLast( array );
	assert_strequal( a.at( array, 0 ), "dog" );
	assert_equal( array->size, 1 );

	//Remove the middle element.
	a.append( array, "bird" );
	a.append( array, "bear" );
	a.removeLast( array );
	assert_strequal( a.at( array, 0 ), "dog" );
	assert_strequal( a.at( array, 1 ), "bird" );
	assert_equal( array->size, 2 );
}

//--------------------------------------------------------------------------------

static void
append( const void* element, void* data )
{
	WArray* array = data;
	warray_append( array, element );
}


void
Test_warray_foreachForeachIndex()
{
	autoWArray* array = a.new( 0, wtypeStr );
	autoWArray* copy = a.new( 0, wtypeStr );

	warray_foreach( array, append, copy );
	assert_true( warray_empty( copy ));

	warray_append( array, "cat" );
	warray_foreach( array, append, copy );
	assert_strequal( warray_first( copy ), "cat" );
	assert_equal( warray_size( copy ), 1 );

	warray_clear( copy );
	warray_append( array, "dog" );
	warray_foreach( array, append, copy );
	assert_strequal( warray_first( copy ), "cat" );
	assert_strequal( warray_at( copy, 1 ), "dog" );
	assert_equal( warray_size( copy ), 2 );

	warray_clear( copy );
	warray_append( array, "mouse" );
	warray_foreach( array, append, copy );
	assert_strequal( warray_first( copy ), "cat" );
	assert_strequal( warray_at( copy, 1 ), "dog" );
	assert_strequal( warray_at( copy, 2 ), "mouse" );
	assert_equal( warray_size( copy ), 3 );
}
void
Test_warray_filterReject()
{
	bool isLongWord( const void* element, const void* unused ) {
		(void) unused;
        return strlen( element ) > 3;
	}

	autoWArray *array = a.new( 0, wtypeStr );

	autoWArray* newArray1 = warray_filter( array, isLongWord, NULL );
	assert_equal( newArray1->size, 0 );

	autoWArray* newArray1b = warray_reject( array, isLongWord, NULL );
	assert_equal( newArray1b->size, 0 );

	warray_append( array, "cat" );
	warray_append( array, "dog" );
	warray_append( array, "sea-hawk" );
	warray_append( array, "chimpanzee" );

    autoWArray* newArray2 = warray_filter( array, isLongWord, NULL );
    assert_strequal( warray_at( newArray2, 0 ), "sea-hawk" );
    assert_strequal( warray_at( newArray2, 1 ), "chimpanzee" );
    assert_equal( newArray2->size, 2 );

    autoWArray* newArray2b = warray_reject( array, isLongWord, NULL );
    assert_strequal( warray_at( newArray2b, 0 ), "cat" );
    assert_strequal( warray_at( newArray2b, 1 ), "dog" );
    assert_equal( newArray2b->size, 2 );
}
static void*
makeItGood( const void* element, const void* mapData )
{
	return str_printf( "My %s is %s.", (char*)element, (char*)mapData );
}
void
Test_warray_map()
{

	autoWArray *array = a.new( 0, wtypeStr );

	autoWArray* newArray1 = warray_map( array, makeItGood, "good", wtypeStr );
	assert_equal( newArray1->size, 0 );

	warray_append( array, "cat" );
	warray_append( array, "dog" );
	warray_append( array, "sea-hawk" );
	warray_append( array, "chimpanzee" );

    autoWArray* newArray2 = warray_map( array, makeItGood, "good", wtypeStr );
    assert_strequal( warray_at( newArray2, 0 ), "My cat is good." );
    assert_strequal( warray_at( newArray2, 1 ), "My dog is good." );
    assert_strequal( warray_at( newArray2, 2 ), "My sea-hawk is good." );
    assert_strequal( warray_at( newArray2, 3 ), "My chimpanzee is good." );
    assert_equal( newArray2->size, 4 );
}
static void*
joinAnimals( const void* element, const void* reduction ) {
	return str_printf( "%s and %s", (char*)reduction, (char*)element );
}
void
Test_warray_reduce()
{
	autoWArray *array = a.new( 0, wtypeStr );

	autoChar* string1 = warray_reduce( array, joinAnimals, "My favorite animals are turtle", wtypeStr );
	assert_strequal( string1, "My favorite animals are turtle");

	warray_append( array, "cat" );
	warray_append( array, "dog" );
	warray_append( array, "sea-hawk" );
	warray_append( array, "chimpanzee" );

    autoChar* string2 = warray_reduce( array, joinAnimals, "My favorite animals are turtle", wtypeStr );
    assert_strequal( string2, "My favorite animals are turtle and cat and dog and sea-hawk and chimpanzee" );
}

//--------------------------------------------------------------------------------

void
Test_warray_minMax()
{
//	int compareStrings( const void* element1, const void* element2 ) {
//		return strcmp( element1, element2 );
//	}

	autoWArray *array = a.new( 0, wtypeStr );

	warray_append( array, "cat" );
	assert_strequal( warray_min( array ), "cat" );
	assert_strequal( warray_max( array ), "cat" );

	warray_append( array, "dog" );
	assert_strequal( warray_min( array ), "cat" );
	assert_strequal( warray_max( array ), "dog" );

	warray_append( array, "sea-hawk" );
	assert_strequal( warray_min( array ), "cat" );
	assert_strequal( warray_max( array ), "sea-hawk" );

	warray_append( array, "chimpanzee" );
	assert_strequal( warray_min( array ), "cat" );
	assert_strequal( warray_max( array ), "sea-hawk" );

	warray_append( array, "ape" );
	assert_strequal( warray_min( array ), "ape" );
	assert_strequal( warray_max( array ), "sea-hawk" );
}
void
Test_warray_indexRindex()
{
	autoWArray* array = a.new( 0, wtypeStr );

	assert_equal( a.index( array, NULL ), -1 );
	assert_equal( a.rindex( array, NULL ), -1 );
	assert_equal( a.index( array, "Test" ), -1 );
	assert_equal( a.rindex( array, "Test" ), -1 );

	a.append( array, "cat" );
	assert_equal( a.index( array, NULL ), -1 );
	assert_equal( a.rindex( array, NULL ), -1 );
	assert_equal( a.index( array, "" ), -1 );
	assert_equal( a.rindex( array, ""), -1 );
	assert_equal( a.index( array, "Test" ), -1 );
	assert_equal( a.rindex( array, "Test" ), -1 );
	assert_equal( a.index( array, "cat" ), 0 );
	assert_equal( a.rindex( array, "cat" ), 0 );

	a.append( array, "dog" );
	assert_equal( a.index( array, "cat" ), 0 );
	assert_equal( a.rindex( array, "cat" ), 0 );
	assert_equal( a.index( array, "dog" ), 1 );
	assert_equal( a.rindex( array, "dog" ), 1 );

	a.set( array, 5, "dog" );
	assert_equal( a.index( array, NULL ), 2 );
	assert_equal( a.rindex( array, NULL ), 4 );
	assert_equal( a.index( array, "" ), -1 );
	assert_equal( a.rindex( array, ""), -1 );
	assert_equal( a.index( array, "cat" ), 0 );
	assert_equal( a.rindex( array, "cat" ), 0 );
	assert_equal( a.index( array, "dog" ), 1 );
	assert_equal( a.rindex( array, "dog" ), 5 );

	a.set( array, 55, "" );
	assert_equal( a.index( array, NULL ), 2 );
	assert_equal( a.rindex( array, NULL ), 54 );
	assert_equal( a.index( array, "" ), 55 );
	assert_equal( a.rindex( array, ""), 55 );
	assert_equal( a.index( array, "cat" ), 0 );
	assert_equal( a.rindex( array, "cat" ), 0 );
	assert_equal( a.index( array, "dog" ), 1 );
	assert_equal( a.rindex( array, "dog" ), 5 );
}
void
Test_warray_count()
{
	autoWArray* array = a.new( 0, wtypeStr );

	assert_equal( warray_count( array, welement_conditionStrEquals, NULL ), 0 );
	assert_equal( a.count( array, welement_conditionStrEquals, "Test" ), 0 );

	a.append( array, "cat" );
	assert_equal( a.count( array, welement_conditionStrEquals, NULL ), 0 );
	assert_equal( a.count( array, welement_conditionStrEquals, "Test" ), 0 );
	assert_equal( a.count( array, welement_conditionStrEquals, "cat" ), 1 );

	a.append( array, "dog" );
	assert_equal( a.count( array, welement_conditionStrEquals, "cat" ), 1 );
	assert_equal( a.count( array, welement_conditionStrEquals, "dog" ), 1 );

	a.set( array, 5, "dog" );
	assert_equal( a.count( array, welement_conditionStrEquals, "cat" ), 1 );
	assert_equal( a.count( array, welement_conditionStrEquals, "dog" ), 2 );

	a.set( array, 55, "" );
	assert_equal( a.count( array, welement_conditionStrEquals, "cat" ), 1 );
	assert_equal( a.count( array, welement_conditionStrEquals, "dog" ), 2 );
	assert_equal( a.count( array, welement_conditionStrEquals, "" ), 1 );
}

//--------------------------------------------------------------------------------

void
Test_warray_toStringFromString()
{
	autoWArray* array = a.new( 0, wtypeStr );

	autoChar* joined1 = warray_toString( array, ", " );
	assert_strequal( joined1, "" );

	autoWArray* split1 = warray_fromString( joined1, ", " );
	assert_equal( warray_size( split1 ), 0 );

    warray_append( array, "cat" );
	autoChar* joined2 = warray_toString( array, ", " );
	assert_strequal( joined2, "cat" );

	autoWArray* split2 = warray_fromString( joined2, ", " );
	assert_strequal( warray_first( split2 ), "cat" );
	assert_equal( warray_size( split2 ), 1 );

    warray_append( array, "dog" );
	autoChar* joined3 = warray_toString( array, ", " );
	assert_strequal( joined3, "cat, dog" );

	autoWArray* split3 = warray_fromString( joined3, ", " );
	assert_strequal( warray_first( split3 ), "cat" );
	assert_strequal( warray_at( split3, 1 ), "dog" );
	assert_equal( warray_size( split3 ), 2 );

	autoWArray* split3b = warray_fromString( joined3, "/" );
	assert_strequal( warray_first( split3b ), "cat, dog" );
	assert_equal( warray_size( split3b ), 1 );

    warray_append( array, "mouse" );
	autoChar* joined4 = warray_toString( array, ", " );
	assert_strequal( joined4, "cat, dog, mouse" );

	autoWArray* split4 = warray_fromString( joined4, ", " );
	assert_strequal( warray_first( split4 ), "cat" );
	assert_strequal( warray_at( split4, 1 ), "dog" );
	assert_strequal( warray_at( split4, 2 ), "mouse" );
	assert_equal( warray_size( split4 ), 3 );

	autoWArray* split4b = warray_fromString( joined4, "," );
	assert_strequal( warray_first( split4b ), "cat" );
	assert_strequal( warray_at( split4b, 1 ), " dog" );
	assert_strequal( warray_at( split4b, 2 ), " mouse" );
	assert_equal( warray_size( split4b ), 3 );
}

//--------------------------------------------------------------------------------

void
Test_warray_allAnyOneNone()
{
	bool equals( const void* element1, const void* element2 ) {
		return strcmp( element1, element2 ) == 0;
	}

	autoWArray* array = a.new( 0, wtypeStr );

	assert_true( warray_all( array, equals, "foo" ));
	assert_false( warray_any( array, equals, "foo" ));
	assert_false( warray_one( array, equals, "foo" ));
	assert_true( warray_none( array, equals, "foo" ));

    warray_append( array, "cat" );
	assert_true( not warray_all( array, equals, "foo" ));
	assert_true( warray_all( array, equals, "cat" ));
	assert_true( not warray_any( array, equals, "foo" ));
	assert_true( warray_any( array, equals, "cat" ));
	assert_true( not warray_one( array, equals, "foo" ));
	assert_true( warray_one( array, equals, "cat" ));
	assert_true( warray_none( array, equals, "foo" ));
	assert_true( not warray_none( array, equals, "cat" ));

    warray_append( array, "cat" );
	assert_true( not warray_all( array, equals, "foo" ));
	assert_true( warray_all( array, equals, "cat" ));
	assert_true( not warray_any( array, equals, "foo" ));
	assert_true( warray_any( array, equals, "cat" ));
	assert_true( not warray_one( array, equals, "foo" ));
	assert_true( not warray_one( array, equals, "cat" ));
	assert_true( warray_none( array, equals, "foo" ));
	assert_true( not warray_none( array, equals, "cat" ));

    warray_append( array, "mouse" );
    warray_append( array, "cow" );
	assert_true( not warray_all( array, equals, "foo" ));
	assert_true( not warray_all( array, equals, "cat" ));
	assert_true( not warray_any( array, equals, "foo" ));
	assert_true( warray_any( array, equals, "mouse" ));
	assert_true( not warray_one( array, equals, "foo" ));
	assert_true( not warray_one( array, equals, "cat" ));
	assert_true( warray_one( array, equals, "mouse" ));
	assert_true( warray_one( array, equals, "cow" ));
	assert_true( warray_none( array, equals, "foo" ));
	assert_true( not warray_none( array, equals, "mouse" ));
	assert_true( not warray_none( array, equals, "cow" ));
}

//--------------------------------------------------------------------------------

void
Test_warray_sort()
{
	WArray* array = a.new( 0, wtypeStr );

	warray_sort( array );
    assert_true( warray_empty( array ));

	warray_append( array, "cat" );
	warray_sort( array );
    assert_strequal( warray_first( array ), "cat" );
    assert_equal( warray_size( array ), 1 );

	warray_append( array, "ape" );
	warray_sort( array );
    assert_strequal( warray_first( array ), "ape" );
    assert_strequal( warray_at( array, 1 ), "cat" );
    assert_equal( warray_size( array ), 2 );

	warray_append_n( array, 5, (void*[]){ "mongoose", "dolphin", "lion", "bird", "yak" });
	warray_sort( array );
    assert_strequal( warray_first( array ), "ape");
    assert_strequal( warray_at( array, 1 ), "bird");
    assert_strequal( warray_at( array, 2 ), "cat");
    assert_strequal( warray_at( array, 3 ), "dolphin");
    assert_strequal( warray_at( array, 4 ), "lion");
    assert_strequal( warray_at( array, 5 ), "mongoose");
    assert_strequal( warray_at( array, 6 ), "yak");
    assert_equal( warray_size( array ), 7 );
}
void
Test_warray_compact()
{
	autoWArray* array = a.new( 0, wtypeStr );

	warray_compact( array );
	assert_true( a.empty( array ));

	a.append( array, NULL );
	warray_compact( array );
	assert_true( a.empty( array ));

	a.set( array, 0, "cat" );
	a.compact( array );
	assert_equal( a.size( array ), 1 );
	assert_strequal( a.first( array ), "cat" );

	a.set( array, 5, "dog" );
	a.set( array, 99, "bird" );
	assert_equal( a.size( array ), 100 );
    a.compact( array );
	assert_equal( a.size( array ), 3 );
	assert_strequal( a.at( array, 0 ), "cat" );
	assert_strequal( a.at( array, 1 ), "dog" );
	assert_strequal( a.at( array, 2 ), "bird" );
}
void
Test_warray_distinct()
{
	autoWArray* array = a.new( 0, wtypeStr );

	warray_distinct( array );
	assert_true( a.empty( array ));

	a.append( array, "cat" );
	warray_distinct( array );
	assert_equal( a.size( array ), 1 );
	assert_strequal( a.first( array ), "cat" );

	a.append( array, "dog" );
	warray_distinct( array );
	assert_equal( a.size( array ), 2 );
	assert_strequal( a.at( array, 0 ), "cat" );
	assert_strequal( a.at( array, 1 ), "dog" );

	a.append( array, "dog" );
	a.append( array, "dog" );
	a.append( array, "dog" );
	warray_distinct( array );
	assert_equal( a.size( array ), 2 );
	assert_strequal( a.at( array, 0 ), "cat" );
	assert_strequal( a.at( array, 1 ), "dog" );

	a.set( array, 10, "dog" );
	warray_distinct( array );
	assert_equal( a.size( array ), 3 );
	assert_true( warray_one( array, welement_conditionStrEquals, "cat" ));
	assert_true( warray_one( array, welement_conditionStrEquals, "dog" ));
	assert_true( warray_one( array, welement_conditionStrEquals, NULL ));
}
void
Test_warray_reverse()
{
	autoWArray* array = a.new( 0, wtypeStr );

	warray_reverse( array );
	assert_true( a.empty( array ));

	a.append( array, "cat" );
	a.reverse( array );
	assert_equal( a.size( array ), 1 );
	assert_strequal( a.first( array ), "cat" );

	a.append( array, "dog" );
	a.reverse( array );
	assert_equal( a.size( array ), 2 );
	assert_strequal( a.at( array, 0 ), "dog" );
	assert_strequal( a.at( array, 1 ), "cat" );

	a.append( array, "bird" );
	a.reverse( array );
	assert_equal( a.size( array ), 3 );
	assert_strequal( a.at( array, 0 ), "bird" );
	assert_strequal( a.at( array, 1 ), "cat" );
	assert_strequal( a.at( array, 2 ), "dog" );

	a.set( array, 5, "" );
	a.reverse( array );
	assert_equal( a.size( array ), 6 );
	assert_strequal( a.at( array, 0 ), "" );
	assert_equal( a.at( array, 1 ), NULL );
	assert_equal( a.at( array, 2 ), NULL );
	assert_strequal( a.at( array, 3 ), "dog" );
	assert_strequal( a.at( array, 4 ), "cat" );
	assert_strequal( a.at( array, 5 ), "bird" );
}
void
Test_warray_concat()
{
	autoWArray* array1 = a.new( 0, wtypeStr );
	autoWArray* array2 = a.new( 0, wtypeStr );

	autoWArray* concat1 = warray_concat( warray_clone( array1 ), array2 );
    assert_true( warray_empty( concat1 ));

	warray_append( array1, "cat" );
	autoWArray* concat2 = warray_concat( warray_clone( array1 ), array2 );
	assert_strequal( warray_first( concat2 ), "cat" );
	assert_equal( concat2->size, 1 );
	autoWArray* concat3 = warray_concat( warray_clone( array2 ), array1 );
	assert_strequal( warray_first( concat3 ), "cat" );
	assert_equal( concat3->size, 1 );

	warray_append( array2, "dog" );
	autoWArray* concat4 = warray_concat( warray_clone( array1 ), array2 );
	assert_strequal( warray_at( concat4, 0 ), "cat" );
	assert_strequal( warray_at( concat4, 1 ), "dog" );
	assert_equal( concat4->size, 2 );

	warray_append( array1, "mouse" );
	warray_append( array1, "bird" );
	warray_append( array2, "dolphin" );
	warray_append( array2, "wolf" );
	autoWArray* concat5 = warray_concat( warray_clone( array1 ), array2 );
	assert_strequal( warray_at( concat5, 0 ), "cat" );
	assert_strequal( warray_at( concat5, 1 ), "mouse" );
	assert_strequal( warray_at( concat5, 2 ), "bird" );
	assert_strequal( warray_at( concat5, 3 ), "dog" );
	assert_strequal( warray_at( concat5, 4 ), "dolphin" );
	assert_strequal( warray_at( concat5, 5 ), "wolf" );
	assert_equal( concat5->size, 6 );
}

//--------------------------------------------------------------------------------

void
Test_warray_union()
{
	autoWArray* array1 = a.new( 0, wtypeStr );
	autoWArray* array2 = a.new( 0, wtypeStr );

	autoWArray* union1 = warray_unite( array1, array2 );
	assert_equal( union1->size, 0 );

	a.append( array1, "cat" );
	autoWArray* union2 = warray_unite( array1, array2 );
	assert_equal( union2->size, 1 );
	assert_strequal( a.at( union2, 0 ), "cat" );

	a.append( array2, "dog" );
	autoWArray* union3 = warray_unite( array1, array2 );
	assert_equal( union3->size, 2 );
	assert_true( a.index( union3, "cat" ) >= 0 );
	assert_true( a.index( union3, "dog" ) >= 0 );

	a.append( array1, "dog" );
	autoWArray* union4 = warray_unite( array1, array2 );
	assert_equal( union4->size, 2 );
	assert_true( a.index( union4, "cat" ) >= 0 );
	assert_true( a.index( union4, "dog" ) >= 0 );

	a.append( array2, "dog" );
	autoWArray* union5 = warray_unite( array1, array2 );
	assert_equal( union5->size, 2 );
	assert_true( a.index( union5, "cat" ) >= 0 );
	assert_true( a.index( union5, "dog" ) >= 0 );

	a.append( array2, "cat" );
	autoWArray* union6 = warray_unite( array1, array2 );
	assert_equal( union6->size, 2 );
	assert_true( a.index( union6, "cat" ) >= 0 );
	assert_true( a.index( union6, "dog" ) >= 0 );

	autoWArray* array3 = a.new( 0, wtypeStr );
	autoWArray* array4 = a.new( 0, wtypeStr );
	a.append_n( array3, 5, (void*[]){ "cat", "", "dog", "cat", "elephant" });
	a.append_n( array4, 6, (void*[]){ "bird", NULL, NULL, "dog", "bird", "mouse" });
	autoWArray* union7 = warray_unite( array3, array4 );
	assert_equal( union7->size, 7 );
	assert_true( warray_contains( union7, "cat" ));
	assert_true( warray_contains( union7, "" ));
	assert_true( a.contains( union7, "dog" ));
	assert_true( a.contains( union7, "elephant" ));
	assert_true( a.contains( union7, "bird" ));
	assert_true( a.contains( union7, NULL ));
	assert_true( a.contains( union7, "mouse" ));
}
void
Test_warray_intersect()
{
	autoWArray* array1 = a.new( 0, wtypeStr );
	autoWArray* array2 = a.new( 0, wtypeStr );

	autoWArray* inter1 = warray_intersect( array1, array2 );
	assert_equal( inter1->size, 0 );

	a.append( array1, "cat" );
	autoWArray* inter2 = a.intersect( array1, array2 );
	assert_equal( inter2->size, 0 );
	assert_equal( a.count( inter2, welement_conditionStrEquals, "cat" ), 0 );

	a.append( array2, "dog" );
	autoWArray* inter3 = a.intersect( array1, array2 );
	assert_equal( inter3->size, 0 );
	assert_equal( a.count( inter3, welement_conditionStrEquals, "cat" ), 0 );
	assert_equal( a.count( inter3, welement_conditionStrEquals, "dog" ), 0 );

	a.append( array1, "dog" );
	autoWArray* inter4 = a.intersect( array1, array2 );
	assert_equal( inter4->size, 1 );
	assert_equal( a.count( inter4, welement_conditionStrEquals, "cat" ), 0 );
	assert_equal( a.count( inter4, welement_conditionStrEquals, "dog" ), 1 );

	a.append( array2, "cat" );
	autoWArray* inter5 = a.intersect( array1, array2 );
	assert_equal( inter5->size, 2 );
	assert_equal( a.count( inter5, welement_conditionStrEquals, "cat" ), 1 );
	assert_equal( a.count( inter5, welement_conditionStrEquals, "dog" ), 1 );

	autoWArray* array3 = a.new( 0, wtypeStr );
	autoWArray* array4 = a.new( 0, wtypeStr );
	a.append_n( array3, 7, (void*[]){ "cat", "dog", NULL, NULL, "", "bird", "elephant" });
	a.append_n( array4, 6, (void*[]){ "dog", "mouse", NULL, "bird", "crocodile", "", "" });
	autoWArray* inter6 = a.intersect( array3, array4 );
	assert_equal( inter6->size, 4 );
	assert_equal( a.count( inter6, welement_conditionStrEquals, "dog" ), 1 );
	assert_equal( a.count( inter6, welement_conditionStrEquals, NULL ), 1 );
	assert_equal( a.count( inter6, welement_conditionStrEquals, "" ), 1 );
	assert_equal( a.count( inter6, welement_conditionStrEquals, "bird" ), 1 );
}
void
Test_warray_symDiff()
{
	autoWArray* array1 = a.new( 0, wtypeStr );
	autoWArray* array2 = a.new( 0, wtypeStr );

	autoWArray* sym1 = warray_symDiff( array1, array2 );
	assert_equal( sym1->size, 0 );

	a.append( array1, "cat" );
	autoWArray* sym2 = a.symDiff( array1, array2 );
	assert_equal( sym2->size, 1 );
	assert_equal( a.count( sym2, welement_conditionStrEquals, "cat" ), 1 );

	a.append( array2, "dog" );
	autoWArray* sym3 = a.symDiff( array1, array2 );
	assert_equal( sym3->size, 2 );
	assert_equal( a.count( sym3, welement_conditionStrEquals, "cat" ), 1 );
	assert_equal( a.count( sym3, welement_conditionStrEquals, "dog" ), 1 );

	a.append( array1, "dog" );
	autoWArray* sym4 = a.symDiff( array1, array2 );
	assert_equal( sym4->size, 1 );
	assert_equal( a.count( sym4, welement_conditionStrEquals, "cat" ), 1 );

	a.append( array2, "cat" );
	autoWArray* sym5 = a.symDiff( array1, array2 );
	assert_equal( sym5->size, 0 );

	autoWArray* array3 = a.new( 0, wtypeStr );
	autoWArray* array4 = a.new( 0, wtypeStr );
	a.append_n( array3, 6, (void*[]){ "cat", "dog", NULL, NULL, "bird", "elephant" });
	a.append_n( array4, 7, (void*[]){ "dog", "mouse", NULL, "bird", "crocodile", "", "" });
	autoWArray* sym6 = a.symDiff( array3, array4 );
	assert_equal( sym6->size, 5 );
	assert_equal( a.count( sym6, welement_conditionStrEquals, "cat" ), 1 );
	assert_equal( a.count( sym6, welement_conditionStrEquals, "elephant" ), 1 );
	assert_equal( a.count( sym6, welement_conditionStrEquals, "mouse" ), 1 );
	assert_equal( a.count( sym6, welement_conditionStrEquals, "crocodile" ), 1 );
	assert_equal( a.count( sym6, welement_conditionStrEquals, "" ), 1 );
}
void
Test_warray_addToSet()
{
	autoWArray* set = a.new( 0, wtypeStr );

	warray_addToSet( set, "cat" );
	assert_equal( a.count( set, welement_conditionStrEquals, "cat" ), 1 );

	warray_addToSet( set, "cat" );
	assert_equal( a.count( set, welement_conditionStrEquals, "cat" ), 1 );

	warray_addToSet( set, "cat" );
	assert_equal( a.count( set, welement_conditionStrEquals, "cat" ), 1 );

	warray_addToSet( set, "" );
	assert_equal( a.count( set, welement_conditionStrEquals, "cat" ), 1 );
	assert_equal( a.count( set, welement_conditionStrEquals, "" ), 1 );

	warray_addToSet( set, "" );
	assert_equal( a.count( set, welement_conditionStrEquals, "cat" ), 1 );
	assert_equal( a.count( set, welement_conditionStrEquals, "" ), 1 );

	warray_addToSet( set, "cat" );
	assert_equal( a.count( set, welement_conditionStrEquals, "cat" ), 1 );
	assert_equal( a.count( set, welement_conditionStrEquals, "" ), 1 );

	warray_addToSet( set, NULL );
	warray_addToSet( set, "dog" );
	warray_addToSet( set, NULL );
	warray_addToSet( set, "dog" );
	assert_equal( a.count( set, welement_conditionStrEquals, "cat" ), 1 );
	assert_equal( a.count( set, welement_conditionStrEquals, "" ), 1 );
	assert_equal( a.count( set, welement_conditionStrEquals, NULL ), 1 );
	assert_equal( a.count( set, welement_conditionStrEquals, "dog" ), 1 );
}

//--------------------------------------------------------------------------------

void
Test_warray_compare()
{
    autoWArray* array1 = a.new( 0, wtypeStr );
    autoWArray* array2 = a.new( 0, wtypeStr );

    assert_equal( warray_compare( array1, array2 ), 0 );

    a.append( array1, NULL );
    assert_equal( warray_compare( array1, array2 ), 1 );

    a.append( array2, NULL );
    assert_equal( warray_compare( array1, array2 ), 0 );

    a.append( array2, "mouse" );
    assert_equal( warray_compare( array1, array2 ), -1 );

    a.append( array1, "lion" );
    assert_equal( warray_compare( array1, array2 ), -1 );

    a.append( array1, "elephant" );
    assert_equal( warray_compare( array1, array2 ), -1 );

    a.prepend( array1, "cat" );
    a.prepend( array2, "" );
    assert_equal( a.compare( array1, array2 ), 1 );

	autoWArray* array3 = a.new( 0, wtypeStr );
	autoWArray* array4 = a.new( 0, wtypeStr );

    a.append_n( array3, 6, (void*[]){ "cat", "", NULL, "lion", "elephant", "mouse" });
    a.append_n( array4, 6, (void*[]){ "cat", "", NULL, "lion", "elephant", "snake" });
	assert_equal( a.compare( array3, array4 ), -1 );

	a.set( array4, 5, "mouse" );
	assert_equal( a.compare( array3, array4 ), 0 );

	a.append( array3, "squirrel" );
	assert_equal( a.compare( array3, array4 ), 1 );
}

static int
comparePerson( const void* key, const void* element )
{
	const Person* person = element;
	return strcmp( key, person->name );
}
void
Test_warray_bsearch()
{
	autoWArray* array1 = a.new( 0, wtypeStr );
	a.append_n( array1, 5, (void*[]){ "cat", "dog", "lion", "mouse", "zebra" });
	assert_equal( warray_bsearch( array1, welement_compareStr, "cat" ), 0 );
	assert_equal( a.bsearch( array1, welement_compareStr, "dog" ), 1 );
	assert_equal( a.bsearch( array1, welement_compareStr, "lion" ), 2 );
	assert_equal( a.bsearch( array1, welement_compareStr, "mouse" ), 3 );
	assert_equal( a.bsearch( array1, welement_compareStr, "zebra" ), 4 );
	assert_equal( a.bsearch( array1, welement_compareStr, "wolpertinger" ), -1 );

	autoWArray* array2 = a.new( 0, personType );
	a.append_n( array2, 5, (void*[]){
		&(Person){	"Abelson",	"Peter",	33	},
		&(Person){	"Johnson",	"Jack",		89	},
		&(Person){	"Johnson",	"Paul",		54	},
		&(Person){	"Smith",	"Greg",		12	},
		&(Person){	"West",		"Mitch", 	66	},
	});

	assert_equal( a.bsearch( array2, comparePerson, "Abelson" ), 0 );
	assert_equal( a.bsearch( array2, comparePerson, "Smith" ), 3 );
	assert_equal( a.bsearch( array2, comparePerson, "West" ), 4 );
	assert_equal( a.bsearch( array2, comparePerson, "Unknown" ), -1 );
}

//--------------------------------------------------------------------------------

void
Test_warray_iterator()
{
	autoWArray* ar = a.new( 0, wtypeStr );
	autoWIterator* iter1a = a.iterator( ar );
	autoWIterator* iter1b = a.iteratorReverse( ar );
	assert_false( it.hasNext( iter1a ));
	assert_false( it.hasNext( iter1b ));

	a.append( ar, "cat" );
	autoWIterator* iter2a = a.iterator( ar );
	autoWIterator* iter2b = a.iteratorReverse( ar );
	assert_true( it.hasNext( iter2a ));
	assert_true( it.hasNext( iter2b ));
	assert_strequal( it.next( iter2a ), "cat" );
	assert_strequal( it.next( iter2b ), "cat" );
	assert_false( it.hasNext( iter2a ));
	assert_false( it.hasNext( iter2b ));

	a.append( ar, "dog" );
	autoWIterator* iter3a = a.iterator( ar );
	autoWIterator* iter3b = a.iteratorReverse( ar );
	assert_true( it.hasNext( iter3a ));
	assert_true( it.hasNext( iter3b ));
	assert_strequal( it.next( iter3a ), "cat" );
	assert_strequal( it.next( iter3b ), "dog" );
	assert_true( it.hasNext( iter3a ));
	assert_true( it.hasNext( iter3b ));
	assert_strequal( it.next( iter3a ), "dog" );
	assert_strequal( it.next( iter3b ), "cat" );
	assert_false( it.hasNext( iter3a ));
	assert_false( it.hasNext( iter3b ));
}
void
Test_warray_iteratorFullExample()
{
	autoWArray* ar1 = a.new( 0, wtypeStr );
	autoWArray* ar2 = a.new( 0, wtypeStr );

	a.append_n( ar1, 3, (void*[]){ "1.", "2.", "3." });
	a.append_n( ar2, 3, (void*[]){ "cat", "dog", "lion" });
	autoWIterator* iter1 = warray_iterator( ar1 );
	autoWIterator* iter2 = warray_iterator( ar2 );

	WArray* ar3 = a.new( 0, wtypeStr );
	while ( witerator_hasNext( iter1 ) and witerator_hasNext( iter2 )) {
		a.append( ar3, witerator_next( iter1 ));
		a.append( ar3, witerator_next( iter2 ));
	}

	assert_false( witerator_hasNext( iter1 ));
	assert_false( witerator_hasNext( iter2 ));
    assert_strequal( a.at( ar3, 0 ), "1." );
    assert_strequal( a.at( ar3, 1 ), "cat" );
    assert_strequal( a.at( ar3, 2 ), "2." );
    assert_strequal( a.at( ar3, 3 ), "dog" );
    assert_strequal( a.at( ar3, 4 ), "3." );
    assert_strequal( a.at( ar3, 5 ), "lion" );
}

//--------------------------------------------------------------------------------

int main() {
	printf( "\n" );

	testsuite( Test_warray_clone_ints );
	testsuite( Test_warray_clone_strings );

	testsuite( Test_warray_append_ints );
	testsuite( Test_warray_append_strings );
	testsuite( Test_warray_prepend_strings );
	testsuite( Test_warray_insert_strings );
	testsuite( Test_warray_insertSorted );
	testsuite( Test_warray_set );
	testsuite( Test_warray_append_n );
	testsuite( Test_warray_prepend_n );

	testsuite( Test_warray_firstLastEmptyNonEmpty );
	testsuite( Test_warray_steal );
	testsuite( Test_warray_stealFirstLast );
	testsuite( Test_warray_removeAt );
	testsuite( Test_warray_removeFirst );
	testsuite( Test_warray_removeLast );
	testsuite( Test_warray_filterReject );
	testsuite( Test_warray_map );
	testsuite( Test_warray_reduce );

	testsuite( Test_warray_minMax );
	testsuite( Test_warray_indexRindex );
	testsuite( Test_warray_count );

	testsuite( Test_warray_toStringFromString );
	testsuite( Test_warray_foreachForeachIndex );
	testsuite( Test_warray_allAnyOneNone );

	testsuite( Test_warray_sort );
	testsuite( Test_warray_compact );
	testsuite( Test_warray_distinct );
	testsuite( Test_warray_reverse );
	testsuite( Test_warray_concat );

	testsuite( Test_warray_union );
	testsuite( Test_warray_intersect );
	testsuite( Test_warray_symDiff );
	testsuite( Test_warray_addToSet );

	testsuite( Test_warray_compare );
	testsuite( Test_warray_bsearch );

	testsuite( Test_warray_iterator );
	testsuite( Test_warray_iteratorFullExample );

	printf( "\n" );
	printf( "----------------------------\n" );
	printf( "| Tests  | Failed | Passed |\n" );
	printf( "| %-6u | %-6u | %-6u |\n", testsFailed+testsPassed, testsFailed, testsPassed );
	printf( "----------------------------\n" );
}

//--------------------------------------------------------------------------------

#if 0
#include "Random.h"
enum {
	FuzztestRuns	= 10000,
};

static bool
isSorted( const WArray* array )
{
	if ( not array->size ) return true;

	WElementCompare* compare = array->type->compare;
	for ( size_t j = 0; j < array->size-1; j++ ) {
		if ( compare( a.at( array, j ), a.at( array, j+1 )) == 1 )
			return false;
	}

	return true;
}

static bool
isCompact( const WArray* array )
{
	return a.none( array, welement_conditionStrEquals, NULL );
}

static bool
isDistinct( const WArray* array )
{
	if ( not array->size ) return true;

	WElementCompare* compare = array->type->compare;
	for ( size_t i = 0; i < array->size-1; i++ ) {
		for ( size_t j = i+1; j < array->size; j++ ) {
			if ( compare( array->data[i], array->data[j] ) == 0 )
				return false;
		}
	}

	return true;
}

static size_t
appendElement( WArray* array, size_t size )
{
	printf( "append, size = %u\n", array->size );
	autoChar* string = Random_string( .isSpace = true, .isNULL = true );
	a.append( array, string );
	if ( string ) assert_strequal( a.last( array ), string );
	else 		  assert_equal( a.last( array ), NULL );

	return size+1;
}
static size_t
prependElement( WArray* array, size_t size )
{
	printf( "prepend, size = %u\n", array->size );
	autoChar* string = Random_string( .isSpace = true, .isNULL = true );
	a.prepend( array, string );
	if ( string ) assert_strequal( a.first( array ), string );
	else 		  assert_equal( a.first( array ), NULL );

	return size+1;
}
static size_t
insertElement( WArray* array, size_t size )
{
	printf( "insert, size = %u\n", array->size );
	autoChar* string = Random_string( .isSpace = true, .isNULL = true );
//TODO: insert up to array->capacity and even higher.
	size_t position = Random_uint( .max = array->size ? array->size-1 : 0 );
	a.insert( array, position, string );
	if ( string ) assert_strequal( a.at( array, position ), string );
	else 		  assert_equal( a.at( array, position ), NULL );

	return max( size+1, position+1 );
}
static size_t
setElement( WArray* array, size_t size )
{
	printf( "set, size = %u\n", array->size );
	autoChar* string = Random_string( .isSpace = true, .isNULL = true );
//TODO: set up to array->capacity and even higher.
	size_t position = Random_uint( .max = array->size ? array->size-1 : 0 );
	a.set( array, position, string );
	if ( string ) assert_strequal( a.at( array, position ), string );
	else 		  assert_equal( a.at( array, position ), NULL );

	return max( size, position+1 );
}
static size_t
removeFirstElement( WArray* array, size_t size )
{
	printf( "removeFirst, size = %u\n", array->size );
	a.removeFirst( array );

	return size-1;
}
static size_t
removeLastElement( WArray* array, size_t size )
{
	printf( "removeLast, size = %u\n", array->size );
	a.removeLast( array );

	return size-1;
}
static size_t
removeAtElement( WArray* array, size_t size )
{
	printf( "removeAt, size = %u\n", array->size );
	size_t position = Random_uint( .max = array->size ? array->size-1 : 0 );
	a.removeAt( array, position );

	return size-1;
}
void
Fuzztest_Array
{
	autoWArray* array = a.new( 5, wtypeStr );
	double direction = 0.01;
	size_t size = 0;
	printf("\n");

	for ( size_t i = 0; i < FuzztestRuns; i++ ) {
        double ratio = Random_ratio();

		//Add elements.
		if ( ratio < 0.30 + direction ) {
			switch( Random_uint( .max = 3 )) {
				case 0: size = appendElement( array, size ); break;
				case 1: size = prependElement( array, size ); break;
				case 2: size = insertElement( array, size ); break;
				case 3: size = setElement( array, size ); break;
				default: die();
			}
        }

        //Remove elements.
		else if ( ratio >= 0.30 + direction and ratio < 0.60 ) {
			if ( array->size )
			switch( Random_uint( .max = 2 )) {
				case 0: size = removeFirstElement( array, size ); break;
				case 1: size = removeLastElement( array, size ); break;
				case 2: size = removeAtElement( array, size ); break;
				default: die();
			}
		}

        //Switch direction.
		else if ( ratio >= 0.60 and ratio < 0.601 ) {
			printf( "change direction, size = %u\n", array->size );
			direction *= -1;
		}

		//Clear elements.
		else if ( ratio >= 0.61 and ratio < 0.6101 ) {
			printf( "clear, size = %u\n", array->size );
			a.clear( array );
			size = 0;
			direction = abs( direction );
		}

		//Sort the array.
		else if ( ratio >= 0.62 and ratio < 0.63 ) {
			printf( "sort, size = %u\n", array->size );
			a.sort( array );
			assert_true( isSorted( array ));
		}

		//Shuffle the array.
		else if ( ratio >= 0.63 and ratio < 0.64 ) {
			printf( "shuffle, size = %u\n", array->size );
			a.shuffle( array );
		}

		//Compact the array.
		else if ( ratio >= 0.64 and ratio < 0.65 ) {
			printf( "compact, size = %u\n", array->size );
			a.compact( array );
			assert_true( isCompact( array ));
			size = array->size;
		}

		//Remove duplicates in the array.
		else if ( ratio >= 0.65 and ratio < 0.66 ) {
			printf( "distinct, size = %u\n", array->size );
			a.distinct( array );
			assert_true( isDistinct( array ));
			size = array->size;
		}

		//Compare fromString( toString()) with the original array.
		else if ( ratio >= 0.66 and ratio < 0.67 ) {
			printf( "toString and fromString, size = %u\n", array->size );
			autoChar* toString = a.toString( array, "," );
			autoWArray* fromString = a.fromString( toString, "," );
			assert_true( a.equal( array, fromString ));
		}

		//Test filter()
		else if ( ratio >= 0.67 and ratio < 0.68 ) {
			printf( "filter() and all(), size = %u\n", array->size );
            bool lessThanSize( const void* element, const void* data ) {
                return strlen( element ) < (int)data;
            }
			int stringSize = Random_uint( .max = 100 );
			autoWArray* less = a.filter( array, lessThanSize, (void*)stringSize );
			assert_true( a.all( less, lessThanSize, (void*)stringSize ));
		}

		//Test reject()
		else if ( ratio >= 0.68 and ratio < 0.69 ) {
			printf( "reject() and none(), size = %u\n", array->size );
            bool lessThanSize( const void* element, const void* data ) {
                return strlen( element ) < (int)data;
            }
			int stringSize = Random_uint( .max = 100 );
			autoWArray* greater = a.reject( array, lessThanSize, (void*)stringSize );
			assert_true( a.none( greater, lessThanSize, (void*)stringSize ));
		}

		assert_equal( size, array->size );
	}
}

//--------------------------------------------------------------------------------
#endif
