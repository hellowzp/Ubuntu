#include <my_global.h>
#include <mysql.h>

#define MAX_VALUE 100

int main(int argc, char **argv)
{

  MYSQL *conn;
  MYSQL_RES *result;
  MYSQL_ROW row;
  int num_fields;
  int i;

  conn = mysql_init(NULL);
  mysql_real_connect(conn, "localhost", "zetcode", "passwd", "testdb", 0, NULL, 0);

  mysql_query(conn, "SELECT sensor_value,sensor_id FROM sensor where sensor_value>MAX_VALUE");
  result = mysql_store_result(conn);

  num_fields = mysql_num_fields(result);

  while ((row = mysql_fetch_row(result)))
  {
      for(i = 0; i < num_fields; i++)
      {
          printf("%s ", row[i] ? row[i] : "NULL");
      }
      printf("\n");
  }

  mysql_free_result(result);
  mysql_close(conn);

}

