
/************************************************************************\

    TinyGL - CPU implementation of OpenGL
    Copyright (c) 1998-2026 Jango73

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.


    List

\************************************************************************/

#include <stdio.h>
#include <math.h>

#include "gllist.h"
#include "glmain.h"

/**********************************************************************************************/

static GLboolean glList_DestroyItem (LPGLLIST List, LPGLLISTITEM Item)
{
  if (!List || !Item) return FALSE;

  if (List->DeleteProc!=NULL)
  {
    List->DeleteProc((GLvoid*)Item->Data);
  }
  else
  {
    glFree((GLvoid*)Item->Data);
  }

  glFree((GLvoid*)Item);

  return TRUE;
}

/**********************************************************************************************/

static GLboolean glList_Check (LPGLLIST List)
{
  if (List->Id[0]=='L' &&
      List->Id[1]=='I' &&
      List->Id[2]=='S' &&
      List->Id[3]=='T' ) return TRUE;

  return FALSE;
}

/**********************************************************************************************/

GLboolean glList_Init (LPGLLIST List, LPGLLISTPROC Proc)
{
  if (List==NULL) return FALSE;

  List->Id[0]='L';
  List->Id[1]='I';
  List->Id[2]='S';
  List->Id[3]='T';

  List->NumItems=0;
  List->Head=NULL;
  List->Tail=NULL;
  List->DeleteProc=Proc;

  return TRUE;
}

/**********************************************************************************************/

static void glList_Update (LPGLLIST List)
{
  LPGLLISTITEM  cur, old;
  GLenum        count;

  if (List==NULL) return;

  // if (List->Head==NULL) { glList_Init(List, List->DeleteProc); return; }
  if (!List->Head) return;

  for (cur=List->Head, old=List->Head, count=0; cur; cur=cur->Next, count++) old=cur;

  List->NumItems=count;
  List->Tail=old;
}

/**********************************************************************************************/

GLboolean glList_Destroy (LPGLLIST List)
{
  LPGLLISTITEM  cur, nxt;

  if (!List || !glList_Check(List) || !List->Head) return FALSE;

  for (cur=List->Head; cur; )
  {
    nxt=cur->Next;
    glList_DestroyItem(List, cur);
    cur=nxt;
  }

  glList_Init(List, (LPGLLISTPROC)NULL);

  return TRUE;
}

/**********************************************************************************************/

GLvoid* glList_Head (LPGLLIST List)
{
  if (!List || !glList_Check(List) || !List->Head) return NULL;
  return (GLvoid*)List->Head->Data;
}

/**********************************************************************************************/

GLvoid* glList_Tail (LPGLLIST List)
{
  if (!List || !glList_Check(List) || !List->Head) return NULL;
  return (GLvoid*)List->Tail->Data;
}

/**********************************************************************************************/

LPGLLISTITEM glList_ListHead (LPGLLIST List)
{
  if (!List || !glList_Check(List) || !List->Head) return NULL;
  return List->Head;
}

/**********************************************************************************************/

LPGLLISTITEM glList_ListTail (LPGLLIST List)
{
  if (!List || !glList_Check(List) || !List->Head) return NULL;
  return List->Tail;
}

/**********************************************************************************************/

GLboolean glList_Add (LPGLLIST List, GLvoid* Item)
{
  LPGLLISTITEM  i;

  if (!List || !glList_Check(List)) return FALSE;

  i=(LPGLLISTITEM)glMalloc(sizeof(GLLISTITEM));
  if (i==NULL) return FALSE;

  i->Data=(LPTHING)Item;
  i->Next=NULL;

  if (List->Head==NULL) List->Head=i;
  else { List->Tail->Next=i; List->Tail=i; }

  List->NumItems++;

  glList_Update(List);

  return TRUE;
}

/**********************************************************************************************/

GLboolean glList_RemoveByPointer (LPGLLIST List, GLvoid* Item)
{
  LPGLLISTITEM  cur, old;
  GLboolean     found=FALSE;

  if (!List || !glList_Check(List) || !List->Head) return FALSE;

  cur=List->Head; old=List->Head;

  while (cur!=NULL)
  {
    if (cur->Data==(LPTHING)Item)
    {
      if (cur==List->Head)
      {
        List->Head=cur->Next;
        glList_DestroyItem(List, cur);
        cur=old=List->Head;
      }
      else
      {
        old->Next=cur->Next;
        glList_DestroyItem(List, cur);
        cur=old->Next;
      }
      List->NumItems--;
      found=TRUE;
      break;
    }
    else { old=cur; cur=cur->Next; }
  }

  glList_Update(List);

  return found;
}

/**********************************************************************************************/

GLboolean glList_RemoveByIndex (LPGLLIST List, GLenum Item)
{
  LPGLLISTITEM  cur, old;
  GLenum        count;
  GLboolean     found=FALSE;

  if (!List || !glList_Check(List) || !List->Head) return FALSE;

  cur=List->Head; old=List->Head; count=0;

  while (cur!=NULL)
  {
    if (count==Item)
    {
      if (cur==List->Head)
      {
        List->Head=cur->Next;
        glList_DestroyItem(List, cur);
        cur=old=List->Head;
      }
      else
      {
        old->Next=cur->Next;
        glList_DestroyItem(List, cur);
        cur=old->Next;
      }
      List->NumItems--;
      found=TRUE;
      break;
    }
    else { old=cur; cur=cur->Next; }
    count++;
  }

  glList_Update(List);

  return found;
}

/**********************************************************************************************/

GLint glList_GetIndex (LPGLLIST List, GLvoid* Item)
{
  LPGLLISTITEM  cur;
  GLint         index;

  if (!List || !glList_Check(List) || !List->Head) return -1;

  for (cur=List->Head, index=0; cur; cur=cur->Next, index++)
  {
    if (cur->Data==(LPTHING)Item) break;
  }

  return index;
}

/**********************************************************************************************/

GLvoid* glList_GetPointer (LPGLLIST List, GLenum Item)
{
  LPGLLISTITEM  cur;
  GLenum        index;

  if (!List || !glList_Check(List) || !List->Head) return NULL;

  for (cur=List->Head, index=0; cur; cur=cur->Next, index++)
  {
    if (index==Item)
    {
      return (GLvoid*)cur->Data;
    }
  }

  return NULL;
}

/**********************************************************************************************/

GLvoid* glList_GetLast (LPGLLIST List)
{
  LPGLLISTITEM  cur;
  GLenum        index;

  if (!List || !glList_Check(List) || !List->Head || !List->Tail) return NULL;

  return (GLvoid*)List->Tail->Data;
}

/**********************************************************************************************/

GLboolean glList_Push (LPGLLIST List, GLvoid* Item)
{
  return FALSE;
}

/**********************************************************************************************/

GLvoid* glList_Pop (LPGLLIST List)
{
  return FALSE;
}

/**********************************************************************************************/
