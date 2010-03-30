#include <grace/smtpd.h>
#include <grace/system.h>
#include <grace/filesystem.h>
#include <grace/defaults.h>

// ==========================================================================
// CONSTRUCTOR smtpd
// ==========================================================================
smtpd::smtpd (void)
{
	listenport = 0;
	minthr = 1;
	maxthr = 2;
	load.o = 0;
	mask = 0;
	_shutdown = false;
}

// ==========================================================================
// DESTRUCTOR smtpd
// ==========================================================================
smtpd::~smtpd (void)
{
}

// ==========================================================================
// METHOD smtpd::listento
// ==========================================================================
void smtpd::listento (int port)
{
	if (listenport) throw smtpdListenPortException();

	lsock.listento (port);
	listenport = port;
}

void smtpd::listento (ipaddress addr, int port)
{
	if (listenport) throw smtpdListenPortException();
	
	lsock.listento (addr, port);
	listenport = port;
}

// ==========================================================================
// METHOD smtpd::minthreads
// ==========================================================================
void smtpd::minthreads (int m)
{
	minthr = m;
}

// ==========================================================================
// METHOD smtpd::maxthreads
// ==========================================================================
void smtpd::maxthreads (int m)
{
	maxthr = m;
}

// ==========================================================================
// METHOD smtpd::start
// ==========================================================================
void smtpd::start (void)
{
	if (! listenport)
	{
		if (::getuid () == 0)
		{
			listento (25);
		}
		else
		{
			listento (2525);
		}
	}
	if (! banner.strlen())
	{
		banner = core.net.hostname();
	}
	spawn();
}

// ==========================================================================
// METHOD smtpd::shutdown
// ==========================================================================
void smtpd::shutdown (void)
{
	while (workers.count())
	{
		_shutdown = true;
		sleep (1);
	}
}

// ==========================================================================
// METHOD smtpd::setbannername
// ==========================================================================
void smtpd::setbannername (const string &str)
{
	banner = str;
}

// ==========================================================================
// METHOD smtpd::run
// ==========================================================================
void smtpd::run (void)
{
	int skimcount = 0;
	int tdelay = 0;
	int i;
	
	for (i=0; i<minthr; ++i)
		new smtpworker (this);
		
	while (! _shutdown)
	{
		sleep (tune::smtpd::mainthread::idle);
		int tload;
		
		sharedsection (load) { tload = load; }
		
		// Can we still take an extra connection?
		if ( (tload+1) >= workers.count() )
		{
			if (workers.count() < maxthr)
			{
				new smtpworker (this);
				tdelay = tune::smtpd::wkthread::minrounds;
			}
		}
		else if (! tdelay)
		{
			if (workers.count() > minthr)
			{
				if ((workers.count() - tload) >
									tune::smtpd::wkthread::minoverhead)
				{
					if (workers.count() != skimcount)
					{
						skimcount = workers.count();
						workers[0].sendevent ("die");
						tdelay = 5;
					}
				}
			}
		}
		if (tdelay) tdelay--;
	}
	
	for (i=workers.count(); i; --i)
	{
		workers[i-1].sendevent ("die");
	}
	while (workers.count())
	{
		sleep (1);
		workers.gc ();
	}
}

#define SENDERROR(str) { \
	if (parent->mask & SMTP_ERROR) \
		parent->eventhandle ( \
			$attr("class", "error") -> \
			$("thread", threadid) -> \
			$("error", str)); \
}

