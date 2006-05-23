// ========================================================================
// value_csv.cpp: CSV conversion methods for two-dimensional value arrays
//
// (C) Copyright 2004-2006 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^

#include <grace/value.h>
#include <grace/file.h>
#include <grace/stack.h>
#include <grace/strutil.h>
#include <grace/filesystem.h>

// ========================================================================
// METHOD ::tocsv
// --------------
// Converts a value object to a string containing data in a quoted
// comma-separated value (CSV) format.
// ========================================================================
string *value::tocsv (bool withHeaders, const char *indexName)
{
	string *out = new string;
	value *child;
	int columnCount;
	int i;
	int row;
	
	// no data, no cookie
	if (! arraysz) return out;
	child = array[0];
	columnCount = child->count();
	
	// print the headers first, if we need them
	if (withHeaders)
	{
		out->printf ("\"%S\"", indexName);
		for (i=0; i<columnCount; ++i)
		{
			string hdr;
			
			if ((*child)[i].label())
				hdr = (*child)[i].label().str();
			else
				hdr.printf ("Value%i",i);

			out->printf (",\"%S\"", hdr.str());
		}
		out->printf ("\n");
	}
	
	string cdata;
	
	// print all rows
	for (row=0; row<arraysz; ++row)
	{
		// speed up the lookup of the current row
		child = array[row];
		
		if (child->label())
		{
			out->printf ("\"%S\",", child->name());
		}
		else
		{
			out->printf ("%i,", row);
		}
		
		// we keep to a strict columncount, if the child data is no
		// perfect two-dimensional array, it will be "right-sized".
		// Extra columns that only appear in later rows are discarded.
		for (i=0; i<columnCount; ++i)
		{
			switch ((*child)[i].itype)
			{
				case i_string:
				case i_ipaddr:
				case i_date:
				case i_bool:
					cdata = strutil::encodecsv ((*child)[i].sval());
					out->printf ("\"%S\"%s",
					     cdata.str(),
						 ((i+1)<columnCount) ? ",":"");
					break;

				case i_double:
					out->printf ("%f%s", (*child)[i].dval(), 
								 ((i+1)<columnCount) ? ",":"");
					break;
					
				case i_int:
					out->printf ("%i%s", (*child)[i].ival(),
								 ((i+1)<columnCount) ? ",":"");
					break;
				
				case i_currency:
					printcurrency (*out, (*child)[i].getcurrency());
					break;
			}
		}
		out->printf ("\n");
	}
	
	return out;
}

// ========================================================================
// METHOD ::savecsv
// ----------------
// Saves a value object to a file containing data in a quoted
// comma-separated value (CSV) format.
// ========================================================================
bool value::savecsv (const string &fileName, bool withHeaders,
					 const char *indexName)
{
	file csvFile;
	value *child;
	int columnCount;
	int i;
	int row;
	string tmpstr;
	
	// cowardly refuse to save an empty file
	if (! arraysz) return false;
	
	// bail out if they won't let us create the file
	if (! csvFile.openwrite (fileName))
	{
		return false;
	}
	
	// figure out the column count
	child = array[0];
	columnCount = child->count();
	
	// print the headers if asked for
	if (withHeaders)
	{
		if (!csvFile.printf ("\"%S\"", indexName)) return false;
		for (i=0; i<columnCount; ++i)
		{
			string hdr;
			
			if ((*child)[i]._name)
				hdr = (*child)[i]._name;
			else
				hdr.printf ("Value%i",i);

			if (!csvFile.printf (",\"%S\"", hdr.str())) return false;
		}
		if (! csvFile.printf ("\n")) return false;
	}
	
	// print the data
	
	string cdata;
	
	for (row=0; row<arraysz; ++row)
	{
		child = array[row];
		
		if (child->label())
		{
			if (!csvFile.printf ("\"%S\",", child->name())) return false;
		}
		else
		{
			if (!csvFile.printf ("%i,", row)) return false;	
		}
		for (i=0; i<columnCount; ++i)
		{
			switch ((*child)[i].itype)
			{
				case i_string:
				case i_ipaddr:
				case i_date:
				case i_bool:
					cdata = strutil::encodecsv ((*child)[i].sval());
					if (! csvFile.printf ("\"%s\"%s", cdata.str(),
											((i+1)<columnCount) ? ",":""))
						return false;
					break;

				case i_double:
					if (! csvFile.printf ("%f%s", (*child)[i].dval(),
										  ((i+1)<columnCount) ? ",":""))
						return false;
					break;

				case i_int:
					if (! csvFile.printf ("%i%s", (*child)[i].ival(),
											((i+1)<columnCount) ? ",":""))
						return false;
					break;
				
				case i_currency:
					printcurrency (tmpstr, (*child)[i].getcurrency());
					if (! csvFile.printf ("%s%s", tmpstr.str(),
										  ((i+1)<columnCount) ? ",":""))
						return false;
					break;
			}
		}
		if (! csvFile.printf ("\n")) return false;
	}
	csvFile.close();
	return true;
}

