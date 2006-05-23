void myclass::printFileNames (value &listOfFileNames)
{
    for (int i=0; i < listOfFileNames.count(); ++i)
    {
        string fName = listOfFileNames[i];
        string fBase;
        
        fBase = fName.cutatlast ('.');
        if (fBase)
        {
            // Print out filename sans extension.
            fout.printf ("%s\n", fBase.str());
        }
        else // Filename had no extension.
        {
            fout.printf ("%s\n", fName.str());
        }
    }
}