// ==========================================================================
// METHOD smtpworker::run
// ==========================================================================
void smtpworker::run (void)
{
	tcpsocket s; // Incoming socket.
	value ev; // Event data.
	string line; // Line of text as read from the socket.
	bool run = true; // True as long as we should be running.
	value env; // SMTP environment
	string helostr; // Remote host self-identification
	string threadid; // Stored thread-id
	string mailfrom; // Remote host mail from
	string myrcpt; // Remote host recipient argument.
	string mailbody; // Received mail body.
	int failcnt; // Failure counter.
	string exip; // Remote host ip.
	
	// Internal states.
	enum smtpstate
	{
		SMTP_WAITHELO,
		SMTP_WAITMAILFROM,
		SMTP_WAITRCPT,
		SMTP_WAITRCPTORDATA,
		SMTP_DATA,
		SMTP_QUIT
	};

	// Some initialization	
	smtpstate st = SMTP_WAITHELO;
	threadid.printf ("smtpworker/%i", tid);
	
	// Send an event if someone cares
	if (parent->mask & SMTP_INFO)
	{
		parent->eventhandle (
			$attr("class", "info") ->
			$("type", "threadstarted") ->
			$("thread", threadid));
	}
	
	// Loop
	while (run)
	{
		// Wait for a connection, if there's nothing to be had
		// we might as well check events.
		while (! parent->tcplock.trylockw(5))
		{
			ev = nextevent();
			if (ev)
			{
				// Oh no, the event of death!
				if (ev.type() == "die")
				{
					run = false;
					if (parent->mask & SMTP_INFO)
					{
						parent->eventhandle (
							$attr("class","info") ->
							$("type", "threadstopped") ->
							$("thread", threadid));
					}
					return;
				}
			}
		}
		
		// We got the lock, now try to accept a socket.
		while (! s)
		{
			s = parent->lsock.tryaccept (2.0);
			if (! s) // No socket, might as well check events.
			{
				ev = nextevent();
				if (ev)
				{
					if (ev.type() == "die")
					{
						parent->tcplock.unlock();
						run = false;
						return;
					}
				}
			}
		}

		// Jay, we have a working socket.
		
		int nload; // New load counter once we're up.
		
		// Get the new load number.
		parent->tcplock.unlock();
		
		exclusiveaccess (parent->load) { nload = parent->load.o++; }
		
		exip = s.peer_name;
		
		// Send an event for the connection, if someone wants it.
		if (parent->mask & SMTP_INFO)
		{
			parent->eventhandle (
				$attr("class", "info") ->
				$("type", "connectionaccepted") ->
				$("thread", threadid) ->
				$("load", nload) ->
				$("ip", exip));
		}
		
		st = SMTP_WAITHELO; // next state.
		
		try
		{
			// Send banner.
			s.puts ("220 %s ESMTP\r\n" %format (parent->banner));
	
mainloop:
			env["ip"] = exip;
			env["transaction-id"] = parent->maketransactionid ();
			while ((st != SMTP_QUIT) && (st != SMTP_DATA))
			{
				line = s.gets();
				if (line.strlen())
				{
					if (line.strcasecmp ("quit") == 0)
					{
						st = SMTP_QUIT;
						break;
					}
					if (line.strncasecmp ("auth plain", 10) == 0)
					{
						string inuser, inpass;
						int wpos = 0;
						
						line = line.mid (11);
						if (! line)
						{
							s.puts ("334\r\n");
							line = s.gets();
						}
						line = line.decode64();
						
						for (int i=0; i<line.strlen(); ++i)
						{
							if (line[i] == '\0') ++wpos;
							else
							{
								if (wpos==1) inuser.strcat (line[i]);
								else inpass.strcat (line[i]);
							}
						}
						if (parent->authplain (inuser, inpass, env))
						{
							s.puts ("235 Authenticated\r\n");
						}
						else
						{
							s.puts ("535 No\r\n");
						}
						continue;
					}
					if (line.strcasecmp ("auth login") == 0)
					{
						string inuser, inpass;
						s.puts ("334 VXNlcm5hbWU6\r\n");
						inuser = s.gets();
						inuser = inuser.decode64();
						s.puts ("334 UGFzc3dvcmQ6\r\n");
						inpass = s.gets();
						inpass = inpass.decode64();
						if (parent->authplain (inuser, inpass, env))
						{
							s.puts ("235 Authenticated\r\n");
						}
						else
						{
							s.puts ("535 No\r\n");
						}
						continue;
					}
					switch (st)
					{
						case SMTP_WAITHELO:
							if (line.strncasecmp ("HELO ", 5) == 0)
							{
								helostr = line.mid (5);
								env["helo"] = helostr;
								s.puts ("250 Hello %s\r\n" %format (exip));
								st = SMTP_WAITMAILFROM;
							}
							else if (line.strncasecmp ("EHLO ", 5) == 0)
							{
								env["helo"] = line.mid (5);
								s.puts ("250-Hello %s\r\n" %format (exip));
								s.puts ("250 AUTH LOGIN PLAIN\r\n");
								st = SMTP_WAITMAILFROM;
							}
							else
							{
								s.printf ("500 %s\r\n", errortext::smtpd::ucommand);
								SENDERROR(errortext::smtpd::ucommand);
							}
							break;
							
						case SMTP_WAITMAILFROM:
							if (line.strncasecmp ("MAIL FROM:", 10) == 0)
							{
								mailfrom = line.mid (10);
								mailfrom.chomp ();
								parent->normalizeaddr (mailfrom);
								
								env["from"] = mailfrom;
								s.printf ("250 OK\r\n");
								st = SMTP_WAITRCPT;
							}
							else
							{
								s.printf ("500 %s\r\n", errortext::smtpd::ucommand);
								SENDERROR(errortext::smtpd::ucommand);
							}
							break;
							
						case SMTP_WAITRCPTORDATA:
							if (line.strcasecmp ("DATA") == 0)
							{
								s.printf ("354 OK enter message\r\n");
								st = SMTP_DATA;
								break;
							}
							// NOTE intentional skipthrough!
						
						case SMTP_WAITRCPT:
							if (line.strncasecmp ("RCPT TO:", 8) == 0)
							{
								myrcpt = line.mid (8);
								myrcpt.chomp();
								parent->normalizeaddr (myrcpt);
								
								if (parent->checkrecipient (mailfrom, myrcpt, env))
								{
									env["rcpt"].newval() = myrcpt;
									s.printf ("250 OK Sure\r\n");
									st = SMTP_WAITRCPTORDATA;
								}
								else
								{
									s.printf ("550 NO GO\r\n");
								}
							}
							else
							{
								s.printf ("500 %s\r\n", errortext::smtpd::ucommand);
								SENDERROR(errortext::smtpd::ucommand);
							}
							break;
							
						default:
							throw smtpdPigsFlyException();
					}
				}
			}
			
			failcnt = 0;
			
			// Handling of DATA mode, wait for \r\n.\r\n.
			while (st != SMTP_QUIT)
			{
				line = s.gets();
				if (line == ".")
				{
					if (parent->deliver (mailbody, env))
					{
						if (parent->mask & SMTP_DELIVERY)
						{
							parent->eventhandle (
								$attr("class", "delivery") ->
								$("thread", threadid) ->
								$("from", env["from"]) ->
								$("rcpt", env["rcpt"]) ->
								$("size", mailbody.strlen()) ->
								$("status", 250) ->
								$("transaction-id", env["transactionid"]));
						}
						
						s.puts ("250 OK %s\r\n" %format (env["transaction-id"]));
						st = SMTP_WAITMAILFROM;
					}
					else
					{
						if (parent->mask & SMTP_DELIVERY)
						{
							parent->eventhandle (
								$attr("class", "delivery") ->
								$("thread", threadid) ->
								$("from", env["from"]) ->
								$("rcpt", env["rcpt"]) ->
								$("size", mailbody.strlen()) ->
								$("status", 550));
						}

						s.printf ("550 Failed\r\n");
						st = SMTP_WAITMAILFROM;
					}
					break;
				}
				else
				{
					mailbody.strcat (line);
					mailbody.strcat ("\r\n");
				}
			}
			
			if (st != SMTP_QUIT)
			{
				env.clear();
				goto mainloop;
			}
			
			s.printf ("221 Bye.\r\n");
			s.close();
		}
		catch (...)
		{
			SENDERROR(errortext::smtpd::pipe);
			s.close();
		}
		
		env.clear();
		exclusiveaccess (parent->load) { nload = parent->load.o--; }
		
		if (parent->mask & SMTP_INFO)
		{
			parent->eventhandle (
				$attr("class", "info") ->
				$("type", "connectionclosed") ->
				$("ip", exip) ->
				$("thread", threadid));
		}
	}
}

