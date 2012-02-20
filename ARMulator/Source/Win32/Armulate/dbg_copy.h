/* functions for copying toolbox values */

#ifndef dbg_copy_h
#define dbg_copy_h

#ifdef DBT_COPY_MACROS

#define Dbg_CopyValue(res, val)         (*(res)) = (*(val)), (res)

#define Dbg_CopyLoc(res, val)           (*(res)) = (*(val)), (res)

#define Dbg_CopyAnyPos(res, val)        (*(res)) = (*(val)), (res)

#define Dbg_CopyBreakPos(res, val)      (*(res)) = (*(val)), (res)

#define Dbg_CopyConstant(res, val)      (*(res)) = (*(val)), (res)

#define Dbg_CopyConstantFromValue(res, val)   *(res->type) = *(val->type); *(res->val) = *(val->val.c), (res)


#define Dbg_CopyValueFromConstant(res, val) { \
  res->formatp = NULL; \
  res->sort = vs_constant; \
  res->type = c->type; \
  res->val.c = c->val; }

#define Dbg_CopyTypeSpec(res, val)      (*(res)) = (*(val)), (res)

#define Dbg_CopyTypeSpec(res, val)      (*(res)) = (*(val))

#else

#define PURIFY

Dbg_Value *Dbg_CopyValue(Dbg_Value *res, Dbg_Value const *val);

Dbg_Loc *Dbg_CopyLoc(Dbg_Loc *res, Dbg_Loc const *val);

Dbg_AnyPos *Dbg_CopyAnyPos(Dbg_AnyPos *res, Dbg_AnyPos const *val);

Dbg_BreakPos *Dbg_CopyBreakPos(Dbg_BreakPos *res, Dbg_BreakPos const *val);

Dbg_Constant *Dbg_CopyConstant(Dbg_Constant *res, Dbg_Constant const *val);

Dbg_Constant *Dbg_CopyConstantFromValue(Dbg_Constant *res, Dbg_Value const *val);

Dbg_Value *Dbg_CopyValueFromConstant(Dbg_Value *res, Dbg_Constant const *val);

Dbg_TypeSpec *Dbg_CopyTypeSpec(Dbg_TypeSpec *res, Dbg_TypeSpec const *val);

void Dbg_CopyConstantVal(Dbg_ConstantVal *res,
                         Dbg_ConstantVal const *val, Dbg_BasicType type);

#endif
#endif
