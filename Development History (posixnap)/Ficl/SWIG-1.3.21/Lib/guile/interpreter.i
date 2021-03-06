//
// $Header: /home/jabba/convert/cvs/Ficl/SWIG-1.3.21/Lib/guile/interpreter.i,v 1.1 2004-02-01 04:04:09 alex Exp $
//
// SWIG file for a simple Guile interpreter
//
/* Revision History
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2000/01/11 21:15:50  beazley
 * Added files
 *
 * Revision 1.1.1.1  1999/02/28 02:00:54  beazley
 * Swig1.1
 *
 * Revision 1.1  1996/05/22 20:02:10  beazley
 * Initial revision
 *
 */

%{

#include <stdio.h>
GSCM_status guile_init();

int main(int argc, char **argv) {
  GSCM_status status;
  GSCM_top_level toplev;
  char *eval_answer;
  char input_str[16384];
  int done;

  /* start a scheme interpreter */
  status = gscm_run_scm(argc, argv, 0, stdout, stderr, guile_init, 0, "#t");
  if (status != GSCM_OK) {
    fputs(gscm_error_msg(status), stderr);
    fputc('\n', stderr);
    printf("Error in startup.\n");
    exit(1);
  }

  /* create the top level environment */
  status = gscm_create_top_level(&toplev);
  if (status != GSCM_OK) {
    fputs(gscm_error_msg(status), stderr);
    fputc('\n', stderr);
    exit(1);
  }

  /* now sit in a scheme eval loop: I input the expressions, have guile
   * evaluate them, and then get another expression.
   */
  done = 0;
  fprintf(stdout,"Guile > ");
  while (!done) {
    if (fgets(input_str,16384,stdin) == NULL) {
      exit(1);
    } else {
      if (strncmp(input_str,"quit",4) == 0) exit(1);
      status = gscm_eval_str(&eval_answer, toplev, input_str);
      fprintf(stdout,"%s\n", eval_answer);
      fprintf(stdout,"Guile > ");
    }
  }

  /* now clean up and quit */
  gscm_destroy_top_level(toplev);
}

%}



