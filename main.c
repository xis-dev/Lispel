#include "lispel.h"

#ifdef _WIN32

static char buffer[2048];

char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

void add_history(char* unused) {}

#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

int main(int argc, char* argv[]) {

    lenv* e = lenv_new();
    lenv_add_builtins(e);

    lval_del(builtin_load(e, lval_add(lval_sexpr(), lval_str("lispel_stl.lpl"))));
    if (argc == 1) {	

    puts("Lispel Version 0.0.0.0.1");
    puts("Press Ctrl+C to exit");

    while (1) {


    // Output prompt and get input
    char* input = readline("lispy> ");

    // Add input to history
    add_history(input); 

    // Read from input to create S-Expression

    lreader reader;
    lreader_init(&reader);

    lval* expr = lval_read_expr(input, &reader, '\0');

    //Evaluate and print input
    lval* x = lval_eval(e, expr);
    lval_println(e, x);

    if (x->type == LVAL_EXIT) {
	lval_del(x);
	free(input);

	printf("Lispel exited at %zu:%zu. \n", reader.r, reader.c);
	break;
    }

    lval_del(x);

    free(input);
    }
    
    }

    // supplied with list of files
    if (argc >= 2) {

	// loop over each supplied file name
	for (int i = 1; i < argc; i++) {
	    // argument list with single argument, the filename
	    lval* args = lval_add(lval_sexpr(), lval_str(argv[i]));

	    // pass to builtin load and get result
	    lval* x = builtin_load(e, args);

	    // if result is an err, be sure to print
	    if (x->type == LVAL_ERR) lval_println(e, x);
	    lval_del(x);
	}
    }

    lenv_del(e);

    return 0;

}


