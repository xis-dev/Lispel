#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#include "lispAssert.c"

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

// Enum for possible lisp value types
typedef enum lval_type lval_type;
enum lval_type{LVAL_ERR, LVAL_EXIT, LVAL_RETURN, LVAL_NUM, LVAL_SYM, LVAL_FUN, LVAL_STR, LVAL_COND, LVAL_SEXPR, LVAL_QEXPR};

char* ltype_name(int t);

struct lval;
typedef struct lval lval;

struct fn_lval;
typedef struct fn_lval fn_lval;

union u_lval;
typedef union u_lval u_lval;

struct lenv;
typedef struct lenv lenv;

typedef lval*(*lbuiltin)(lenv*, lval*);


struct lenv {

    lenv* par; 

    int count;
    char** syms;
    lval** vals;
    
    int builtin_count;
    char** builtin_syms;
    lval** builtin_vals;
};

// Reader defines where we are in current reading context
typedef struct lreader {
    size_t r;
    size_t c;
    size_t pos;
} lreader;

void lreader_init(lreader* r) {
    r->r = 1;
    r->c = 1;
    r->pos = 0;
}

void lreader_nextchar(lreader* r) {
    ++(r->pos);
    ++(r->c);
}

void lreader_newline(lreader* r) {
    ++(r->r);
    r->c = 1;
    ++(r->pos);
}

typedef struct expr_lval {
    int count;
    lval** cell;
} expr_lval;

typedef struct fn_lval {
    lbuiltin builtin;
    lenv* env;
    lval* formals;
    lval* body;
} fn_lval;

union u_lval {
    long num;

    // Errors, symbols, exit, strings
    char* err;
    char* sym;
    char* exit;
    char* str;

    fn_lval func;

    uint8_t cond;

    expr_lval children;

    lval* ret;
};


// Lisp value struct
// TODO: Replace with union
struct lval{ 
    int type;

    u_lval val;
};

lenv* lenv_new(void);
void  lenv_del(lenv* e);
lenv* lenv_copy(lenv* e);

lval* lval_read(char* s, lreader* rd);
lval* lval_read_expr(char* s, lreader* rd, char end);
lval* lval_read_sym(char* s, lreader* rd);
lval* lval_read_str(char* s, lreader* rd);

// Possible unescapable characters
char* lval_str_unescapable = "abfnrtv\\\'\"";
char lval_str_unescape(char x);

// Possible escapable characters
char* lval_str_escapable = "\a\b\f\n\r\t\v\\\'\"";
char* lval_str_escape(char x);

lval* lenv_get(lenv* e, lval* k);
void lenv_put(lenv* e, lval* k, lval* v);
void lenv_def(lenv* e, lval* k, lval* v);

lval* lval_num(long x);
lval* lval_err(char* fmt, ...);
lval* lval_sym(char* s);
lval* lval_sexpr();
lval* lval_qexpr();
lval* lval_builtin(lbuiltin func);
lval* lval_exit(char* fmt, ...);
lval* lval_cond(bool c);
lval* lval_lambda(lval* formals, lval* body);
lval* lval_str(char* s);
lval* lval_return(lval* r);

lval* lval_copy(lval* v);
void lval_del(lval* v);

// Conditional comparisons
lval* lval_equals(lval* a, lval* b);
lval* lval_greater(lval* a, lval* b);
lval* lval_less(lval* a, lval* b);

lval* lval_add(lval* v, lval* x);

void lval_print(lenv* e, lval* v);
void lval_expr_print(lenv* e, lval* v, char open, char close);
void lval_println(lenv* e, lval* v) {lval_print(e, v); putchar('\n');}
void lval_print_str(lval* v);
void lval_print_cond(uint8_t v);



lval* lval_eval(lenv* e, lval* v);
lval* lval_call(lenv* e, lval* f, lval* a);

lval* builtin(lval* a, char* func);



void lenv_add_builtins(lenv* e);
void lenv_add_builtin_fn(lenv* e, char* name, lbuiltin fn);
void lenv_add_builtin_sym(lenv* e, char* sym);
void lenv_add_builtin_fns(lenv* e);
void lenv_add_builtin_syms(lenv* e);

void lenv_put_builtin(lenv* e, char* name, lval* v);

lval* builtin_op(lenv* e, lval* a, char* op);

// Base built in functions
lval* builtin_load(lenv* e, lval* a);
lval* builtin_return(lenv* e, lval* a);
lval* builtin_print(lenv* e, lval* a);
lval* builtin_error(lenv* e, lval* a);
lval* builtin_read(lenv* e, lval* a);
lval* builtin_show(lenv* e, lval* a);
lval* builtin_head(lenv* e, lval* a);
lval* builtin_list(lenv* e, lval* a);
lval* builtin_tail(lenv* e, lval* a);
lval* builtin_join(lenv* e, lval* a);
lval* builtin_cons(lenv* e, lval* a);
lval* builtin_len (lenv* e, lval* a);
lval* builtin_init(lenv* e, lval* a);
lval* builtin_eval(lenv* e, lval* a);
lval* builtin_exit(lenv* e, lval* a);
lval* builtin_lambda(lenv* e, lval* a);
lval* builtin_fn  (lenv* e, lval* a);
lval* builtin_negate(lenv* e, lval* a);

// Conditional functions
lval* builtin_if(lenv* e, lval* a);
lval* builtin_equals(lenv* e, lval* a);
lval* builtin_not_equals(lenv* e, lval* a);
lval* builtin_greater(lenv* e, lval* a);
lval* builtin_greater_or_equal(lenv* e, lval* a);
lval* builtin_less(lenv* e, lval* a);
lval* builtin_less_or_equal(lenv* e, lval* a);
lval* builtin_or(lenv* e, lval* a);
lval* builtin_and(lenv* e, lval* a);

lval* builtin_var(lenv* e, lval* a, char* n);

// Symbol definition functions
lval* builtin_def(lenv* e, lval* a);
lval* builtin_put(lenv* e, lval* a);

lval* lval_join(lval* x, lval* y);
lval* lval_join_str(lval* x, lval* y);

char* lenv_get_builtin_name(lenv* e, lbuiltin f);

bool lval_assert_types(lval* v, size_t type_count, ...) {
    if (type_count == 0) return false;


    va_list arg_list;

    va_start(arg_list, type_count);

    for (int i = 0; i < type_count; ++i) {
	if (v->type == va_arg(arg_list, lval_type)) { 

	    va_end(arg_list);
	    return true;
	}
    }

    va_end(arg_list);

    return false;
}

lval* lval_assert_types_err(const char* func_name, lval* v, size_t count, ...) {
    // Could exit instead since this is an assertion
    if (count == 0) return lval_exit("Exited %s, while trying to assert types, expected one or more types to assert", func_name);

    // Create buffer for error string
    char err_str[512];

    va_list arg_list;

    va_start(arg_list, count);

    // Put base(single type) error string into buffer
    sprintf(err_str, "Function %s passed in incorrect number of arguments. Got %s. Expected %s ", func_name, 
	    ltype_name(v->type), ltype_name(va_arg(arg_list, lval_type)));
    --count;

    // Append other types into string
    for (size_t i = 0; i < count; ++i) {
	if (i == (count - 1)) {
	    sprintf(err_str + strlen(err_str), "or %s", ltype_name(va_arg(arg_list, lval_type)));
	}
	else {
	    sprintf(err_str + strlen(err_str), ", %s ", ltype_name(va_arg(arg_list, lval_type)));
	}

    }

    sprintf(err_str + strlen(err_str), ".");

    // Cleanup argument list
    va_end(arg_list);

    return lval_err(err_str);
}



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


