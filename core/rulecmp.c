
   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*             CLIPS Version 6.24  05/17/06            */
   /*                                                     */
   /*            DEFRULE CONSTRUCTS-TO-C MODULE           */
   /*******************************************************/

/*************************************************************/
/* Purpose: Implements the constructs-to-c feature for the   */
/*    defrule construct.                                     */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.24: Removed DYNAMIC_SALIENCE and                   */
/*            LOGICAL_DEPENDENCIES compilation flags.        */
/*                                                           */
/*************************************************************/

#define _RULECMP_SOURCE_

#include "setup.h"

#if DEFRULE_CONSTRUCT && (! RUN_TIME) && CONSTRUCT_COMPILER

#include <stdio.h>
#define _STDIO_INCLUDED_
#include <string.h>

#include "envrnmnt.h"
#include "factbld.h"
#include "reteutil.h"

#include "rulecmp.h"

/***************************************/
/* LOCAL INTERNAL FUNCTION DEFINITIONS */
/***************************************/

   static int                     ConstructToCode(void *,EXEC_STATUS,char *,char *,char *,int,FILE *,int,int);
   static void                    JoinToCode(void *,EXEC_STATUS,FILE *,struct joinNode *,int,int);
   static void                    LinkToCode(void *,EXEC_STATUS,FILE *,struct joinLink *,int,int);
   static void                    DefruleModuleToCode(void *,EXEC_STATUS,FILE *,struct defmodule *,int,int,int);
   static void                    DefruleToCode(void *,EXEC_STATUS,FILE *,struct defrule *,int,int,int);
   static void                    CloseDefruleFiles(void *,EXEC_STATUS,FILE *,FILE *,FILE *,FILE*,int);
   static void                    BeforeDefrulesCode(void *,EXEC_STATUS);
   static void                    InitDefruleCode(void *,EXEC_STATUS,FILE *,int,int);
   static int                     RuleCompilerTraverseJoins(void *,EXEC_STATUS,struct joinNode *,char *,char *,char *,int,
                                                            FILE *,int,int,FILE **,FILE **,
                                                            int *,int *,int *,int *,int *);
   static int                     TraverseJoinLinks(void *,EXEC_STATUS,struct joinLink *,char *,char *,char *,int,FILE *,
                                                    int,int,FILE **,int *,int *, int *);
  
/***********************************************************/
/* DefruleCompilerSetup: Initializes the defrule construct */
/*   for use with the constructs-to-c command.             */
/***********************************************************/
globle void DefruleCompilerSetup(
  void *theEnv,
  EXEC_STATUS)
  {
   DefruleData(theEnv,execStatus)->DefruleCodeItem = AddCodeGeneratorItem(theEnv,execStatus,"defrules",0,BeforeDefrulesCode,
                                          InitDefruleCode,ConstructToCode,4);
  }

/**************************************************************/
/* BeforeDefrulesCode: Assigns each defrule and join with a   */
/*   unique ID which will be used for pointer references when */
/*   the data structures are written to a file as C code      */
/**************************************************************/
static void BeforeDefrulesCode(
  void *theEnv,
  EXEC_STATUS)
  {
   long int moduleCount, ruleCount, joinCount, linkCount;

   TagRuleNetwork(theEnv,execStatus,&moduleCount,&ruleCount,&joinCount, &linkCount);
  }

