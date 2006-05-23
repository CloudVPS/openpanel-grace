#include <grace/application.h>
#include <grace/filesystem.h>
#include <grace/smtpd.h>
#include <grace/smtp.h>

class smtptestApp : public application
{
public:
		 	 smtptestApp (void) :
				application ("grace.testsuite.smtp")
			 {
			 }
			~smtptestApp (void)
			 {
			 }

	int		 main (void);
};

class mysmtpd : public smtpd
{
public:
				 mysmtpd (void) : smtpd ()
				 {
				 	mask = 0;
				 }
				~mysmtpd (void)
				 {
				 }
				
	bool		 checkrecipient (const string &mf, const string &rc,
								 value &env)
				 {
				 	return true;
				 }
	bool		 deliver (const string &body, value &env)
				 {
				 	fs.save ("out.msg", body);
				 	env.rmval ("transaction-id");
				 	env.rmval ("helo");
				 	env.savexml ("out.xml");
				 	return true;
				 }
};

APPOBJECT(smtptestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int smtptestApp::main (void)
{
	mysmtpd sd;
	smtpsocket ss;
	sd.listento (8525);
	sd.start ();
	
	ss.setsmtphost ("localhost", 8525);
	ss.setsender ("pi@madscience.nl", "Pim van Riezen");
	if (! ss.sendmessage ("test@test.test", "test", "testing one two three"))
	{
		ferr.printf ("FAIL sendmessage: %s\n", ss.error().str());
		return 1;
	}
	sd.shutdown();
	return 0;
}

