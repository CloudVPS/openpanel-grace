THROWS_EXCEPTION (accessException, 0xe87a8081, "Access violation");

extern value ACCESSDB;

void performUpdate (const value &updateData, const statstring &user)
{
	if (ACCESSDB[user]["mayUpdate"] == false)
	{
		throw (accessException ("Unauthorized Update");
	}
	...
}

void performDelete (const value &deleteData, const statstring &user)
{
	if (ACCESSDB[user]["delete"] == false)
	{
		throw (accessException ("Unauthorized Delete");
	}
	...
}

void mainFunction (void)
{
	...
	try
	{
		...
		performUpdate (someData, "pete");
		...
		performDelete (someData, "pete");
	}
	catch (accessException e)
	{
		printerror ("Access violation: %s\n", e.description);
	}
}