/*********************************************************/
/* ConstructToCode: Produces defrule code for a run-time */
/*   module created using the constructs-to-c function.  */
/*********************************************************/
static int ConstructToCode(
  void *theEnv,
  EXEC_STATUS,
  char *fileName,
  char *pathName,
  char *fileNameBuffer,
  int fileID,
  FILE *headerFP,
  int imageID,
  int maxIndices)
  {
   int fileCount = 1;
   struct defmodule *theModule;
   struct defrule *theDefrule;
   int joinArrayCount = 0, joinArrayVersion = 1;
   int linkArrayCount = 0, linkArrayVersion = 1;
   int moduleCount = 0, moduleArrayCount = 0, moduleArrayVersion = 1;
   int defruleArrayCount = 0, defruleArrayVersion = 1;
   FILE *joinFile = NULL, *moduleFile = NULL, *defruleFile = NULL, *linkFile = NULL;

   /*==============================================*/
   /* Include the appropriate defrule header file. */
   /*==============================================*/

   fprintf(headerFP,"#include \"ruledef.h\"\n");

   /*======================================*/
   /* Save the left and right prime links. */
   /*======================================*/
   
   if (! TraverseJoinLinks(theEnv,execStatus,DefruleData(theEnv,execStatus)->LeftPrimeJoins,fileName,pathName,fileNameBuffer,fileID,headerFP,imageID,
                           maxIndices,&linkFile,&fileCount,&linkArrayVersion,&linkArrayCount))
     {
      CloseDefruleFiles(theEnv,execStatus,moduleFile,defruleFile,joinFile,linkFile,maxIndices);
      return(0);
     }

   if (! TraverseJoinLinks(theEnv,execStatus,DefruleData(theEnv,execStatus)->RightPrimeJoins,fileName,pathName,fileNameBuffer,fileID,headerFP,imageID,
                           maxIndices,&linkFile,&fileCount,&linkArrayVersion,&linkArrayCount))
     {
      CloseDefruleFiles(theEnv,execStatus,moduleFile,defruleFile,joinFile,linkFile,maxIndices);
      return(0);
     }
     
   /*=========================================================*/
   /* Loop through all the modules, all the defrules, and all */
   /* the join nodes writing their C code representation to   */
   /* the file as they are traversed.                         */
   /*=========================================================*/

   for (theModule = (struct defmodule *) EnvGetNextDefmodule(theEnv,execStatus,NULL);
        theModule != NULL;
        theModule = (struct defmodule *) EnvGetNextDefmodule(theEnv,execStatus,theModule))
     {
      /*=========================*/
      /* Set the current module. */
      /*=========================*/

      EnvSetCurrentModule(theEnv,execStatus,(void *) theModule);

      /*==========================*/
      /* Save the defrule module. */
      /*==========================*/

      moduleFile = OpenFileIfNeeded(theEnv,execStatus,moduleFile,fileName,pathName,fileNameBuffer,fileID,imageID,&fileCount,
                                    moduleArrayVersion,headerFP,
                                    "struct defruleModule",ModulePrefix(DefruleData(theEnv,execStatus)->DefruleCodeItem),
                                    FALSE,NULL);

      if (moduleFile == NULL)
        {
         CloseDefruleFiles(theEnv,execStatus,moduleFile,defruleFile,joinFile,linkFile,maxIndices);
         return(0);
        }

      DefruleModuleToCode(theEnv,execStatus,moduleFile,theModule,imageID,maxIndices,moduleCount);
      moduleFile = CloseFileIfNeeded(theEnv,execStatus,moduleFile,&moduleArrayCount,&moduleArrayVersion,
                                     maxIndices,NULL,NULL);

      /*=========================================*/
      /* Loop through all of the defrules (and   */
      /* their disjuncts) in the current module. */
      /*=========================================*/

      theDefrule = (struct defrule *) EnvGetNextDefrule(theEnv,execStatus,NULL);

      while (theDefrule != NULL)
        {
         /*===================================*/
         /* Save the defrule data structures. */
         /*===================================*/

         defruleFile = OpenFileIfNeeded(theEnv,execStatus,defruleFile,fileName,pathName,fileNameBuffer,fileID,imageID,&fileCount,
                                        defruleArrayVersion,headerFP,
                                        "struct defrule",ConstructPrefix(DefruleData(theEnv,execStatus)->DefruleCodeItem),
                                        FALSE,NULL);
         if (defruleFile == NULL)
           {
            CloseDefruleFiles(theEnv,execStatus,moduleFile,defruleFile,joinFile,linkFile,maxIndices);
            return(0);
           }

         DefruleToCode(theEnv,execStatus,defruleFile,theDefrule,imageID,maxIndices,
                        moduleCount);
         defruleArrayCount++;
         defruleFile = CloseFileIfNeeded(theEnv,execStatus,defruleFile,&defruleArrayCount,&defruleArrayVersion,
                                         maxIndices,NULL,NULL);

         /*================================*/
         /* Save the join data structures. */
         /*================================*/

         if (! RuleCompilerTraverseJoins(theEnv,execStatus,theDefrule->lastJoin,fileName,pathName,fileNameBuffer,fileID,headerFP,imageID,
                                         maxIndices,&joinFile,&linkFile,&fileCount,&joinArrayVersion,&joinArrayCount,
                                         &linkArrayVersion,&linkArrayCount))
           {
            CloseDefruleFiles(theEnv,execStatus,moduleFile,defruleFile,joinFile,linkFile,maxIndices);
            return(0);
           }

         /*==========================================*/
         /* Move on to the next disjunct or defrule. */
         /*==========================================*/

         if (theDefrule->disjunct != NULL) theDefrule = theDefrule->disjunct;
         else theDefrule = (struct defrule *) EnvGetNextDefrule(theEnv,execStatus,theDefrule);
        }

      moduleCount++;
      moduleArrayCount++;
     }

   CloseDefruleFiles(theEnv,execStatus,moduleFile,defruleFile,joinFile,linkFile,maxIndices);

   return(1);
  }

