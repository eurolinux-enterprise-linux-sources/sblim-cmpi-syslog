/*
 * syslogsettingparse.c
 * 
 * (C) Copyright IBM Corp. 2003, 2009
 *
 * THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
 * ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 * CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 * 
 * You can obtain a current copy of the Eclipse Public License from
 * http://www.opensource.org/licenses/eclipse-1.0.php
 * 
 * Author: R Sharada <sharada@in.ibm.com>
 *
 * Interface Type: Common Manageability Programming Interface ( CMPI )
 * 
 * Description: Setting parse utility functions for CIM Linux Syslog 
 * Configuration Provider
 */


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>

#include "syslogconfutil.h"

#define LINE_MAX 2048

char facility[256], priority[256], path[256], action[256];
FILE *ifp, *ofp = NULL;
int line_max = LINE_MAX;


int send_HUP_to_syslogd()
{
char buf[32];
pid_t pid;
                                                                                                                             
	ifp=popen("/sbin/pidof syslogd", "r");

	if ( ifp == NULL ) {
		syslog_debug(stderr, "Error in sending SIGHUP to syslogd.\n");
		return 1;
	}
                                                                                                                             
	fgets(buf, 32, ifp);
	pid=(pid_t)atol(buf);

	if(pid)
		kill(pid, SIGHUP);  /* Send HUP signal */
	else
		syslog_debug(stderr, "Warning: syslogd doesn't seem to be running.\n");
	
	fclose(ifp);

	return 0;
}
	



/* Utility function that reads the lines of the conf file and outputs each rule,
   one per line, to a new temporary file. It calls the function parse_list() to
   do this. Returns 0 on success and 1 on error */

int setting_read() 
{
  char *cline, *cbuf = NULL;
  char *p = NULL;
  int lineno = 0;
  int ret = 0;
  FILE *fp = NULL;

  syslog_debug(stderr, "--- setting_read() \n");

  /* opening the temp file, as we do not want to be working on the main syslog
     file as of now. The syslog.conf file is copied to a temp file */
  fp = fopen(TmpConfFile, "r");
  if ( fp == NULL ) {
    syslog_debug(stderr, "Error in opening syslog configuration file\n");
    return 1;
  }
  cbuf = (char *)malloc(LINE_MAX);
  if ( cbuf == NULL ) {
    syslog_debug(stderr, "Cannot allocate space for configuration\n");
    fclose(fp);
    return 1;
  }
  cline = cbuf;
  while ( fgets(cline, line_max, fp) != NULL) {
    for (p = cline; isspace(*p); ++p) ;

    if (*p == '\0' || *p == '#') {
      lineno++;
      continue;
    }
    lineno++;
    strcpy(cline, p);
    ret = parse_list(cbuf);
  }
  fclose(fp);
  free(cbuf);
  rename(ConfOutFile, ConfInFile);

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
  FILE *fwp = NULL;
  char outbuf[2000];

  syslog_debug(stderr, "--- parse_list() \n");

  bzero(&buf, sizeof(buf));
  bzero(&outbuf, sizeof(outbuf));
  bzero(&fac, sizeof(fac));
  bzero(&prio, sizeof(prio));
  bzero(&path, sizeof(path));

  fwp = fopen(ConfOutFile, "a");
  if ( fwp == NULL ) {
    syslog_debug(stderr, "Error in opening syslog writeout configuration file\n");
    return 1;
  }

  p = strdup((char*)line);
  q = strtok(p, "\t");
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
       fputs(outbuf, fwp);
       while (*p == ',' || *p == ' ')
	            p++;
     }
     p = q;
   }
   fclose(fwp);

   return 0;
}

/* Utility function that is called by the syslog setting enum functions to 
   create or delete a rule in the conf file. It uses two functions, 
   parse_create() and parse_delete to do the job. Returns 0 on success and 
   1 on error */