// ==========================================================================
// METHOD smtpd::maketransactionid
// ==========================================================================
string *smtpd::maketransactionid (void)
{
	returnclass (string) res retain;
	static bool initialized = false;
	
	if (! initialized)
	{
		initialized = true;
		srand (core.time.now());
	}
	
	res = "%08x-%04x-%08x" %format ((unsigned) core.time.now(),
									core.proc.self(), rand());
	return &res;
}

// ==========================================================================
// METHOD smtpd::checkrecipient
// ==========================================================================
bool smtpd::checkrecipient (const string &mailfrom, const string &rcptto,
							value &env)
{
	return true;
}

// ==========================================================================
// METHOD smtpd::deliver
// ==========================================================================
bool smtpd::deliver (const string &mailbody, value &env)
{
	string transid;
	string fn_dat;
	string fn_xml;
	
	transid = env["transaction-id"];
	fn_dat = "%S.dat" %format (transid);
	fn_xml = "%S.xml" %format (transid);
	
	fs.save (fn_dat, mailbody);
	env.savexml (fn_xml);
	return true;
}

bool smtpd::authplain (const string &user, const string &pass,
					   value &env)
{
	return false;
}

// ==========================================================================
// METHOD smtpd::eventhandle
// ==========================================================================
void smtpd::eventhandle (const value &ev)
{
}

// ==========================================================================
// METHOD smtpd::normalizeaddr
// ==========================================================================
void smtpd::normalizeaddr (string &str)
{
	str = str.filter ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
					  "0123456789+-%@_./^!#$&=:;,~");
}
