
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

#ifndef GLLIST_H_INCLUDED
#define GLLIST_H_INCLUDED

/**********************************************************************************************/

  typedef unsigned char* LPTHING;

  typedef struct GLLISTITEM_STRUCT GLLISTITEM;
  typedef GLLISTITEM* LPGLLISTITEM;

  struct GLLISTITEM_STRUCT {
    LPTHING             Data;
    LPGLLISTITEM        Next;
  };

/**********************************************************************************************/

  typedef void (*LPGLLISTPROC)(GLvoid*);

  typedef struct tag_GLLIST {
    char                Id [4];
    GLenum              NumItems;
    LPGLLISTITEM        Head;
    LPGLLISTITEM        Tail;
    LPGLLISTPROC        DeleteProc;
  } GLLIST, *LPGLLIST;

/**********************************************************************************************/

  GLboolean     glList_Init             (LPGLLIST List, LPGLLISTPROC Proc);
  GLboolean     glList_Destroy          (LPGLLIST List);
  GLvoid*       glList_Head             (LPGLLIST List);
  GLvoid*       glList_Tail             (LPGLLIST List);
  LPGLLISTITEM  glList_ListHead         (LPGLLIST List);
  LPGLLISTITEM  glList_ListTail         (LPGLLIST List);
  GLboolean     glList_Add              (LPGLLIST List, GLvoid* Item);
  GLboolean     glList_RemoveByPointer  (LPGLLIST List, GLvoid* Item);
  GLboolean     glList_RemoveByIndex    (LPGLLIST List, GLenum  Item);
  GLint         glList_GetIndex         (LPGLLIST List, GLvoid* Item);
  GLvoid*       glList_GetPointer       (LPGLLIST List, GLenum  Item);
  GLvoid*       glList_GetLast          (LPGLLIST List);
  GLboolean     glList_Push             (LPGLLIST List, GLvoid* Item);
  GLvoid*       glList_Pop              (LPGLLIST List);

/**********************************************************************************************/

#endif
