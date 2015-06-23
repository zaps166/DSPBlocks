#ifndef RINGBUFFER_HPP
#define RINGBUFFER_HPP

#include <string.h>
#include <stdlib.h>

template < typename T > class RingBuffer
{
public:
	inline RingBuffer() :
		d( NULL ),
		s( 0 ), p( 0 )
	{}
	inline ~RingBuffer()
	{
		free( d );
	}

	void resize( int new_s, bool zero_memory = false )
	{
		if ( !new_s )
		{
			free( d );
			d = NULL;
			s = p = 0;
		}
		else if ( new_s != s )
		{
			d = ( T * )realloc( d, new_s * sizeof( T ) );
			if ( new_s > s )
				memset( d + s, 0, ( new_s - s ) * sizeof( T ) );
			else if ( p >= new_s )
				p = new_s - 1;
			s = new_s;
		}
		if ( zero_memory )
			memset( d, 0, s * sizeof( T ) );
	}

	inline bool notEmpty() const
	{
		return s;
	}

	inline void set( const T &t )
	{
		d[ p ] = t;
	}
	inline void advance()
	{
		if ( --p < 0 )
			p = s - 1;
	}

	inline void operator +=( const T &t )
	{
		set( t );
		if ( ++p >= s )
			p = 0;
	}
	inline T get()
	{
		const int last_p = p;
		if ( ++p >= s )
			p = 0;
		return d[ last_p ];
	}

	inline T &operator []( int idx )
	{
		int o = p + idx;
		while ( o < 0 )
			o += s;
		while ( o >= s )
			o -= s;
		return d[ o ];
	}

	inline void getChunks( T *&chunk1, int &s1, T *&chunk2, int &s2 )
	{
		chunk1 = d + p;
		s1 = s - p;
		chunk2 = d;
		s2 = s - s1;
	}
private:
	RingBuffer( const RingBuffer & );
	RingBuffer &operator=( const RingBuffer & );

	T *d;
	int s, p;
};

#endif // RINGBUFFER_HPP
