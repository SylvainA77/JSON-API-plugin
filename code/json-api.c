#include <mysql.h>
#include <ctype.h>
#include <string.h>
#include <microhttpd.h>
#include <cjson/cJSON.h>

#define PLUGIN_NAME          "JSON2SQL"
#define PLUGIN_AUTHOR        "Sylvain Arbaudie"
#define PLUGIN_DESCRIPTION   "JSON API Plugin for MariaDB"
#define PLUGIN_LICENSE_GPL   "GNU AGPL v3"

// TODO : managing port via a system variable
#define PORT 3000

// TODO : managing credentials through JWTs and request body
#define APIUSER "apiadmin"
#define APIPASSWD "Ap1-4dmiN"

// Plugin declaration structure
static struct st_mysql_daemon json_api_plugin = {
    MYSQL_DAEMON_INTERFACE_VERSION
};

// Defining the resources we want to check
// TODO : ability to declare the list in a config file / system variable
const char *resources[] = {
    "/v1/tables/",
    "/v1/procedures/",
    "/v1/procedures/"
};
const int num_resources = sizeof(resources) / sizeof(resources[0]);

// Function to check if the requested URL is a valid resource
int is_valid_resource(const char *url) {
    for (int i = 0; i < num_resources; i++) {
        if (strcmp(url, resources[i]) == 0) {
            return 1; // Valid resource found
        }
    }
    return 0; // No valid resource found
}

// TODO : stick to the HTTP API return codes best practices
static int send_json_response(struct MHD_Connection *connection, const char *json_string) {
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(json_string), (void *)json_string, MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(response, "Content-Type", "application/json");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}

static char* handle_get_request(const char *url) {
    char schema[64];  
    char table[64];
    char pkname[64];
    char pkvalue[64];
    char procname[64];
    char query[512]="";
    
    if (sscanf(url, "/v1/tables/%64[^/]/%64[^/]/%64[^/]/%64s", schema, table, colname, colvalue) == 4) {   
        // Here query database
        snprintf(query, sizeof(query), "SELECT * FROM %s.%s WHERE %s = '%s'", schema, table, colname, colvalue);
        // initialize the JSON answer
        cJSON *json = cJSON_CreateObject();
        cJSON_AddStringToObject(json, "source", "tables");
        cJSON_AddStringToObject(json, "schema", schema);
        cJSON_AddStringToObject(json, "table", table);
        cJSON_AddStringToObject(json, "column-name", colname);
        cJSON_AddStringToObject(json, "column-value", colvalue);
//    } else if (sscanf(url, "/v1/procedures/%64[^/]/%64s", schema, procname) == 2) { 
//        // Here query database
//        snprintf(query, sizeof(query), "'use %s; call %s()'", schema, procname);
//        // initialize the JSON answer
//        cJSON *json = cJSON_CreateObject();
//        cJSON_AddStringToObject(json, "source", "procedure");
//        cJSON_AddStringToObject(json, "schema", schema);
//        cJSON_AddStringToObject(json, "procname", procname);
    }

    if (query[0] != '\0') {
        MYSQL *conn = mysql_init(NULL);
        if (conn == NULL) {
           fprintf(stderr, "mysql_init() failed\n");
           cJSON_AddStringToObject(json, "status", "INIT failed");
           char *json_string = cJSON_PrintUnformatted(json);
           cJSON_Delete(json);
           return json_string; // Caller is responsible for freeing this memory
        }
        
        if (mysql_real_connect(conn, "localhost", APIUSER, APIPASSWD, schema, 0, NULL, 0) == NULL) {
           fprintf(stderr, "mysql_real_connect() failed\n");
           mysql_close(conn);
           cJSON_AddStringToObject(json, "status", "CONNECT failed");
           char *json_string = cJSON_PrintUnformatted(json);
           cJSON_Delete(json);
           return json_string; // Caller is responsible for freeing this memory
        }
      
        if (mysql_query(conn, query)) {
           fprintf(stderr, "mysql_query() failed\n");
           mysql_close(conn);
           cJSON_AddStringToObject(json, "status", "QUERY failed");
           char *json_string = cJSON_PrintUnformatted(json);
           cJSON_Delete(json);
           return json_string; // Caller is responsible for freeing this memory
        }
        
       MYSQL_RES *result = mysql_store_result(conn);
       if (result == NULL) {
          fprintf(stderr, "mysql_store_result() failed\n");
          mysql_close(conn);
          cJSON_AddStringToObject(json, "status", "RESULT failed");
          char *json_string = cJSON_PrintUnformatted(json);
          cJSON_Delete(json);
          return json_string; // Caller is responsible for freeing this memory
       }

       unsigned long long num_rows = mysql_num_rows(result);

        if (num_rows > 0) {
           cJSON_AddStringToObject(json, "status", "OK");
           cJSON_AddNumberToObject(json, "rows", (double)num_rows);

           // Create a JSON array to hold all rows
           cJSON *rows_array = cJSON_CreateArray();

           unsigned int num_fields = mysql_num_fields(result);
           MYSQL_FIELD *fields = mysql_fetch_fields(result);

           MYSQL_ROW row;
           while ((row = mysql_fetch_row(result))) {
                 // Create a JSON object for each row
                 cJSON *row_object = cJSON_CreateObject();
                 // handling of rows in the following format : "columnname":"columnvalue"
                 for (unsigned int i = 0; i < num_fields; i++) {
                     cJSON_AddStringToObject(row_object, fields[i].name, row[i] ? row[i] : "");
                 }
        
                // Add the row object to the array
                cJSON_AddItemToArray(rows_array, row_object);
           }
           cJSON_AddItemToObject(json, "data", rows_array);
        } else {
           cJSON_AddStringToObject(json, "status", "NO DATA FOUND");
           cJSON_AddNumberToObject(json, "rows", 0);
        }

    }
      // request format is KO
      return NULL;
}

