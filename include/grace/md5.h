#ifndef _GRACEMD5_H
#define _GRACEMD5_H 1

#include <grace/str.h>

typedef unsigned char md5_byte_t;
typedef unsigned int md5_word_t;

/// Class for building an MD5 checksum.
class md5checksum
{
public:
				 /// Constructor. Initializes the values.
				 md5checksum (void);
				 
				 /// Destructor.
				~md5checksum (void);
			
				 /// Initialize internal values to start-up state.
	void		 init (void);
	
				 /// Add text to the data to be checksummed.
	void		 append (const string &);
	void		 append (const md5_byte_t *data, int sz);
	
				 /// Get the binary MD5 checksum.
	string		*checksum (void);
	
				 /// Get the base64-encoded MD5 checksum.
	string		*base64 (void);
	
				 /// Get the hex-encoded MD5 checksum.
	string		*hex (void);

protected:
	md5_word_t	 count[2];
	md5_word_t	 abcd[4];
	md5_byte_t	 buf[64];
	
	void		 process (const md5_byte_t *data);
	void		 finish (md5_byte_t digest[16]);
};

#endif
