/* Declarations for objc-act.c.
   Copyright (C) 1990, 2000, 2001 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifndef GCC_OBJC_ACT_H
#define GCC_OBJC_ACT_H

/*** Public Interface (procedures) ***/

const char *objc_init				PARAMS ((const char *));
const char *objc_printable_name			PARAMS ((tree, int));

/* used by yyparse */

/* APPLE LOCAL Objective-C++ */
void objc_finish_file				PARAMS ((void));
tree start_class				PARAMS ((enum tree_code, tree, tree, tree));
tree continue_class				PARAMS ((tree));
void finish_class				PARAMS ((tree));
void start_method_def				PARAMS ((tree));
void continue_method_def			PARAMS ((void));
void finish_method_def				PARAMS ((void));
tree start_protocol				PARAMS ((enum tree_code, tree, tree));
void finish_protocol				PARAMS ((tree));
/* APPLE LOCAL msg send super  */
/* The 'add_objc_decls' routine is no more.  */

/* APPLE LOCAL begin Panther ObjC enhancements */
tree objc_build_throw_stmt			PARAMS ((tree));
tree objc_build_try_catch_finally_stmt		PARAMS ((int, int));
void objc_build_synchronized_prologue		PARAMS ((tree));
tree objc_build_synchronized_epilogue		PARAMS ((void));
tree objc_build_try_prologue			PARAMS ((void));
void objc_build_try_epilogue			PARAMS ((int));
void objc_build_catch_stmt			PARAMS ((tree));
void objc_build_catch_epilogue			PARAMS ((void));
tree objc_build_finally_prologue		PARAMS ((void));
tree objc_build_finally_epilogue		PARAMS ((void));
/* APPLE LOCAL end Panther ObjC enhancements */

/* APPLE LOCAL begin XJR */
int objc_is_object_ptr_type			PARAMS ((tree, int));
/* APPLE LOCAL end XJR */

tree is_ivar					PARAMS ((tree, tree));
int is_private					PARAMS ((tree));
int is_public					PARAMS ((tree, tree));
tree add_instance_variable			PARAMS ((tree, int, tree, tree, tree, tree));
tree add_class_method				PARAMS ((tree, tree));
tree add_instance_method			PARAMS ((tree, tree));
tree get_super_receiver				PARAMS ((void));
/* APPLE LOCAL msg send super */
void objc_clear_super_receiver			PARAMS ((void));
/* APPLE LOCAL Objective-C++ */
tree get_class_ivars_from_name			PARAMS ((tree));
/* APPLE LOCAL bitfields */
/* 'get_class_ivars' has been made static.  */
tree get_class_reference			PARAMS ((tree));
tree get_static_reference			PARAMS ((tree, tree));
tree get_object_reference			PARAMS ((tree));
tree build_message_expr				PARAMS ((tree));
tree finish_message_expr			PARAMS ((tree, tree, tree));
tree build_selector_expr			PARAMS ((tree));
tree build_ivar_reference			PARAMS ((tree));
tree build_keyword_decl				PARAMS ((tree, tree, tree));
tree build_method_decl				PARAMS ((enum tree_code, tree, tree, tree));
tree build_protocol_expr			PARAMS ((tree));
tree build_objc_string_object			PARAMS ((tree));

void objc_declare_alias				PARAMS ((tree, tree));
void objc_declare_class				PARAMS ((tree));
void objc_declare_protocols			PARAMS ((tree));

/* the following routines are used to implement statically typed objects */

int objc_comptypes				PARAMS ((tree, tree, int));
void objc_check_decl				PARAMS ((tree));

/* NeXT extensions */

tree build_encode_expr				PARAMS ((tree));

/* Objective-C structures */

/* KEYWORD_DECL */
#define KEYWORD_KEY_NAME(DECL) ((DECL)->decl.name)
#define KEYWORD_ARG_NAME(DECL) ((DECL)->decl.arguments)