static char* handle_post_request(const char *url) {

  // request format is KO  
  return NULL;
}

static char* handle_put_request(const char *url) {

  // request format is KO  
  return NULL;
}

// v1 : "point delete" only
static char* handle_delete_request(const char *url) {

    // request format is KO
    return NULL;
}

static int request_handler(void *cls, struct MHD_Connection *connection,
                           const char *url, const char *method,
                           const char *version, const char *upload_data,
                           size_t *upload_data_size, void **con_cls) {

if (strcmp(method, "GET") == 0) {
char *response = handle_get_request(url);
    if (response) {
        int ret = send_json_response(connection, response);
        free(response); // Free the allocated JSON string
        return ret;
    } else {
        return send_json_response(connection, "{\"message\": \"Invalid GET request\"}");
    }
    } else if (strcmp(method, "POST") == 0) {
char *response = handle_post_request(url);
    if (response) {
        int ret = send_json_response(connection, response);
        free(response); // Free the allocated JSON string
        return ret;
    } else {
        return send_json_response(connection, "{\"message\": \"Invalid POST request\"}");
    }
    } else if (strcmp(method, "PUT") == 0) {
char *response = handle_put_request(url);
    if (response) {
        int ret = send_json_response(connection, response);
        free(response); // Free the allocated JSON string
        return ret;
    } else {
        return send_json_response(connection, "{\"message\": \"Invalid PUT request\"}");
    }
    } else if (strcmp(method, "DELETE") == 0) {
char *response = handle_delete_request(url);
    if (response) {
        int ret = send_json_response(connection, response);
        free(response); // Free the allocated JSON string
        return ret;
    } else {
        return send_json_response(connection, "{\"message\": \"Invalid DELETE request\"}");
    }
    } else {
        // Method not supported
        return send_json_response(connection, "{\"error\": \"Unsupported HTTP method\"}");
    }
}

static int json_api_plugin_init(void *p) {
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_DEBUG,
                              PORT, NULL, NULL,
                              &request_handler, NULL,
                              MHD_OPTION_END);

    if (daemon == NULL) {
       fprintf(stderr, "Failed to start server\n");
       return 1;
    }
    printf("Server running on port %d\n", PORT);
    return 0;
}

static int json_api_plugin_deinit(void *p) {
    if (daemon != NULL) {
        MHD_stop_daemon(daemon);
        daemon = NULL;
        printf("HTTP server stopped.\n");
    }
    return 0;
}

// Plugin descriptor
mysql_declare_plugin(json_api)
{
    MYSQL_DAEMON_PLUGIN,
    &json_api_plugin,
    PLUGIN_NAME,
    PLUGIN_AUTHOR,
    PLUGIN_DESCRIPTION,
    PLUGIN_LICENSE_GPL,
    json_api_plugin_init,
    json_api_plugin_deinit,
    0x0100,
    NULL,
    NULL,
    NULL,
    0
}
mysql_declare_plugin_end;
