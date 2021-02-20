/* References
 * https://stackoverflow.com/questions/19885360/how-can-i-print-the-symbolic-name-of-an-errno-in-c
 * https://github.com/mentalisttraceur/errnoname
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#define ERROR_C_FILE  "liberror.c"
#define ERROR_H_FILE  "error.h"

#define EUNKNOWN_NAME "EUNKNOWN"
#define EUNKNOWN_STR  "Unknown error"

#if defined (__linux__) || defined (__unix__)
int main (void) {
  char cmd[] =
    "printf \"#include <errno.h>\" | cpp -dD | sed -n '/#define E/p' | sed "
    "'s/^\\(.*\\)\\s\\(.*\\)\\s\\(.*\\)$/#ifndef \\2\\n  \\1 \\2 \\3\\n#endif\\n/'";

  FILE *fp_source = popen (cmd, "r");
  if (NULL == fp_source) {
    fprintf (stderr, "%s\n", strerror (errno));
    return 1;
  }

  FILE *fp_dest = fopen (ERROR_C_FILE, "w");
  if (NULL == fp_dest) {
    fprintf (stderr, "%s\n", strerror (errno));
    return 1;
  }

  fprintf (fp_dest,
    "#ifdef TEST\n"
    "#include <stddef.h>\n"
    "#include <stdio.h>\n"
    "#include <stdlib.h>\n"
    "#include <string.h>\n"
    "#endif\n\n");

  fprintf (fp_dest,
      "#include \"error.h\"\n\n"
      "static const struct sys_error_list_type {\n"
      "  const char *name;\n"
      "  const char *msg;\n"
      "  int errnum;\n"
    "} sys_error_list[] = {\n"
    "  { \"SUCCESS\", \"%s\", 0},\n",
    strerror (0));

  char name[128];
  char *buf = NULL;
  size_t len;
  ssize_t nread;
  int num_items;
  int err;
  int prev_err = 0;
  int num_err = 0;

  while (-1 != (nread = getline (&buf, &len, fp_source))) {
    if (nread < 2) continue;
    if (0 == strncmp (buf, "#ifndef", 7)) continue;
    if (0 == strncmp (buf, "#endif", 6)) continue;
    num_items = sscanf (buf, "  #define %s %d\n", name, &err);
    if (2 != num_items) {    // this is not going to work anyway with current code
#if 0                        // so just document it
      if ((sp = strstr (buf, "ENOTSUP")) != NULL)
        fprintf (fp_dest, "  { \"ENOTSUP\", \"%s\", %d},\n", strerror(ENOTSUP), ENOTSUP);
#endif
      continue;
    }

    if (prev_err != err - 1) {
      fprintf (fp_dest, "  { \"%s\", \"%s\", %d},\n", EUNKNOWN_NAME, EUNKNOWN_STR, err - 1);
      num_err++;
    }

    prev_err = err;
    num_err++;

    fprintf (fp_dest, "  { \"%s\", \"%s\", %d},\n", name, strerror(err), err);
  }

  fprintf (fp_dest, "  { \"%s\", \"%s\", %d}\n", EUNKNOWN_NAME, EUNKNOWN_STR, num_err + 1);
  fprintf (fp_dest, "};\n\n");

  num_err++;
  fprintf (fp_dest,
    "static int sys_last_error = %d;\n\n", num_err);

  fprintf (fp_dest,
    "static int get_error_num (int errnum) {\n"
     "  if (errnum > sys_last_error || errnum < 0)\n"
     "    return EUNKNOWN;\n\n"
     "  return errnum;\n"
     "}\n\n"
    "char *errno_name (int errnum) {\n"
     "  errnum = get_error_num (errnum);\n"
     "  return (char *) sys_error_list[errnum].name;\n"
     "}\n\n"
    "char *errno_name_s (int errnum, char *buf, size_t buflen) {\n"
     "  char *name = errno_name (errnum);\n\n"
     "  size_t len = strlen (name);\n\n"
     "  size_t idx = 0;\n"
     "  for (;idx < len && idx < buflen - 1; idx++) {\n"
     "    buf[idx] = name[idx];\n"
     "  }\n\n"
     "  buf[idx] = '\\0';\n\n"
     "  return buf;\n"
     "}\n\n"
    "char *errno_string (int errnum) {\n"
     "  errnum = get_error_num (errnum);\n"
     "  return (char *) sys_error_list[errnum].msg;\n"
     "}\n\n"
    "char *errno_string_s (int errnum, char *buf, size_t buflen) {\n"
     "  char *msg = errno_string (errnum);\n\n"
     "  size_t len = strlen (msg);\n\n"
     "  size_t idx = 0;\n"
     "  for (;idx < len && idx < buflen - 1; idx++) {\n"
     "    buf[idx] = msg[idx];\n"
     "  }\n\n"
     "  buf[idx] = '\\0';\n\n"
     "  return buf;\n"
     "}\n\n"
     "int error_exists (int errnum) {\n"
     "  errnum = get_error_num (errnum);\n"
     "  return (errnum != EUNKNOWN);\n"
     "}\n\n");

  fprintf (fp_dest,
    "#ifdef TEST\n"
    "int main (int argc, char **argv) {\n"
    "  if (argc == 1) return 1;\n\n"
    "  char buf[128];\n"
    "  int idx = atoi (argv[1]);\n\n"
    "  fprintf (stdout, \"%%s\\n\", errno_string (idx));\n\n"
    "  fprintf (stdout, \"%%s\\n\", errno_string_s (idx, buf, 128));\n\n"
    "  fprintf (stdout, \"%%s\\n\", errno_name (idx));\n\n"
    "  fprintf (stdout, \"%%s\\n\", errno_name_s (idx, buf, 128));\n\n"
    "  return 0;\n"
    "}\n"
    "#endif\n");

  fclose (fp_source);
  fclose (fp_dest);

  fp_dest = fopen (ERROR_H_FILE, "w");
  if (NULL == fp_dest) {
    fprintf (stderr, "%s\n", strerror (errno));
    return 1;
  }

  fprintf (fp_dest,
    "#ifndef ERROR_H\n"
    "#define ERROR_H\n\n"
    "#define EUNKNOWN %d\n\n"
    "char *errno_name (int);\n"
    "char *errno_name_s (int, char *, size_t);\n"
    "char *errno_string (int);\n"
    "char *errno_string_s (int, char *, size_t);\n"
    "int error_exists (int);\n\n"
    "#endif /* ERROR_H */\n",
    num_err);

  fclose (fp_dest);

  return 0;
}
#else
int main (void) {
  fprintf (stderr, "this functionality has been implemented only for Linux platform\n");
  return 1;
}
#endif
