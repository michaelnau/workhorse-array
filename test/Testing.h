#ifndef TESTING_H_INCLUDED
#define TESTING_H_INCLUDED

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

//-------------------------------------------------------------------------

typedef struct TestSuite TestSuite;

//-------------------------------------------------------------------------

#ifdef TEST_IMPLEMENTATION
	size_t testsFailed;
	size_t testsPassed;
#else
	extern size_t testsFailed;
	extern size_t testsPassed;
#endif // TEST_IMPLEMENTATION

//-------------------------------------------------------------------------

static inline void
__wassert( bool condition, const char* text, const char* file, int line, const char* function )
{
	if ( condition )
		testsPassed++;
	else {
		testsFailed++;
		fprintf( stderr, "%s:%i, %s, %s\n", file, line, function, text );
	}
}

#define testsuite( functionname )					\
do {												\
	size_t __testsFailedOld = testsFailed;			\
	size_t __testsPassedOld = testsPassed;			\
	printf( "Testing %s", #functionname );			\
	functionname();									\
	if ( testsFailed > __testsFailedOld )			\
		printf( " - *** %u failed, %u passed ***\n",\
		testsFailed - __testsFailedOld,				\
		testsPassed - __testsPassedOld );			\
	else											\
		printf( " - %u passed\n",					\
		testsPassed - __testsPassedOld );			\
}while(0)

//-------------------------------------------------------------------------
//	Simple assertions
//-------------------------------------------------------------------------

#define assert_true( condition )					\
	__wassert( (condition), #condition, __FILE__, __LINE__, __func__ )

#define assert_false( condition )					\
	__wassert( !(condition), "not" #condition, __FILE__, __LINE__, __func__ )

#define assert_fail()								\
	__wassert( false, "May not reach this location.", __FILE__, __LINE__, __func__ )

#define assert_pass()								\
	__wassert( true, "", __FILE__, __LINE__, __func__ )

#define assert_null( value )						\
	__wassert( (!value), #value " should be NULL.", __FILE__, __LINE__, __func__ )

#define assert_nonnull( value )						\
	__wassert( (value), #value " should not be NULL.", __FILE__, __LINE__, __func__ )

//-------------------------------------------------------------------------
//	Integer assertions
//-------------------------------------------------------------------------

//FIX: Store int1 in variable to avoid evaluating it twice. Dto. in all other macros
#define assert_equal( int1, int2 )					\
	__wassert( ((int)int1) == ((int)int2), #int1 " should equal " #int2, __FILE__, __LINE__, __func__ )

#define assert_unequal( int1, int2 )				\
	__wassert( ((int)int1) != ((int)int2), #int1 " shoud not equal " #int2, __FILE__, __LINE__, __func__ )

#define assert_less( int1, int2 )					\
	__wassert( ((int)int1) < ((int)int2), #int1 " should be less than " #int2, __FILE__, __LINE__, __func__ )

#define assert_greater( int1, int2 )				\
	__wassert( ((int)int1) > ((int)int2), #int1 " should be greater than " #int2, __FILE__, __LINE__, __func__ )

#define assert_inRange( intTest, intLow, intHigh )	\
	__wassert( (intTest) >= (intLow) && (intTest) <= (intHigh), #intLow " <= " #intTest "<=" #intHigh, __FILE__, __LINE__, __func__ )

//-------------------------------------------------------------------------
//	C String assertions
//-------------------------------------------------------------------------

#define assert_strequal( str1, str2 )				\
	__wassert( strcmp((str1), (str2)) == 0, #str1 " == " #str2, __FILE__, __LINE__, __func__ )

#define assert_strunequal( str1, str2 )				\
	__wassert( strcmp((str1), (str2)) != 0, #str1 " != " #str2, __FILE__, __LINE__, __func__ )

#define assert_strcontains( str1, str2 )			\
	__wassert( strstr((str1), (str2)), #str1 " contains " #str2, __FILE__, __LINE__, __func__ )

//-------------------------------------------------------------------------
//	Double assertions
//-------------------------------------------------------------------------

#define assert_dequal( double1, double2 )			\
	__wassert( fabs((double1) - (double2)) < 0.001, #double1 " should equal " #double2, __FILE__, __LINE__, __func__ )

#define assert_dunequal( double1, double2 )			\
	__wassert( fabs((double1) - (double2)) >= 0.001, #double1 " should not equal " #double2, __FILE__, __LINE__, __func__ )

//-------------------------------------------------------------------------

#endif // TESTING_H_INCLUDED
