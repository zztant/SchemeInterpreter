#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"object.h"
#include"eval.h"
#include"lex.h"
#include"print.h"
#include"gc.h"
#include"vm.h"
#include"compile.h"


void error_compile(){
	fprintf(stderr,"ERROR:: compile error,wrong scheme expressions\n");
	exit(1);
}

/* 指令也为pair,car指向参数，cdr指向下一条指令 */

object* make_code(object_type type, object* argu, object* next){
	object* obj = alloc_object();
	obj->type = type;
	obj->data.s_pair.car = argu;
	obj->data.s_pair.cdr = next;
	return obj;
}

object* compile_atom(object* atom, secd_vm* vm){
	vm->code = make_code(LDC,atom,vm->code);
	return true;
}

object* compile_symbol(object* symbol, secd_vm* vm){
	vm->code = make_code(LD,symbol,vm->code);
	return true;
}

object* compile_proc(object* exp, secd_vm* vm){
	vm->code = make_code(AP,make_null_list(),vm->code);
	compile(exp,vm);
	return true;
}

object* compile_argu(object* exp, secd_vm* vm){
	if(is_null_list(exp))
		return true;
	compile(car(exp),vm);
	compile_argu(cdr(exp),vm);
	return true;
}

char count_argu_num(object* exp){
	if(!is_null_list(exp)&&!is_pair(exp))
		error_compile();
	if(is_null_list(exp))
		return 0;
	else
		return 1 + count_argu_num(cdr(exp));
}

object* compile_pair(object* exp, secd_vm* vm){
	if(is_null_list(exp)) 
		return true;
	compile_proc(car(exp),vm);
	char c = count_argu_num(cdr(exp));
	object* argu_num = make_rational(c,1);
	vm->code = make_code(LIST,argu_num,vm->code);
	compile_argu(cdr(exp),vm);
	return true;
}

object* make_lambda(object* argu, object* exp){
	return cons(make_symbol("lambda"),
			    cons(argu,
					 exp));
}

object* make_define(object* argu1, object* argu2){
	return cons(make_symbol("define"),
			    cons(argu1,
					 cons(argu2,make_null_list())));
}

object* compile_def(object* exp, secd_vm* vm){
	object* argu1 = cadr(exp);
	object* argu2 = cddr(exp);
	if(is_pair(argu1)){
		object* exp = argu2;
		object* arg1 = car(argu1);
		object* arg2 = cdr(argu1);
		exp = make_define(arg1,make_lambda(arg2,exp));
		compile_def(exp,vm);
	}
	else{
		vm->code = make_code(DEFINE,argu1,vm->code);

		object* exp = cons(make_symbol("begin"),
				           argu2);
		compile(exp,vm);
	}
	return true;
}

object* compile_quoted(object* exp, secd_vm* vm){
	vm->code = make_code(LDC,cadr(exp),vm->code);
	return true;
}

object* compile_set(object* exp, secd_vm* vm){
	object* argu1 = cadr(exp);
	object* argu2 = caddr(exp);
	vm->code = make_code(SET,argu1,vm->code);
	compile(argu2,vm);
	return true;
}

object* compile_ldf(object* param, object* body, secd_vm* vm){
	secd_vm tmp;
	tmp.code = make_code(RET,make_null_list(),make_null_list());
	compile(body,&tmp);
	vm->code = make_code(LDF,cons(param,tmp.code),vm->code);
	return true;
}
//LDF指令的car为形参列表，cdr指向第一条code

object* compile_lambda(object* exp, secd_vm* vm){
	object* body = cddr(exp);
	object* param = cadr(exp);
	object* error_test = param;
	while(!is_null_list(error_test)){
		if(!is_symbol(car(error_test)))
			error_type("lambda");
		error_test = cdr(error_test);
	}
	body = cons(make_symbol("begin"),body);
	compile_ldf(param,body,vm);
	return true;
}

object* compile_let(object* exp, secd_vm* vm){
	object* bind_list = cadr(exp);
	object* var_list = make_null_list();
	object* val_list = make_null_list();
	exp = cddr(exp);
	while(!is_null_list(bind_list)){
		var_list = cons(caar(bind_list),var_list);
		val_list = cons(cadar(bind_list),val_list);
		bind_list = cdr(bind_list);
	}
	exp = cons(cons(make_symbol("lambda"),cons(var_list,exp)),val_list);
	compile(exp,vm);
}

