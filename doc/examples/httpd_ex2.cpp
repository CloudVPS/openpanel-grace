class helloworld : public httpdobject
{
public:
                 helloworld (httpd &pparent, const string &match)
                 	: httpdobject (pparent, match)
                 {
                 }
                ~helloworld (void)
                 {
                 }
                 
    int          run (string &uri, string &postbody,
                      value &inhdr, string &out, value &outhdr,
                      value &env, tcpsocket &s);
};

int helloworld::run (string &uri, string &postbody, value &inhdr,
                     string &out, value &outhdr, value &env,
                     tcpsocket &s)
{
    out = "<html><body><h1>Hello, world</h1></body></html>";
    outhdr["Content-type"] = "text/html";
    return 200;
}
