#ifndef ARRAY_SUGAR_H_INCLUDED
#define ARRAY_SUGAR_H_INCLUDED

#include "WArray.h"

//------------------------------------------------------------

#define Array_append( array, ... )											\
({																			\
	void* __elements[] = { __VA_ARGS__ };									\
	Array_append_n( array, sizeof(__elements)/sizeof(void*), __elements );	\
})

#define Array_prepend( array, ... )											\
({																			\
	void* __elements[] = { __VA_ARGS__ };									\
	Array_prepend_n( array, sizeof(__elements)/sizeof(void*), __elements );	\
})

#define Array_set( array, position, ... )											\
({																					\
	void* __elements[] = { __VA_ARGS__ };											\
	Array_set_n( array, position, sizeof(__elements)/sizeof(void*), __elements );	\
})

#define Array_insert( array, position, ... )											\
({																						\
	void* __elements[] = { __VA_ARGS__ };												\
	Array_insert_n( array, position, sizeof(__elements)/sizeof(void*), __elements );	\
})

//------------------------------------------------------------

#endif // ARRAY_SUGAR_H_INCLUDED
