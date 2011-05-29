/*
* Copyright © 2011, David Delassus <david.jose.delassus@gmail.com>
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __SPLITTED_WINDOW_H
#define __SPLITTED_WINDOW_H

/*!
 * \defgroup split SplittedWindow
 * \ingroup interface
 * SplittedWindow structure definition
 *
 * @{
 */

typedef struct _SplittedWindowObject SplittedWindowObject;
typedef struct _SplittedWindow SplittedWindow;

/*!
 * \class SplittedWindowObject
 * Internal object's structure for SplittedWindow
 */
struct _SplittedWindowObject
{
     GtkWidget *widget;            /*!< GtkPaned, or Notebook */

     SplittedWindowObject *parent; /*!< Parent structure (NULL for root node) */
     SplittedWindowObject *c1;     /*!< Left child (NULL if none) */
     SplittedWindowObject *c2;     /*!< Right child (NULL if none) */
};

/*!
 * \class SplittedWindow
 * #SplittedWindow object.
 */
struct _SplittedWindow
{
     SplittedWindowObject *root;   /*!< Root node of the #SplittedWindow's tree. */
     SplittedWindowObject *focus;  /*!< Focused node */
};

SplittedWindow *splittedwindow_new (void);

void splittedwindow_vsplit (SplittedWindow *sw, GtkWidget *nb);
void splittedwindow_split (SplittedWindow *sw, GtkWidget *nb);
void splittedwindow_close (SplittedWindow *sw);

/*! @} */

#endif /* __SPLITTED_WINDOW_H */