char* ltype_name(int t) {
    switch(t) {
	case LVAL_FUN: return "Function";
	case LVAL_NUM: return "Number";
	case LVAL_ERR: return "Error";
	case LVAL_SYM: return "Symbol";
	case LVAL_STR: return "String";
	case LVAL_SEXPR: return "S-Expression";
	case LVAL_QEXPR: return "Q-Expression";
	case LVAL_COND: return "Condition";
	// Should actually be of holding type so in practice this never gets used
	case LVAL_RETURN: return "Return";
	default: return "Unknown";
    }
}

lenv* lenv_new(void) {
    lenv* e = malloc(sizeof(lenv));

    e->par = NULL;

    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;

    e->builtin_count = 0;
    e->builtin_syms = NULL;
    e->builtin_vals = NULL;
    return e;
}

void lenv_del(lenv* e) {

    for (int i = 0; i < e->builtin_count; ++i) {
	free(e->builtin_syms[i]);
	lval_del(e->builtin_vals[i]);
    }

    free(e->builtin_syms);
    free(e->builtin_vals);

    for (int i = 0; i < e->count; ++i) {
	free(e->syms[i]);
	lval_del(e->vals[i]);
    }

    free(e->syms);
    free(e->vals);
    free(e);
}

lenv* lenv_copy(lenv* e) {
    lenv* n = malloc(sizeof(lenv));
    n->par = e->par;

    n->builtin_count = e->builtin_count;
    n->builtin_syms = malloc(sizeof(char*) * e->builtin_count);
    n->builtin_vals = malloc(sizeof(lval*) * e->builtin_count);

    for (int i = 0; i < e->builtin_count; ++i) {
	n->builtin_syms[i] = malloc(strlen(e->builtin_syms[i]) + 1);
	strcpy(n->builtin_syms[i], e->builtin_syms[i]);

	n->builtin_vals[i] = lval_copy(e->builtin_vals[i]);

    }

    n->count = e->count;
    n->syms = malloc(sizeof(char*) * e->count);
    n->vals = malloc(sizeof(lval*) * e->count);

    for (int i = 0; i < e->count; ++i) {
	n->syms[i] = malloc(strlen(e->syms[i]) + 1);
	strcpy(n->syms[i], e->syms[i]);

	n->vals[i] = lval_copy(e->vals[i]);

    }
    return n;
}

lval* lval_read_expr(char* s, lreader* rd, char end) {

    // Create new qexpr or sexpr
    
    lval* x = (end == '}') ? lval_qexpr() : lval_sexpr();

    // While not at end char, keep reading lvals
    while (s[rd->pos] != end) {
	lval* y = lval_read(s, rd);

	// If an err then return and stop
	if (y->type == LVAL_ERR) {
	    lval_del(x);
	    return y;
	}
	else {
	    lval_add(x, y);
	}
    }

    // Move past end char
    lreader_nextchar(rd);

    return x;
}

lval* lval_read_sym(char* s, lreader* rd) {

    lval* x = NULL;

    // Allocate empty string
    char* part = calloc(1, 1);

    // While valid identifier characters
    while (strchr("abcdefghijklmnopqrstuvwxyz"
		   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		   "0123456789_+-*\\/=<>!|&", s[rd->pos]) && s[rd->pos] != '\0') {
	
	// Append char to end of string
	/** Realloc with 1 extra character space
	 *  Set last element of char array to null terminator
	 *  Set last string character to new character
	 * */
	part = realloc(part, strlen(part) + 2);
	part[strlen(part)+1] = '\0';
	part[strlen(part)+0] = s[rd->pos];
	lreader_nextchar(rd);
    }

    // Check if identifier looks like number
    bool is_num = strlen(part) > 1 ? strchr("-0123456789", part[0]) : strchr("0123456789", part[0]);
    // Start reading from second string element to confirm if it indeed a number
    for (int i = 1; i < strlen(part); ++i) {
	if (!strchr("0123456789", part[i])) {
	    is_num = false;
	    break;
	}
    }


    // Add symbol or number as lval
    if (is_num) {
	// Clear errno and try to read from part till theres nothing left
	// If we did not encounter a pole/range error, create a number from x,lest return an error
	errno = 0;
	long n = strtol(part, NULL, 10);
	x = errno != ERANGE ? lval_num(n) : lval_err("Invalid Number '%s' at %i:%i", part, rd->r, rd->c);
    }
    else {
	// Not a number, create a symbol type from the string
	// Ensuring symbols dont start with certain characters(numbers)	
	bool invalid_sym_start = strchr("0123456789", part[0]);
	x = !invalid_sym_start ? lval_sym(part) : lval_err("Found invalid start character '%c' at start of symbol at %i:%i", part[0], rd->r, rd->c);
    }

    // Free temp string
    free(part);

    return x;
}

lval* lval_read_str(char* s, lreader* rd) {
    // Allocate empty string
    char* part = calloc(1, 1);

    // While we arent reading the ending quotes
    while (s[rd->pos] != '"') {
	char c = s[rd->pos];

	// If end of input then there is an unterminated string literal
	// Found end of input before last quote
	if (c == '\0') {
	    free(part);
	    return lval_err("Unexpected end of input at string literal");
	}

	// Check if char is a backslash and escape the character after it
	if (c == '\\') {
	    lreader_nextchar(rd);
	    // Check next character is escapable
	    if (strchr(lval_str_unescapable, s[rd->pos])) {
		c = lval_str_unescape(s[rd->pos]);
	    }
	    else {
		free(part);
		return lval_err("Invalid escape character %c", c);
	    }
	}

	// Append character to string
	part = realloc(part, strlen(part) + 2);
	part[strlen(part) + 1] = '\0';
	part[strlen(part)] = c;
	lreader_nextchar(rd);
    }

    // Move forward past final " char
    lreader_nextchar(rd);

    // Add lval and free temp string
    lval* x = lval_str(part);

    free(part);

    return x;
}

char lval_str_unescape(char x) {
    switch(x) {
	case 'a': return '\a';
	case 'b'  : return '\b';
	case 'f'  : return '\f';
	case 'n'  : return '\n';
	case 'r'  : return '\r';
	case 't'  : return '\t';
	case 'v'  : return '\v';
	case '\\' : return '\\';
	case '\'' : return '\'';
	case '\"' : return '\"';
    }

    return '\0';
}


char* lval_str_escape(char x) {
    switch (x) {
	case '\a'  : return "\\a";
	case '\b'  : return "\\b";
	case '\f'  : return "\\f";
	case '\n'  : return "\\n";
	case '\r'  : return "\\r";
	case '\t'  : return "\\t";
	case '\v'  : return "\\v";
	case '\\'  : return "\\\\";
	case '\''  : return "\\\'";
	case '\"'  : return "\\\"";
    }

    return "";
}

lval* lenv_get(lenv* e, lval* k) { 
    LASSERT(k, k->type == LVAL_SYM,
	    "Cannot get non symbol '%s' from environment", 
	    k->val.sym);


    for (int i = 0; i < e->builtin_count; ++i) {
	if (strcmp(e->builtin_syms[i], k->val.sym) == 0) {
	    return lval_copy(e->builtin_vals[i]);
	}
    }
    for (int i = 0; i < e->count; ++i) {
	if (strcmp(e->syms[i], k->val.sym) == 0) {
	    return lval_copy(e->vals[i]);
	}
    }

    // Symbol not found in local environment, check parent
    if (e->par) return lenv_get(e->par, k);

    return lval_err("Unbound symbol! '%s'", k->val.sym);
}

