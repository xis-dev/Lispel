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
	
// Temporary row and column for now, thinking of storing in env but unsure
// or env holds some temporary reader that gets pos from ast parser
// would need to know when eval is completely finished though
#define LEXIT(args) \
    lval* exit = lval_exit("Prompt exited successfully", \
	    0, 0); \
    lval_del(args); \
	return exit; 

