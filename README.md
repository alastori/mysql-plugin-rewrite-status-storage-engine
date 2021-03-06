
MySQL Plugin: Rewrite Status Variable Storage Engine
====================================================

## Synopsis

This is a simple preparse [MySQL Rewrite Query Plugin](https://dev.mysql.com/doc/refman/5.7/en/plugin-types.html#query-rewrite-plugin-type) that replaces the query ```SELECT @@storage_engine``` by ```SELECT @@default_storage_engine```. Works with MySQL 5.7.

## Example

```
mysql>  SELECT @@storage_engine;
+--------------------------+
| @@default_storage_engine |
+--------------------------+
| InnoDB                   |
+--------------------------+
1 row in set, 1 warning (0.00 sec)

mysql> SHOW WARNINGS;
+-------+------+----------------------------------------------------------------------------------------------------------+
| Level | Code | Message                                                                                                  |
+-------+------+----------------------------------------------------------------------------------------------------------+
| Note  | 1105 | Query 'SELECT @@storage_engine' rewritten to 'SELECT @@default_storage_engine' by a query rewrite plugin |
+-------+------+----------------------------------------------------------------------------------------------------------+
1 row in set (0.00 sec)
```

## Motivation

Some applications will try to query ```SELECT @@storage_engine;``` in MySQL 5.7 and will receive the error ```ERROR 1193 (HY000): Unknown system variable 'storage_engine'```. This plugin will replace the query with ```SELECT @@default_storage_engine``` preventing the error.

## Installation of compiled library

1. Put the rewrite_status_storage_engine.so library in your mysql plugin directory (the directory named by the MySQL plugin_dir system variable)
  * If you use Linux 64-bit, you can download the compiled library [rewrite_status_storage_engine.so here](https://github.com/alastori/mysql-plugin-rewrite-status-storage-engine/raw/master/rewrite_status_storage_engine.so)
  * If you want or need compile by yourself, put this project in plugin folder of [mysql/mysql-server](https://dev.mysql.com/doc/mysql-sourcebuild-excerpt/5.7/en/installing-development-tree.html) and compile (ex. ```cmake; make```)
2. Make sure the library has the right permissions:  
  ```chmod 755 rewrite_status_storage_engine.so```
3. Install the plugin in MySQL:  
  ```INSTALL PLUGIN rewrite_status_storage_engine SONAME 'rewrite_status_storage_engine.so';```
4. Check if plugin is installed and active:  
  ```
  mysql> SHOW PLUGINS\G  

     Name: rewrite_status_storage_engine
   Status: ACTIVE
     Type: AUDIT
  Library: rewrite_status_storage_engine.so
  License: GPL  
  ```

## How to use

After installation, just try it!
```
mysql> select @@version;
+-------------------------------------------+
| @@version                                 |
+-------------------------------------------+
| 5.7.15-enterprise-commercial-advanced-log |
+-------------------------------------------+
1 row in set (0.00 sec)

mysql>  SELECT @@storage_engine;
+--------------------------+
| @@default_storage_engine |
+--------------------------+
| InnoDB                   |
+--------------------------+
1 row in set, 1 warning (0.00 sec)

mysql> show warnings;
+-------+------+----------------------------------------------------------------------------------------------------------+
| Level | Code | Message                                                                                                  |
+-------+------+----------------------------------------------------------------------------------------------------------+
| Note  | 1105 | Query 'SELECT @@storage_engine' rewritten to 'SELECT @@default_storage_engine' by a query rewrite plugin |
+-------+------+----------------------------------------------------------------------------------------------------------+
1 row in set (0.00 sec)
```

## Uninstall

You can remove the plugin with:  
```mysql> UNINSTALL PLUGIN plugin 'rewrite_status_storage_engine';```

Optionally remove the library:  
```rm rewrite_status_storage_engine.so```

## Contributors

The code of this plugin was based on Oracle MySQL 5.7 [rewrite_example](https://github.com/mysql/mysql-server/tree/5.7/plugin/rewrite_example) plugin.

## License

[GNU General Public License (GPL) version 2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