void lenv_put(lenv* e, lval* k, lval* v) {
    // Check if variable already exists
   
    // Cannot redefine builtin variables
    for (int i = 0; i < e->builtin_count; ++i) {
	if (strcmp(e->builtin_syms[i], k->val.sym) == 0) return; 
    }

    for (int i = 0; i < e->count; ++i) {
	// If var is found, delete item at that position
	// and replace with variable supplied by user
	if (strcmp(e->syms[i], k->val.sym) == 0) {
	    lval_del(e->vals[i]);
	    e->vals[i] = lval_copy(v);
	    return;
	}
    }

    // If no existing entry, allocate space for new entry
    e->count++;
    e->vals = realloc(e->vals, sizeof(lval*) * e->count);
    e->syms = realloc(e->syms, sizeof(char*) * e->count);

    // Copy contents of lval and symbol string into new location
    e->vals[e->count - 1] = lval_copy(v);
    e->syms[e->count - 1] = malloc(strlen(k->val.sym) + 1);
    strcpy(e->syms[e->count-1], k->val.sym);
 
}

void lenv_def(lenv* e, lval* k, lval* v) {
    // Iterate till e has no parents
    while (e->par) e = e->par;

    lenv_put(e, k, v);
}

lval* lval_num(long x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->val.num = x;
    return v;

}

lval* lval_err(char* fmt, ...) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;

    // Create a va list and init it
    va_list va;
    va_start(va, fmt);

    // Allocate 512 bytes of space
    v->val.err = malloc(512);

    // Printf error string with max of 511 chars
    vsnprintf(v->val.err, 511 , fmt, va );

    // Reallocate to number of bytes actually used
    v->val.err = realloc(v->val.err, strlen(v->val.err) + 1);

    // Cleanup va list
    va_end(va);

    return v;
}


lval* lval_sym(char* s) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->val.sym = malloc(strlen(s) + 1);
    strcpy(v->val.sym, s);
    return v;
}

lval* lval_sexpr() {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->val.children.count = 0;
    v->val.children.cell = NULL;
    return v;
}

lval* lval_qexpr() {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
    v->val.children.count = 0;
    v->val.children.cell = NULL;
    return v;
} 

lval* lval_builtin(lbuiltin func) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
    v->val.func.builtin = func;
    return v;
}

lval* lval_exit(char* fmt, ...) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_EXIT;

    va_list va;
    va_start(va, fmt);

    // Alloc 512 bytes of space
    v->val.exit = malloc(512);

    // Printf the exit string with max of 511 chars
    vsnprintf(v->val.exit, 511, fmt, va);

    //Realloc to number of bytes actually used
    v->val.exit = realloc(v->val.exit, strlen(v->val.exit) + 1);

    // Cleanup variable list
    va_end(va);

    return v;
}

lval* lval_cond(bool c) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_COND;
    v->val.cond = c == 0 ? 0 : 1;
    return v;
}

lval* lval_lambda(lval* formals, lval* body){
    lval* v = malloc(sizeof(lval));
    v->type  = LVAL_FUN;

    // Set builtin to null
    v->val.func.builtin = NULL;

    // Built new environment
    v->val.func.env = lenv_new();

    v->val.func.formals = formals;
    v->val.func.body = body;

    return v;
}

lval* lval_str(char* s) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_STR;
    v->val.str = malloc(strlen(s) + 1);
    strcpy(v->val.str, s);

    return v;
}

lval* lval_return(lval* r) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_RETURN;
    v->val.ret  = lval_copy(r);
    
    lval_del(r);

    return v;
}

lval* lval_copy(lval* v) { 
    lval* x = malloc(sizeof(lval));

    x->type = v->type;
    switch(v->type) {
	// Copy functions and numbers directly
	case LVAL_FUN: 
	    if (v->val.func.builtin) x->val.func.builtin = v->val.func.builtin;
	    else {
		x->val.func.builtin = NULL;
		x->val.func.env = lenv_copy(v->val.func.env);
		x->val.func.formals = lval_copy(v->val.func.formals);
		x->val.func.body = lval_copy(v->val.func.body);
	    }

	    break;

	case LVAL_NUM: x->val.num = v->val.num; break;
	case LVAL_COND: x->val.cond = v->val.cond; break;

	// Copy strings using malloc and strcpy
	case LVAL_ERR: 
		       x->val.err = malloc(strlen(v->val.err) + 1);
		       strcpy(x->val.err, v->val.err); break;
	case LVAL_SYM:
		       x->val.sym = malloc(strlen(v->val.sym) + 1);
		       strcpy(x->val.sym, v->val.sym); break;

	case LVAL_EXIT:
		       x->val.exit = malloc(strlen(v->val.exit) + 1);
		       strcpy(x->val.exit, v->val.exit); break;
		       
	case LVAL_STR:
		       x->val.str = malloc(strlen(v->val.str) + 1);
		       strcpy(x->val.str, v->val.str); break;

	// Copy lists by copying sub expression
	case LVAL_SEXPR:
	case LVAL_QEXPR:
		       x->val.children.count = v->val.children.count;
		       x->val.children.cell = v->val.children.count != 0 ? malloc(sizeof(lval*) * v->val.children.count): NULL;
		       for (int i = 0; i < v->val.children.count; ++i) {
			    x->val.children.cell[i] = lval_copy(v->val.children.cell[i]);
		       }
		       break;

	case LVAL_RETURN:
		      x->val.ret = lval_copy(v->val.ret); break; 
    }

    return x;

}
void lval_del(lval* v) {
    switch(v->type) {
	case LVAL_NUM: break;
		       
	// Free error and symbol strings
	case LVAL_ERR: free(v->val.err); break;
	case LVAL_SYM: free(v->val.sym); break;
	case LVAL_EXIT: free(v->val.exit); break;
	case LVAL_STR: free(v->val.str); break;

	case LVAL_FUN: 
			if(!v->val.func.builtin) {
			    lenv_del(v->val.func.env);
			    lval_del(v->val.func.formals);
			    lval_del(v->val.func.body);
			}
			break;
	case LVAL_COND:
			break;
	case LVAL_QEXPR:
	case LVAL_SEXPR: 
		for (int i = 0; i < v->val.children.count; ++i) {
		   lval_del(v->val.children.cell[i]); 	    
		}

	// Free memory allocated to contain pointers
	free(v->val.children.cell);
	break;

	case LVAL_RETURN:
		lval_del(v->val.ret);
		break;
    }

    // Free struct itself
    free(v);
}

lval* lval_equals(lval* a, lval* b) {

    if (a->type != b->type) {
        return lval_err("Cannot perform equation on different types. "
                        "Got '%s' on arg 1 and '%s' on arg 2.",
                        ltype_name(a->type), ltype_name(b->type));
    }

    switch (a->type) {
        case LVAL_NUM:
            return lval_cond(a->val.num == b->val.num);

        case LVAL_ERR:
            return lval_cond(strcmp(a->val.err, b->val.err) == 0);

        case LVAL_COND:
            return lval_cond(a->val.cond == b->val.cond);

        case LVAL_SYM:
            return lval_cond(strcmp(a->val.sym, b->val.sym) == 0);

	case LVAL_STR:
	    return lval_cond(strcmp(a->val.str, b->val.str) == 0);

        case LVAL_FUN:
            return lval_err("Cannot perform equation on function types");

        case LVAL_SEXPR:
        case LVAL_QEXPR: {

            if (a->val.children.count != b->val.children.count)
                return lval_cond(false);

            for (int i = 0; i < a->val.children.count; ++i) {
                lval* eq = lval_equals(a->val.children.cell[i], b->val.children.cell[i]);

                if (eq->type == LVAL_ERR)
                    return eq;

                if (eq->val.cond == 0) {
                    lval_del(eq);
                    return lval_cond(false);
                }

                lval_del(eq);
            }

            return lval_cond(true);
        }

        case LVAL_EXIT:
            return lval_err("Cannot perform equation on exit values");

	case LVAL_RETURN:
	    return lval_equals(a->val.ret, b->val.ret);
        default:
            break;
    }

    return lval_cond(false);
}

