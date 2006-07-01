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
            
    int      cmdEcho (const value &args)
             {
                bool first = true;
                value v = args;
                v.rmindex (0);
                foreach (arg, v)
                {
                    if (! first) { first = true; fout.puts (" "); }
                    fout.puts (arg.sval());
                }
                fout.puts ("\n");
                return 0;
             }
             
    int      cmdCalc (const value &args)
             {
                int result;
                
                caseselector (args[2])
                {
                    incaseof ("+") :
                        result = args[1].ival() + args[3].ival();
                        break;
                    
                    incaseof ("-"):
                        result = args[1].ival() - args[3].ival();
                        break;
                        
                    defaultcase:
                        fout.printf ("%% Unknown operand\n");
                        return 0;
                }
                fout.printf ("%i\n", result);
                return 0;
             }
             
    int      cmdExit (const value &args) { return 1; }
    
    int      main (void)
             {
                shell.addsyntax ("echo *", &myshell::cmdEcho);
                shell.addsyntax ("echo * #", &myshell::cmdEcho);
                shell.addsyntax ("calc * + *", &myshell::cmdCalc);
                shell.addsyntax ("calc * - *", &myshell::cmdCalc);
                shell.addsyntax ("exit", &myshell::cmdExit);
                shell.run ("myshell$ ");
                return 0;
             }
    
    cli<myshell> shell;
};