/* INSTANCE_METHOD_DECL, CLASS_METHOD_DECL */
#define METHOD_SEL_NAME(DECL) ((DECL)->decl.name)
#define METHOD_SEL_ARGS(DECL) ((DECL)->decl.arguments)
#define METHOD_ADD_ARGS(DECL) ((DECL)->decl.result)
#define METHOD_DEFINITION(DECL) ((DECL)->decl.initial)
#define METHOD_ENCODING(DECL) ((DECL)->decl.context)

/* CLASS_INTERFACE_TYPE, CLASS_IMPLEMENTATION_TYPE,
   CATEGORY_INTERFACE_TYPE, CATEGORY_IMPLEMENTATION_TYPE,
   PROTOCOL_INTERFACE_TYPE */
#define CLASS_NAME(CLASS) ((CLASS)->type.name)
#define CLASS_SUPER_NAME(CLASS) ((CLASS)->type.context)
#define CLASS_IVARS(CLASS) TREE_VEC_ELT (TYPE_BINFO (CLASS), 0)
#define CLASS_RAW_IVARS(CLASS) TREE_VEC_ELT (TYPE_BINFO (CLASS), 1)
#define CLASS_NST_METHODS(CLASS) ((CLASS)->type.minval)
#define CLASS_CLS_METHODS(CLASS) ((CLASS)->type.maxval)
#define CLASS_STATIC_TEMPLATE(CLASS) TREE_VEC_ELT (TYPE_BINFO (CLASS), 2)
#define CLASS_CATEGORY_LIST(CLASS) TREE_VEC_ELT (TYPE_BINFO (CLASS), 3)
#define CLASS_PROTOCOL_LIST(CLASS) TREE_VEC_ELT (TYPE_BINFO (CLASS), 4)
#define CLASS_OWN_IVARS(CLASS) TREE_VEC_ELT (TYPE_BINFO (CLASS), 5)
#define PROTOCOL_NAME(CLASS) ((CLASS)->type.name)
#define PROTOCOL_LIST(CLASS) TREE_VEC_ELT (TYPE_BINFO (CLASS), 0)
#define PROTOCOL_NST_METHODS(CLASS) ((CLASS)->type.minval)
#define PROTOCOL_CLS_METHODS(CLASS) ((CLASS)->type.maxval)
#define PROTOCOL_FORWARD_DECL(CLASS) TREE_VEC_ELT (TYPE_BINFO (CLASS), 1)
#define PROTOCOL_DEFINED(CLASS) TREE_USED (CLASS)
#define TYPE_PROTOCOL_LIST(TYPE) ((TYPE)->type.context)

/* APPLE LOCAL objc speedup dpatel */
#define IDENTIFIER_INTERFACE_VALUE(NODE) (((struct lang_identifier *) (NODE))->interface_value)

/* Set by `continue_class' and checked by `is_public'.  */

#define TREE_STATIC_TEMPLATE(record_type) (TREE_PUBLIC (record_type))
#define TYPED_OBJECT(type) \
       (TREE_CODE (type) == RECORD_TYPE && TREE_STATIC_TEMPLATE (type))
/* APPLE LOCAL type aliasing */
#define OBJC_TYPE_NAME(type) TYPE_NAME(type)

/* Define the Objective-C or Objective-C++ language-specific tree codes.  */

#define DEFTREECODE(SYM, NAME, TYPE, LENGTH) SYM,
enum objc_tree_code {
/* APPLE LOCAL begin Objective C++ */
#if defined (GCC_CP_TREE_H)
  LAST_BASE_TREE_CODE = LAST_CPLUS_TREE_CODE,
#elif defined (GCC_C_TREE_H)
  LAST_BASE_TREE_CODE = LAST_C_TREE_CODE,
#else
#error You must include <c-tree.h> or <cp/cp-tree.h> before <objc/objc-act.h>
/* APPLE LOCAL end Objective-C++ */
#endif
#include "objc-tree.def"
  LAST_OBJC_TREE_CODE
};
#undef DEFTREECODE

/* Hash tables to manage the global pool of method prototypes.  */

typedef struct hashed_entry	*hash;
typedef struct hashed_attribute	*attr;

struct hashed_attribute GTY(())
{
  attr next;
  tree value;
};
struct hashed_entry GTY(())
{
  attr list;
  hash next;
  tree key;
};