lval* lval_greater(lval* a, lval* b) {
    if (a->type != b->type) 
	return lval_err("Cannot perform greater operation on different types."
			"Got '%s' on arg 1 and '%s' on arg 2."
			,ltype_name(a->type), ltype_name(b->type));

    switch(a->type) {
	case LVAL_NUM: return lval_cond(a->val.num > b->val.num);
	default: return lval_err("Greater operation can only be performed on numeric types");
    }
    return lval_cond(false);
}

lval* lval_less(lval* a, lval* b) {
    if (a->type != b->type) 
	return lval_err("Cannot perform less operation on different types."
			"Got '%s' on arg 1 and '%s' on arg 2."
			,ltype_name(a->type), ltype_name(b->type));

    switch(a->type) {
	case LVAL_NUM: return lval_cond(a->val.num < b->val.num);
	default: return lval_err("Less operation can only be performed on numeric types");
    }
    return lval_cond(false);
}



lval* lval_add(lval* v, lval* x) {
    v->val.children.count++;
    v->val.children.cell = realloc(v->val.children.cell, sizeof(lval*) * v->val.children.count);

    assert(v->val.children.cell);

    v->val.children.cell[v->val.children.count-1] = x;
    return v;
}

lval* lval_read(char* s, lreader* rd) {

    // Skip all heading white spaces and comments
    while (strchr(" \t\v\r\n;", s[rd->pos]) && s[rd->pos] != '\0') {
	// Continue through comment till the next line or end of input
	if (s[rd->pos] == ';') {
	    while (s[rd->pos] != '\n' && s[rd->pos] != '\0') lreader_nextchar(rd);
	}

	if (s[rd->pos] == '\n') {
	    lreader_newline(rd);
	}
	else {
	    lreader_nextchar(rd);
	}

    }
    
    lval* x = NULL;

    // If we reach end of input we're missing something
    if (s[rd->pos] == '\0') {
	return lval_err("Unexpected end of input at %i:%i", rd->r, rd->c);
    }

    // If next char is ( then read as S-Expression
    else if(s [rd->pos] == '(') { 
	lreader_nextchar(rd);
	x = lval_read_expr(s, rd, ')');
    }

    // If next char is { then read as Q-Expression
    else if (s[rd->pos] == '{') {
	lreader_nextchar(rd);
	x = lval_read_expr(s, rd, '}');
    }

    // If next char is part of a symbol then read as symbol
    else if (strchr("abcdefghijklmnopqrstuvwxyz"
		    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		    "0123456789_+-*\\/=<>!|&^", s[rd->pos])) {
			
			x = lval_read_sym(s, rd);	
    }

    // If next char is " then read as string
    else if (strchr("\"", s[rd->pos])) {
	lreader_nextchar(rd);
	x = lval_read_str(s, rd);
    }

    // Encountered some unexpected char
    else {
	x = lval_err("Unexpected char '%c' at %i:%i", s[rd->pos], rd->r, rd->c);
    }
 
    // Skip all trailing white spaces and comments
    while (strchr(" \t\v\r\n;", s[rd->pos]) && s[rd->pos] != '\0') {
	// Continue through comment till the next line or end of input
	if (s[rd->pos] == ';') {
	    while (s[rd->pos] != '\n' && s[rd->pos] != '\0') lreader_nextchar(rd);
	}

	if (s[rd->pos] == '\n') {
	    lreader_newline(rd);
	}
	else {
	    lreader_nextchar(rd);
	}

    }

    return x;
}


void lval_expr_print(lenv* e, lval* v, char open, char close) {
    putchar(open);

    for (int i = 0; i < v->val.children.count; ++i) {
	// Print value contained within
	lval_print(e, v->val.children.cell[i]);

	// Dont print trailing space if last element
	if (i != (v->val.children.count -1)) {
	    putchar(' ');
	}
    }

    putchar(close);
}

void lval_print_str(lval* v) {
    putchar('"');

    // Loop over all characters in string
    for (int i = 0; i < strlen(v->val.str); ++i) {
	if (strchr(lval_str_escapable, v->val.str[i])) {
	    // If character is escapable then escape it
	    printf("%s", lval_str_escape(v->val.str[i]));
	}
	else {
	    // Otherwise print char as is
	    putchar(v->val.str[i]);
	}
    }

    putchar('"');
}

void lval_print_cond(uint8_t v) {
    if (v == 0) {

    printf("False."); 
    return;

    }

    printf("True.");
}

void lval_print(lenv* e, lval* v) {
    switch(v->type) {
	case LVAL_NUM: printf("%li", v->val.num); break;
	case LVAL_ERR: printf("Error: %s", v->val.err); break;
	case LVAL_EXIT: printf("Exit: %s", v->val.exit); break;
	case LVAL_SYM: printf("%s", v->val.sym); break;
	case LVAL_STR: lval_print_str(v); break;
	case LVAL_COND: lval_print_cond(v->val.cond); break;
	case LVAL_FUN: 
		if (v->val.func.builtin) printf(lenv_get_builtin_name(e, v->val.func.builtin)); 
		else {
		    printf("(\\");
		    lval_print(e, v->val.func.formals);
		    putchar(' ');
		    lval_print(e, v->val.func.body);
		    putchar(')');
		}
		break;
	case LVAL_SEXPR: lval_expr_print(e, v, '(', ')'); break;
	case LVAL_QEXPR: lval_expr_print(e, v, '{', '}'); break;
	case LVAL_RETURN: lval_print(e, v->val.ret); break;
    }
}


lval* lval_pop(lval* v, int i) {
    // Find item at i
    lval* x = v->val.children.cell[i];

    // Shift memory after the item i over the top
    memmove(&v->val.children.cell[i], &v->val.children.cell[i+1],
	    sizeof(lval*) * (v->val.children.count - i - 1));

    // Decrease item count in list
    --v->val.children.count;

    if (v->val.children.count == 0) {
	free(v->val.children.cell);
	v->val.children.cell = NULL;
	return x;
    }
    // Reallocate the memory used 
    v->val.children.cell = realloc(v->val.children.cell, sizeof(lval*) * v->val.children.count);
   assert(v->val.children.cell);
    return x;
}

lval* lval_take(lval* v, int i) {
    // If we're going to destroy the whole thing anyways
    // isnt mem shifting and realloc worthless expense, make a peek func?
    lval* x = lval_pop(v, i);
    lval_del(v);
    return x;
}

lval* builtin_add(lenv* e, lval* a) {
    return builtin_op(e, a, "+");
}

lval* builtin_sub(lenv* e, lval* a) {
    return builtin_op(e, a, "-");
}

lval* builtin_mul(lenv* e, lval* a) {
    return builtin_op(e, a, "*");
}

lval* builtin_div(lenv* e, lval* a) {
    return builtin_op(e, a, "/");
}

