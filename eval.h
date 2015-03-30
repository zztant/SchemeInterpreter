object* prim_proc_plus(object* arguments);
object* prim_proc_minus(object* arguments);
object* prim_proc_mul(object* arguments);
object* prim_proc_div(object* arguments);
object* prim_proc_define(object* arguments,object* env);
object* prim_proc_set(object* arguments, object* env);
object* prim_proc_and(object* argu);
object* prim_proc_or(object* argu);
object* prim_proc_equal(object* argu);
object* prim_proc_lower(object* argu);
object* prim_proc_greater(object* argu);
object* prim_proc_cons(object* arguments);
object* prim_proc_is_null(object* exp);
object* prim_proc_is_proc(object* exp);
object* prim_proc_is_pair(object* exp);
object* prim_proc_is_string(object* exp);
object* prim_proc_is_boolean(object* exp);
object* prim_proc_is_symbol(object* exp);
object* prim_proc_eq(object* exp);
object* prim_proc_car(object* exp);
object* prim_proc_cdr(object* exp);
object* prim_proc_apply(object* exp);
object* prim_proc_eval();
char is_definition(object* exp);
char is_quoted(object* exp);
char is_set(object* exp);
char is_lambda(object* exp);
char is_if(object* exp);
object* apply(object* proc, object* arguments);
