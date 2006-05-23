#include "mysql.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <grace/str.h>
#include <grace/value.h>

extern "C" int			 dbdriver_init (void);
extern "C" void 		*dbdriver_open (const value &);
extern "C" value		*dbdriver_execute (void *, string &);
extern "C" void			 dbdriver_close (void *);
extern "C" const char		*dbdriver_error (void *);
extern "C" unsigned int		 dbdriver_insert_id (void *);

struct mysql_environment
{
	MYSQL			*mysql;
	MYSQL_RES		*mysql_res;
	MYSQL_ROW		 mysql_row;
};

extern "C" int dbdriver_init (void)
{
	return 1;
}

extern "C" void *dbdriver_open (const value &res)
{
	mysql_environment *myenv;
	char error [256];

	myenv = new mysql_environment;
	
	myenv->mysql = new MYSQL;

	mysql_init (myenv->mysql);
	int port = res["port"];
	if (! port) port = 3306;
	if (!mysql_real_connect (myenv->mysql, res["host"].cval(),
				 res["username"].cval(),
				 res["password"].cval(),
				 res["database"].cval(),
				 port,
				 NULL,
				 0))
	{
		delete (myenv->mysql);
		delete (myenv);
		sprintf (error, "unable to connect to database [%s@%s:%s]\n",
				res["username"].cval(), res["host"].cval(), res["database"].cval());
		fputs (error, stderr);
		return (NULL);
	}

	/*	
	if (mysql_select_db (myenv->mysql, res["database"].cval()))
	{
		mysql_close (myenv->mysql);
		delete (myenv->mysql);
		delete (myenv);
		sprintf (error, "unable to select database [%s@%s:%s]\n",
				res["username"].cval(), res["host"].cval(), res["database"].cval());
		fputs (error, stderr);
		return (NULL);
	}
	*/
	return ((void *) myenv);
}

extern "C" value *dbdriver_execute (void *E, string &sql)
{
	value *result = new value;

	mysql_environment *env = (mysql_environment *) E;
	int i;
	int row;
	int tsz;
	int rsz;
	int numf;
	MYSQL_FIELD *fld;
	value fields;
	
	if (env->mysql)
	{
		if (mysql_query (env->mysql, sql.str()) == 0)
		{
			row = 0;
			if (env->mysql_res = mysql_store_result (env->mysql))
			{
				while (fld = mysql_fetch_field (env->mysql_res))
				{
					if ((fld->type == FIELD_TYPE_DECIMAL) ||
						(fld->type == FIELD_TYPE_TINY) ||
						(fld->type == FIELD_TYPE_SHORT) ||
						(fld->type == FIELD_TYPE_LONG) ||
						(fld->type == FIELD_TYPE_INT24))
					{
						fields[(const char *) fld->name] = 1;
					}
					else
					{
						fields[(const char *) fld->name] = 0;
					}
				}
				
				while (env->mysql_row = mysql_fetch_row (env->mysql_res))
				{
					numf = mysql_num_fields (env->mysql_res);
					
					(void) (*result).newval();
					
					for (i=0; i<numf; ++i)
					{
						char *enc;
						if (env->mysql_row[i])
						{
							if (! strchr (env->mysql_row[i], '%'))
							{
								if (fields[i].ival())
								{
									(*result)[row][fields[i].name()] =
										::atoi (env->mysql_row[i]);
								}
								else
								{
									(*result)[row][fields[i].name()] =
										env->mysql_row[i];
								}
							}
							else
							{
								int   spos, dpos;
								char *dtmp = new char[strlen(env->mysql_row[i])];
								
								dpos = 0;
								
								for (spos=0; env->mysql_row[i][spos]; ++spos)
								{
									char *c = env->mysql_row[i]+spos;
									
									if (*c == '\\')
									{
										++c;
										++spos;
										dtmp[dpos++] = *c;
									}
									
									else if (*c != '%')
									{
										dtmp[dpos++] = *c;
									}
									else
									{
										if (c[1] == '%')
										{
											dtmp[dpos++] = *c;
											++spos;
										}
										else
										{
											if (c[1] && c[2])
											{
												char leftd, rightd;
												leftd = tolower(c[1]);
												rightd = tolower(c[2]);
												
												leftd = (((leftd>='0')&&(leftd<='9')) ? leftd - '0' : (leftd - 'a')+10 ) & 15;
												rightd = (((rightd>='0')&&(rightd<='9')) ? rightd - '0' : (rightd - 'a')+10 ) & 15;
												dtmp[dpos++] = rightd + (leftd << 4);
												spos += 2;
											}
										}
									}
								}
								dtmp[dpos] = 0;
								
								(*result)[row][fields[i].name()] = dtmp;
								delete[] dtmp;
							}
						}
						else
						{
							(*result)[row].newval() = "";
						}
					}
					++row;
				}
				mysql_free_result (env->mysql_res);
			}
		}
		else
		{
			delete result;
			return NULL;
		}
	}
	return (result);
}

extern "C" void dbdriver_close (void *E)
{
	mysql_environment *env = (mysql_environment *) E;
	
	if (E)
	{
		if (env->mysql)
		{
			mysql_close (env->mysql);
			delete (env->mysql);
		}
		delete (env);
	}
}

extern "C" const char *dbdriver_error (void *E)
{
	mysql_environment *env = (mysql_environment *) E;
	
	if (E)
	{
		if (env->mysql)
		{
			return mysql_error (env->mysql);
		}
	}
	return (char *) "";
}

extern "C" unsigned int dbdriver_insert_id (void *E)
{
	mysql_environment *env = (mysql_environment *) E;
	
	if (E)
	{
		return mysql_insert_id (env->mysql);
	}
	return 0;
}