int setting_parse(char *rule_facility, char *rule_priority, char *rule_path, char *rule_action)
{
  char *cline, *cbuf = NULL;
  char *p, *tmpp, *tmpq = NULL;
  FILE *fp = NULL;
  int line_max = LINE_MAX;
  int lineno = 0;
  int new = 0;
  int ret = 0;
  char tmppath[256];
 
  syslog_debug(stderr, "--- setting_parse() \n");

  bzero(tmppath, sizeof(tmppath));
  bzero(facility, sizeof(facility));
  bzero(priority, sizeof(priority));
  bzero(path, sizeof(path));

  sprintf(facility, "%s", rule_facility);
  sprintf(priority, "%s", rule_priority);
  sprintf(path, "%s", rule_path);
  path[strlen(path)] = '\n';
  sprintf(action, "%s\n", rule_action);

  // splitting into the number of lines of syslog.conf without comment lines 
  
  /* opening the temp file for now, as we do not want to be working on the main
     syslog file, as of now */
  fp = fopen(TmpConfFile, "r");
  if ( fp == NULL ) {
    syslog_debug(stderr, "Error in opening syslog configuration file\n");
    return 1;
  }

  cbuf = malloc(LINE_MAX);
  if ( cbuf == NULL ) {
    syslog_debug(stderr, "Cannot allocate space for configuration\n");
    fclose(fp);
    return 1;
  }

  cline = cbuf;
  while (fgets(cline, line_max, fp) != NULL) {
    for (p = cline; isspace(*p); ++p);
 
    if (*p == '\0' || *p == '#') {
      lineno++;
      new++;
      continue;
    }
    lineno++;
    
    //get path from rule string
    tmpp = (char*)cline;
    tmpq = strtok(tmpp, "\t");
    tmpq = strtok(NULL, "\n");
    while(isspace(*tmpq)) tmpq++;
    strcpy(tmppath, tmpq);
    tmppath[strlen(tmppath)] = '\n';
    tmpp = NULL;
    tmpq = NULL;

    if (strcmp(tmppath, path)==0) { 
      bzero(tmppath, sizeof(tmppath));
      strcpy(cline, p); // copy into new buffer - cline
      p = strtok(cline, "\t"); // Get the first token, the filter expression
      strcpy(cline, p);
      if (strncmp(action, "create", 6) == 0) {
        syslog_debug(stderr, "creating / inserting a rule\n");
        ret = parse_create(cbuf);
      }
      else if (strncmp(action, "delete", 6) == 0) {
        syslog_debug(stderr, "--- deleting a rule\n");
        ret = parse_delete(cbuf);
      }
      goto done;
    } // match in rule based on path
    else {
      bzero(tmppath, sizeof(tmppath));
      new++;
      continue;
    } // no match in rule for that path in the conf file
  } // End of file 

  if (new == lineno) { // No match occurred for that path in conf file
    if (strncmp(action, "delete", 6) == 0) {
      syslog_debug(stderr, "Deletion : no such line match\n");
      ret = 0;
      goto done;
    }
    else if (strncmp(action, "create", 6) == 0){
      syslog_debug(stderr, "Creation: a line to be created\n");
      create_in_file(facility, priority, path);
    }
  }
done:
  fclose(fp);
  free(cbuf);
  rename(ConfOutFile, TmpConfFile);
  ret = send_HUP_to_syslogd();

  return ret;
}

/* Utility function that creates a new instance, by either appending to an
   existing line or adding a new line in the conf file. Returns 0 on success */

int parse_create(const char *line)
{
  char *nline, *nbuf, *rline, *rbuf, *oline, *obuf, *p = NULL;
  int match = 0;
  int ret = 0;

  syslog_debug(stderr, "--- parse_create() \n");

  nbuf = malloc(LINE_MAX);
  if(!nbuf) {
    syslog_debug(stderr, "Cannot allocate space.\n");
    return 1;
  }
  nline = nbuf;
  rbuf = malloc(LINE_MAX);
  if(!rbuf) {
    syslog_debug(stderr, "Cannot allocate space.\n");
    return 1;
  }
  rline = rbuf;
  obuf = malloc(LINE_MAX);
  if(!obuf) {
    syslog_debug(stderr, "Cannot allocate space.\n");
    return 1;
  }
  oline = obuf;
 
  strcpy(nline, line);
  while ((p = strtok(nline, ";")) != NULL) {
    strcpy(rline, p);
    if (strstr(rline, facility)) {
       if (strstr(rline, priority)) {
         match = 1;
         syslog_debug(stderr, "rule exists! exiting\n");
         break;
       }
    }
    nline = NULL;
  } // End of while - end of tokens separated by ;
 
  if (!match) {
    syslog_debug(stderr, "no match\n");
    strcpy(oline, line);
    strcat(oline, ";");
    strcat(oline, facility);
    strcat(oline, ".");
    strcat(oline, priority);
    strcat(oline, "\t");
    strcat(oline, " ");
    strcat(oline, "\t");
    strcat(oline, path);
    ret = copy_to_file(oline);
  }
  free(nbuf);
  free(rbuf);
  free(obuf);

  return ret;
}