lval* builtin_var(lenv* e, lval* a, char* func) {
    LASSERT_TYPE(func, a, 0, LVAL_QEXPR);

    lval* syms = a->val.children.cell[0];

    // Ensure we are evaluating symbol 
    // and early error out if built in
    for (int i = 0; i < syms->val.children.count; ++i) {
	LASSERT(a, (syms->val.children.cell[i]->type == LVAL_SYM),
		"Function '%s' cannot define non-symbol."
		"Got %s, Expected %s.", func,
		ltype_name(syms->val.children.cell[i]->type),
		ltype_name(LVAL_SYM));

	for (int j = 0; j < e->builtin_count; ++j) {
	    if(strcmp(e->builtin_syms[j], syms->val.children.cell[i]->val.sym) == 0) {
		lval_del(a);
		return lval_err("Attempting to redefine builtin '%s'", 
				e->builtin_syms[j]);
	    }
	    }
    }

    LASSERT(a, (syms->val.children.count == a->val.children.count - 1),
	    "Function '%s' passed too many arguments for symbols."
	    "Got %i, Expected %i", func, syms->val.children.count, a->val.children.count - 1);

    for (int i = 0; i < syms->val.children.count; ++i) {
	// If 'def' define in globally. If 'put' define in locally
	if (strcmp(func, "def") == 0) {
	    lenv_def(e, syms->val.children.cell[i], a->val.children.cell[i+1]);
	}

	if (strcmp(func, "=") == 0) {
	    lenv_put(e, syms->val.children.cell[i], a->val.children.cell[i+1]);
	}
    }

    lval_del(a);

    return lval_sexpr();
} 

lval* builtin_def(lenv* e , lval* a) {
    return builtin_var(e, a, "def");
}

lval* builtin_put(lenv* e, lval* a) {
    return builtin_var(e, a, "=");
}


char* lenv_get_builtin_name(lenv* e, lbuiltin f) {
    
    for (int i = 0; i < e->builtin_count; ++i) {
	if (e->builtin_vals[i]->type == LVAL_FUN) {
	    if (e->builtin_vals[i]->val.func.builtin == f) return e->builtin_syms[i];
	}
    }

    return "Unknown builtin function"; 
}

void lenv_add_builtins(lenv* e) {
   lenv_add_builtin_fns(e);

   // symbols
   lenv_add_builtin_syms(e);

   // Conditionals
   lenv_put_builtin(e, "true", lval_cond(true));
   lenv_put_builtin(e, "false", lval_cond(false));

   // General keywords
   lenv_put_builtin(e, "nil", lval_qexpr());
}

void lenv_add_builtin_sym(lenv* e, char* sym)  {
    lval* v = lval_sym(sym);

    lenv_put_builtin(e, sym, v);
}

void lenv_add_builtin_fn(lenv* e, char* name, lbuiltin func) {
    lval* v = lval_builtin(func);

    lenv_put_builtin(e, name, v);
}

void lenv_put_builtin(lenv* e, char* name, lval* v) {
    for (int i = 0; i < e->builtin_count; ++i) {
	if (strcmp(e->builtin_syms[i], name) == 0) {
	    lval_del(e->builtin_vals[i]);
	   e->builtin_vals[i] = lval_copy(v); 
	    return;
	}
    }

    ++e->builtin_count;

    e->builtin_syms = realloc(e->builtin_syms, sizeof(char*) * e->builtin_count);
    e->builtin_vals = realloc(e->builtin_vals, sizeof(lval*) * e->builtin_count);

    e->builtin_syms[e->builtin_count - 1] = malloc(strlen(name) + 1);
    strcpy(e->builtin_syms[e->builtin_count - 1], name);

    e->builtin_vals[e->builtin_count - 1] = lval_copy(v);

    lval_del(v);
}

void lenv_add_builtin_fns(lenv* e) {
    // List functions
    lenv_add_builtin_fn(e, "list", builtin_list);
    lenv_add_builtin_fn(e, "head", builtin_head);
    lenv_add_builtin_fn(e, "tail", builtin_tail);
    lenv_add_builtin_fn(e, "eval", builtin_eval);
    lenv_add_builtin_fn(e, "join", builtin_join);
    lenv_add_builtin_fn(e, "cons", builtin_cons);
    lenv_add_builtin_fn(e, "len",  builtin_len);
    lenv_add_builtin_fn(e, "init", builtin_init);
    lenv_add_builtin_fn(e, "def",  builtin_def);
    lenv_add_builtin_fn(e, "exit", builtin_exit);
    lenv_add_builtin_fn(e, "\\", 	builtin_lambda);
    lenv_add_builtin_fn(e, "=", 	builtin_put);
    lenv_add_builtin_fn(e, "fn", 	builtin_fn);
    lenv_add_builtin_fn(e, "load", 	builtin_load);
    lenv_add_builtin_fn(e, "print", 	builtin_print);
    lenv_add_builtin_fn(e, "error", 	builtin_error);
    lenv_add_builtin_fn(e, "return", 	builtin_return);
    lenv_add_builtin_fn(e, "read", 	builtin_read);

    // Conditional Functions
    lenv_add_builtin_fn(e, "==", 	builtin_equals);
    lenv_add_builtin_fn(e, "if", 	builtin_if);
    lenv_add_builtin_fn(e, "!=", 	builtin_not_equals);
    lenv_add_builtin_fn(e, "not", 	builtin_not_equals);
    lenv_add_builtin_fn(e, ">" , 	builtin_greater);
    lenv_add_builtin_fn(e, ">=", 	builtin_greater_or_equal);
    lenv_add_builtin_fn(e, "<" , 	builtin_less);
    lenv_add_builtin_fn(e, "<=", 	builtin_less_or_equal);
    lenv_add_builtin_fn(e, "||", 	builtin_or);
    lenv_add_builtin_fn(e, "or", 	builtin_or);
    lenv_add_builtin_fn(e, "&&", 	builtin_and);
    lenv_add_builtin_fn(e, "and", 	builtin_and);
    lenv_add_builtin_fn(e, "!", 	builtin_negate);

    // Mathematical Functions 
    lenv_add_builtin_fn(e, "+", builtin_add);
    lenv_add_builtin_fn(e, "-", builtin_sub);
    lenv_add_builtin_fn(e, "*", builtin_mul);
    lenv_add_builtin_fn(e, "/", builtin_div);
}

void lenv_add_builtin_syms(lenv* e) {
    lenv_add_builtin_sym(e, "else");
}

lval* builtin_op(lenv* e, lval* a, char* op) {
    // Ensure all arguments are numbers
    for (int i = 0; i < a->val.children.count; ++i) {
	if (a->val.children.cell[i]->type != LVAL_NUM) {
	    lval_del(a);
	    return lval_err("Cannot operate on non-number!");
	}
    }

    // Pop first element
    lval* x = lval_pop(a, 0);

    // If no arguments and sub then perform unary negation
    if ((strcmp(op, "-") == 0) && a->val.children.count == 0) x->val.num = -x->val.num;

    // While there are still elements remaining
    while (a->val.children.count > 0) {
	
	// Pop next element
	lval* y = lval_pop(a, 0);

	if(strcmp(op, "+") == 0) x->val.num += y->val.num;
	if(strcmp(op, "-") == 0) x->val.num -= y->val.num;
	if(strcmp(op, "*") == 0) x->val.num *= y->val.num;
	if(strcmp(op, "/") == 0) { 
	    if (y->val.num == 0) {
		lval_del(x);
		lval_del(y);
		x = lval_err("Division by Zero!");
		break;
	    }
	    x->val.num /= y->val.num;
	}


	if (strcmp(op, "%") == 0) {
	    if (y->val.num == 0) {
		lval_del(x);
		lval_del(y);
		x = lval_err("Modulo of Zero!");
		break;
	    }
		x->val.num -= ((int)(x->val.num / y->val.num) * y->val.num); 
	}
    if (strcmp(op, "^") == 0) {
	if (y->val.num == 0) {
	    x->val.num = 1;
	    continue;
	};
	long val = x->val.num;
	for (int i = 0; i < y->val.num - 1; ++i) {
	val *= x->val.num;
	} 
	// Cant represent floating point num atm, truncate for now
	// TODO: Truncating does nothing itll always be zero, leave till floating point impl
	x->val.num = (y->val.num < 0) ? (long)(1/val) : val;
    }

    lval_del(y);
    }

    lval_del(a);
    return x;
}

