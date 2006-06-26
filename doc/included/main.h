/*!

\mainpage GNU Rapid Application Coding Environment (GRACE)

\section intro_sec Introduction
The \b grace library is a C++ toolkit for creating stable Unix services
with strong multithreading, messaging, connectivity and data exchange
with other programs.

\section intro_classes GRACE Classes

\subsection intro_classes_data Data containers
  - String container: \link string string \endlink
  - Static key string container: \link statstring statstring \endlink
  - Date container: \link timestamp timestamp \endlink
  - Dynamic array template: \link array<kind> array \endlink
  - Indexed array template: \link dictionary<kind> dictionary \endlink
  - Structured storage: \link value value \endlink

\subsection intro_classes_translation Data translation
  - Extra string manipulation and processing: \link strutil strutil \endlink
  - XML input and output translation: \link xmlschema xmlschema \endlink
  - Rich data validation: \link validator validator \endlink

\subsection intro_classes_io Input/Output classes
  - Filesystem abstraction with alias-paths: \link filesystem filesystem \endlink
  - File-based i/o blocking/non-blocking: \link file file \endlink
  - TCP socket: \link tcpsocket tcpsocket \endlink
  - TCP socket with client SSL encoding: \link sslsocket sslsocket \endlink
  - TCP listening socket: \link tcplistener tcplistener \endlink
  - SMTP socket: \link smtpsocket smtpsocket \endlink
  - HTTP socket: \link httpsocket httpsocket \endlink
  - HTTPS socket: \link httpssocket httpssocket \endlink
  - HTTP daemon: \link httpd httpd \endlink
  - VT100 terminal buffer: \link termbuffer termbuffer \endlink
  - VT100 command line interface: \link terminal terminal \endlink

\subsection intro_classes_tasks Task Management
  - Forked background task: \link process process \endlink
  - Background system task with a pipe: \link systemprocess systemprocess \endlink
  - POSIX thread with message queue: \link thread thread \endlink
  - POSIX read/write lock: \link lock<kind> lock \endlink
  - POSIX conditional: \link conditional conditional \endlink
  - Collection of POSIX worker threads with message queue: 
    - \link groupthread groupthread \endlink
    - \link threadgroup threadgroup \endlink

\subsection intro_classes_app Application Design
  - Command line app with argument parsing: \link application application \endlink
  - CGI-spec app with variable parsing: \link cgi cgi \endlink
  - Unix background process with logging: \link daemon daemon \endlink
  - Configuration and change control: \link configdb<appclass> configdb \endlink

\subsection intro_macros Macros:
  - foreach (node, collection) { ... } implemented through the 
    \link visitor<kind> visitor \endlink protocol.
  - caseselector (object) { ... } works on any native or object type that
    understands operator==.
  - exclusivesection (lockedobject) {...}, sharedsection (lockedobject) {...},
    breaksection {...} and unprotected (lockedobject) {...} all need a
    \link lock<kind> lock \endlink object.
*/
