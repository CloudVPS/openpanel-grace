#include <grace/application.h>
#include <grace/str.h>
#include <grace/array.h>

class myApp : public application
{
             myApp (void) : application ("tld.example.app.test.1")
             {
             }
            ~myApp (void);
    
    int      main (void);
};

class messageDestination
{
            messageDestination (const string &pHost, int pPort)
            {
                hostName = pHost;
                port = pPort;
            }
            
    string  hostName;
    int     port;
};

int myApp::test (void)
{
    array<messageDestination> dest;

    dest.add (new messageDestination ("localhost",1515));
    dest.add (new messageDestination ("192.168.10.10",4343));
    
    for (int i=0; i < dest.count(); ++i)
    {
        fout.printf ("Host %s port %i\n", dest[i].hostName.str(),
                     dest[i].port);
    }
    return 0;
}