lval* builtin_load(lenv* e, lval* a) {
    LASSERT_NUM ("load", a, 1);
    LASSERT_TYPE("load", a, 0, LVAL_STR);

    // Open file and check if it exists
    FILE* f = fopen(a->val.children.cell[0]->val.str, "rb");
    if (!f) {
	lval* err = lval_err("Could not load Library %s", a->val.children.cell[0]->val.str);
	lval_del(a);
	return err;
    }

    // Read file contents
    // Seek to end of file so we can use ftell(beginning to current stream pos) to get file length
    fseek(f, 0, SEEK_END);
    long f_len = ftell(f);

    // Return to beginning of file for reading
    fseek(f, 0, SEEK_SET);
    // Allocate memory for all characters + null terminator
    char* input = calloc(f_len + 1, 1);

    /** 
     * Read up to f_len, into input, 1 byte from f(file stream)
     */
    fread(input, 1, f_len, f);
    // Close the file
    fclose(f);

    // Read from input to create an S-Expression
    lreader reader;
    lreader_init(&reader);
    lval* expr = lval_read_expr(input, &reader, '\0');
    free(input);

    // Evaluate all expressions contained within the S-Expression
    if (expr->type != LVAL_ERR) {
	while (expr->val.children.count) {
	    lval* x = lval_eval(e, lval_pop(expr, 0));
	    if (x->type == LVAL_ERR) lval_println(e, x);
	    lval_del(x);

	}
    }
    else {
	lval_println(e, expr);
    }

    lval_del(expr);
    lval_del(a);

    return lval_sexpr();
}

lval* builtin_return(lenv* e, lval* a) {
    LASSERT_NUM(lenv_get_builtin_name(e, &builtin_return), a, 1);

    lval* r = lval_eval(e, lval_take(a, 0));

    return lval_return(r);
}

lval* builtin_print(lenv* e, lval* a){
    
    // Print each argument followed by a space 
    for (int i = 0; i < a->val.children.count; ++i) {
	lval_print(e, a->val.children.cell[i]);
	putchar(' ');
    }

    // Print a new line and delete args
    putchar('\n');
    lval_del(a);

    return lval_sexpr();
}

lval* builtin_error(lenv* e, lval* a){
    LASSERT_NUM(lenv_get_builtin_name(e, &builtin_error), a, 1);
    LASSERT_TYPE("error", a, 0, LVAL_STR);

    // Construct err from first argument
    lval* err = lval_err(a->val.children.cell[0]->val.str);

    lval_del(a);
    return err;
}

lval* builtin_read(lenv* e, lval* a) {
    LASSERT_NUM("read", a, 1);

    LASSERT_TYPE("read", a, 0, LVAL_STR);

    lval* s = lval_take(a, 0);

    lreader reader;
    lreader_init(&reader);

    lval* q = lval_qexpr();
    lval_add(q, lval_read_expr(s->val.str, &reader, '\0'));

    lval_del(s);

    return q;
}

//lval* builtin_show(lenv* e, lval* a);

lval* builtin_head(lenv* e, lval* a) {
    // Check error conditions
    LASSERT_NUM(lenv_get_builtin_name(e, &builtin_head), a, 1);

    LASSERT_TYPES(lenv_get_builtin_name(e, &builtin_head), a, 0, 2, LVAL_QEXPR, LVAL_STR);

    // Otherwise, no errors, take first argument
    lval* v = lval_take(a, 0);

    switch(v->type) {
	case LVAL_STR:
	    LASSERT(v, strlen(v->val.str) > 0,
		    "Function 'head' passed in empty string !");

	    // Resize the string
	    v->val.str = realloc(v->val.str, 2);
	    v->val.str[1] = '\0';

	    if (!v->val.str) {
		lval_del(v);
		return lval_err("Failed to allocate memory for string while performing function 'head'");
	    }

	    break;
	default:
	    LASSERT(v, v->val.children.count != 0, 
	    "Function 'head' passed {}!");

	    // Delete all elements that are not head and return
	    while (v->val.children.count > 1) {lval_del(lval_pop(v, 1));}
	    break;
    }

    return v;
}

lval* builtin_tail(lenv* e, lval* a) {
    // Check error conditions
    LASSERT_NUM("tail", a, 1);

    LASSERT_TYPES("tail", a, 0, 2, LVAL_QEXPR, LVAL_STR);

    lval* v = lval_take(a, 0);

    switch (v->type) {
	case LVAL_STR:
	    // Ensure we have at least an element in the string
	    LASSERT(v, strlen(v->val.str) > 0,
		    "Function 'tail' passed in empty string \"\"!");

	    // Resize string excluding first element, single element str has empty tail
	    v->val.str = memmove(v->val.str, v->val.str + 1, strlen(v->val.str));
	    break;
	default:
	    // Ensure we dont have an empty expression
	    LASSERT(v, v->val.children.count != 0, 
	    "Function 'tail' passed {}!");

	    // Delete first element and return
	    lval_del(lval_pop(v, 0));
	    break;
    }

    return v;

}

lval* builtin_list(lenv* e, lval* a) {
    a->type = LVAL_QEXPR;
    return a;
}

lval* builtin_eval(lenv* e, lval* a) {
    LASSERT(a, a->val.children.count == 1,
	    "Function 'eval' passed too many arguments!");

    LASSERT(a, a->val.children.cell[0]->type == LVAL_QEXPR,
	    "Function 'eval' passed incorrect type!");

    lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(e, x);
}

lval* builtin_exit(lenv* e, lval* a) {
    // Delete all children of a and simply return whatever a was
    
    //LASSERT(a, a->val.children.count == 0,
//	    "Function 'exit' passed in '%i' parameters. Expected 0",
//	    a->val.children.count);

    // Or ensure its called with empty qexpr or sexpr
    //while (a->val.children.count != 0) {
//	lval_del(lval_pop(a, 0));
  //  }
    // Ensure we didnt come across an error
    if (a->type == LVAL_ERR) lval_println(e, a);

    LEXIT(a);

    return a;
}

lval* builtin_lambda(lenv* e, lval* a) {
    // Check two arguments, each of which are Q-Expressions
    LASSERT_NUM("\\", a, 2);
    LASSERT_TYPE("\\", a, 0, LVAL_QEXPR);
    LASSERT_TYPE("\\", a, 1, LVAL_QEXPR);

    // Check first Q-Expression only contains symbols
    for (int i = 0; i < a->val.children.cell[0]->val.children.count; ++i) {
	LASSERT(a, (a->val.children.cell[0]->val.children.cell[i]->type == LVAL_SYM),
		"Cannot define non-symbol. Got %s, Expected %s.",
		ltype_name(a->val.children.cell[0]->val.children.cell[i]->type), ltype_name(LVAL_SYM));
    }

    // Pop first two args and pass them to lval_lambd
    lval* formals = lval_pop(a, 0);
    lval* body = lval_pop(a, 0);

    lval_del(a);

    return lval_lambda(formals, body);
}

