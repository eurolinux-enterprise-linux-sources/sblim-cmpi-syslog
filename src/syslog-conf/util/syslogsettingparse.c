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
#include <errno.h>
#include <fcntl.h>
#include <glob.h>
#include <unistd.h>

#include "syslogconfutil.h"

#define LINE_MAX 2048
#define BUF_SIZE 0x10000   /* 64 kB */

/* We need tri-state status values to control the loop and recursion */
typedef enum {
  RET_SUCCESS = 0,
  RET_ERROR = 1,    /* general error or not found, may be ignored/skipped */
  RET_FAILED = 2,   /* handled but error occurred, more serious */
  RET_NOT_DONE = 3  /* not handled yet */
} ReturnState;

ReturnState send_HUP_to_syslogd()
{
	FILE *ifp;
	char buf[32];
	pid_t pid;
                                                                                                                             
	ifp=popen("/sbin/pidof " SYSLOG_DAEMON, "r");

	if ( ifp == NULL ) {
		syslog_debug(stderr, "Error in sending SIGHUP to syslogd.\n");
		return RET_ERROR;
	}
                                                                                                                             
	fgets(buf, 32, ifp);
	pid=(pid_t)atol(buf);

	if(pid)
		kill(pid, SIGHUP);  /* Send HUP signal */
	else
		syslog_debug(stderr, "Warning: syslogd doesn't seem to be running.\n");
	
	fclose(ifp);

	return RET_SUCCESS;
}
	

static void
parse_rsyslog_include (const char *ref)
{
  char cmdbuffer[1024];
  char *buf = NULL;
  char *p;
  FILE *f;

  bzero(&cmdbuffer, sizeof(cmdbuffer));
  buf = (char *) malloc(LINE_MAX);
  if (buf == NULL) {
    syslog_debug (stderr, "Cannot allocate space for configuration\n");
    return;
  }

  sprintf(cmdbuffer,
          "files=(%s); cat \"${files[@]}\" | grep -v '^[#$&]'",
          ref);
  f = popen (cmdbuffer, "r");
  if (f != NULL) {
    while (fgets (buf, LINE_MAX, f) != NULL) {
      for (p = buf; isspace(*p); ++p);
      if (*p != '\0' && *p != '\n')
        parse_list (p);
    }
    pclose (f);
  }
  free(buf);
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
  while ( fgets(cline, LINE_MAX, fp) != NULL) {
    for (p = cline; isspace(*p); ++p) ;

    /* Handle rsyslog includes */
    if (strncasecmp ("$IncludeConfig", p, 14) == 0) {
      p += 14;
      while (isspace (*p))
        p++;
      chomp (p);
      parse_rsyslog_include (p);
      continue;
    }

    if (
	*p == '\0' 
	|| *p == '#'
	/* Ignore directive of rsyslog own. */
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
  rename(ConfOutFile, ConfInFile);

  return ret;
}

static void
get_last_token (const char *line, const char *delim, char *out_last, char *out_prev)
{
  char *p, *q;
  char *last = NULL;
  int i;

  p = strdup ((char *) line);
  for (q = strtok (p, delim); q != NULL; q = strtok (NULL, delim)) {
    while (isspace (*q))
      q++;
    free (last);
    last = strdup (q);
  }
  free (p);

  /* No token found, return whole string in out_prev */
  if (last == NULL) {
    if (out_last)
      out_last[0] = '\0';
    if (out_prev)
      strcpy (out_prev, line);
    return;
  }

  if (out_last)
    strcpy (out_last, last);
  if (out_prev) {
    /* FIXME: assuming the last token is present in the source string only once */
    q = strstr (line, last);
    if (q != NULL) {
      i = q - line;
      /* Trim trailing whitespaces */
      while (i > 0 && isspace (line[i - 1]))
        i--;
      strncpy (out_prev, line, i);
      out_prev[i] = '\0';
    }
  }
  free (last);
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

  syslog_debug(stderr, "--- parse_list() \n");

  bzero(&buf, sizeof(buf));
  bzero(&fac, sizeof(fac));
  bzero(&prio, sizeof(prio));
  bzero(&path, sizeof(path));

  fwp = fopen(ConfOutFile, "a");
  if ( fwp == NULL ) {
    syslog_debug(stderr, "Error in opening syslog writeout configuration file\n");
    return 1;
  }

  get_last_token (line, "\t\n ", &path[0], NULL);

  p = NULL;
  q = NULL;

  if (line && *line == ':') {
    /* Handle filters */
    get_last_token (line, "\t\n ", NULL, &fac[0]);
    fputs (fac, fwp);
    fputs ("\n", fwp);
    fputs ("\n", fwp);
    fputs (path, fwp);
    fputs ("\n", fwp);
  } else {
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
         fputs (fac, fwp);
         fputs ("\n", fwp);
         fputs (strlen (prio) > 0 ? prio : "none", fwp);
         fputs ("\n", fwp);
         fputs (path, fwp);
         fputs ("\n", fwp);
         while (*p == ',' || *p == ' ')
	            p++;
       }
       p = q;
     }
   }
   fclose(fwp);

   return 0;
}