/* Returns 0 on success and 1 on error */
int copy_to_file(char *buf)
{
  char *line, *lbuf = NULL;
  int ret = 0;
  
  syslog_debug(stderr, "--- copy_to_file() \n");

  lbuf=malloc(LINE_MAX);  
  if(!lbuf) {
    syslog_debug(stderr, "Cannot allocate space.\n");
    return 1;
  }
  line = lbuf;
  /* opening the temp file for now, as we do not want to be working on the main
     syslog conf file, as of now */
  ifp = fopen(TmpConfFile, "r");
  if ( ifp == NULL ) {
    syslog_debug(stderr, "Error in opening syslog configuration file\n");
    free(lbuf);
    return 1;
  }

  // write to the confoutfile, which is later copied back into the temp file 
  ofp = fopen(ConfOutFile, "w");
  if ( ofp == NULL ) {
    syslog_debug(stderr, "Error in opening temp syslog configuration file\n");
    fclose(ifp);
    free(lbuf);
    return 1;
  }
   
  while (fgets(line, line_max, ifp) != NULL) {
    if ( strstr(line, path)) 
     fputs(buf, ofp);
   else
     fputs(line, ofp);
  }
  fclose(ifp);
  fclose(ofp);
  free(lbuf);

  return ret;
}

/* Returns 0 on success and 1 on error */
int create_in_file(char *facility, char *priority, char *path)
{
  char *line, *lbuf, *cline, *cbuf = NULL;
  int ret = 0;
  
  syslog_debug(stderr, "--- create_in_file() \n");

  lbuf=malloc(LINE_MAX);
  if(!lbuf) {
    syslog_debug(stderr, "Cannot allocate space.\n");
    return 1;
  }
  line = lbuf;
  cbuf=malloc(LINE_MAX);
  if(!cbuf) {
    syslog_debug(stderr, "Cannot allocate space.\n");
    return 1;
  }
  cline = cbuf;

  bzero(&cbuf, sizeof(cbuf));
  bzero(&lbuf, sizeof(lbuf));

  /* opening the temp file for now, as we do not want to be working on the main
     syslog conf file, as of now */
  ifp = fopen(TmpConfFile, "r");
  if ( ifp == NULL ) {
    syslog_debug(stderr, "Error in opening syslog configuration file\n");
    free(lbuf);
    free(cbuf);
    return 1;
  }
  
  // write to the confoutfile, which is later copied back into the temp file 
  ofp = fopen(ConfOutFile, "w");
  if ( ofp == NULL ) {
    syslog_debug(stderr, "Error in opening temp syslog configuration file\n");
    fclose(ifp);
    free(lbuf);
    free(cbuf);
    return 1;
  }
   
  strcpy(cline, facility);
  strcat(cline, ".");
  strcat(cline, priority);
  strcat(cline, "\t");
  strcat(cline, " ");
  strcat(cline, "\t");
  strcat(cline, path);
  strcat(cline, "\n");
  while (fgets(line, line_max, ifp) != NULL) {
    fputs(line, ofp);
  }   
  fputs(cline, ofp);
  fclose(ifp);
  fclose(ofp);
  free(lbuf);
  free(cbuf);

  return ret;
}

/* Utility function that deletes a rule from the syslog conf file. Returns 0 on
   success */

