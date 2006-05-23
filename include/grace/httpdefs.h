// ========================================================================
// httpdefs.h HTTP return code defenitions
//
// (C) Copyright 2004 Pim van Riezen <pi@madscience.nl>
//                    Madscience Labs, Rotterdam 
// ========================================================================

#ifndef _HTTP_DEFENITIONS_H
#define _HTTP_DEFENITIONS_H



//
// INFORMATIONAL
//
#define HTTP_CONTINUE					100
#define HTTP_SWITCH_PROTOCOLS			101


//
// SUCCESSFUL
//
#define HTTP_OK							200
#define HTTP_CREATED                    201
#define HTTP_ACCEPTED                   202
#define HTTP_NONAUTH                    203
#define HTTP_NO_CONTENT                 204
#define HTTP_RESET_CONTENT              205
#define HTTP_PARTIAL_CONTENT            206

//
// REDIRECTION
//
#define HTTP_MULTIPLE_CHOICES           300
#define HTTP_MOVED_PERM                 301
#define HTTP_FOUND                      302
#define HTTP_SEE_OTHER                  303
#define HTTP_NOT_MODIFIED				304
#define HTTP_USE_PROXY					305

// Deprecated
// #define HTTP_							306
#define HTTP_TEMP_REDIRECT				307


//
// CLIENT ERRORS
//
#define HTTP_BAD_REQUEST                400
#define HTTP_UNAUTHORIZED               401
#define HTTP_PAYMENT_REQUIRED			402		// prefer using a 404
#define HTTP_FORBIDDEN                  403
#define HTTP_NOT_FOUND                  404
#define HTTP_METHOD_NOT_ALLOWED         405
#define HTTP_NOT_ACCEPTABLE             406
#define HTTP_PROXY_AUTH_REQUIRED		407
#define HTTP_REQUEST_TIMEOUT			408
#define HTTP_CONFILCT					409
#define HTTP_GONE						410
#define HTTP_LENGTH_REQUIRED			411
#define HTTP_PRECONDITION_FAILED		412
#define HTTP_ENTITY_TOO_LARGE           413
#define HTTP_URI_TOO_LONG               414
#define HTTP_UNSUPPORTED_MEDIA			415
#define HTTP_RANGE_NOT_SATISFIABLE		416
#define HTTP_EXPECTATION_FAILED			417

//
// SERVER ERRORS
//
#define HTTP_INTERNAL_ERROR             500
#define HTTP_NOT_IMPLEMENTED            501
#define	HTTP_BAD_GATEWAY				502
#define HTTP_SERVICE_UNAVAILABLE		503
#define HTTP_GATEWAY_TIMEOUT			504
#define HTTP_UNSUPPORTED_VER			505		// unsopported version



#endif
