/*********************************************************************/
/* RuleCompilerTraverseJoins: Traverses the join network for a rule. */
/*********************************************************************/
static int RuleCompilerTraverseJoins(
  void *theEnv,
  EXEC_STATUS,
  struct joinNode *joinPtr,
  char *fileName,
  char *pathName,
  char *fileNameBuffer,
  int fileID,
  FILE *headerFP,
  int imageID,
  int maxIndices,
  FILE **joinFile,
  FILE **linkFile,
  int *fileCount,
  int *joinArrayVersion,
  int *joinArrayCount,
  int *linkArrayVersion,
  int *linkArrayCount)
  {      
   for (;
        joinPtr != NULL;
        joinPtr = joinPtr->lastLevel)
     { 
      if (joinPtr->marked)
        {
         *joinFile = OpenFileIfNeeded(theEnv,execStatus,*joinFile,fileName,pathName,fileNameBuffer,fileID,imageID,fileCount,
                                      *joinArrayVersion,headerFP,
                                      "struct joinNode",JoinPrefix(),FALSE,NULL);
         if (*joinFile == NULL)
           { return(FALSE); }

         JoinToCode(theEnv,execStatus,*joinFile,joinPtr,imageID,maxIndices);
         (*joinArrayCount)++;
         *joinFile = CloseFileIfNeeded(theEnv,execStatus,*joinFile,joinArrayCount,joinArrayVersion,
                                       maxIndices,NULL,NULL);
                      
                                       
         if (! TraverseJoinLinks(theEnv,execStatus,joinPtr->nextLinks,fileName,pathName,fileNameBuffer,fileID,headerFP,imageID,
                                 maxIndices,linkFile,fileCount,linkArrayVersion,linkArrayCount))
           { return(FALSE); } 
        }
      
      if (joinPtr->joinFromTheRight)
        { 
         if (RuleCompilerTraverseJoins(theEnv,execStatus,(struct joinNode *) joinPtr->rightSideEntryStructure,fileName,pathName,
                                       fileNameBuffer,fileID,headerFP,imageID,maxIndices,joinFile,linkFile,fileCount,
                                       joinArrayVersion,joinArrayCount,
                                       linkArrayVersion,linkArrayCount) == FALSE)
           { return(FALSE); }
        }
     }

   return(TRUE);
  }

/*******************************************************/
/* TraverseJoinLinks: Writes out a list of join links. */
/*******************************************************/
static int TraverseJoinLinks(
  void *theEnv,
  EXEC_STATUS,
  struct joinLink *linkPtr,
  char *fileName,
  char *pathName,
  char *fileNameBuffer,
  int fileID,
  FILE *headerFP,
  int imageID,
  int maxIndices,
  FILE **linkFile,
  int *fileCount,
  int *linkArrayVersion,
  int *linkArrayCount)
  {   
   for (;
        linkPtr != NULL;
        linkPtr = linkPtr->next)
     {
      *linkFile = OpenFileIfNeeded(theEnv,execStatus,*linkFile,fileName,pathName,fileNameBuffer,fileID,imageID,fileCount,
                                   *linkArrayVersion,headerFP,
                                   "struct joinLink",LinkPrefix(),FALSE,NULL);
           
      if (*linkFile == NULL)
        { return(FALSE); }
           
      LinkToCode(theEnv,execStatus,*linkFile,linkPtr,imageID,maxIndices);
      (*linkArrayCount)++;
      *linkFile = CloseFileIfNeeded(theEnv,execStatus,*linkFile,linkArrayCount,linkArrayVersion,
                                    maxIndices,NULL,NULL);
     }

   return(TRUE);
  }

