// ========================================================================
// defaults.h: Collection of arbitrary choices for limits and defaults
//
// (C) Copyright 2005 Pim van Riezen <pi@madscience.nl>
//                    Madscience Labs, Rotterdam 
// ========================================================================

#ifndef _DEFAULTS_H
#define _DEFAULTS_H 1

#define KB *1024
#define MB *(1024*1024)
#define GB *(1024*1024*1024)

// --- Default sizes --------------------------------------------------------
/// Default maximum size of a file::log logfile.
#define DEFAULT_SZ_LOGFILE					    2 MB
/// Default size of the readbuffer of the file class.
#define DEFAULT_SZ_FILE_READBUF				    8 KB
/// Default size of the writebuffer of the file class.
#define DEFAULT_SZ_FILE_WRITEBUF			    8 KB

// --- Default size limits --------------------------------------------------
/// Default maximum HTTP post size for cgi objects.
#define DEFAULT_LIM_CGI_POSTSIZE				8 MB
/// Default maximum HTTP post size for httpd objects.
#define DEFAULT_LIM_HTTPD_POSTSIZE				2 MB
/// Default maximum chunk size for http chunked encoding.
#define DEFAULT_LIM_HTTP_CHUNKSIZE			  512 KB

// --- Random tunables ------------------------------------------------------
#define TUNE_HTTPD_MAINTHREAD_IDLE			    5
#define TUNE_HTTPD_WKTHREAD_MINROUNDS			5
#define TUNE_HTTPD_WKTHREAD_MINOVERHEAD			2
#define TUNE_HTTPD_KEEPALIVE_TRIG				2
#define TUNE_TCPLISTENER_BACKLOG				32

#endif
