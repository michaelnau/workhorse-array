#define _GNU_SOURCE
#include "Array.h"
#include "Basic.h"
#include "Contract.h"
#include "CString.h"
#include <iso646.h>
#include "Testing.h"

//--------------------------------------------------------------------------------

ArrayNamespace a = arrayNamespace;
IteratorNamespace it = iteratorNameSpace;

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
		.name = str_dup( person->name ),
		.firstname = str_dup( person->firstname ),
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

ElementType* personType = &(ElementType){
    .clone = (ElementClone*)clonePerson,
    .delete = (ElementDelete*)deletePerson
};

//--------------------------------------------------------------------------------

void
Test_Array_clone_ints()
{
	autoArray *array = Array_new( 0, elementInt );
	autoArray *clone1 = Array_clone( array );
	assert_true( Array_empty( clone1 ));

	Array_append( array, (void*)1 );
	autoArray *clone2 = Array_clone( array );
	assert_equal( Array_at( clone2, 0 ), 1 );
	assert_true( Array_nonEmpty( clone2 ));
	assert_equal( Array_size( clone2 ), 1 );

	Array_append( array, (void*)2 );
	Array_append( array, (void*)3 );
	Array_append( array, (void*)4 );
	Array_append( array, (void*)5 );
	autoArray *clone3 = Array_clone( array );
	assert_equal( Array_at( clone3, 0 ), 1 );
	assert_equal( Array_at( clone3, 1 ), 2 );
	assert_equal( Array_at( clone3, 2 ), 3 );
	assert_equal( Array_at( clone3, 3 ), 4 );
	assert_equal( Array_at( clone3, 4 ), 5 );
	assert_equal( Array_size( clone3 ), 5 );
}
void
Test_Array_clone_strings()
{
	autoArray *array = Array_new( 0, elementStr );
	autoArray *clone1 = Array_clone( array );
	assert_true( Array_empty( clone1 ));

	Array_append( array, "cat" );
	autoArray *clone2 = Array_clone( array );
	assert_strequal( Array_at( clone2, 0 ), "cat" );
	assert_equal( Array_size( clone2 ), 1 );

	Array_append( array, "dog" );
	Array_append( array, "bird" );
	Array_append( array, "hawk" );
	autoArray *clone3 = Array_clone( array );
	assert_strequal( Array_at( clone3, 0 ), "cat" );
	assert_strequal( Array_at( clone3, 1 ), "dog" );
	assert_strequal( Array_at( clone3, 2 ), "bird" );
	assert_strequal( Array_at( clone3, 3 ), "hawk" );
	assert_equal( Array_size( clone3 ), 4 );
}

//--------------------------------------------------------------------------------

