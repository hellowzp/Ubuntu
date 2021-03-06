#ifndef _SENSOR_DB_H_
#define _SENSOR_DB_H_

#include <mysql.h>
#include <stdio.h>
#include "config.h"

#ifdef GROUPT

#ifndef HOST
#define HOST "studev.groept.be"
#endif

#ifndef DATABASE
#define DATABASE "a13_syssoft"
#endif

#ifndef USER
#define USER "a13_syssoft"
#endif

#ifndef PASSWORD
#define PASSWORD "a13_syssoft"
#endif

#else
#define HOST "192.168.56.1"
#define DATABASE "sensors"
#define USER "remote_root"
#define PASSWORD "secret"

#endif

#define TABLE_NAME "lu_xuemei"
/*
 * Make a connection to MySQL database
 * Create a table named 'yourname' if the table does not exist
 * If the table existed, clear up the existing data if clear_up_flag is set to 1
 * return the connection for success, NULL if an error occurs
 */
MYSQL *init_connection();

/*
 * Disconnect MySQL database
 */
void disconnect(MYSQL *conn);

/*
 * Write an INSERT query to insert a single sensor measurement
 * return zero for success, and non-zero if an error occurs
 */
int insert_sensor(MYSQL *conn, sensor_id_t id, sensor_value_t value, sensor_ts_t ts);

/*
 * Write a SELECT query to return all sensor measurements existed in the table
 * return MYSQL_RES with all the results
 */
MYSQL_RES *find_sensor_all(MYSQL *conn);

/*
 * Return the number of records containing in the result
 */
int get_result_size(MYSQL_RES *result);

/*
 * Print all the records containing in the result
 */
void print_result(MYSQL_RES *result);

void free_result(MYSQL_RES* result);


#ifdef SENSOR_DB_EXTRA
  /*
  * Write an INSERT query to insert all sensor measurements existed in the file pointed by sensor_data
  * return zero for success, and non-zero if an error occurs
  */
  int insert_sensor_from_file(MYSQL *conn, FILE *sensor_data);

  /*
  * Write a SELECT query to return all sensor measurements containing 'value_t'
  * return MYSQL_RES with all the results
  */
  MYSQL_RES *find_sensor_by_value(MYSQL *conn, sensor_value_t value_t);

  /*
  * Write a SELECT query to return all sensor measurement that its value exceeds 'value_t'
  * return MYSQL_RES with all the results
  */
  MYSQL_RES *find_sensor_exceed_value(MYSQL *conn, sensor_value_t value_t);

  /*
  * Write a SELECT query to return all sensor measurement containing timestamp 'ts_t'
  * return MYSQL_RES with all the results
  */
  MYSQL_RES *find_sensor_by_timestamp(MYSQL *conn, sensor_ts_t ts_t);

  /*
  * Write a SELECT query to return all sensor measurement recorded latter than timestamp 'ts_t'
  * return MYSQL_RES with all the results
  */
  MYSQL_RES *find_sensor_latter_timestamp(MYSQL *conn, sensor_ts_t ts_t);
#endif //SENSOR_DB_EXTRA

#endif //_SENSOR_DB_H_ 