extern GTY ((length ("SIZEHASHTABLE"))) hash *nst_method_hash_list;
extern GTY ((length ("SIZEHASHTABLE"))) hash *cls_method_hash_list;

#define SIZEHASHTABLE 		257

/* Objective-C/Objective-C++ @implementation list.  */

struct imp_entry GTY(())
{
  struct imp_entry *next;
  tree imp_context;
  tree imp_template;
  tree class_decl;		/* _OBJC_CLASS_<my_name>; */
  tree meta_decl;		/* _OBJC_METACLASS_<my_name>; */
};

extern GTY(()) struct imp_entry *imp_list;
extern int imp_count;	/* `@implementation' */
extern int cat_count;	/* `@category' */

/* Objective-C/Objective-C++ global tree enumeration.  */

enum objc_tree_index
{
    OCTI_STATIC_NST,
    OCTI_STATIC_NST_DECL,
    OCTI_SELF_ID,
    OCTI_UCMD_ID,
    OCTI_UNUSED_LIST,
    OCTI_ELLIPSIS_NODE,

    OCTI_SELF_DECL,
    OCTI_UMSG_DECL,
    /* APPLE LOCAL XJR */
    OCTI_UMSG_FAST_DECL,
    OCTI_UMSG_SUPER_DECL,
    /* APPLE LOCAL begin objc stret methods */
    OCTI_UMSG_STRET_DECL,
    OCTI_UMSG_SUPER_STRET_DECL,
    /* APPLE LOCAL end objc stret methods */
    OCTI_GET_CLASS_DECL,
    OCTI_GET_MCLASS_DECL,
    OCTI_SUPER_TYPE,
    OCTI_SEL_TYPE,
    OCTI_ID_TYPE,
    OCTI_CLS_TYPE,
    OCTI_NST_TYPE,
    OCTI_PROTO_TYPE,

    OCTI_CLS_CHAIN,
    OCTI_ALIAS_CHAIN,
    OCTI_INTF_CHAIN,
    OCTI_PROTO_CHAIN,
    OCTI_IMPL_CHAIN,
    OCTI_CLS_REF_CHAIN,
    OCTI_SEL_REF_CHAIN,
    OCTI_IVAR_CHAIN,
    OCTI_CLS_NAMES_CHAIN,
    OCTI_METH_VAR_NAMES_CHAIN,
    OCTI_METH_VAR_TYPES_CHAIN,

    OCTI_SYMBOLS_DECL,
    OCTI_NST_VAR_DECL,
    OCTI_CLS_VAR_DECL,
    OCTI_NST_METH_DECL,
    OCTI_CLS_METH_DECL,
    OCTI_CLS_DECL,
    OCTI_MCLS_DECL,
    OCTI_SEL_TABLE_DECL,
    OCTI_MODULES_DECL,
    OCTI_STRG_DECL,

    OCTI_INTF_CTX,
    OCTI_IMPL_CTX,
    OCTI_METH_CTX,
    OCTI_IVAR_CTX,

    OCTI_IMPL_TEMPL,
    OCTI_CLS_TEMPL,
    OCTI_CAT_TEMPL,
    OCTI_UPRIV_REC,
    OCTI_PROTO_TEMPL,
    OCTI_SEL_TEMPL,
    OCTI_UCLS_SUPER_REF,
    OCTI_UUCLS_SUPER_REF,
    OCTI_METH_TEMPL,
    OCTI_IVAR_TEMPL,
    OCTI_SYMTAB_TEMPL,
    OCTI_MODULE_TEMPL,
    OCTI_SUPER_TEMPL,
    OCTI_OBJ_REF,
    OCTI_METH_PROTO_TEMPL,
    OCTI_FUNCTION1_TEMPL,
    OCTI_FUNCTION2_TEMPL,

