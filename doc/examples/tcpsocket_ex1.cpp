#include <grace/application.h>
#include <grace/tcpsocket.h>

/// RADB lookup application.
/// Connects to whois.radb.net and queries the first commandline argument.
class radblookupApp : public application
{
public:
         radblookupApp (void) : application ("example.app.radblookup")
         {
         }
        ~radblookupApp (void)
         {
         }

    int  main (void);
};

APPOBJECT(radblookupApp);

int radblookupApp::main (void)
{
    tcpsocket outsock;
    string line;
    
    // Handle connection-related exceptions.
    try
    {
        // Try to connect to the RADB whois host.
        if (outsock.connect ("whois.radb.net", 63))
        {
            // Send the whois query.
            outsock.printf ("%s\r\n", argv["*"][0].cval());
            
            // Get all reply data.
            while (! outsock.eof())
            {
                line = outsock.gets();
                fout.printf ("%s\n", line.str());
            }
            
            // Close the connection.
            outsock.close();
        }
        else // Connect failed.
        {
            ferr.printf ("%% Connection error\n");
            return 1;
        }
    }
    catch (...)
    {
        ferr.printf ("%% Connection exception\n");
        return 1;
    }
    return 0;
}
