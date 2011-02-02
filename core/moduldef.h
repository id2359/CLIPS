   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*             CLIPS Version 6.24  06/05/06            */
   /*                                                     */
   /*                DEFMODULE HEADER FILE                */
   /*******************************************************/

/*************************************************************/
/* Purpose: Defines basic defmodule primitive functions such */
/*   as allocating and deallocating, traversing, and finding */
/*   defmodule data structures.                              */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Revision History:                                         */
/*      6.23: Corrected compilation errors for files         */
/*            generated by constructs-to-c. DR0861           */
/*                                                           */
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*************************************************************/

#ifndef _H_moduldef
#define _H_moduldef

struct defmodule;
struct portItem;
struct defmoduleItemHeader;
struct moduleItem;

#ifndef _STDIO_INCLUDED_
#include <stdio.h>
#define _STDIO_INCLUDED_
#endif

#ifndef _H_conscomp
#include "conscomp.h"
#endif
#ifndef _H_modulpsr
#include "modulpsr.h"
#endif
#ifndef _H_utility
#include "utility.h"
#endif
#ifndef _H_symbol
#include "symbol.h"
#endif
#ifndef _H_evaluatn
#include "evaluatn.h"
#endif
#ifndef _H_constrct
#include "constrct.h"
#endif

/**********************************************************************/
/* defmodule                                                          */
/* ----------                                                         */
/* name: The name of the defmodule (stored as a reference in the      */
/*   table).                                                          */
/*                                                                    */
/* ppForm: The pretty print representation of the defmodule (used by  */
/*   the save and ppdefmodule commands).                              */
/*                                                                    */
/* itemsArray: An array of pointers to the module specific data used  */
/*   by each construct specified with the RegisterModuleItem          */
/*   function. The data pointer stored in the array is allocated by   */
/*   the allocateFunction in moduleItem data structure.               */
/*                                                                    */
/* importList: The list of items which are being imported by this     */
/*   module from other modules.                                       */
/*                                                                    */
/* next: A pointer to the next defmodule data structure.              */
/**********************************************************************/
struct defmodule
  {
   struct symbolHashNode *name;
   char *ppForm;
   struct defmoduleItemHeader **itemsArray;
   struct portItem *importList;
   struct portItem *exportList;
   unsigned visitedFlag;
   long bsaveID;
   struct userData *usrData;
   struct defmodule *next;
  };

struct portItem
  {
   struct symbolHashNode *moduleName;
   struct symbolHashNode *constructType;
   struct symbolHashNode *constructName;
   struct portItem *next;
  };

struct defmoduleItemHeader
  {
   struct defmodule *theModule;
   struct constructHeader *firstItem;
   struct constructHeader *lastItem;
  };

#define MIHS (struct defmoduleItemHeader *)

/**********************************************************************/
/* moduleItem                                                         */
/* ----------                                                         */
/* name: The name of the construct which can be placed in a module.   */
/*   For example, "defrule".                                          */
/*                                                                    */
/* allocateFunction: Used to allocate a data structure containing all */
/*   pertinent information related to a specific construct for a      */
/*   given module. For example, the deffacts construct stores a       */
/*   pointer to the first and last deffacts for each each module.     */
/*                                                                    */
/* freeFunction: Used to deallocate a data structure allocated by     */
/*   the allocateFunction. In addition, the freeFunction deletes      */
/*   all constructs of the specified type in the given module.        */
/*                                                                    */
/* bloadModuleReference: Used during a binary load to establish a     */
/*   link between the defmodule data structure and the data structure */
/*   containing all pertinent module information for a specific       */
/*   construct.                                                       */
/*                                                                    */
/* findFunction: Used to determine if a specified construct is in a   */
/*   specific module. The name is the specific construct is passed as */
/*   a string and the function returns a pointer to the specified     */
/*   construct if it exists.                                          */
/*                                                                    */
/* exportable: If TRUE, then the specified construct type can be      */
/*   exported (and hence imported). If FALSE, it can't be exported.   */
/*                                                                    */
/* next: A pointer to the next moduleItem data structure.             */
/**********************************************************************/

struct moduleItem
  {
   char *name;
   int moduleIndex;
   void *(*allocateFunction)(void *);
   void  (*freeFunction)(void *,EXEC_STATUS,void *);
   void *(*bloadModuleReference)(void *,EXEC_STATUS,int);
   void  (*constructsToCModuleReference)(void *,EXEC_STATUS,FILE *,int,int,int);
   void *(*findFunction)(void *,EXEC_STATUS,char *);
   struct moduleItem *next;
  };

typedef struct moduleStackItem
  {
   intBool changeFlag;
   struct defmodule *theModule;
   struct moduleStackItem *next;
  } MODULE_STACK_ITEM;

