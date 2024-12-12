#include "common.h"

char* handle_get_request(const char *url) {
// initialize the JSON answer
    cJSON *json = cJSON_CreateObject();
    char *json_string;

#if GETCORK == 1
    cJSON_AddStringToObject(json, "status", "GETCORK");
    cJSON_AddStringToObject(json, "method", "GET");
    cJSON_AddStringToObject(json, "url", url);
    cJSON_AddNumberToObject(json, "httpcode", HTTP_OK);
    json_string = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    return json_string; // Caller is responsible for freeing this memory
#else
// initializing the variables
    char query[512]="";
    char schema[64];
    char table[64];
    char colname[64];
    char colvalue[64];
// check request format, parameter extraction & statement exec
  if (sscanf(url, "/v1/tables/%64[^/]/%64[^/]/%64[^/]/%64s", schema, table, colname, colvalue) == 4) {
// building query
      snprintf(query, sizeof(query), "SELECT * FROM %s.%s WHERE %s = '%s'", schema, table, colname, colvalue);
    } else if (strcmp(url, "/v1/status/") == 0) {
      snprintf(query, sizeof(query), "SHOW GLOBAL STATUS");
    } else if (strcmp(url, "/v1/") == 0) {
// healthcheck
      cJSON_AddStringToObject(json, "status", "OK");
      cJSON_AddNumberToObject(json, "httpcode", HTTP_OK);
// clean exit procedure w/ housekeeping
      json_string = cJSON_PrintUnformatted(json);
      cJSON_Delete(json);
      return json_string; // Caller is responsible for freeing this memory
    } else {
// request format is KO
  cJSON_AddStringToObject(json, "error", "Invalid GET request");
  cJSON_AddNumberToObject(json, "httpcode", HTTP_BAD_REQUEST);
// clean exit procedure w/ housekeeping
  json_string = cJSON_PrintUnformatted(json);
  cJSON_Delete(json);
  } // end if sscanbf
// we establish internal local connexion
        MYSQL *connection = mysql_init(NULL);
        if (mysql_real_connect_local(connection) == NULL) {
          cJSON_AddStringToObject(json, "status", "CONNECTION failed");
          cJSON_AddNumberToObject(json, "mariadbcode", mysql_errno(connection));
          cJSON_AddNumberToObject(json, "httpcode", HTTP_INTERNAL_SERVER_ERROR);
          // clean exit procedure
          json_string = cJSON_PrintUnformatted(json);
          cJSON_Delete(json);
          return json_string; // Caller is responsible for freeing this memory
        } else if (mysql_real_query(connection, STRING_WITH_LEN(query))) {
// executing the query
          cJSON_AddStringToObject(json, "status", "QUERY failed");
          cJSON_AddNumberToObject(json, "mariadbcode",mysql_errno(connection));
          cJSON_AddNumberToObject(json, "httpcode", HTTP_INTERNAL_SERVER_ERROR);
// clean exit procedure
          json_string = cJSON_PrintUnformatted(json);
          mysql_close(connection);
          cJSON_Delete(json);
          return json_string; // Caller is responsible for freeing this memory
        } else { // else if connect
// check resulset existence
          MYSQL_RES *resultset = mysql_store_result(connection);
          if (resultset == NULL) {
             cJSON_AddStringToObject(json, "status", "RESULT failed");
             cJSON_AddNumberToObject(json, "mariadbcode",mysql_errno(connection));
             cJSON_AddNumberToObject(json, "httpcode", HTTP_INTERNAL_SERVER_ERROR);
// clean exit procedure w/ housekeeping
             json_string = cJSON_PrintUnformatted(json);
             cJSON_Delete(json);
             mysql_close(connection);
             mysql_free_result(resultset);
          } else { // else if resultset
// resulset to json translation
             int num_rows = (int)mysql_num_rows(resultset);
             cJSON_AddNumberToObject(json, "rows", num_rows);
             if(num_rows > 0) {
// DATA FOUND
                cJSON_AddStringToObject(json, "status", "OK");
#if ARRAYCORK == 1
                cJSON_AddStringToObject(json, "data", "");
#else
                // Create a JSON array to hold all rows
                cJSON *rows_array = cJSON_CreateArray();
                unsigned int num_fields = mysql_num_fields(resultset);
                MYSQL_FIELD *fields = mysql_fetch_fields(resultset);
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(resultset))) {
// Create a JSON object for each row
                    cJSON *row_object = cJSON_CreateObject();
// handling of rows in the following format : "columnname":"columnvalue"
                    for (unsigned int i = 0; i < num_fields; i++) {
                        cJSON_AddStringToObject(row_object, fields[i].name, row[i] ? row[i] : "");
                    } // end for
 // Add the row object to the array
                   cJSON_AddItemToArray(rows_array, row_object);
              } // end while
              cJSON_AddItemToObject(json, "data", rows_array);
#endif // ARRAYCORK
             cJSON_AddNumberToObject(json, "mariadbcode",0);
             cJSON_AddNumberToObject(json, "httpcode", HTTP_OK);
// clean exit procedure w/ housekeeping
             json_string = cJSON_PrintUnformatted(json);
             cJSON_Delete(json);
             mysql_close(connection);
             mysql_free_result(resultset);
        } else { // else num_rows
// NO DATA FOUND
          cJSON_AddStringToObject(json, "status", "NO DATA FOUND");
          cJSON_AddNumberToObject(json, "mariadbcode", 0);
          cJSON_AddNumberToObject(json, "httpcode", HTTP_OK);
// clean exit procedure w/ housekeeping
          json_string = cJSON_PrintUnformatted(json);
          cJSON_Delete(json);
          mysql_close(connection);
          mysql_free_result(resultset);
        } // end if else num_rows
     } // end if resultset
     } // end if connect
return json_string; // Caller is responsible for freeing this memory
#endif // GETCORK
} // end function