/* A drop-in replacement for rename():
     Atomically rename a file and handle cross-device transfer if needed */
static int
atomic_rename (const char *oldpath, const char *newpath)
{
  void *buf;
  char new_tmp[2048];
  int fin, fout;
  int ret;
  ssize_t r, w;

  ret = rename (oldpath, newpath) != 0;
  if (ret && errno == EXDEV) {
    if (snprintf (new_tmp, sizeof (new_tmp), "%s.sblim-XXXXXX", newpath) <= 0)
      return -1;
    /* files are on different filesystems, fall back to copy, rename & delete */
    fin = open (oldpath, O_RDONLY);
    if (fin == -1)
      return -1;
    mktemp (new_tmp);
    fout = open (new_tmp, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fout == -1) {
      close (fin);
      return -1;
    }
    buf = malloc (BUF_SIZE);
    if (buf == NULL) {
      close (fin);
      close (fout);
      return -1;
    }

    while ((r = read (fin, buf, BUF_SIZE)) > 0) {
      w = write (fout, buf, r);
      if (w < r) {
        syslog_debug(stderr, "error writing block: %s\n", strerror(errno));
        close (fin);
        close (fout);
        free (buf);
        return -1;
      }
    }
    ret = r >= 0;
    if (! ret)
        syslog_debug(stderr, "error reading block: %s\n", strerror(errno));

    free (buf);
    ret = close (fin) == 0 && ret;
    ret = close (fout) == 0 && ret;
    if (! ret) {
      unlink (new_tmp);
      return -1;
    }

    /* Atomically rename the newly copied tmp file over the target file */
    ret = rename (new_tmp, newpath);
    if (ret < 0)
      return -1;

    /* Remove the source file */
    unlink (oldpath);

    return 0;
  }
  return ret;
}

ReturnState copy_to_file (const char *filename, char *buf, const char *rule_path)
{
  FILE *ifp, *ofp;
  char *line, *lbuf = NULL;
  ReturnState ret = RET_SUCCESS;
  
  syslog_debug(stderr, "--- copy_to_file() \n");

  lbuf=malloc(LINE_MAX);  
  if(!lbuf) {
    syslog_debug(stderr, "Cannot allocate space.\n");
    return RET_ERROR;
  }
  line = lbuf;
  /* opening the temp file for now, as we do not want to be working on the main
     syslog conf file, as of now */
  ifp = fopen(filename, "r");
  if ( ifp == NULL ) {
    syslog_debug(stderr, "Error in opening syslog configuration file\n");
    free(lbuf);
    return RET_ERROR;
  }

  // write to the confoutfile, which is later copied back into the temp file 
  ofp = fopen(ConfOutFile, "w");
  if ( ofp == NULL ) {
    syslog_debug(stderr, "Error in opening temp syslog configuration file\n");
    fclose(ifp);
    free(lbuf);
    return RET_FAILED;
  }
   
  while (fgets(line, LINE_MAX, ifp) != NULL) {
    if ( strstr(line, rule_path)) 
     fputs(buf, ofp);
   else
     fputs(line, ofp);
  }
  fclose(ifp);
  fclose(ofp);
  free(lbuf);

  return ret;
}