#define DEFMODULE_DATA 4

struct defmoduleData
  {   
   struct moduleItem *LastModuleItem;
   struct callFunctionItem *AfterModuleChangeFunctions;
   MODULE_STACK_ITEM *ModuleStack;
   intBool CallModuleChangeFunctions;
   struct defmodule *ListOfDefmodules;
   struct defmodule *CurrentModule;
   struct defmodule *LastDefmodule;
   int NumberOfModuleItems;
   struct moduleItem *ListOfModuleItems;
   long ModuleChangeIndex;
   int MainModuleRedefinable;
#if (! RUN_TIME) && (! BLOAD_ONLY)
   struct portConstructItem *ListOfPortConstructItems;
   long NumberOfDefmodules;
   struct callFunctionItem *AfterModuleDefinedFunctions;
#endif
#if CONSTRUCT_COMPILER && (! RUN_TIME)
   struct CodeGeneratorItem *DefmoduleCodeItem;
#endif
#if (BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE) && (! RUN_TIME)
   long BNumberOfDefmodules;
   long NumberOfPortItems;
   struct portItem *PortItemArray;
   struct defmodule *DefmoduleArray;
#endif
  };
  
#define DefmoduleData(theEnv,execStatus) ((struct defmoduleData *) GetEnvironmentData(theEnv,execStatus,DEFMODULE_DATA))

#ifdef LOCALE
#undef LOCALE
#endif

#ifdef _MODULDEF_SOURCE_
#define LOCALE
#else
#define LOCALE extern
#endif

#define FindDefmodule(a) EnvFindDefmodule(GetCurrentEnvironment(),a)
#define GetCurrentModule() EnvGetCurrentModule(GetCurrentEnvironment())
#define GetDefmoduleName(a) EnvGetDefmoduleName(GetCurrentEnvironment(),a)
#define GetDefmodulePPForm(a) EnvGetDefmodulePPForm(GetCurrentEnvironment(),a)
#define GetNextDefmodule(a) EnvGetNextDefmodule(GetCurrentEnvironment(),a)
#define SetCurrentModule(a) EnvSetCurrentModule(GetCurrentEnvironment(),a)

   LOCALE void                           InitializeDefmodules(void *,EXEC_STATUS);
   LOCALE void                          *EnvFindDefmodule(void *,EXEC_STATUS,char *);
   LOCALE char                          *EnvGetDefmoduleName(void *,EXEC_STATUS,void *);
   LOCALE char                          *EnvGetDefmodulePPForm(void *,EXEC_STATUS,void *);
   LOCALE void                          *EnvGetNextDefmodule(void *,EXEC_STATUS,void *);
   LOCALE void                           RemoveAllDefmodules(void *,EXEC_STATUS);
   LOCALE int                            AllocateModuleStorage(void);
   LOCALE int                            RegisterModuleItem(void *,EXEC_STATUS,char *,
                                                            void *(*)(void *),
                                                            void (*)(void *,EXEC_STATUS,void *),
                                                            void *(*)(void *,EXEC_STATUS,int),
                                                            void (*)(void *,EXEC_STATUS,FILE *,int,int,int),
                                                            void *(*)(void *,EXEC_STATUS,char *));
   LOCALE void                          *GetModuleItem(void *,EXEC_STATUS,struct defmodule *,int);
   LOCALE void                           SetModuleItem(void *,EXEC_STATUS,struct defmodule *,int,void *);
   LOCALE void                          *EnvGetCurrentModule(void *,EXEC_STATUS);
   LOCALE void                          *EnvSetCurrentModule(void *,EXEC_STATUS,void *);
   LOCALE void                          *GetCurrentModuleCommand(void *,EXEC_STATUS);
   LOCALE void                          *SetCurrentModuleCommand(void *,EXEC_STATUS);
   LOCALE int                            GetNumberOfModuleItems(void *,EXEC_STATUS);
   LOCALE void                           CreateMainModule(void *,EXEC_STATUS);
   LOCALE void                           SetListOfDefmodules(void *,EXEC_STATUS,void *);
   LOCALE struct moduleItem             *GetListOfModuleItems(void *,EXEC_STATUS);
   LOCALE struct moduleItem             *FindModuleItem(void *,EXEC_STATUS,char *);
   LOCALE void                           SaveCurrentModule(void *,EXEC_STATUS);
   LOCALE void                           RestoreCurrentModule(void *,EXEC_STATUS);
   LOCALE void                           AddAfterModuleChangeFunction(void *,EXEC_STATUS,char *,void (*)(void *),int);
   LOCALE void                           IllegalModuleSpecifierMessage(void *,EXEC_STATUS);
   LOCALE void                           AllocateDefmoduleGlobals(void *,EXEC_STATUS);

#endif


