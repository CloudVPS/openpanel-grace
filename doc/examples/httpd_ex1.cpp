#include <grace/daemon.h>
#include <grace/httpd.h>

/// Web Server.
/// Implements the HTTP protocol on port 8080.
/// Serves files from /var/www/sites.
class webserverApp : public daemon
{
public:
             webserverApp (void) : daemon ("tld.example.app.webserver")
             {
             }
            ~webserverApp (void)
             {
             }
             
    int      main (void);
};

bool _shouldShutDown; ///< True if a SIGTERM was received.
APPOBJECT(webserverApp);

/// Signal handler for SIGTERM.
void termHandler (int sig)
{
    _shouldShutDown = true;
}

int webserverApp::main (void)
{
    value vhosts;
    
    vhosts["*"] = "default"; // /var/www/sites/default
    vhosts["www.example.tld"] = "example"; // /var/www/sites/example
    
    // Spawn to background.
    daemonize();
    
    // Set handler for SIGTERM.
    signal (SIGTERM, termHandler);

    // Set up the httpd object. 
    httpd server (8080);
    server.minthreads (2);
    server.maxthreads (16);
    server.systempath ("/var/www");
    
    // Set up http logging.
    new httpdlogger (server, "/var/www/log/access.log", "/var/www/log/error.log");
    
    // Set up vhosts.
    new httpdvhost (server, vhosts);
    
    // Set up file serving.
    new httpdfileshare (server, "*", "/var/www/sites");
    
    // Start http server.
    server.start ();
    _shouldShutDown = false;

    while (! _shouldShutDown)
    {
        sleep (1);
    }
    
    server.shutdown ();
    return 0;
}