lval* builtin_fn  (lenv* e, lval* a) {

    LASSERT_NUM(lenv_get_builtin_name(e, &builtin_fn), a, 1);

    // Take off qexpr
    lval* q = lval_take(a, 0);

   LASSERT_NUM(lenv_get_builtin_name(e, &builtin_fn), q, 3);
    
    LASSERT_TYPE(lenv_get_builtin_name(e, &builtin_fn), q,
		 0, LVAL_SYM);

    LASSERT_TYPE(lenv_get_builtin_name(e, &builtin_fn), q,
		 1, LVAL_QEXPR);
    
    LASSERT_TYPE(lenv_get_builtin_name(e, &builtin_fn), q,
		 2, LVAL_QEXPR);

    // Define function in environment and pop off requried arguments
    lval* sym = lval_pop(q, 0);
    lval* formals = lval_pop(q, 0);
    lval* body    = lval_pop(q, 0);

    lval* f = lval_lambda(formals, body);
    lenv_def(e, sym, f);

    lval_del(f);
    lval_del(q);

    lval_del(sym);
    return lval_sexpr();

}

lval* builtin_negate(lenv* e, lval* a) {
    LASSERT_NUM(lenv_get_builtin_name(e, &builtin_negate), a, 1);
    LASSERT_TYPE(lenv_get_builtin_name(e, &builtin_negate), a, 0, LVAL_COND);

    lval* c = lval_take(a, 0);
    c->val.cond = c->val.cond == 0 ? 1 : 0;

    return c;
}

lval* builtin_if(lenv* e, lval* v) {

    if (v->val.children.count < 2) {
	lval* err = lval_err("'if' statement passed in too little arguments. "
			     "Got '%i'. Expected 2.", v->val.children.count);
	lval_del(v);
	return err;

    }
    else if (v->val.children.count != 2 && v->val.children.count != 4) {
	lval* err = lval_err("'if' statement passed in incorrect number of arguments. "
			     "Got '%i'. Expected 2 or 4 with else branch.", v->val.children.count);
	lval_del(v);
	return err;
    }

    // lval* expr = lval_pop(v, 0);
    // lval* c = lval_eval(e, expr);

    // if (c->type != LVAL_COND && c->type != LVAL_ERR) {
//	lval* err = lval_err("'if' statement expected condition as first argument. "
//			     "Got '%s'.", ltype_name(c->type));

//	lval_del(c);
//	lval_del(v);
//	return err;
  //  }
    // Return any errors that may have occured during eval
    // else if(c->type == LVAL_ERR) {
// 	lval_del(v);
//	return c;
  //  }


    LASSERT_TYPE("if", v, 0, LVAL_COND);
    LASSERT_TYPE("if", v, 1, LVAL_QEXPR);

    // If there is an else statement
    if (v->val.children.count == 4) {
	LASSERT_TYPE("if", v, 2, LVAL_SYM);
	// Early return out if not else statement
	if (strcmp(v->val.children.cell[2]->val.sym, "else") != 0) {
	    lval* err = lval_err("Expected 'else' keyword in 'if' statement. "
			    "Got '%s'.", v->val.children.cell[1]->val.sym);
	    lval_del(v);
	    return err;
	}

	LASSERT_TYPE("if", v, 3, LVAL_QEXPR);
    }

    lval* c = lval_pop(v, 0);
    lval* q;
    // If condition is true eval first qexpr
    if (c->val.cond != 0) {
    q = lval_pop(v, 0);

    }
    // Condition is false and there is an else branch, eval last qexpr
    else if (v->val.children.count == 3) {
    q = lval_pop(v, 2);
    }
    // No else branch, return empty sexpr
    else {
	lval_del(v);
	lval_del(c);
	return lval_sexpr();
    }

    lval* result = builtin_eval(e, lval_add(lval_sexpr(), q));
 
    lval_del(c);

    lval_del(v);
    return result;
}

lval* builtin_equals(lenv* e, lval* a) {
    LASSERT_NUM("==", a, 2);
    // Check if second argument has the same type as first
    LASSERT_TYPE("==", a, 1, a->val.children.cell[0]->type);

    lval* c = lval_equals(a->val.children.cell[0], a->val.children.cell[1]);

    lval_del(a);

    return c;
}


lval* builtin_not_equals(lenv* e, lval* a) {
    lval* c = builtin_equals(e, a);
    
    if (c->type == LVAL_COND) builtin_negate(e, lval_add(lval_sexpr(), c));

    return c;
    
}

lval* builtin_greater(lenv* e, lval* a) {
    LASSERT_NUM(">", a, 2);
    LASSERT_TYPE(">", a, 1, a->val.children.cell[0]->type);

    lval* c = lval_greater(a->val.children.cell[0], a->val.children.cell[1]);

    lval_del(a);
    
    return c;
}

lval* builtin_greater_or_equal(lenv* e, lval* a) {
    LASSERT_NUM(">=", a, 2);
    LASSERT_TYPE(">=", a, 1, a->val.children.cell[0]->type);

    lval* g = builtin_greater(e, lval_copy(a));
    lval* eq = builtin_equals(e, lval_copy(a));

    lval* expr = lval_sexpr();
    lval_add(expr, g);
    lval_add(expr, eq);

    lval_del(a);

    return builtin_or(e, expr);
}

lval* builtin_less(lenv* e, lval* a) { 
    LASSERT_NUM("<", a, 2);
    LASSERT_TYPE("<", a, 1, a->val.children.cell[0]->type);

    lval* c = lval_less(a->val.children.cell[0], a->val.children.cell[1]);

    lval_del(a);
    
    return c;
}

lval* builtin_less_or_equal(lenv* e, lval* a) {

    LASSERT_NUM("<=", a, 2);
    LASSERT_TYPE("<=", a, 1, a->val.children.cell[0]->type);

    lval* g = builtin_less(e, lval_copy(a));
    lval* eq = builtin_equals(e, lval_copy(a));

    lval* expr = lval_sexpr();
    lval_add(expr, g);
    lval_add(expr, eq);

    lval_del(a);

    return builtin_or(e, expr);
}

lval* builtin_or(lenv* e, lval* a) {
   for (int i = 0; i < a->val.children.count; ++i) {
       LASSERT_TYPE(lenv_get_builtin_name(e, &builtin_or), a, i, LVAL_COND);

    if (a->val.children.cell[i]->val.cond != 0) {
	lval_del(a);	    
	return lval_cond(true);
	}

   } 

   lval_del(a);

   return lval_cond(false);
}

lval* builtin_and(lenv* e, lval* a) {
    

   for (int i = 0; i < a->val.children.count; ++i) {
       LASSERT_TYPE(lenv_get_builtin_name(e, &builtin_and), a, i, LVAL_COND);

	if (a->val.children.cell[i]->val.cond == 0) {
	    lval_del(a);		    
	    return lval_cond(false);
	}
   } 

   lval_del(a);

   return lval_cond(true);
}

