#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "config.h"

#define LINE_MAX 2048
                                                                                
char facility[256], priority[256], path[256], action[256];
FILE *ifp, *ofp = NULL;
int line_max = LINE_MAX;
                                                                                
                                                                                
/* Utility function that reads the lines of the conf file and outputs each rule,   one per line, to a new temporary file. It calls the function parse_list() to
   do this. Returns 0 on success and 1 on error */
                                                                                
int setting_read()
{
  char *cline, *cbuf = NULL;
  char *p = NULL;
  int lineno = 0;
  int ret = 0;
  FILE *fp = NULL;
                                                                                
                                                                                
  /* opening the temp file, as we do not want to be working on the main syslog
     file as of now. The syslog.conf file is copied to a temp file */
  fp = fopen(SYSLOG_CONF_DIR "/" SYSLOG_CONF_FILE, "r");
  if ( fp == NULL ) {
    fprintf(stderr, 
	    "Error in opening syslog configuration file(%s)\n", 
	    SYSLOG_CONF_DIR "/" SYSLOG_CONF_FILE);
    return 1;
  }
  cbuf = (char *)malloc(LINE_MAX);
  if ( cbuf == NULL ) {
    fprintf(stderr, "Cannot allocate space for configuration\n");
    ret = 1;
    return ret;
  }
  cline = cbuf;
  while ( fgets(cline, line_max, fp) != NULL) {
    for (p = cline; isspace(*p); ++p) ;
                                                                                
    if (
	*p == '\0'
	|| *p == '#'
	/* Ignore directive of rsyslog own.
	   TODO: $IncludeConfig should be handled. */
	|| *p == '$'
	) {
      lineno++;
      continue;
   }
    lineno++;
    strcpy(cline, p);
    ret = parse_list(cbuf);
  }
  fclose(fp);
  free(cbuf);
                                                                                
  return ret;
}
                                                                                
/* Utility function used by the setting_read function for splitting the setting
   rules. Returns 0 on success and 1 on error */
                                                                                
int parse_list( const char *line )
{
  char *bp, *p, *q = NULL;
  char buf[LINE_MAX];
  char fac[256];
  char prio[256];
  char path[256];
  char outbuf[2000];
                                                                                
                                                                                
  bzero(&buf, sizeof(buf));
  bzero(&outbuf, sizeof(outbuf));
  bzero(&fac, sizeof(fac));
  bzero(&prio, sizeof(prio));
  bzero(&path, sizeof(path));

  p = strdup((char*)line);
  q = strtok(p, "\t ");
  q = strtok(NULL, "\n");
  while(isspace(*q)) q++;
  strcpy(path, q);
  path[strlen(path)] = '\n';
                                                                                
  free(p);
  p = NULL;
  q = NULL;
                                                                                
  for (p=(char *)line; *p && *p != '\t' && *p != ' ';) {
    for (q=p; *q && *q != '\t' && *q++ != '.'; ) continue;
    for (bp=buf; *q && !strchr("\t ,;", *q); )
      *bp++ = *q++;
    *bp = '\0';
                                                                                
    while (strchr(",;", *q))
      q++;
                                                                                
    bp = buf;
    strcpy(prio, bp);
                                                                                
     while (*p && !strchr("\t .;", *p)) {
       for (bp=buf; *p && !strchr("\t ,;.", *p); )
         *bp++ = *p++;
       *bp = '\0';
       strcpy(fac, buf);
       strcpy(outbuf, fac);
       strcat(outbuf, "\t");
       strcat(outbuf, prio);
       strcat(outbuf, "\t");
       strcat(outbuf, path);
       fputs(outbuf, stdout);
       while (*p == ',' || *p == ' ')
                    p++;
     }
     p = q;
   }
                                                                                
   return 0;
}

int main()
{
 setting_read();
}
