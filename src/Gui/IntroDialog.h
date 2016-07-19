/*****************************************************************************
 * IntroDialog.h: Introduction dialog
 *****************************************************************************
 * Copyright (C) 2010 the VLMC team
 *
 * Authors: Ludovic Fauvet <etix@l0cal.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifndef INTRODIALOG_H
#define INTRODIALOG_H

#include "ui/IntroDialog.h"

class IntroDialog : public QDialog
{
    Q_OBJECT
public:
    explicit IntroDialog( QWidget *parent = 0 );

protected:
    void changeEvent( QEvent *e );

private:
    Ui::IntroDialog ui;
};

#endif // INTRODIALOG_H