int parse_delete(const char *line)
{
  char *nline, *nbuf = NULL;
  char *p, *q = NULL;
  char *k, *j = NULL;
  char *rline, *rbuf = NULL;
  char *pbuf, *pline = NULL;
  char *oline, *obuf = NULL;
  char *otline, *otbuf = NULL;
  char *xline, *xbuf = NULL;
  char array[10][50];
  int index = 0;
  int count = 0;
  int i = 0;
  int ret = 0;
  int one = 0;
  int first = 1;
                                                                                
  syslog_debug(stderr, "--- parse_delete() \n");

  nbuf = malloc(LINE_MAX);
  if(!nbuf) {
    syslog_debug(stderr, "Cannot allocate space.\n");
    return 1;
  }
  nline = nbuf;
  rbuf = malloc(LINE_MAX);
  if(!rbuf) {
    syslog_debug(stderr, "Cannot allocate space.\n");
    free(nbuf);
    return 1;
  }
  rline = rbuf;
  pbuf = malloc(LINE_MAX);
  if(!pbuf) {
    syslog_debug(stderr, "Cannot allocate space.\n");
    free(nbuf);
    free(rbuf);
    return 1;
  }
  pline = pbuf;
  obuf = malloc(LINE_MAX);
  if(!obuf) {
    syslog_debug(stderr, "Cannot allocate space.\n");
    free(nbuf);
    free(rbuf);
    free(pbuf);
    return 1;
  }
  oline = obuf;
  otbuf = malloc(LINE_MAX);
  if(!otbuf) {
    syslog_debug(stderr, "Cannot allocate space.\n");
    free(nbuf);
    free(rbuf);
    free(pbuf);
    free(obuf);
    return 1;
  }
  otline = otbuf;
  xbuf = malloc(LINE_MAX);
  if(!xbuf) {
    syslog_debug(stderr, "Cannot allocate space.\n");
    free(nbuf);
    free(rbuf);
    free(pbuf);
    free(obuf);
    free(otbuf);
    return 1;
  }
  xline = xbuf;
                                                                                
  bzero(nbuf, sizeof(nbuf));
  bzero(rbuf, sizeof(rbuf));
  bzero(pbuf, sizeof(pbuf));
  bzero(obuf, sizeof(obuf));
  bzero(otbuf, sizeof(otbuf));
  bzero(xbuf, sizeof(xbuf));
  bzero(&array, sizeof(array));

  strcpy(nline, line);
  while((p = strtok(nline, ";")) != NULL) {
    strcpy(array[index], p);
    index++;
    nline = NULL;
    if (index > 10) {
      syslog_debug(stderr, "out of bounds of array\n");
      free(nbuf);
      free(rbuf);
      free(pbuf);
      free(obuf);
      free(otbuf);
      free(xbuf);
      return 1;
    }
  } // End of while - tokens separated by ;

  count = index - 1;
  for(i=0; i<index; i++) {
    strcpy(rline, array[i]);
    if (strstr(rline, facility) && strstr(rline, priority)) {
      strcpy(xline, rline);
      if (strchr(xline, ',')) {
        while((q = strtok(xline, ",")) != NULL) {
          strcpy(pline, q);
          if (!strchr(pline, '.')) { // Not Last Filter
            if (!strcmp(pline, facility)) {
              syslog_debug(stderr, "Facility Match\n");
            }
            else {
              strcat(oline, pline);
              strcat(oline, ",");
            }
          }
          else { // Last Filter
            if (strstr(pline, facility)) {
	      j = otline;	    
              for ( k=oline;*k != ','; k++, j++) {
		*j = *k;      
	      }	      
	      *j = '\0';
	      strcpy(oline, otline);
              strcat(oline, ".");
              strcat(oline, priority);
            }
            else {
              strcat(oline, pline);
            }
          }
          xline = NULL;
          syslog_debug(stderr, "End of Inner While\n");
        }
      }
      else {
        syslog_debug(stderr, "Single Rule\n");
	if (count == 0) {
          strcat(oline, "");
	  one = 1;
	}  
	else if (count > 0) {
          one = 0;
	}
      }
    }
   
    else {
      syslog_debug(stderr, "Rule No Match\n");
      if (first) {
	// This is the first rule in the line. copy as is, without ';'
	first = 0;
	strcat(oline, rline);
      }	      
      else {
	// This is not first rule in the line. copy with a preceding ';'
	strcat(oline, ";");      
        strcat(oline, rline);
      }
    }
    syslog_debug(stderr, "Back in Outer for\n");
  } // End of for loop - list of sub-expressions tokens
  if ( !one) { 
  strcat(oline, "\t");
  strcat(oline, " ");
  strcat(oline, "\t");
  strcat(oline, path);
  }
  ret = copy_to_file(oline);

  free(nbuf);
  free(rbuf);
  free(pbuf);
  free(obuf);
  free(otbuf);
  free(xbuf);

  return ret;
}






