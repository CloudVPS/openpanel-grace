#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/terminal.h>

class myshell : public application
{
public:
             myshell (void) : application ("com.panelsix.apps.myshell"),
                              shell (this, fin, fout)
             {
             }
            ~myshell (void);
            
    int      cmdPrintVersion (const value &args)
             {
                fout.printf ("MyShell version 1.0\n");
                return 0;
             }
    int      cmdListFiles (const value &args)
             {
                value dir;
                dir = fs.ls ();
                foreach (file, dir)
                {
                    fout.printf ("%s\n", file.id().str());
                }
                return 0;
             }
    int      cmdExit (const value &args) { return 1; }
    
    int      main (void)
             {
                shell.addsyntax ("show version", &myshell::cmdPrintVersion);
                shell.addsyntax ("show files", &myshell::cmdListFiles);
                shell.addsyntax ("exit", &myshell::cmdExit);
                
                shell.addhelp ("show", "Display information");
                shell.addhelp ("show version", "Application version");
                shell.addhelp ("show files", "Files in the directory");
                
                shell.run ("myshell$ ");
                return 0;
             }
    
    cli<myshell> shell;
};
