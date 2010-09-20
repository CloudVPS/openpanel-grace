#include <grace/application.h>
#include <grace/filesystem.h>
#include <querido/sqlite.h>
#include <querido/table.h>

class queridotestApp : public application
{
public:
		 	 queridotestApp (void) :
				application ("grace.testsuite.querido")
			 {
			 }
			~queridotestApp (void)
			 {
			 }

	int		 main (void);
};

APPOBJECT(queridotestApp);

#define FAIL(foo) { ferr.printf (foo "\n"); return 1; }

int queridotestApp::main (void)
{
	dbengine DB (dbengine::SQLite);
	if (! DB.open ($("path","db.sqlite"))) FAIL ("dbopen");
	
	value vtmp;
	DB.query ("PRAGMA table_info(users)", vtmp);
	vtmp.savexml ("table.users.xml");
	DB.query ("PRAGMA table_info(messages)", vtmp);
	vtmp.savexml ("table.messages.xml");
	
	dbtable User (DB, "users");
	dbtable Message (DB, "messages");
	dbquery Q (DB);

	Q.select (Message["sender"].as("sender"),
			  Message["subject"],
			  Message["date"]);
	Q.from (User,Message);
	Q.where ((User["id"] == Message["rcpt"]) && 
			 (User["name"] == "john"));
	Q.orderby (Message["date"]);
	Q.limit (5);

	fs.save ("out.sql", Q.sqlquery());
	value res = Q.exec ();
	res.savexml ("out.xml");
	
	User.setindexcolumn ("name");
	fout.writeln ("** rowexist=%s" %format (User.rowexists("pi") ? "y":"n"));
	fout.writeln ("** name=%s" %format (User.row("pi")["name"]));
	res = User.row ("pi");
	res.savexml ("row.xml");
	
	return 0;
}