    OCTI_OBJ_ID,
    OCTI_CLS_ID,
    OCTI_ID_ID,
    OCTI_CNST_STR_ID,
    OCTI_CNST_STR_TYPE,
    OCTI_CNST_STR_GLOB_ID,
    OCTI_STRING_CLASS_DECL,
    OCTI_SUPER_DECL,
    /* APPLE LOCAL begin Panther ObjC enhancements */
    OCTI_UMSG_NONNIL_DECL,
    OCTI_UMSG_NONNIL_STRET_DECL,
    OCTI_STORAGE_CLS,
    OCTI_EXCEPTION_EXTRACT_DECL,
    OCTI_EXCEPTION_TRY_ENTER_DECL,
    OCTI_EXCEPTION_TRY_EXIT_DECL,
    OCTI_EXCEPTION_MATCH_DECL,
    OCTI_EXCEPTION_THROW_DECL,
    OCTI_SYNC_ENTER_DECL,
    OCTI_SYNC_EXIT_DECL,
    OCTI_SETJMP_DECL,
    OCTI_EXCDATA_TEMPL,
    OCTI_STACK_EXCEPTION_DATA_DECL,
    OCTI_LOCAL_EXCEPTION_DECL,
    OCTI_RETHROW_EXCEPTION_DECL,
    OCTI_EVAL_ONCE_DECL,
    OCTI_EXCEPTION_BLK_STACK,
    OCTI_CATCH_TYPE,
    /* APPLE LOCAL end Panther ObjC enhancements */

    /* APPLE LOCAL begin XJR */
    OCTI_ASSIGN_IVAR_DECL,
    OCTI_ASSIGN_IVAR_FAST_DECL,
    OCTI_ASSIGN_GLOBAL_DECL,
    OCTI_ASSIGN_STRONGCAST_DECL,
    /* APPLE LOCAL end XJR */

    OCTI_MAX
};

extern GTY(()) tree objc_global_trees[OCTI_MAX];

/* List of classes with list of their static instances.  */
#define objc_static_instances	objc_global_trees[OCTI_STATIC_NST]

/* The declaration of the array administrating the static instances.  */
#define static_instances_decl	objc_global_trees[OCTI_STATIC_NST_DECL]

/* Some commonly used instances of "identifier_node".  */

#define self_id			objc_global_trees[OCTI_SELF_ID]
#define ucmd_id			objc_global_trees[OCTI_UCMD_ID]
#define unused_list		objc_global_trees[OCTI_UNUSED_LIST]
#define objc_ellipsis_node	objc_global_trees[OCTI_ELLIPSIS_NODE]

#define self_decl		objc_global_trees[OCTI_SELF_DECL]
#define umsg_decl		objc_global_trees[OCTI_UMSG_DECL]
/* APPLE LOCAL XJR */
#define umsg_fast_decl		objc_global_trees[OCTI_UMSG_FAST_DECL]
#define umsg_super_decl		objc_global_trees[OCTI_UMSG_SUPER_DECL]
/* APPLE LOCAL begin objc stret methods */
#define umsg_stret_decl		objc_global_trees[OCTI_UMSG_STRET_DECL]
#define umsg_super_stret_decl	objc_global_trees[OCTI_UMSG_SUPER_STRET_DECL]
/* APPLE LOCAL end objc stret methods */
#define objc_get_class_decl	objc_global_trees[OCTI_GET_CLASS_DECL]
#define objc_get_meta_class_decl			\
				objc_global_trees[OCTI_GET_MCLASS_DECL]

#define super_type		objc_global_trees[OCTI_SUPER_TYPE]
#define selector_type		objc_global_trees[OCTI_SEL_TYPE]
#define id_type			objc_global_trees[OCTI_ID_TYPE]
#define objc_class_type		objc_global_trees[OCTI_CLS_TYPE]
#define instance_type		objc_global_trees[OCTI_NST_TYPE]
#define protocol_type		objc_global_trees[OCTI_PROTO_TYPE]

/* Type checking macros.  */

/* APPLE LOCAL begin XJR */
#define IS_ID(TYPE) \
  (POINTER_TYPE_P (TYPE) && TREE_TYPE (TYPE) == TREE_TYPE (id_type))
#define IS_CLASS(TYPE) \
  (POINTER_TYPE_P (TYPE) && TREE_TYPE (TYPE) == TREE_TYPE (objc_class_type))
