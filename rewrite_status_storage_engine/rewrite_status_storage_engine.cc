/*  Copyright (c) 2016, Airton Lastori. All rights reserved.

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
    02110-1301  USA */

#include <ctype.h>
#include <string.h>

#include <my_global.h>
#include <mysql/plugin.h>
#include <mysql/plugin_audit.h>
#include <mysql/service_mysql_alloc.h>
#include <my_thread.h> // my_thread_handle needed by mysql_memory.h
#include <mysql/psi/mysql_memory.h>

/* instrument the memory allocation */
#ifdef HAVE_PSI_INTERFACE
static PSI_memory_key key_memory_rewrite_status_storage_engine;

static PSI_memory_info all_rewrite_memory[]=
{
  { &key_memory_rewrite_status_storage_engine, "rewrite_status_storage_engine", 0 }
};

static int plugin_init(MYSQL_PLUGIN)
{
  const char* category= "sql";
  int count;

  count= array_elements(all_rewrite_memory);
  mysql_memory_register(category, all_rewrite_memory, count);
  return 0; /* success */
}
#else
#define plugin_init NULL
#define key_memory_rewrite_status_storage_engine PSI_NOT_INSTRUMENTED
#endif /* HAVE_PSI_INTERFACE */


static int rewrite_lower(MYSQL_THD thd, mysql_event_class_t event_class,
                         const void *event)
{
  if (event_class == MYSQL_AUDIT_PARSE_CLASS)
  {
    const struct mysql_event_parse *event_parse=
      static_cast<const struct mysql_event_parse *>(event);
    if (event_parse->event_subclass == MYSQL_AUDIT_PARSE_PREPARSE)
    {
      const char* strToCompare = "select @@storage_engine";
      bool eq = 1;
      for (unsigned i= 0; i < event_parse->query.length; i++)
      {
        if(strToCompare[i] != tolower(event_parse->query.str[i]))
        {
          eq = 0;
          break;
        }
      }

      if (eq == 1) 
      {
        const char* strToRewrite = "SELECT @@default_storage_engine";
        size_t query_length= strlen(strToRewrite);
        char *rewritten_query=
          static_cast<char *>(my_malloc(key_memory_rewrite_status_storage_engine,
                                       query_length + 1, MYF(0)));
                                       
        for (unsigned i= 0; i < query_length + 1; i++)
          rewritten_query[i]= strToRewrite[i];
      
        event_parse->rewritten_query->str = rewritten_query;
        event_parse->rewritten_query->length= query_length;
        
        *((int *)event_parse->flags)|=
                        (int)MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN;
      }
    }
  }

  return 0;
}

/* Audit plugin descriptor */
static struct st_mysql_audit rewrite_status_storage_engine_descriptor=
{
  MYSQL_AUDIT_INTERFACE_VERSION,                    /* interface version */
  NULL,                                             /* release_thd()     */
  rewrite_lower,                                    /* event_notify()    */
  { 0,
    0,
    (unsigned long) MYSQL_AUDIT_PARSE_ALL, }        /* class mask        */
};

/* Plugin descriptor */
mysql_declare_plugin(audit_log)
{
  MYSQL_AUDIT_PLUGIN,                           /* plugin type                   */
  &rewrite_status_storage_engine_descriptor,    /* type specific descriptor      */
  "rewrite_status_storage_engine",              /* plugin name                   */
  "Airton Lastori",                             /* author                        */
  "An query rewrite plugin to keep"
  " compatibility with old status"
  " variable storage_engine",                   /* description                   */
  PLUGIN_LICENSE_GPL,                           /* license                       */
  plugin_init,                                  /* plugin initializer            */
  NULL,                                         /* plugin deinitializer          */
  0x0002,                                       /* version                       */
  NULL,                                         /* status variables              */
  NULL,                                         /* system variables              */
  NULL,                                         /* reserverd                     */
  0                                             /* flags                         */
}
mysql_declare_plugin_end;