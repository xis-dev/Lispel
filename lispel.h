#ifndef LISPEL_H
#define LISPEL_H 

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#define LASSERT(args, cond, fmt, ...) \
    if (!(cond)) { \
	lval* err = lval_err(fmt, ##__VA_ARGS__); \
	lval_del(args); \
	return err; \
    }

// Could change to lassert_types but then need array of types
#define LASSERT_TYPE(func_name, args, idx_arg, expected) \
    if(args->val.children.cell[idx_arg]->type != expected) { \
	lval* err = lval_err("Function '%s' passed incorrect type for \
			      argument '%i'. Got %s, Expected %s", \
			      func_name, idx_arg, \
			      ltype_name(args->val.children.cell[idx_arg]->type), ltype_name(expected)); \
	lval_del(args); \
	return err; \
    }

#define LASSERT_TYPES(func_name, args, idx_arg, type_count, ...) \
    if (!lval_assert_types(args->val.children.cell[idx_arg], type_count, ##__VA_ARGS__)) { \
	lval* err = lval_assert_types_err(func_name, args->val.children.cell[idx_arg], type_count, ##__VA_ARGS__); \
	lval_del(args); \
	return err; \
    }

#define LASSERT_NUM(func, args, num) \
  LASSERT(args, args->val.children.count == num, \
    "Function '%s' passed incorrect number of arguments. " \
    "Got %i, Expected %i.", \
    func, args->val.children.count, num)

#define LASSERT_NOT_EMPTY(func, args, index) \
  LASSERT(args, args->val.children.cell[index]->count != 0, \
    "Function '%s' passed {} for argument %i.", func, index);
	
#define LEXIT(args) \
    lval* exit = lval_exit("Prompt exited successfully", \
	    0, 0); \
    lval_del(args); \
	return exit; 

struct lval;
typedef struct lval lval;

struct fn_lval;
typedef struct fn_lval fn_lval;

union u_lval;
typedef union u_lval u_lval;

struct lenv;
typedef struct lenv lenv;

typedef lval*(*lbuiltin)(lenv*, lval*);

// Enum for possible lisp value types
typedef enum {LVAL_ERR, LVAL_EXIT, LVAL_RETURN, LVAL_NUM, LVAL_SYM, LVAL_FUN, LVAL_STR, LVAL_COND, LVAL_SEXPR, LVAL_QEXPR} lval_type;

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

void lreader_init(lreader* r);

void lreader_nextchar(lreader* r);

void lreader_newline(lreader* r);

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
struct lval{ 
    int type;

    u_lval val;
};

lenv* lenv_new(void);
void  lenv_del(lenv* e);
lenv* lenv_copy(lenv* e);

char* ltype_name(int t);

lval* lval_read(char* s, lreader* rd);
lval* lval_read_expr(char* s, lreader* rd, char end);
lval* lval_read_sym(char* s, lreader* rd);
lval* lval_read_str(char* s, lreader* rd);

char lval_str_unescape(char x);
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
void lval_println(lenv* e, lval* v);
void lval_print_str(lval* v);
void lval_print_cond(uint8_t v);


bool lval_assert_types(lval* v, size_t type_count, ...);

lval* lval_assert_types_err(const char* func_name, lval* v, size_t count, ...);

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

#endif