#define IS_PROTOCOL_QUALIFIED_ID(TYPE) \
  (IS_ID (TYPE) && TYPE_PROTOCOL_LIST (TYPE))
#define IS_SUPER(TYPE) \
  (POINTER_TYPE_P (TYPE) && TREE_TYPE (TYPE) == objc_super_template)
/* APPLE LOCAL end XJR */

#define class_chain		objc_global_trees[OCTI_CLS_CHAIN]
#define alias_chain		objc_global_trees[OCTI_ALIAS_CHAIN]
#define interface_chain		objc_global_trees[OCTI_INTF_CHAIN]
#define protocol_chain		objc_global_trees[OCTI_PROTO_CHAIN]
#define implemented_classes	objc_global_trees[OCTI_IMPL_CHAIN]

/* Chains to manage selectors that are referenced and defined in the
   module.  */

#define cls_ref_chain		objc_global_trees[OCTI_CLS_REF_CHAIN]	/* Classes referenced.  */
#define sel_ref_chain		objc_global_trees[OCTI_SEL_REF_CHAIN]	/* Selectors referenced.  */
#define objc_ivar_chain		objc_global_trees[OCTI_IVAR_CHAIN]

/* Chains to manage uniquing of strings.  */

#define class_names_chain	objc_global_trees[OCTI_CLS_NAMES_CHAIN]
#define meth_var_names_chain	objc_global_trees[OCTI_METH_VAR_NAMES_CHAIN]
#define meth_var_types_chain	objc_global_trees[OCTI_METH_VAR_TYPES_CHAIN]


/* Backend data declarations.  */

#define UOBJC_SYMBOLS_decl		objc_global_trees[OCTI_SYMBOLS_DECL]
#define UOBJC_INSTANCE_VARIABLES_decl	objc_global_trees[OCTI_NST_VAR_DECL]
#define UOBJC_CLASS_VARIABLES_decl	objc_global_trees[OCTI_CLS_VAR_DECL]
#define UOBJC_INSTANCE_METHODS_decl	objc_global_trees[OCTI_NST_METH_DECL]
#define UOBJC_CLASS_METHODS_decl	objc_global_trees[OCTI_CLS_METH_DECL]
#define UOBJC_CLASS_decl		objc_global_trees[OCTI_CLS_DECL]
#define UOBJC_METACLASS_decl		objc_global_trees[OCTI_MCLS_DECL]
#define UOBJC_SELECTOR_TABLE_decl	objc_global_trees[OCTI_SEL_TABLE_DECL]
#define UOBJC_MODULES_decl		objc_global_trees[OCTI_MODULES_DECL]
#define UOBJC_STRINGS_decl		objc_global_trees[OCTI_STRG_DECL]

/* The following are used when compiling a class implementation.
   implementation_template will normally be an interface, however if
   none exists this will be equal to objc_implementation_context...it is
   set in start_class.  */

#define objc_interface_context		objc_global_trees[OCTI_INTF_CTX]
#define objc_implementation_context	objc_global_trees[OCTI_IMPL_CTX]
#define objc_method_context		objc_global_trees[OCTI_METH_CTX]
#define objc_ivar_context		objc_global_trees[OCTI_IVAR_CTX]

#define implementation_template	objc_global_trees[OCTI_IMPL_TEMPL]
#define objc_class_template	objc_global_trees[OCTI_CLS_TEMPL]
#define objc_category_template	objc_global_trees[OCTI_CAT_TEMPL]
#define uprivate_record		objc_global_trees[OCTI_UPRIV_REC]
#define objc_protocol_template	objc_global_trees[OCTI_PROTO_TEMPL]
#define objc_selector_template	objc_global_trees[OCTI_SEL_TEMPL]
#define ucls_super_ref		objc_global_trees[OCTI_UCLS_SUPER_REF]
#define uucls_super_ref		objc_global_trees[OCTI_UUCLS_SUPER_REF]