object* compile_letx(object* exp,secd_vm* vm){
	object* bind_list = cadr(exp);
	object* var_list = make_null_list();
	object* val_list = make_null_list();
	exp = caddr(exp);
	while(!is_null_list(bind_list)){
		var_list = cons(caar(bind_list),var_list);
		val_list = cons(cadar(bind_list),val_list);
		bind_list = cdr(bind_list);
	}
	while(!is_null_list(var_list)){
		exp = cons( cons(make_symbol("lambda"),cons(cons(car(var_list),make_null_list()),cons(exp,make_null_list()))),cons(car(val_list),make_null_list()));
		var_list = cdr(var_list);
		val_list = cdr(val_list);
	}
	compile(exp,vm);
}


object* compile_if(object* exp, secd_vm* vm){
	secd_vm tmp1,tmp2;
	tmp1.code = make_code(JOIN,make_null_list(),make_null_list());
	tmp2.code = make_code(JOIN,make_null_list(),make_null_list());
	object* cond = cadr(exp);
	object* yes = caddr(exp);
	object* no = cadddr(exp);
	compile(yes,&tmp1);
	compile(no,&tmp2);
	vm->code = make_code(SEL,cons(tmp1.code,tmp2.code),vm->code);
	compile(cond,vm);
	return true;
}

object* compile_begin(object* exp, secd_vm* vm){
	if(is_null_list(exp))
		return true;
	compile_begin(cdr(exp),vm);
	compile(car(exp),vm);
}

object* compile_callcc(object* exp, secd_vm* vm){
	vm->code = make_code(CALLCC,make_null_list(),vm->code);
	compile_argu(cdr(exp),vm);
}

//SEL指令的两个指针分别指向两个分支的code

object* compile(object* exp, secd_vm* vm){
	if( is_atom(exp) )
		return compile_atom(exp,vm);
	else if(is_definition(exp))
		return compile_def(exp,vm);
	else if(is_quoted(exp))
		return compile_quoted(exp,vm);
	else if(is_callcc(exp))
		return compile_callcc(exp,vm);
	else if(is_set(exp))
		return compile_set(exp,vm);
	else if(is_lambda(exp))
		return compile_lambda(exp,vm);
	else if(is_if(exp))
		return compile_if(exp,vm);
	else if(is_symbol(exp))
		return compile_symbol(exp,vm);
	else if(is_begin(exp))
		return compile_begin(cdr(exp),vm);
	else if(is_let(exp))
		return compile_let(exp,vm);
	else if(is_letx(exp))
		return compile_letx(exp,vm);
	else if(is_eval(exp)){
		exp = cadadr(exp);
		return compile(exp,vm);
	}
	else if(is_pair(exp))
		return compile_pair(exp,vm);
	else
		error_compile();
}

void compile_file(FILE* file, secd_vm* vm){
	object* stmt;
	if( (stmt = read_exp(file))!=NULL){
		compile_file(file,vm);
		compile(stmt,vm);
	}
	return;
}
void print_code_sel(object* code){
	printf("(");
	print_code(caar(code));
	printf("|");
	print_code(cdar(code));
	printf(")");
}

void print_code_ldf(object* code){
	printf("(");
	print_code(cdar(code));
	printf(")");
}

void print_code(object* code){
	   if(!is_null_list(code)){
		switch(code->type){
			case NIL: printf("NIL->"); break;
			case LDC: printf("LDC->,"); break;
			case LD: printf("LD->,"); break;

			case SEL: printf("SEL->,"); print_code_sel(code); break;

			case JOIN: printf("JOIN->,"); break;
			case AP: printf("AP->,"); break;
			case DUM: printf("DUM->,"); break;
			case RAP: printf("RAP->,"); break;

			case LDF: printf("LDF->,"); print_code_ldf(code); break;
			case CALLCC: printf("CALLCC->,"); break;
			case RET: printf("RET->,"); break;
			case DEFINE: printf("DEFINE->,"); break;
			case LIST: printf("LIST->,"); break;
			case SET: printf("SET->,"); break;
		}
		print_code(cdr(code));
	}
}
