// global libs inclusion
//preventing inclusion duplication 
#ifndef GLOBALS_H
#define GLOBALS_H

// MariaDB headers
#ifndef MYSQL_DYNAMIC_PLUGIN
#define MYSQL_DYNAMIC_PLUGIN
#endif

// C shenanigans
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// MariaDB headers
#include <mysql.h>
#include <mysql/service_sql.h>

// cJSON headers
#include <cjson/cJSON.h>

//logging func header
#include "log.h"

// global variables sharing
// Defining the resources we want to check
// TODO : ability to declare the list in a config file / system variable
// extern const char *resources[] ;
// extern const int num_resources;

// grant verification
// extern int has_required_privilege(const char *user);

// check for resource exposition
// extern int is_exposed_resource(const char *url);

#define PLUGIN_NAME          "json2sql"
#define PLUGIN_AUTHOR        "Sylvain Arbaudie <arbaudie.it@gmail.com>"
#define PLUGIN_DESCRIPTION   "simple JSON-to-SQL API Plugin for MariaDB"

// corks definitions
#define GETCORK           0 // inside method handler
#define PUTCORK           1 // inside method handler
#define POSTCORK          1 // inside method handler
#define PATCHCORK         1 // inside method handler
#define DELETECORK        1 // inside method handler
#define HANDLERCORK       0 // global handler cork
#define GETMETHODCORK     0 // around method handler call
#define PUTMETHODCORK     1 // around method handler call
#define POSTMETHODCORK    1 // around method handler call
#define PATCHMETHODCORK   1 // around method handler call
#define DELETEMETHODCORK  1 // around method handler call

// defining use HTTP response codes
#define HTTP_OK                     200
#define HTTP_INTERNAL_SERVER_ERROR  500
#define HTTP_BAD_REQUEST            400
#define HTTP_UNAUTHORIZED           401
#define HTTP_FORBIDDEN              403
#define HTTP_NOT_FOUND              404
#define HTTP_METHOD_NOT_ALLOWED     405
#define HTTP_UNSUPPORTED_MEDIA_TYPE 415

// methodd <=> privilege translation
// #define GET    "select"
// #define PUT    "execute"
// #define POST   "insert"
// #define PATCH  "update"
// #define DELETE "delete"

// default exposed resources
// #define HEALTHCHECK_RESOURCE   "/v1/"
// #define STATUS_RESOURCE        "/v1/status/"
// #define SUBSCRIPTION_RESOURCE  "/v1/subscription/"
// #define RESOURCES_RESOURCE     "/v1/resources/"

// ease the use of mysql_real_query
#define STRING_WITH_LEN(X) (X), ((size_t) (sizeof(X) - 1))

// TODO : managing port via a system variable
#define PORT 3000
#define ADDRESS "0.0.0.0"

// TODO : managing credentials through JWTs and request body
// #define APIUSER "apiadmin"

#endif // GLOBALS_H