void
Test_Array_append_ints()
{
	autoArray *array = Array_new( 0, elementInt );
	assert_equal( Array_size( array ), 0 );

	Array_append( array, (void*)1 );
	assert_equal( Array_at( array, 0 ), 1 );
	assert_equal( Array_size( array ), 1 );

	Array_append( array, (void*)2 );
	assert_equal( Array_at( array, 0 ), 1 );
	assert_equal( Array_at( array, 1 ), 2 );
	assert_equal( Array_size( array ), 2 );

	Array_append( array, (void*)3 );
	Array_append( array, (void*)4 );

	Array_append( array, (void*)5 );
	assert_equal( Array_at( array, 0 ), 1 );
	assert_equal( Array_at( array, 1 ), 2 );
	assert_equal( Array_at( array, 2 ), 3 );
	assert_equal( Array_at( array, 3 ), 4 );
	assert_equal( Array_at( array, 4 ), 5 );
	assert_equal( Array_size( array ), 5 );
}
void
Test_Array_append_strings()
{
	autoArray *array = a.new( 0, elementStr );

	Array_append( array, "cat" );
	const char* string1 = Array_at( array, 0 );
	assert_strequal( string1, "cat" );
	assert_equal( Array_size( array ), 1 );

	Array_append( array, "cow" );
	Array_append( array, "dog" );
	Array_append( array, "horse" );
	const char* string2 = Array_at( array, 0 );
	assert_strequal( string2, "cat" );
	const char* string3 = Array_at( array, 1 );
	assert_strequal( string3, "cow" );
	const char* string4 = Array_at( array, 2 );
	assert_strequal( string4, "dog" );
	const char* string5 = Array_at( array, 3 );
	assert_strequal( string5, "horse" );
	assert_equal( Array_size( array ), 4 );
}
void
Test_Array_prepend_strings()
{
	autoArray *array = a.new( 3, elementStr );

	Array_prepend( array, "cat" );
	const char*string1 = Array_at( array, 0 );
	assert_strequal( string1, "cat" );
	assert_equal( Array_size( array ), 1 );

	Array_prepend( array, "cow" );
	Array_prepend( array, "dog" );
	Array_prepend( array, "horse" );
	assert_strequal( Array_at( array, 0 ), "horse" );
	assert_strequal( Array_at( array, 1 ), "dog" );
	assert_strequal( Array_at( array, 2 ), "cow" );
	assert_strequal( Array_at( array, 3 ), "cat" );
	assert_equal( Array_size( array ), 4 );
}
void
Test_Array_insert_strings()
{
	autoArray *array = a.new( 3, elementStr );

	//Insert 1st element
	Array_insert( array, 0, "cat" );
	assert_strequal( Array_at( array, 0 ), "cat" );
	assert_equal( Array_size( array ), 1 );

	//Insert before all other elements
	Array_insert( array, 0, "cow" );
	assert_strequal( Array_at( array, 0 ), "cow" );
	assert_equal( Array_size( array ), 2 );

//	//Insert as last element
	Array_insert( array, 2, "fish" );
	assert_strequal( Array_at( array, 2 ), "fish" );
	assert_equal( Array_size( array ), 3 );

//	//Insert after all elements leaving a gap
	Array_insert( array, 4, "dog" );
	Array_insert( array, 5, "wolf" );
	Array_insert( array, 8, "bird" );
	assert_strequal( Array_at( array, 0 ), "cow" );
	assert_strequal( Array_at( array, 1 ), "cat" );
	assert_strequal( Array_at( array, 2 ), "fish" );
	assert_null( Array_at( array, 3 ));
	assert_strequal( Array_at( array, 4 ), "dog" );
	assert_strequal( Array_at( array, 5 ), "wolf" );
	assert_null( Array_at( array, 6 ));
	assert_null( Array_at( array, 7 ));
	assert_strequal( Array_at( array, 8 ), "bird" );
	assert_equal( Array_size( array ), 9 );
}
void
Test_Array_insertSorted()
{
	autoArray* array = Array_new( 0, elementStr );

    Array_insertSorted( array, "lion" );
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
Test_Array_set()
{
	autoArray *array = a.new( 0, elementStr );

	a.set( array, 0, "cat" );
	assert_equal( a.size( array ), 1 );
	assert_strequal( a.at( array, 0 ), "cat" );

	Array_append( array, "dog" );
	Array_append( array, "bird" );

	a.set( array, 0, "tiger" );
	assert_strequal( Array_at( array, 0 ), "tiger" );
	assert_strequal( Array_at( array, 1 ), "dog" );
	assert_strequal( Array_at( array, 2 ), "bird" );
	assert_equal( Array_size( array ), 3 );

	Array_set( array, 2, "hawk" );
	assert_equal( Array_size( array ), 3 );

	Array_set( array, 4, "mongoose" );
	assert_strequal( Array_at( array, 0 ), "tiger" );
	assert_strequal( Array_at( array, 1 ), "dog" );
	assert_strequal( Array_at( array, 2 ), "hawk" );
	assert_null( Array_at( array, 3 ));
	assert_strequal( Array_at( array, 4 ), "mongoose" );
	assert_equal( Array_size( array ), 5 );
}

//--------------------------------------------------------------------------------

void
Test_Array_append_n()
{
	autoArray* array = a.new( 0, elementStr );

	Array_append_n( array, 1, (void*[]){ "cat" });
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
Test_Array_prepend_n()
{
	autoArray* array = a.new( 0, elementStr );

	Array_prepend_n( array, 1, (void*[]){ "cat" });
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
Test_Array_firstLastEmptyNonEmpty()
{
	autoArray *array = a.new( 0, elementStr );
	assert_true( Array_empty( array ));
	assert_false( Array_nonEmpty( array ));

	Array_append( array, "cat" );
	assert_false( Array_empty( array ));
	assert_true( Array_nonEmpty( array ));
	assert_strequal( Array_first( array ), "cat" );
	assert_strequal( Array_last( array ), "cat" );

	Array_append( array, "dog" );
	assert_false( Array_empty( array ));
	assert_true( Array_nonEmpty( array ));
	assert_strequal( Array_first( array ), "cat" );
	assert_strequal( Array_last( array ), "dog" );

	Array_insert( array, 15, "tiger" );
	assert_false( Array_empty( array ));
	assert_true( Array_nonEmpty( array ));
	assert_strequal( Array_first( array ), "cat" );
	assert_strequal( Array_last( array ), "tiger" );
}
void
Test_Array_steal()
{
	autoArray *array = a.new( 0, elementStr );

	//Steal the only element.
	Array_append( array, "cat" );
	autoChar* cat = Array_stealAt( array, 0 );
	assert_strequal( cat, "cat" );
	assert_equal( array->size, 0 );

	//Steal the last element.
	Array_append( array, "dog" );
	Array_append( array, "tiger" );
	autoChar* tiger = Array_stealAt( array, 1 );
	assert_strequal( tiger, "tiger" );
	assert_strequal( Array_at( array, 0 ), "dog" );
	assert_equal( array->size, 1 );

	//Steal the middle element.
	Array_append( array, "bird" );
	Array_append( array, "bear" );
	autoChar* bird = Array_stealAt( array, 1 );
	assert_strequal( bird, "bird" );
	assert_strequal( Array_at( array, 0 ), "dog" );
	assert_strequal( Array_at( array, 1 ), "bear" );
	assert_equal( array->size, 2 );

	//Steal a sample element.
	Array_append( array, "hawk" );
	autoChar* sample1 = Array_stealSample( array );
	autoChar* sample2 = Array_stealSample( array );
	autoChar* sample3 = Array_stealSample( array );
	assert_true( sample1 != sample2 and sample2 != sample3 );
	assert_equal( array->size, 0 );
}
void
Test_Array_stealFirstLast()
{
	autoArray* array = a.new( 0, elementStr );

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
Test_Array_removeAt()
{
	autoArray* array = a.new( 0, elementStr );

	//Remove the only element.
	a.append( array, "cat" );
	Array_removeAt( array, 0 );
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
	Array_removeAt( array, 0 );
	assert_strequal( a.at( array, 0 ), "bear" );
	assert_strequal( a.at( array, 1 ), "cat" );
	assert_equal( array->size, 2 );

}
void
Test_Array_removeFirst()
{
	autoArray* array = a.new( 0, elementStr );

	a.append( array, "cat" );
	Array_removeFirst( array );
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
Test_Array_removeLast()
{
	autoArray* array = a.new( 0, elementStr );

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

void
Test_Array_foreachForeachIndex()
{
	void append( const void* element, void* data ) {
		Array* array = data;
        Array_append( array, element );
	}

	autoArray* array = a.new( 0, elementStr );
	autoArray* copy = a.new( 0, elementStr );

	Array_foreach( array, append, copy );
	assert_true( Array_empty( copy ));

	Array_append( array, "cat" );
	Array_foreach( array, append, copy );
	assert_strequal( Array_first( copy ), "cat" );
	assert_equal( Array_size( copy ), 1 );

	Array_clear( copy );
	Array_append( array, "dog" );
	Array_foreach( array, append, copy );
	assert_strequal( Array_first( copy ), "cat" );
	assert_strequal( Array_at( copy, 1 ), "dog" );
	assert_equal( Array_size( copy ), 2 );

	Array_clear( copy );
	Array_append( array, "mouse" );
	Array_foreach( array, append, copy );
	assert_strequal( Array_first( copy ), "cat" );
	assert_strequal( Array_at( copy, 1 ), "dog" );
	assert_strequal( Array_at( copy, 2 ), "mouse" );
	assert_equal( Array_size( copy ), 3 );
}
void
Test_Array_filterReject()
{
	bool isLongWord( const void* element, const void* filterData ) {
        return strlen( element ) > 3;
	}

	autoArray *array = a.new( 0, elementStr );

	autoArray* newArray1 = Array_filter( array, isLongWord, NULL );
	assert_equal( newArray1->size, 0 );

	autoArray* newArray1b = Array_reject( array, isLongWord, NULL );
	assert_equal( newArray1b->size, 0 );

	Array_append( array, "cat" );
	Array_append( array, "dog" );
	Array_append( array, "sea-hawk" );
	Array_append( array, "chimpanzee" );

    autoArray* newArray2 = Array_filter( array, isLongWord, NULL );
    assert_strequal( Array_at( newArray2, 0 ), "sea-hawk" );
    assert_strequal( Array_at( newArray2, 1 ), "chimpanzee" );
    assert_equal( newArray2->size, 2 );

    autoArray* newArray2b = Array_reject( array, isLongWord, NULL );
    assert_strequal( Array_at( newArray2b, 0 ), "cat" );
    assert_strequal( Array_at( newArray2b, 1 ), "dog" );
    assert_equal( newArray2b->size, 2 );
}
void
Test_Array_map()
{
	void* makeItGood( const void* element, const void* mapData ) {
		return str_printf( "My %s is %s.", (char*)element, (char*)mapData );
	}

	autoArray *array = a.new( 0, elementStr );

	autoArray* newArray1 = Array_map( array, makeItGood, "good", elementStr );
	assert_equal( newArray1->size, 0 );

	Array_append( array, "cat" );
	Array_append( array, "dog" );
	Array_append( array, "sea-hawk" );
	Array_append( array, "chimpanzee" );

    autoArray* newArray2 = Array_map( array, makeItGood, "good", elementStr );
    assert_strequal( Array_at( newArray2, 0 ), "My cat is good." );
    assert_strequal( Array_at( newArray2, 1 ), "My dog is good." );
    assert_strequal( Array_at( newArray2, 2 ), "My sea-hawk is good." );
    assert_strequal( Array_at( newArray2, 3 ), "My chimpanzee is good." );
    assert_equal( newArray2->size, 4 );
}
void
Test_Array_reduce()
{
	void* joinAnimals( const void* element, const void* reduction ) {
		return str_printf( "%s and %s", (char*)reduction, (char*)element );
	}

	autoArray *array = a.new( 0, elementStr );

	autoChar* string1 = Array_reduce( array, joinAnimals, "My favorite animals are turtle", elementStr );
	assert_strequal( string1, "My favorite animals are turtle");

	Array_append( array, "cat" );
	Array_append( array, "dog" );
	Array_append( array, "sea-hawk" );
	Array_append( array, "chimpanzee" );

    autoChar* string2 = Array_reduce( array, joinAnimals, "My favorite animals are turtle", elementStr );
    assert_strequal( string2, "My favorite animals are turtle and cat and dog and sea-hawk and chimpanzee" );
}

//--------------------------------------------------------------------------------

void
Test_Array_minMax()
{
	int compareStrings( const void* element1, const void* element2 ) {
		return strcmp( element1, element2 );
	}

	autoArray *array = a.new( 0, elementStr );

	Array_append( array, "cat" );
	assert_strequal( Array_min( array ), "cat" );
	assert_strequal( Array_max( array ), "cat" );

	Array_append( array, "dog" );
	assert_strequal( Array_min( array ), "cat" );
	assert_strequal( Array_max( array ), "dog" );

	Array_append( array, "sea-hawk" );
	assert_strequal( Array_min( array ), "cat" );
	assert_strequal( Array_max( array ), "sea-hawk" );

	Array_append( array, "chimpanzee" );
	assert_strequal( Array_min( array ), "cat" );
	assert_strequal( Array_max( array ), "sea-hawk" );

	Array_append( array, "ape" );
	assert_strequal( Array_min( array ), "ape" );
	assert_strequal( Array_max( array ), "sea-hawk" );
}
void
Test_Array_indexRindex()
{
	autoArray* array = a.new( 0, elementStr );

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
Test_Array_count()
{
	autoArray* array = a.new( 0, elementStr );

	assert_equal( Array_count( array, Element_conditionStrEquals, NULL ), 0 );
	assert_equal( a.count( array, Element_conditionStrEquals, "Test" ), 0 );

	a.append( array, "cat" );
	assert_equal( a.count( array, Element_conditionStrEquals, NULL ), 0 );
	assert_equal( a.count( array, Element_conditionStrEquals, "Test" ), 0 );
	assert_equal( a.count( array, Element_conditionStrEquals, "cat" ), 1 );

	a.append( array, "dog" );
	assert_equal( a.count( array, Element_conditionStrEquals, "cat" ), 1 );
	assert_equal( a.count( array, Element_conditionStrEquals, "dog" ), 1 );

	a.set( array, 5, "dog" );
	assert_equal( a.count( array, Element_conditionStrEquals, "cat" ), 1 );
	assert_equal( a.count( array, Element_conditionStrEquals, "dog" ), 2 );

	a.set( array, 55, "" );
	assert_equal( a.count( array, Element_conditionStrEquals, "cat" ), 1 );
	assert_equal( a.count( array, Element_conditionStrEquals, "dog" ), 2 );
	assert_equal( a.count( array, Element_conditionStrEquals, "" ), 1 );
}

//--------------------------------------------------------------------------------

void
Test_Array_toStringFromString()
{
	autoArray* array = a.new( 0, elementStr );

	autoChar* joined1 = Array_toString( array, ", " );
	assert_strequal( joined1, "" );

	autoArray* split1 = Array_fromString( joined1, ", " );
	assert_equal( Array_size( split1 ), 0 );

    Array_append( array, "cat" );
	autoChar* joined2 = Array_toString( array, ", " );
	assert_strequal( joined2, "cat" );

	autoArray* split2 = Array_fromString( joined2, ", " );
	assert_strequal( Array_first( split2 ), "cat" );
	assert_equal( Array_size( split2 ), 1 );

    Array_append( array, "dog" );
	autoChar* joined3 = Array_toString( array, ", " );
	assert_strequal( joined3, "cat, dog" );

	autoArray* split3 = Array_fromString( joined3, ", " );
	assert_strequal( Array_first( split3 ), "cat" );
	assert_strequal( Array_at( split3, 1 ), "dog" );
	assert_equal( Array_size( split3 ), 2 );

	autoArray* split3b = Array_fromString( joined3, "/" );
	assert_strequal( Array_first( split3b ), "cat, dog" );
	assert_equal( Array_size( split3b ), 1 );

    Array_append( array, "mouse" );
	autoChar* joined4 = Array_toString( array, ", " );
	assert_strequal( joined4, "cat, dog, mouse" );

	autoArray* split4 = Array_fromString( joined4, ", " );
	assert_strequal( Array_first( split4 ), "cat" );
	assert_strequal( Array_at( split4, 1 ), "dog" );
	assert_strequal( Array_at( split4, 2 ), "mouse" );
	assert_equal( Array_size( split4 ), 3 );

	autoArray* split4b = Array_fromString( joined4, "," );
	assert_strequal( Array_first( split4b ), "cat" );
	assert_strequal( Array_at( split4b, 1 ), " dog" );
	assert_strequal( Array_at( split4b, 2 ), " mouse" );
	assert_equal( Array_size( split4b ), 3 );
}

//--------------------------------------------------------------------------------

void
Test_Array_allAnyOneNone()
{
	bool equals( const void* element1, const void* element2 ) {
		return strcmp( element1, element2 ) == 0;
	}

	autoArray* array = a.new( 0, elementStr );

	assert_true( Array_all( array, equals, "foo" ));
	assert_false( Array_any( array, equals, "foo" ));
	assert_false( Array_one( array, equals, "foo" ));
	assert_true( Array_none( array, equals, "foo" ));

    Array_append( array, "cat" );
	assert_true( not Array_all( array, equals, "foo" ));
	assert_true( Array_all( array, equals, "cat" ));
	assert_true( not Array_any( array, equals, "foo" ));
	assert_true( Array_any( array, equals, "cat" ));
	assert_true( not Array_one( array, equals, "foo" ));
	assert_true( Array_one( array, equals, "cat" ));
	assert_true( Array_none( array, equals, "foo" ));
	assert_true( not Array_none( array, equals, "cat" ));

    Array_append( array, "cat" );
	assert_true( not Array_all( array, equals, "foo" ));
	assert_true( Array_all( array, equals, "cat" ));
	assert_true( not Array_any( array, equals, "foo" ));
	assert_true( Array_any( array, equals, "cat" ));
	assert_true( not Array_one( array, equals, "foo" ));
	assert_true( not Array_one( array, equals, "cat" ));
	assert_true( Array_none( array, equals, "foo" ));
	assert_true( not Array_none( array, equals, "cat" ));

    Array_append( array, "mouse" );
    Array_append( array, "cow" );
	assert_true( not Array_all( array, equals, "foo" ));
	assert_true( not Array_all( array, equals, "cat" ));
	assert_true( not Array_any( array, equals, "foo" ));
	assert_true( Array_any( array, equals, "mouse" ));
	assert_true( not Array_one( array, equals, "foo" ));
	assert_true( not Array_one( array, equals, "cat" ));
	assert_true( Array_one( array, equals, "mouse" ));
	assert_true( Array_one( array, equals, "cow" ));
	assert_true( Array_none( array, equals, "foo" ));
	assert_true( not Array_none( array, equals, "mouse" ));
	assert_true( not Array_none( array, equals, "cow" ));
}

//--------------------------------------------------------------------------------

void
Test_Array_sort()
{
	Array* array = a.new( 0, elementStr );

	Array_sort( array );
    assert_true( Array_empty( array ));

	Array_append( array, "cat" );
	Array_sort( array );
    assert_strequal( Array_first( array ), "cat" );
    assert_equal( Array_size( array ), 1 );

	Array_append( array, "ape" );
	Array_sort( array );
    assert_strequal( Array_first( array ), "ape" );
    assert_strequal( Array_at( array, 1 ), "cat" );
    assert_equal( Array_size( array ), 2 );

	Array_append_n( array, 5, (void*[]){ "mongoose", "dolphin", "lion", "bird", "yak" });
	Array_sort( array );
    assert_strequal( Array_first( array ), "ape");
    assert_strequal( Array_at( array, 1 ), "bird");
    assert_strequal( Array_at( array, 2 ), "cat");
    assert_strequal( Array_at( array, 3 ), "dolphin");
    assert_strequal( Array_at( array, 4 ), "lion");
    assert_strequal( Array_at( array, 5 ), "mongoose");
    assert_strequal( Array_at( array, 6 ), "yak");
    assert_equal( Array_size( array ), 7 );
}
void
Test_Array_compact()
{
	autoArray* array = a.new( 0, elementStr );

	Array_compact( array );
	assert_true( a.empty( array ));

	a.append( array, NULL );
	Array_compact( array );
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
Test_Array_distinct()
{
	autoArray* array = a.new( 0, elementStr );

	Array_distinct( array );
	assert_true( a.empty( array ));

	a.append( array, "cat" );
	Array_distinct( array );
	assert_equal( a.size( array ), 1 );
	assert_strequal( a.first( array ), "cat" );

	a.append( array, "dog" );
	Array_distinct( array );
	assert_equal( a.size( array ), 2 );
	assert_strequal( a.at( array, 0 ), "cat" );
	assert_strequal( a.at( array, 1 ), "dog" );

	a.append( array, "dog" );
	a.append( array, "dog" );
	a.append( array, "dog" );
	Array_distinct( array );
	assert_equal( a.size( array ), 2 );
	assert_strequal( a.at( array, 0 ), "cat" );
	assert_strequal( a.at( array, 1 ), "dog" );

	a.set( array, 10, "dog" );
	Array_distinct( array );
	assert_equal( a.size( array ), 3 );
	assert_true( Array_one( array, Element_conditionStrEquals, "cat" ));
	assert_true( Array_one( array, Element_conditionStrEquals, "dog" ));
	assert_true( Array_one( array, Element_conditionStrEquals, NULL ));
}
void
Test_Array_reverse()
{
	autoArray* array = a.new( 0, elementStr );

	Array_reverse( array );
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
Test_Array_concat()
{
	autoArray* array1 = a.new( 0, elementStr );
	autoArray* array2 = a.new( 0, elementStr );

	autoArray* concat1 = Array_concat( Array_clone( array1 ), array2 );
    assert_true( Array_empty( concat1 ));

	Array_append( array1, "cat" );
	autoArray* concat2 = Array_concat( Array_clone( array1 ), array2 );
	assert_strequal( Array_first( concat2 ), "cat" );
	assert_equal( concat2->size, 1 );
	autoArray* concat3 = Array_concat( Array_clone( array2 ), array1 );
	assert_strequal( Array_first( concat3 ), "cat" );
	assert_equal( concat3->size, 1 );

	Array_append( array2, "dog" );
	autoArray* concat4 = Array_concat( Array_clone( array1 ), array2 );
	assert_strequal( Array_at( concat4, 0 ), "cat" );
	assert_strequal( Array_at( concat4, 1 ), "dog" );
	assert_equal( concat4->size, 2 );

	Array_append( array1, "mouse" );
	Array_append( array1, "bird" );
	Array_append( array2, "dolphin" );
	Array_append( array2, "wolf" );
	autoArray* concat5 = Array_concat( Array_clone( array1 ), array2 );
	assert_strequal( Array_at( concat5, 0 ), "cat" );
	assert_strequal( Array_at( concat5, 1 ), "mouse" );
	assert_strequal( Array_at( concat5, 2 ), "bird" );
	assert_strequal( Array_at( concat5, 3 ), "dog" );
	assert_strequal( Array_at( concat5, 4 ), "dolphin" );
	assert_strequal( Array_at( concat5, 5 ), "wolf" );
	assert_equal( concat5->size, 6 );
}

//--------------------------------------------------------------------------------

void
Test_Array_union()
{
	autoArray* array1 = a.new( 0, elementStr );
	autoArray* array2 = a.new( 0, elementStr );

	autoArray* union1 = Array_unite( array1, array2 );
	assert_equal( union1->size, 0 );

	a.append( array1, "cat" );
	autoArray* union2 = Array_unite( array1, array2 );
	assert_equal( union2->size, 1 );
	assert_strequal( a.at( union2, 0 ), "cat" );

	a.append( array2, "dog" );
	autoArray* union3 = Array_unite( array1, array2 );
	assert_equal( union3->size, 2 );
	assert_true( a.index( union3, "cat" ) >= 0 );
	assert_true( a.index( union3, "dog" ) >= 0 );

	a.append( array1, "dog" );
	autoArray* union4 = Array_unite( array1, array2 );
	assert_equal( union4->size, 2 );
	assert_true( a.index( union4, "cat" ) >= 0 );
	assert_true( a.index( union4, "dog" ) >= 0 );

	a.append( array2, "dog" );
	autoArray* union5 = Array_unite( array1, array2 );
	assert_equal( union5->size, 2 );
	assert_true( a.index( union5, "cat" ) >= 0 );
	assert_true( a.index( union5, "dog" ) >= 0 );

	a.append( array2, "cat" );
	autoArray* union6 = Array_unite( array1, array2 );
	assert_equal( union6->size, 2 );
	assert_true( a.index( union6, "cat" ) >= 0 );
	assert_true( a.index( union6, "dog" ) >= 0 );

	autoArray* array3 = a.new( 0, elementStr );
	autoArray* array4 = a.new( 0, elementStr );
	a.append_n( array3, 5, (void*[]){ "cat", "", "dog", "cat", "elephant" });
	a.append_n( array4, 6, (void*[]){ "bird", NULL, NULL, "dog", "bird", "mouse" });
	autoArray* union7 = Array_unite( array3, array4 );
	assert_equal( union7->size, 7 );
	assert_true( Array_contains( union7, "cat" ));
	assert_true( Array_contains( union7, "" ));
	assert_true( a.contains( union7, "dog" ));
	assert_true( a.contains( union7, "elephant" ));
	assert_true( a.contains( union7, "bird" ));
	assert_true( a.contains( union7, NULL ));
	assert_true( a.contains( union7, "mouse" ));
}
void
Test_Array_intersect()
{
	autoArray* array1 = a.new( 0, elementStr );
	autoArray* array2 = a.new( 0, elementStr );

	autoArray* inter1 = Array_intersect( array1, array2 );
	assert_equal( inter1->size, 0 );

	a.append( array1, "cat" );
	autoArray* inter2 = a.intersect( array1, array2 );
	assert_equal( inter2->size, 0 );
	assert_equal( a.count( inter2, Element_conditionStrEquals, "cat" ), 0 );

	a.append( array2, "dog" );
	autoArray* inter3 = a.intersect( array1, array2 );
	assert_equal( inter3->size, 0 );
	assert_equal( a.count( inter3, Element_conditionStrEquals, "cat" ), 0 );
	assert_equal( a.count( inter3, Element_conditionStrEquals, "dog" ), 0 );

	a.append( array1, "dog" );
	autoArray* inter4 = a.intersect( array1, array2 );
	assert_equal( inter4->size, 1 );
	assert_equal( a.count( inter4, Element_conditionStrEquals, "cat" ), 0 );
	assert_equal( a.count( inter4, Element_conditionStrEquals, "dog" ), 1 );

	a.append( array2, "cat" );
	autoArray* inter5 = a.intersect( array1, array2 );
	assert_equal( inter5->size, 2 );
	assert_equal( a.count( inter5, Element_conditionStrEquals, "cat" ), 1 );
	assert_equal( a.count( inter5, Element_conditionStrEquals, "dog" ), 1 );

	autoArray* array3 = a.new( 0, elementStr );
	autoArray* array4 = a.new( 0, elementStr );
	a.append_n( array3, 7, (void*[]){ "cat", "dog", NULL, NULL, "", "bird", "elephant" });
	a.append_n( array4, 6, (void*[]){ "dog", "mouse", NULL, "bird", "crocodile", "", "" });
	autoArray* inter6 = a.intersect( array3, array4 );
	assert_equal( inter6->size, 4 );
	assert_equal( a.count( inter6, Element_conditionStrEquals, "dog" ), 1 );
	assert_equal( a.count( inter6, Element_conditionStrEquals, NULL ), 1 );
	assert_equal( a.count( inter6, Element_conditionStrEquals, "" ), 1 );
	assert_equal( a.count( inter6, Element_conditionStrEquals, "bird" ), 1 );
}
void
Test_Array_symDiff()
{
	autoArray* array1 = a.new( 0, elementStr );
	autoArray* array2 = a.new( 0, elementStr );

	autoArray* sym1 = Array_symDiff( array1, array2 );
	assert_equal( sym1->size, 0 );

	a.append( array1, "cat" );
	autoArray* sym2 = a.symDiff( array1, array2 );
	assert_equal( sym2->size, 1 );
	assert_equal( a.count( sym2, Element_conditionStrEquals, "cat" ), 1 );

	a.append( array2, "dog" );
	autoArray* sym3 = a.symDiff( array1, array2 );
	assert_equal( sym3->size, 2 );
	assert_equal( a.count( sym3, Element_conditionStrEquals, "cat" ), 1 );
	assert_equal( a.count( sym3, Element_conditionStrEquals, "dog" ), 1 );

	a.append( array1, "dog" );
	autoArray* sym4 = a.symDiff( array1, array2 );
	assert_equal( sym4->size, 1 );
	assert_equal( a.count( sym4, Element_conditionStrEquals, "cat" ), 1 );

	a.append( array2, "cat" );
	autoArray* sym5 = a.symDiff( array1, array2 );
	assert_equal( sym5->size, 0 );

	autoArray* array3 = a.new( 0, elementStr );
	autoArray* array4 = a.new( 0, elementStr );
	a.append_n( array3, 6, (void*[]){ "cat", "dog", NULL, NULL, "bird", "elephant" });
	a.append_n( array4, 7, (void*[]){ "dog", "mouse", NULL, "bird", "crocodile", "", "" });
	autoArray* sym6 = a.symDiff( array3, array4 );
	assert_equal( sym6->size, 5 );
	assert_equal( a.count( sym6, Element_conditionStrEquals, "cat" ), 1 );
	assert_equal( a.count( sym6, Element_conditionStrEquals, "elephant" ), 1 );
	assert_equal( a.count( sym6, Element_conditionStrEquals, "mouse" ), 1 );
	assert_equal( a.count( sym6, Element_conditionStrEquals, "crocodile" ), 1 );
	assert_equal( a.count( sym6, Element_conditionStrEquals, "" ), 1 );
}
void
Test_Array_addToSet()
{
	autoArray* set = a.new( 0, elementStr );

	Array_addToSet( set, "cat" );
	assert_equal( a.count( set, Element_conditionStrEquals, "cat" ), 1 );

	Array_addToSet( set, "cat" );
	assert_equal( a.count( set, Element_conditionStrEquals, "cat" ), 1 );

	Array_addToSet( set, "cat" );
	assert_equal( a.count( set, Element_conditionStrEquals, "cat" ), 1 );

	Array_addToSet( set, "" );
	assert_equal( a.count( set, Element_conditionStrEquals, "cat" ), 1 );
	assert_equal( a.count( set, Element_conditionStrEquals, "" ), 1 );

	Array_addToSet( set, "" );
	assert_equal( a.count( set, Element_conditionStrEquals, "cat" ), 1 );
	assert_equal( a.count( set, Element_conditionStrEquals, "" ), 1 );

	Array_addToSet( set, "cat" );
	assert_equal( a.count( set, Element_conditionStrEquals, "cat" ), 1 );
	assert_equal( a.count( set, Element_conditionStrEquals, "" ), 1 );

	Array_addToSet( set, NULL );
	Array_addToSet( set, "dog" );
	Array_addToSet( set, NULL );
	Array_addToSet( set, "dog" );
	assert_equal( a.count( set, Element_conditionStrEquals, "cat" ), 1 );
	assert_equal( a.count( set, Element_conditionStrEquals, "" ), 1 );
	assert_equal( a.count( set, Element_conditionStrEquals, NULL ), 1 );
	assert_equal( a.count( set, Element_conditionStrEquals, "dog" ), 1 );
}

//--------------------------------------------------------------------------------

void
Test_Array_compare()
{
    autoArray* array1 = a.new( 0, elementStr );
    autoArray* array2 = a.new( 0, elementStr );

    assert_equal( Array_compare( array1, array2 ), 0 );

    a.append( array1, NULL );
    assert_equal( Array_compare( array1, array2 ), 1 );

    a.append( array2, NULL );
    assert_equal( Array_compare( array1, array2 ), 0 );

    a.append( array2, "mouse" );
    assert_equal( Array_compare( array1, array2 ), -1 );

    a.append( array1, "lion" );
    assert_equal( Array_compare( array1, array2 ), -1 );

    a.append( array1, "elephant" );
    assert_equal( Array_compare( array1, array2 ), -1 );

    a.prepend( array1, "cat" );
    a.prepend( array2, "" );
    assert_equal( a.compare( array1, array2 ), 1 );

	autoArray* array3 = a.new( 0, elementStr );
	autoArray* array4 = a.new( 0, elementStr );

    a.append_n( array3, 6, (void*[]){ "cat", "", NULL, "lion", "elephant", "mouse" });
    a.append_n( array4, 6, (void*[]){ "cat", "", NULL, "lion", "elephant", "snake" });
	assert_equal( a.compare( array3, array4 ), -1 );

	a.set( array4, 5, "mouse" );
	assert_equal( a.compare( array3, array4 ), 0 );

	a.append( array3, "squirrel" );
	assert_equal( a.compare( array3, array4 ), 1 );
}

void
Test_Array_bsearch()
{
	autoArray* array1 = a.new( 0, elementStr );
	a.append_n( array1, 5, (void*[]){ "cat", "dog", "lion", "mouse", "zebra" });
	assert_equal( Array_bsearch( array1, Element_compareStr, "cat" ), 0 );
	assert_equal( a.bsearch( array1, Element_compareStr, "dog" ), 1 );
	assert_equal( a.bsearch( array1, Element_compareStr, "lion" ), 2 );
	assert_equal( a.bsearch( array1, Element_compareStr, "mouse" ), 3 );
	assert_equal( a.bsearch( array1, Element_compareStr, "zebra" ), 4 );
	assert_equal( a.bsearch( array1, Element_compareStr, "wolpertinger" ), -1 );

	autoArray* array2 = a.new( 0, personType );
	a.append_n( array2, 5, (void*[]){
		&(Person){	"Abelson",	"Peter",	33	},
		&(Person){	"Johnson",	"Jack",		89	},
		&(Person){	"Johnson",	"Paul",		54	},
		&(Person){	"Smith",	"Greg",		12	},
		&(Person){	"West",		"Mitch", 	66	},
	});
	int comparePerson( const void* key, const void* element ) {
		const Person* person = element;
		return strcmp( key, person->name );
	}

	assert_equal( a.bsearch( array2, comparePerson, "Abelson" ), 0 );
	assert_equal( a.bsearch( array2, comparePerson, "Smith" ), 3 );
	assert_equal( a.bsearch( array2, comparePerson, "West" ), 4 );
	assert_equal( a.bsearch( array2, comparePerson, "Unknown" ), -1 );
}

//--------------------------------------------------------------------------------

void
Test_Array_iterator()
{
	autoArray* ar = a.new( 0, elementStr );
	autoIterator* iter1a = a.iterator( ar );
	autoIterator* iter1b = a.iteratorReverse( ar );
	assert_false( it.hasNext( iter1a ));
	assert_false( it.hasNext( iter1b ));

	a.append( ar, "cat" );
	autoIterator* iter2a = a.iterator( ar );
	autoIterator* iter2b = a.iteratorReverse( ar );
	assert_true( it.hasNext( iter2a ));
	assert_true( it.hasNext( iter2b ));
	assert_strequal( it.next( iter2a ), "cat" );
	assert_strequal( it.next( iter2b ), "cat" );
	assert_false( it.hasNext( iter2a ));
	assert_false( it.hasNext( iter2b ));

	a.append( ar, "dog" );
	autoIterator* iter3a = a.iterator( ar );
	autoIterator* iter3b = a.iteratorReverse( ar );
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
Test_Array_iteratorFullExample()
{
	autoArray* ar1 = a.new( 0, elementStr );
	autoArray* ar2 = a.new( 0, elementStr );

	a.append_n( ar1, 3, (void*[]){ "1.", "2.", "3." });
	a.append_n( ar2, 3, (void*[]){ "cat", "dog", "lion" });
	autoIterator* iter1 = Array_iterator( ar1 );
	autoIterator* iter2 = Array_iterator( ar2 );

	Array* ar3 = a.new( 0, elementStr );
	while ( Iterator_hasNext( iter1 ) and Iterator_hasNext( iter2 )) {
		a.append( ar3, Iterator_next( iter1 ));
		a.append( ar3, Iterator_next( iter2 ));
	}

	assert_false( Iterator_hasNext( iter1 ));
	assert_false( Iterator_hasNext( iter2 ));
    assert_strequal( a.at( ar3, 0 ), "1." );
    assert_strequal( a.at( ar3, 1 ), "cat" );
    assert_strequal( a.at( ar3, 2 ), "2." );
    assert_strequal( a.at( ar3, 3 ), "dog" );
    assert_strequal( a.at( ar3, 4 ), "3." );
    assert_strequal( a.at( ar3, 5 ), "lion" );
}

//--------------------------------------------------------------------------------

void Testsuite_Array()
{
	Test_Array_clone_ints();
	Test_Array_clone_strings();

	Test_Array_append_ints();
	Test_Array_append_strings();
	Test_Array_prepend_strings();
	Test_Array_insert_strings();
	Test_Array_insertSorted();
	Test_Array_set();
	Test_Array_append_n();
	Test_Array_prepend_n();

	Test_Array_firstLastEmptyNonEmpty();
	Test_Array_steal();
	Test_Array_stealFirstLast();
	Test_Array_removeAt();
	Test_Array_removeFirst();
	Test_Array_removeLast();
	Test_Array_filterReject();
	Test_Array_map();
	Test_Array_reduce();

	Test_Array_minMax();
	Test_Array_indexRindex();
	Test_Array_count();

	Test_Array_toStringFromString();
	Test_Array_foreachForeachIndex();
	Test_Array_allAnyOneNone();

	Test_Array_sort();
	Test_Array_compact();
	Test_Array_distinct();
	Test_Array_reverse();
	Test_Array_concat();

	Test_Array_union();
	Test_Array_intersect();
	Test_Array_symDiff();
	Test_Array_addToSet();

	Test_Array_compare();
	Test_Array_bsearch();

	Test_Array_iterator();
	Test_Array_iteratorFullExample();
}

//--------------------------------------------------------------------------------

#include "Random.h"
enum {
	FuzztestRuns	= 10000,
};

static bool
isSorted( const Array* array )
{
	if ( not array->size ) return true;

	ElementCompare* compare = array->type->compare;
	for ( size_t j = 0; j < array->size-1; j++ ) {
		if ( compare( a.at( array, j ), a.at( array, j+1 )) == 1 )
			return false;
	}

	return true;
}

static bool
isCompact( const Array* array )
{
	return a.none( array, Element_conditionStrEquals, NULL );
}

static bool
isDistinct( const Array* array )
{
	if ( not array->size ) return true;

	ElementCompare* compare = array->type->compare;
	for ( size_t i = 0; i < array->size-1; i++ ) {
		for ( size_t j = i+1; j < array->size; j++ ) {
			if ( compare( array->data[i], array->data[j] ) == 0 )
				return false;
		}
	}

	return true;
}

static size_t
appendElement( Array* array, size_t size )
{
	printf( "append, size = %u\n", array->size );
	autoChar* string = Random_string( .isSpace = true, .isNULL = true );
	a.append( array, string );
	if ( string ) assert_strequal( a.last( array ), string );
	else 		  assert_equal( a.last( array ), NULL );

	return size+1;
}
static size_t
prependElement( Array* array, size_t size )
{
	printf( "prepend, size = %u\n", array->size );
	autoChar* string = Random_string( .isSpace = true, .isNULL = true );
	a.prepend( array, string );
	if ( string ) assert_strequal( a.first( array ), string );
	else 		  assert_equal( a.first( array ), NULL );

	return size+1;
}
static size_t
insertElement( Array* array, size_t size )
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
setElement( Array* array, size_t size )
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
removeFirstElement( Array* array, size_t size )
{
	printf( "removeFirst, size = %u\n", array->size );
	a.removeFirst( array );

	return size-1;
}
static size_t
removeLastElement( Array* array, size_t size )
{
	printf( "removeLast, size = %u\n", array->size );
	a.removeLast( array );

	return size-1;
}
static size_t
removeAtElement( Array* array, size_t size )
{
	printf( "removeAt, size = %u\n", array->size );
	size_t position = Random_uint( .max = array->size ? array->size-1 : 0 );
	a.removeAt( array, position );

	return size-1;
}
void
Fuzztest_Array()
{
	autoArray* array = a.new( 5, elementStr );
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
			autoArray* fromString = a.fromString( toString, "," );
			assert_true( a.equal( array, fromString ));
		}

		//Test filter()
		else if ( ratio >= 0.67 and ratio < 0.68 ) {
			printf( "filter() and all(), size = %u\n", array->size );
            bool lessThanSize( const void* element, const void* data ) {
                return strlen( element ) < (int)data;
            }
			int stringSize = Random_uint( .max = 100 );
			autoArray* less = a.filter( array, lessThanSize, (void*)stringSize );
			assert_true( a.all( less, lessThanSize, (void*)stringSize ));
		}

		//Test reject()
		else if ( ratio >= 0.68 and ratio < 0.69 ) {
			printf( "reject() and none(), size = %u\n", array->size );
            bool lessThanSize( const void* element, const void* data ) {
                return strlen( element ) < (int)data;
            }
			int stringSize = Random_uint( .max = 100 );
			autoArray* greater = a.reject( array, lessThanSize, (void*)stringSize );
			assert_true( a.none( greater, lessThanSize, (void*)stringSize ));
		}

		assert_equal( size, array->size );
	}
}

//--------------------------------------------------------------------------------