// ========================================================================
// METHOD ::fromcsv
// ----------------
// Converts a string containing CSV format data to a two-dimensional array
// of values.
// ========================================================================
bool value::fromcsv (const string &csvData, bool withHeaders,
					 const string &key)
{
	value headerNames;
	int rowc, colc;
	bool withKey;
	int keyPos;
	int numColumns, numRows;
	value rowSplit;
	value columnSplit;
	string row;

	if (key.strlen()) withKey = true;
	else withKey = false;
	keyPos = 0;
	
	clear();
	
	rowSplit = strutil::splitlines (csvData);
	rowc = 0;
	numRows = rowSplit.count();
	numColumns = 0;
	
	if (! numRows) return false;
	
	if (withHeaders)
	{
		row = rowSplit[0].sval();
		columnSplit = strutil::splitcsv (row);
		numColumns = columnSplit.count();
		if (! numColumns) return false;
		
		for (colc=0; colc<numColumns; ++colc)
		{
			headerNames.newval() = columnSplit[colc].sval();
			if (headerNames[-1].sval() == key)
			{
				keyPos = colc;	
			}
		}
		++rowc;
	}
	for (;rowc<numRows;++rowc)
	{
		row = rowSplit[rowc].sval();
		
		if (row.strlen())
		{
	 		columnSplit = strutil::splitquoted (row, ',');
 		
 			if (withKey)
 			{
 				(*this)[columnSplit[keyPos].sval()].type ("row");
 			}
 			else
 			{
 				newval("row");
 			}
 			for (colc=0; colc<numColumns; ++colc)
 			{
 				if (withHeaders)
 				{
 					(*this)[-1][headerNames[colc].sval()] =
 						columnSplit[colc];
 				}
 				else
 				{
 					(*this)[-1].newval() = columnSplit[colc];
  				}
 			}
 		}
	}
	return true;
}

// ========================================================================
// METHOD ::loadcsv
// ----------------
// Loads a file in CSV format into a two-dimensional array.
// ========================================================================
bool value::loadcsv (const string &filename, bool withHeaders,
					 const string &key)
{
	file csvFile;
	
	if (! csvFile.openread (filename))
		return false;
		
	value headerNames;
	int rowc, colc;
	int numColumns;
	bool withKey;
	int keyPos;
	value columnSplit;
	string row;
	
	clear();
	if (key.strlen()) withKey = true;
	else withKey = false;
	
	keyPos = 0;
	
	if (csvFile.eof())
	{
		csvFile.close();
		return false;
	}
	
	numColumns = 0;

	if (withHeaders)
	{
		row = csvFile.gets();
		columnSplit = strutil::splitcsv(row);
		numColumns = columnSplit.count();
		if (! numColumns)
		{
			csvFile.close();
			return false;
		}
		
		for (colc=0; colc<numColumns; ++colc)
		{
			if (withKey && (columnSplit[colc].sval() == key))
			{
				keyPos = colc;
			}
			headerNames.newval() = columnSplit[colc].sval();
		}
	}
	try
	{
		while (! csvFile.eof())
		{
			row = csvFile.gets();
			if (row.strlen())
			{
				columnSplit = strutil::splitcsv (row);
				
				if (withKey)
				{
					(*this)[columnSplit[keyPos].sval()].type ("row");
				}
				else
				{
					newval("row");
				}
				
				for (colc=0; colc<numColumns; ++colc)
				{
					if (withHeaders)
					{
						if (colc != keyPos)
						{
							(*this)[-1][headerNames[colc].sval()] =
								columnSplit[colc];
						}
					}
					else
					{
						(*this)[-1].newval() = columnSplit[colc];
					}
				}
			}
		}
	}
	catch (...)
	{
	}
	csvFile.close();
	return true;
}