/* APPLE LOCAL begin Panther ObjC enhancements */
#define umsg_nonnil_decl	objc_global_trees[OCTI_UMSG_NONNIL_DECL]
#define umsg_nonnil_stret_decl	objc_global_trees[OCTI_UMSG_NONNIL_STRET_DECL]
#define objc_storage_class	objc_global_trees[OCTI_STORAGE_CLS]
#define objc_exception_extract_decl		\
				objc_global_trees[OCTI_EXCEPTION_EXTRACT_DECL]
#define objc_exception_try_enter_decl		\
				objc_global_trees[OCTI_EXCEPTION_TRY_ENTER_DECL]
#define objc_exception_try_exit_decl		\
				objc_global_trees[OCTI_EXCEPTION_TRY_EXIT_DECL]
#define objc_exception_match_decl		\
				objc_global_trees[OCTI_EXCEPTION_MATCH_DECL]
#define objc_exception_throw_decl		\
				objc_global_trees[OCTI_EXCEPTION_THROW_DECL]
#define objc_sync_enter_decl	objc_global_trees[OCTI_SYNC_ENTER_DECL]
#define objc_sync_exit_decl	objc_global_trees[OCTI_SYNC_EXIT_DECL]
#define objc_exception_data_template		\
				objc_global_trees[OCTI_EXCDATA_TEMPL]
#define objc_setjmp_decl	objc_global_trees[OCTI_SETJMP_DECL]
#define objc_stack_exception_data		\
				objc_global_trees[OCTI_STACK_EXCEPTION_DATA_DECL]
#define objc_caught_exception	objc_global_trees[OCTI_LOCAL_EXCEPTION_DECL]	
#define objc_rethrow_exception	objc_global_trees[OCTI_RETHROW_EXCEPTION_DECL]	
#define objc_eval_once		objc_global_trees[OCTI_EVAL_ONCE_DECL]	
#define objc_exception_block_stack		\
				objc_global_trees[OCTI_EXCEPTION_BLK_STACK]
#define objc_catch_type		objc_global_trees[OCTI_CATCH_TYPE]
/* APPLE LOCAL end Panther ObjC enhancements */

/* APPLE LOCAL begin XJR */
#define objc_assign_ivar_decl	objc_global_trees[OCTI_ASSIGN_IVAR_DECL]
#define objc_assign_ivar_fast_decl		\
				objc_global_trees[OCTI_ASSIGN_IVAR_FAST_DECL]
#define objc_assign_global_decl	objc_global_trees[OCTI_ASSIGN_GLOBAL_DECL]
#define objc_assign_strong_cast_decl		\
				objc_global_trees[OCTI_ASSIGN_STRONGCAST_DECL]
/* APPLE LOCAL end XJR */

#define objc_method_template	objc_global_trees[OCTI_METH_TEMPL]
#define objc_ivar_template	objc_global_trees[OCTI_IVAR_TEMPL]
#define objc_symtab_template	objc_global_trees[OCTI_SYMTAB_TEMPL]
#define objc_module_template	objc_global_trees[OCTI_MODULE_TEMPL]
#define objc_super_template	objc_global_trees[OCTI_SUPER_TEMPL]
#define objc_object_reference	objc_global_trees[OCTI_OBJ_REF]
#define objc_method_prototype_template		\
				objc_global_trees[OCTI_METH_PROTO_TEMPL]
#define function1_template	objc_global_trees[OCTI_FUNCTION1_TEMPL]
#define function2_template	objc_global_trees[OCTI_FUNCTION2_TEMPL]
				
#define objc_object_id		objc_global_trees[OCTI_OBJ_ID]
#define objc_class_id		objc_global_trees[OCTI_CLS_ID]
#define objc_id_id		objc_global_trees[OCTI_ID_ID]
#define constant_string_id	objc_global_trees[OCTI_CNST_STR_ID]
#define constant_string_type	objc_global_trees[OCTI_CNST_STR_TYPE]
#define constant_string_global_id		\
				objc_global_trees[OCTI_CNST_STR_GLOB_ID]
#define string_class_decl	objc_global_trees[OCTI_STRING_CLASS_DECL]
#define UOBJC_SUPER_decl	objc_global_trees[OCTI_SUPER_DECL]

#endif /* GCC_OBJC_ACT_H */