/********************************************************/
/* CloseDefruleFiles: Closes all of the C files created */
/*   for defrule. Called when an error occurs or when   */
/*   the defrules have all been written to the files.   */
/********************************************************/
static void CloseDefruleFiles(
  void *theEnv,
  EXEC_STATUS,
  FILE *moduleFile,
  FILE *defruleFile,
  FILE *joinFile,
  FILE *linkFile,
  int maxIndices)
  {
   int count = maxIndices;
   int arrayVersion = 0;

   if (linkFile != NULL)
     {
      count = maxIndices;
      CloseFileIfNeeded(theEnv,execStatus,linkFile,&count,&arrayVersion,maxIndices,NULL,NULL);
     }

   if (joinFile != NULL)
     {
      count = maxIndices;
      CloseFileIfNeeded(theEnv,execStatus,joinFile,&count,&arrayVersion,maxIndices,NULL,NULL);
     }

   if (defruleFile != NULL)
     {
      count = maxIndices;
      CloseFileIfNeeded(theEnv,execStatus,defruleFile,&count,&arrayVersion,maxIndices,NULL,NULL);
     }

   if (moduleFile != NULL)
     {
      count = maxIndices;
      CloseFileIfNeeded(theEnv,execStatus,moduleFile,&count,&arrayVersion,maxIndices,NULL,NULL);
     }
  }

/*********************************************************/
/* DefruleModuleToCode: Writes the C code representation */
/*   of a single defrule module to the specified file.   */
/*********************************************************/
#if WIN_BTC
#pragma argsused
#endif
static void DefruleModuleToCode(
  void *theEnv,
  EXEC_STATUS,
  FILE *theFile,
  struct defmodule *theModule,
  int imageID,
  int maxIndices,
  int moduleCount)
  {
#if MAC_MCW || WIN_MCW || MAC_XCD
#pragma unused(moduleCount)
#endif

   fprintf(theFile,"{");

   ConstructModuleToCode(theEnv,execStatus,theFile,theModule,imageID,maxIndices,
                                  DefruleData(theEnv,execStatus)->DefruleModuleIndex,ConstructPrefix(DefruleData(theEnv,execStatus)->DefruleCodeItem));

   fprintf(theFile,",NULL}");
  }

/**********************************************************/
/* DefruleToCode: Writes the C code representation of a   */
/*   single defrule data structure to the specified file. */
/**********************************************************/
static void DefruleToCode(
  void *theEnv,
  EXEC_STATUS,
  FILE *theFile,
  struct defrule *theDefrule,
  int imageID,
  int maxIndices,
  int moduleCount)
  {
   /*==================*/
   /* Construct Header */
   /*==================*/

   fprintf(theFile,"{");

   ConstructHeaderToCode(theEnv,execStatus,theFile,&theDefrule->header,imageID,maxIndices,
                                  moduleCount,ModulePrefix(DefruleData(theEnv,execStatus)->DefruleCodeItem),
                                  ConstructPrefix(DefruleData(theEnv,execStatus)->DefruleCodeItem));

   /*==========================*/
   /* Flags and Integer Values */
   /*==========================*/

   fprintf(theFile,",%d,%d,%d,%d,%d,%d,%d,%d,",
                   theDefrule->salience,theDefrule->localVarCnt,
                   theDefrule->complexity,theDefrule->afterBreakpoint,
                   theDefrule->watchActivation,theDefrule->watchFiring,
                   theDefrule->autoFocus,theDefrule->executing);

   /*==================*/
   /* Dynamic Salience */
   /*==================*/

   ExpressionToCode(theEnv,execStatus,theFile,theDefrule->dynamicSalience);
   fprintf(theFile,",");

   /*=============*/
   /* RHS Actions */
   /*=============*/

   ExpressionToCode(theEnv,execStatus,theFile,theDefrule->actions);
   fprintf(theFile,",");

   /*=========================*/
   /* Logical Dependency Join */
   /*=========================*/

   if (theDefrule->logicalJoin != NULL)
     {
      fprintf(theFile,"&%s%d_%ld[%ld],",JoinPrefix(),
                     imageID,(theDefrule->logicalJoin->bsaveID / maxIndices) + 1,
                             theDefrule->logicalJoin->bsaveID % maxIndices);
     }
   else
     { fprintf(theFile,"NULL,"); }

   /*===========*/
   /* Last Join */
   /*===========*/

   if (theDefrule->lastJoin != NULL)
     {
      fprintf(theFile,"&%s%d_%ld[%ld],",JoinPrefix(),
                     imageID,(theDefrule->lastJoin->bsaveID / maxIndices) + 1,
                             theDefrule->lastJoin->bsaveID % maxIndices);
     }
   else
     { fprintf(theFile,"NULL,"); }

   /*===============*/
   /* Next Disjunct */
   /*===============*/

   if (theDefrule->disjunct != NULL)
     {
      fprintf(theFile,"&%s%d_%ld[%ld]}",ConstructPrefix(DefruleData(theEnv,execStatus)->DefruleCodeItem),
                     imageID,(theDefrule->disjunct->header.bsaveID / maxIndices) + 1,
                             theDefrule->disjunct->header.bsaveID % maxIndices);
     }
   else
     { fprintf(theFile,"NULL}"); }
  }

