THROWS_EXCEPTION (myCustomException, 0xe87a8081, "Custom exception");

try
{
	...
	if (nastystuff) throw (myCustomException());
}
catch (exception e)
{
	if (e.code == myCustomExceptionClass::getcode())
	{
		// handle custom exception.
	}
}
