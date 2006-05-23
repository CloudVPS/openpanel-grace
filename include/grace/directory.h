// ========================================================================
// directory.h: Search and access class for hierarchical/structured
//              data stores.
//
// (C) Copyright 2004 Pim van Riezen <pi@madscience.nl>
//                    Madscience Labs, Rotterdam
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^

#ifndef _DIRECTORY_H
#define _DIRECTORY_H 1

// ========================================================================
// A dtransaction object is used to keep changes made to objects inside
// a directory. The idea is to be able to combine multiple changes to
// the same directory node into one change action for directory types where
// this would make sense. The directory::commit action takes the assembled
// transactions and executes them towards the source.
// ========================================================================

class dtransaction
{
public:
						 dtransaction (class dreference *, value *);
						~dtransaction (void);
	
	class dreference	*dreference (void);
	value				*value (void);

protected:	
	dtransaction		*next;
	dreference			*_reference;
	value				*_value;
};

// ========================================================================
// The directory base class defines behavior of a directory. Specific
// implementations implement methods to access directory data, list
// available keys and change or create objects.
//
// The dreference class is used to keep as 'pointers' to directory nodes.
// They refer back to the directory to perform any operations.
// ========================================================================

class directory
{
public:
						 directory (void);
	virtual				~directory (void);
	
	dreference			&operator[] (const statstring &);
	virtual void		 commit (void);
	virtual void		 rollback (void);
	dreference			&root (void);
	
	bool				 autocommit (void);
	void				 autocommit (bool);
	
protected:
	void				 addtransaction (dreference *, value *);
	virtual value		*getvalue (dreference *);
	virtual value		*getkeys (dreference *);
};

// ========================================================================
// The dreference class is used as a temporary pointer to a directory
// node. If changes are made, they are added to the directory's
// transaction log. If data is requested, the dreference uses the
// directory's virtual methods to get it.
// ========================================================================

class dreference
{
public:
					 dreference (const dreference &);
					 dreference (dreference *);
					 dreference (directory *root);
					 dreference (directory *root, dreference *parent);
					~dreference (void);
	
	void			 uncache (void);
	
	dreference		&operator= (const string &);
	dreference		&operator= (const statstring &);
	dreference		&operator= (const value &);
	dreference		&operator= (value *);
	dreference		&operator= (string *);
	dreference		&operator= (bool);
	dreference		&operator= (int);
	dreference		&operator= (unsigned int);
	dreference		&operator= (long long);
	dreference		&operator= (unsigned long long);
	dreference		&operator= (time_t);
	
	bool			 operator== (const string &);
	bool			 operator== (const statstring &);
	bool			 operator== (const value &);
	bool			 operator== (bool);
	bool			 operator== (int);
	bool			 operator== (unsigned int);
	bool			 operator== (long long);
	bool			 operator== (unsigned long long);
	bool			 operator== (time_t);

	bool			 operator!= (const string &);
	bool			 operator!= (const statstring &);
	bool			 operator!= (const value &);
	bool			 operator!= (bool);
	bool			 operator!= (int);
	bool			 operator!= (unsigned int);
	bool			 operator!= (long long);
	bool			 operator!= (unsigned long long);
	bool			 operator!= (time_t);

	dreference		&operator[] (const statstring &);
	dreference		&operator[] (int);
	
protected:
	value			*cache; // Updated from the directory class
};

#endif