/***************************************************/
/* JoinToCode: Writes the C code representation of */
/*   a single join node to the specified file.     */
/***************************************************/
static void JoinToCode(
  void *theEnv,
  EXEC_STATUS,
  FILE *joinFile,
  struct joinNode *theJoin,
  int imageID,
  int maxIndices)
  {
   struct patternParser *theParser;

   /*===========================*/
   /* Mark the join as visited. */
   /*===========================*/

   theJoin->marked = 0;

   /*===========================*/
   /* Flags and Integer Values. */
   /*===========================*/

   fprintf(joinFile,"{%d,%d,%d,%d,%d,0,0,%d,%d,0,0,0,0,",
                   theJoin->firstJoin,theJoin->logicalJoin,
                   theJoin->joinFromTheRight,theJoin->patternIsNegated,
                   theJoin->patternIsExists,
                   theJoin->rhsType,theJoin->depth);

   /*==========================*/
   /* Left and right Memories. */
   /*==========================*/

   fprintf(joinFile,"NULL,NULL,");

   /*====================*/
   /* Network Expression */
   /*====================*/

   PrintHashedExpressionReference(theEnv,execStatus,joinFile,theJoin->networkTest,imageID,maxIndices);
   fprintf(joinFile,",");

   PrintHashedExpressionReference(theEnv,execStatus,joinFile,theJoin->secondaryNetworkTest,imageID,maxIndices);
   fprintf(joinFile,",");

   PrintHashedExpressionReference(theEnv,execStatus,joinFile,theJoin->leftHash,imageID,maxIndices);
   fprintf(joinFile,",");

   PrintHashedExpressionReference(theEnv,execStatus,joinFile,theJoin->rightHash,imageID,maxIndices);
   fprintf(joinFile,",");
   
   /*============================*/
   /* Right Side Entry Structure */
   /*============================*/

   if (theJoin->rightSideEntryStructure == NULL)
     { fprintf(joinFile,"NULL,"); }
   else if (theJoin->joinFromTheRight == FALSE)
     {
      theParser = GetPatternParser(theEnv,execStatus,(int) theJoin->rhsType);
      if (theParser->codeReferenceFunction == NULL) fprintf(joinFile,"NULL,");
      else
        {
         fprintf(joinFile,"VS ");
         (*theParser->codeReferenceFunction)(theEnv,execStatus,theJoin->rightSideEntryStructure,
                                             joinFile,imageID,maxIndices);
         fprintf(joinFile,",");
        }
     }
   else
     {
      fprintf(joinFile,"&%s%d_%ld[%ld],",JoinPrefix(),
              imageID,(((struct joinNode *) theJoin->rightSideEntryStructure)->bsaveID / maxIndices) + 1,
                      ((struct joinNode *) theJoin->rightSideEntryStructure)->bsaveID % maxIndices);
     }

   /*=================*/
   /* Next Join Level */
   /*=================*/

   if (theJoin->nextLinks == NULL)
     { fprintf(joinFile,"NULL,"); }
   else
     {
      fprintf(joinFile,"&%s%d_%ld[%ld],",LinkPrefix(),
                    imageID,(theJoin->nextLinks->bsaveID / maxIndices) + 1,
                            theJoin->nextLinks->bsaveID % maxIndices);
     }

   /*=================*/
   /* Last Join Level */
   /*=================*/

   if (theJoin->lastLevel == NULL)
     { fprintf(joinFile,"NULL,"); }
   else
     {
      fprintf(joinFile,"&%s%d_%ld[%ld],",JoinPrefix(),
                    imageID,(theJoin->lastLevel->bsaveID / maxIndices) + 1,
                            theJoin->lastLevel->bsaveID % maxIndices);
     }

   /*==================*/
   /* Right Match Node */
   /*==================*/

   if (theJoin->rightMatchNode == NULL)
     { fprintf(joinFile,"NULL,"); }
   else
     {
      fprintf(joinFile,"&%s%d_%ld[%ld],",JoinPrefix(),
                    imageID,(theJoin->rightMatchNode->bsaveID / maxIndices) + 1,
                            theJoin->rightMatchNode->bsaveID % maxIndices);
     }

   /*==================*/
   /* Rule to Activate */
   /*==================*/

   if (theJoin->ruleToActivate == NULL)
     { fprintf(joinFile,"NULL}"); }
   else
     {
      fprintf(joinFile,"&%s%d_%ld[%ld]}",ConstructPrefix(DefruleData(theEnv,execStatus)->DefruleCodeItem),imageID,
                                    (theJoin->ruleToActivate->header.bsaveID / maxIndices) + 1,
                                    theJoin->ruleToActivate->header.bsaveID % maxIndices);
     }
  }

