#ifndef YYERRCODE
#define YYERRCODE 256
#endif

#define TOK_INTEGER 257
#define TOK_DOUBLE 258
#define TOK_STRING 259
#define TOK_TRUE 260
#define TOK_FALSE 261
#define TOK_NULL 262
#define TOK_ISA 263
#define TOK_EQ 264
#define TOK_NE 265
#define TOK_LT 266
#define TOK_LE 267
#define TOK_GT 268
#define TOK_GE 269
#define TOK_NOT 270
#define TOK_OR 271
#define TOK_AND 272
#define TOK_IS 273
#define TOK_IDENTIFIER 274
#define TOK_SELECT 275
#define TOK_WHERE 276
#define TOK_FROM 277
#define TOK_UNEXPECTED_CHAR 278
typedef union 
{
   int intValue;
   double doubleValue;
   char* strValue;
   void* nodeValue;
} YYSTYPE;
extern YYSTYPE WQL_lval;