lval* builtin_join(lenv* e, lval* a) {
    LASSERT(a, a->val.children.count > 0, 
	    "Function 'join' got 0 arguments, Expected 1 or more.");

    LASSERT(a, a->val.children.cell[0]->type == LVAL_QEXPR ||
	       a->val.children.cell[0]->type == LVAL_STR,
		"Function 'join' passed %s. Expected %s or %s",
		a->val.children.cell[0]->type, LVAL_QEXPR, LVAL_STR);

    for (int i = 0; i < a->val.children.count; ++i) {
	// Ensure all arguments are the same type
	LASSERT_TYPE(lenv_get_builtin_name(e, &builtin_join), a, i, a->val.children.cell[0]->type);
    }

    lval* x = lval_pop(a, 0);

    while (a->val.children.count) {
	switch (a->val.children.cell[0]->type) {
	    // TODO: Consider popping characters off string then making symbol types with them, but type matching is cleaner
	    case LVAL_STR:
		x = lval_join_str(x, lval_pop(a,0));
		break;
	    default:
		x = lval_join(x, lval_pop(a, 0));
		break;
	}
    }

    lval_del(a);

    return x;
}

lval* lval_join(lval* x, lval* y) {
    // For each cell in y, add it to x
    while (y->val.children.count) {
	x = lval_add(x, lval_pop(y, 0));
    }

    // Delete the empty 'y' and return 'x'
    lval_del(y);
    return x;
}

lval* lval_join_str(lval* x, lval* y) {

    size_t x_str_len = strlen(x->val.str);
    size_t y_str_len = strlen(y->val.str);
    x->val.str = realloc(x->val.str, x_str_len + y_str_len + 1);
    // Copy y into x starting from null terminator of x
    strcpy(x->val.str + x_str_len, y->val.str); 

    lval_del(y);

    return x;
}

lval* builtin_cons(lenv* e, lval* a) {
    // Error checking
   
   LASSERT(a, a->val.children.count == 2,
	   "Function 'cons' passed incorrect number of args!");

   LASSERT(a, a->val.children.cell[1]->type == LVAL_QEXPR,
	   "Function 'cons' passed in qexpr with non qexpr as second argument!");

    // Take off overall Q-Expression
    // lval* v = lval_pop(a, 0);
    lval* result = lval_qexpr();
    // Pop off value 
    lval_add(result, lval_pop(a, 0));

    // Take out qexpr, cleaning v
    lval* q = lval_take(a, 0);

    while (q->val.children.count > 0) {
	lval_add(result, lval_pop(q, 0));
    }

    lval_del(q);

    return result;

}

lval* builtin_len(lenv* e, lval* a) {
    LASSERT(a, a->val.children.count == 1, 
	    "Function 'len' passed incorrect number of args!");

    LASSERT(a, a->val.children.cell[0]->type == LVAL_QEXPR,
	    "Function 'len' passed non qexpr argument!");

    // Could return qexpr with number in value 
    // but guess number alone works just fine
    lval* result = lval_num(a->val.children.cell[0]->val.children.count);

    lval_del(a);

    return result; 
}

lval* builtin_init(lenv* e, lval* a) {
    LASSERT(a, a->val.children.count == 1, 
	    "Function 'init' passed incorrect number of args!");

    LASSERT(a, a->val.children.cell[0]->type == LVAL_QEXPR,
	    "Function 'init' passed non Q-Expression argument!");

    // Could just return empty if count is 1 or 0 but the expectation is init of a qexpr, the init wouldnt be empty, there wouldnt be one
    LASSERT(a, a->val.children.cell[0]->val.children.count > 1,
	    "Function 'init' passed Q-Expression with incorrect number of args!");

    lval* q = lval_take(a, 0);
    lval* final = lval_pop(q, q->val.children.count - 1);

    lval_del(final);

    return q;
    
}
lval* lval_eval_sexpr(lenv* e, lval* v) {
    // Evaluate children
    for (int i = 0; i < v->val.children.count; ++i) {
	v->val.children.cell[i] = lval_eval(e, v->val.children.cell[i]);
	if (v->val.children.cell[i]->type == LVAL_ERR  ||
	    v->val.children.cell[i]->type == LVAL_EXIT ||
	    v->val.children.cell[i]->type == LVAL_RETURN) return lval_take(v,i); 
    }



    // Empty expression
    if (v->val.children.count == 0) return v;

    // Single expression
    if (v->val.children.count == 1){
	return lval_take(v, 0);
    }

    // Ensure first element is a function after eval
    lval* f = lval_pop(v, 0);
    if (f->type != LVAL_FUN) {
	lval* err = lval_err("S-Expression starts with incorrect type."
			     "Got %s, Expected %s.",
			     ltype_name(f->type), ltype_name(LVAL_FUN));
	lval_del(v); 
	lval_del(f);
	return err;
    }

    // Call function
    lval* result = lval_call(e, f, v);
    lval_del(f);
    return result;
}

lval* lval_eval(lenv* e, lval* v) {

    if (v->type == LVAL_SYM) {
	lval* x = lenv_get(e, v);
	lval_del(v);
	return x;
    }
    // Evaluate only S-Expressions, other lval types remain the same
    if (v->type == LVAL_SEXPR) return lval_eval_sexpr(e, v);

    return v;
}

lval* lval_call(lenv* e, lval* f, lval* a) {
    // If builtin then simply call the func ptr 
    if (f->val.func.builtin) return f->val.func.builtin(e, a);

    // Record argument counts
    int given = a->val.children.count;
    int total = f->val.func.formals->val.children.count;


    // While arguments still remain to be processed
    while (a->val.children.count) {
	// If we ran out of format args to bind 
	if (f->val.func.formals->val.children.count == 0) {
	    lval_del(a);
	    return lval_err("Function passed too many arguments."
			    "Got %i, Expected %i", given, total);
	}

	// Pop first symbol from formals
	lval* sym = lval_pop(f->val.func.formals, 0);

	if (strcmp(sym->val.sym, "&") == 0) {
	    // Ensure '&' is followed by another symbol

	    // TODO: Use LASSERT
	    if (f->val.func.formals->val.children.count != 1) {
		lval_del(a);
		return lval_err("Function format invalid."
				"Multi-Parameter Symbol '&' not followed by single symbol.");
	    }

	    // Next format should be bound to remaining arguments
	    lval* nsym = lval_pop(f->val.func.formals, 0);
	    lenv_put(f->val.func.env, nsym, builtin_list(e, a));
	    lval_del(sym);
	    lval_del(nsym);
	    break;
	}
	// Pop next argument from list
	lval* val = lval_pop(a, 0);

	// Bind a copy into function's environment
	lenv_put(f->val.func.env, sym, val);

	// Delete symbol and value
	lval_del(sym);
	lval_del(val);

    }

    // Argument list is now bound and can be cleaned up
    lval_del(a);

    // If '&' remains in formal list, bind to empty list
	if (f->val.func.formals->val.children.count > 0 &&
	    strcmp(f->val.func.formals->val.children.cell[0]->val.sym, "&") == 0) {
	    // Can't we just return
	    return lval_err("Invalid function arguments "
			    "Multi-Parameter symbol '&' requires "
			    "at least 1 argument. "
			    "Function got %i arguments, expected "
			    "%i or more arguments.",
			    given, given + 1); 
	}

	// If all formals have been bound, evaluate
	if (f->val.func.formals->val.children.count == 0) {
	    
	    // Set environment parent to evaluation environment
	    f->val.func.env->par = e;

	    // Eval and return
	    return builtin_eval(f->val.func.env, lval_add(lval_sexpr(), lval_copy(f->val.func.body)));
	}
	// Otherwise return partially evaluated function
	return lval_copy(f);
    }