/***************************************************/
/* LinkToCode: Writes the C code representation of */
/*   a single join node to the specified file.     */
/***************************************************/
static void LinkToCode(
  void *theEnv,
  EXEC_STATUS,
  FILE *theFile,
  struct joinLink *theLink,
  int imageID,
  int maxIndices)
  {    
   /*==================*/
   /* Enter Direction. */
   /*==================*/

   fprintf(theFile,"{%d,",theLink->enterDirection);

   /*======*/
   /* Join */
   /*======*/

   if (theLink->join == NULL)
     { fprintf(theFile,"NULL,"); }
   else
     {
      fprintf(theFile,"&%s%d_%ld[%ld],",JoinPrefix(),
                    imageID,(theLink->join->bsaveID / maxIndices) + 1,
                            theLink->join->bsaveID % maxIndices);
     }

   /*======*/
   /* Next */
   /*======*/

   if (theLink->next == NULL)
     { fprintf(theFile,"NULL,"); }
   else
     {
      fprintf(theFile,"&%s%d_%ld[%ld],",LinkPrefix(),
                    imageID,(theLink->next->bsaveID / maxIndices) + 1,
                            theLink->next->bsaveID % maxIndices);
     }
     
   /*===========*/
   /* Bsave ID. */
   /*===========*/

   fprintf(theFile,"0}");
  }

/*************************************************************/
/* DefruleCModuleReference: Writes the C code representation */
/*   of a reference to a defrule module data structure.      */
/*************************************************************/
globle void DefruleCModuleReference(
  void *theEnv,
  EXEC_STATUS,
  FILE *theFile,
  int count,
  int imageID,
  int maxIndices)
  {
   fprintf(theFile,"MIHS &%s%d_%d[%d]",ModulePrefix(DefruleData(theEnv,execStatus)->DefruleCodeItem),
                      imageID,
                      (count / maxIndices) + 1,
                      (count % maxIndices));
  }


/*****************************************************************/
/* InitDefruleCode: Writes out initialization code for defrules. */
/*****************************************************************/
#if WIN_BTC
#pragma argsused
#endif
static void InitDefruleCode(
  void *theEnv,
  EXEC_STATUS,
  FILE *initFP,
  int imageID,
  int maxIndices)
  {
#if MAC_MCW || WIN_MCW || MAC_XCD
#pragma unused(maxIndices)
#pragma unused(theEnv,execStatus)
#pragma unused(imageID)
#endif

   fprintf(initFP,"   DefruleRunTimeInitialize(theEnv,execStatus,");

   if (DefruleData(theEnv,execStatus)->RightPrimeJoins == NULL)
     { fprintf(initFP,"NULL,"); }
   else
     {
      fprintf(initFP,"&%s%d_%ld[%ld],",LinkPrefix(),
                    imageID,(DefruleData(theEnv,execStatus)->RightPrimeJoins->bsaveID / maxIndices) + 1,
                             DefruleData(theEnv,execStatus)->RightPrimeJoins->bsaveID % maxIndices);
     }

   if (DefruleData(theEnv,execStatus)->LeftPrimeJoins == NULL)
     { fprintf(initFP,"NULL);\n"); }
   else
     {
      fprintf(initFP,"&%s%d_%ld[%ld]);\n",LinkPrefix(),
                    imageID,(DefruleData(theEnv,execStatus)->LeftPrimeJoins->bsaveID / maxIndices) + 1,
                             DefruleData(theEnv,execStatus)->LeftPrimeJoins->bsaveID % maxIndices);
     }
  }

#endif /* DEFRULE_CONSTRUCT && (! RUN_TIME) && CONSTRUCT_COMPILER */