/* Utility function that creates a new instance, by either appending to an
   existing line or adding a new line in the conf file. */
ReturnState parse_create (const char *filename,
                          const char *line,
                          const char *facility,
                          const char *priority,
                          const char *path)
{
  char *nline, *nbuf, *rline, *rbuf, *oline, *obuf, *p = NULL;
  int match = 0;
  ReturnState ret = RET_SUCCESS;

  syslog_debug(stderr, "--- parse_create() \n");

  nbuf = malloc(LINE_MAX);
  if(!nbuf) {
    syslog_debug(stderr, "Cannot allocate space.\n");
    return RET_ERROR;
  }
  nline = nbuf;
  rbuf = malloc(LINE_MAX);
  if(!rbuf) {
    syslog_debug(stderr, "Cannot allocate space.\n");
    return RET_ERROR;
  }
  rline = rbuf;
  obuf = malloc(LINE_MAX);
  if(!obuf) {
    syslog_debug(stderr, "Cannot allocate space.\n");
    return RET_ERROR;
  }
  oline = obuf;
 
  strcpy(nline, line);
  while ((p = strtok(nline, ";")) != NULL) {
    strcpy(rline, p);
    if (strstr(rline, facility)) {
       if (strstr(rline, priority)) {
         match = 1;
         syslog_debug(stderr, "rule exists! exiting\n");
         ret = RET_FAILED;
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
    strcat(oline, "\n");
    ret = copy_to_file(filename, oline, path);
  }
  free(nbuf);
  free(rbuf);
  free(obuf);

  return ret;
}

ReturnState create_in_file (const char *filename,
                            const char *facility,
                            const char *priority,
                            const char *path)
{
  FILE *ifp, *ofp = NULL;
  char *line, *lbuf, *cline, *cbuf = NULL;
  ReturnState ret = RET_SUCCESS;
  
  syslog_debug(stderr, "--- create_in_file() \n");

  lbuf=malloc(LINE_MAX);
  if(!lbuf) {
    syslog_debug(stderr, "Cannot allocate space.\n");
    return RET_ERROR;
  }
  line = lbuf;
  cbuf=malloc(LINE_MAX);
  if(!cbuf) {
    syslog_debug(stderr, "Cannot allocate space.\n");
    return RET_ERROR;
  }
  cline = cbuf;

  bzero(&cbuf, sizeof(cbuf));
  bzero(&lbuf, sizeof(lbuf));

  /* opening the temp file for now, as we do not want to be working on the main
     syslog conf file, as of now */
  ifp = fopen(filename, "r");
  if ( ifp == NULL ) {
    syslog_debug(stderr, "Error in opening syslog configuration file\n");
    free(lbuf);
    free(cbuf);
    return RET_ERROR;
  }
  
  // write to the confoutfile, which is later copied back into the temp file 
  ofp = fopen(ConfOutFile, "w");
  if ( ofp == NULL ) {
    syslog_debug(stderr, "Error in opening temp syslog configuration file\n");
    fclose(ifp);
    free(lbuf);
    free(cbuf);
    return RET_FAILED;
  }
   
  strcpy(cline, facility);
  strcat(cline, ".");
  strcat(cline, priority);
  strcat(cline, "\t");
  strcat(cline, " ");
  strcat(cline, "\t");
  strcat(cline, path);
  strcat(cline, "\n");
  while (fgets(line, LINE_MAX, ifp) != NULL) {
    fputs(line, ofp);
  }   
  fputs(cline, ofp);
  fclose(ifp);
  fclose(ofp);
  free(lbuf);
  free(cbuf);

  return ret;
}

/* Utility function that deletes a rule from the syslog conf file. */
ReturnState parse_delete (const char *filename,
                          const char *line,
                          const char *facility,
                          const char *priority,
                          const char *path)
{
  char *q;
  char *j;
  char *oline = NULL;
  char *otline = NULL;
  char *xline;
  char *array[100];
  char *s;
  int index = 0;
  int i;
  ReturnState ret = RET_SUCCESS;
  int fac_match, prio_match;

  syslog_debug(stderr, "--- parse_delete() \n");

  oline = malloc (LINE_MAX);
  otline = malloc (LINE_MAX);
  if (oline == NULL || otline == NULL) {
    syslog_debug (stderr, "Cannot allocate memory.\n");
    free (oline);
    free (otline);
    return RET_ERROR;
  }
  bzero (oline, LINE_MAX);
  bzero (otline, LINE_MAX);
  bzero (&array, sizeof (array));

  /* Strip the target (logfile) part */
  get_last_token (line, "\t\n ", NULL, otline);
  s = otline;
  while ((q = strtok (s, ";")) != NULL) {
    array[index] = strdup (q);
    index++;
    s = NULL;
    if (index > 100) {
      syslog_debug (stderr, "out of bounds of array\n");
      free (oline);
      free (otline);
      for (i = 0; i < 100; i++)
        free (array[i]);
      return RET_ERROR;
    }
  } // End of while - tokens separated by ;

  for (i = 0; i < index; i++) {
    if (strchr (array[i], ',')) {
      *otline = '\0';
      fac_match = 0;
      prio_match = 0;
      xline = strdup (array[i]);
      while ((q = strtok (xline, ",")) != NULL) {
        if ((j = strchr (q, '.')) == NULL) { // Not Last Filter
          if (strcmp (q, facility) == 0) {
            syslog_debug (stderr, "Facility Match\n");
            fac_match = 1;
          }
          else {
            strcat (otline, q);
            strcat (otline, ",");
          }
        }
        else { // Last Filter
          if (strncmp (q, facility, j - q) == 0) {
            fac_match = 1;
            if (*otline != '\0') {
              /* otline should not be empty at this moment, otherwise it's a badly
                 formatted rule line, so let's not add anything */
              otline[strlen (otline) - 1] = '\0';  /* remove trailing comma */
              strcat (otline, j);  /* copy the priority incl. leading dot */
            }
          }
          else {
            strcat (otline, q);
          }
          prio_match = strcmp (j + 1, priority) == 0;
        }
        xline = NULL;
        syslog_debug (stderr, "End of Inner While\n");
      }
      free (xline);
      if (*oline != '\0')
        strcat (oline, ";");
      if (fac_match && prio_match)
        strcat (oline, otline);    /* positive match, copy the newly constructed rule */
      else
        strcat (oline, array[i]);  /* no match, copy the original line */
    }
    else {
      syslog_debug (stderr, "Single Rule\n");
      s = malloc (strlen (facility) + strlen (priority) + 2);
      sprintf (s, "%s.%s", facility, priority);
      if (strcmp (s, array[i]) != 0) {
        if (*oline != '\0')
          strcat (oline, ";");
        strcat (oline, array[i]);
      }
      free (s);
    }
  } // End of for loop - list of sub-expressions tokens
  if (*oline != '\0') {
    strcat (oline, "\t \t");
    strcat (oline, path);
    strcat (oline, "\n");
  }
  ret = copy_to_file (filename, oline, path);

  free (oline);
  free (otline);
  for (i = 0; i < 100; i++)
    free (array[i]);

  return ret;
}

ReturnState
setting_parse_internal (const char *filename,
                        const char *rule_facility,
                        const char *rule_priority,
                        const char *rule_path,
                        SyslogParseAction rule_action,
                        int master)
{
  char *cbuf = NULL;
  char *p = NULL;
  FILE *fp = NULL;
  ReturnState ret = RET_NOT_DONE;
  char tmppath[256];
  char tmprule[256];
  char *include_path = NULL;
 
  syslog_debug(stderr, "--- setting_parse(\"%s\") \n", filename);

  bzero(tmppath, sizeof(tmppath));
  bzero(tmprule, sizeof(tmprule));

  fp = fopen(filename, "r");
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

  while (fgets (cbuf, LINE_MAX, fp) != NULL) {
    for (p = cbuf; isspace(*p); ++p);

    /* Save the rsyslog include path for later use */
    if (strncasecmp ("$IncludeConfig", p, 14) == 0) {
      p += 14;
      while (isspace (*p))
        p++;
      chomp (p);
      include_path = strdup (p);
      continue;
    }

    if (*p == '\0' || *p == '#' || *p == '$')
      continue;

    /* Parse out path and the rule from the input line */
    get_last_token (cbuf, "\t\n ", tmppath, tmprule);
    if (strcmp (tmppath, rule_path) == 0) {
      switch (rule_action) {
        case ACTION_CREATE:
          syslog_debug (stderr, "creating / inserting a rule\n");
          ret = parse_create (filename, tmprule, rule_facility, rule_priority, rule_path);
          break;
        case ACTION_DELETE:
          syslog_debug (stderr, "--- deleting a rule\n");
          ret = parse_delete (filename, cbuf, rule_facility, rule_priority, rule_path);
          break;
      }
      break;
    }
  } // End of file 

  fclose (fp);
  free (cbuf);

  if (ret == RET_NOT_DONE && include_path) {
    /* go through includes */
    glob_t globbuf;
    ReturnState child_ret;
    int i;

    syslog_debug (stderr, "--- processing rsyslog include \"%s\" \n", include_path);
    i = glob (include_path, GLOB_NOSORT | GLOB_NOESCAPE, NULL, &globbuf);
    free (include_path);
    if (i == 0) {
      for (i = 0; i < globbuf.gl_pathc; i++) {
        child_ret = setting_parse_internal (globbuf.gl_pathv[i], rule_facility, rule_priority, rule_path, rule_action, 0);
        /* We found the right place to modify and either succeeded or failed, report that to the user */
        if (child_ret == RET_SUCCESS || child_ret == RET_FAILED) {
          ret = child_ret;
          break;
        }
      }
      globfree (&globbuf);
      if (ret != RET_NOT_DONE)
        /* No need to go through the final rename */
        return ret;
    }
  }

  /* No match occurred for the path in config files, perform the global action
     in case we're operating on a master config file */
  if (ret == RET_NOT_DONE && master) {
    switch (rule_action) {
      case ACTION_CREATE:
        syslog_debug (stderr, "Creation: a line to be created\n");
        ret = create_in_file (filename, rule_facility, rule_priority, rule_path);
        break;
      case ACTION_DELETE:
        syslog_debug (stderr, "Deletion : no such line match\n");
        ret = RET_ERROR;
        break;
    }
  }

  if (ret == RET_SUCCESS && atomic_rename (ConfOutFile, filename) != 0) {
    syslog_debug (stderr, "Atomic rename failed: %s\n", strerror (errno));
    ret = RET_FAILED;
  }

  return ret;
}

/* Utility function that is called by the syslog setting enum functions to 
   create or delete a rule in the conf file. It uses two functions, 
   parse_create() and parse_delete to do the job. Returns 0 on success and 
   1 on error */

int setting_parse (const char *rule_facility,
                   const char *rule_priority,
                   const char *rule_path,
                   SyslogParseAction rule_action)
{
  ReturnState ret;

  ret = setting_parse_internal (TmpConfFile, rule_facility, rule_priority, rule_path, rule_action, 1);
  if (ret == RET_SUCCESS)
    ret = send_HUP_to_syslogd ();

  return ret != RET_SUCCESS;
}
