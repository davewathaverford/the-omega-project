/* $Id: omega.h,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */


// Sorry about the following unslightly billboard, but we need to
//  put it somewhere  :-)    
//   -- The Omega Project 

/*********************************************************************
    Copyright (C) 1994-1996 by the Omega Project

    All rights reserved.

    NOTICE:  This software is provided ``as is'', without any
    warranty, including any implied warranty for merchantability or
    fitness for a particular purpose.  Under no circumstances shall
    the Omega Project or its agents be liable for any use of, misuse
    of, or inability to use this software, including incidental and
    consequential damages.

    License is hereby given to use, modify, and redistribute this
    software, in whole or in part, for any purpose, commercial or
    non-commercial, provided that the user agrees to the terms of this
    copyright notice, including disclaimer of warranty, and provided
    that this copyright notice, including disclaimer of warranty, is
    preserved in the source code and documentation of anything derived
    from this software.  Any redistributor of this software or
    anything derived from this software assumes responsibility for
    ensuring that any parties to whom such a redistribution is made
    are fully aware of the terms of this license and disclaimer.

    The Omega project can be contacted at omega@cs.umd.edu
    or http://www.cs.umd.edu/projects/omega

*********************************************************************/

#ifndef Already_Included_Omega
#define Already_Included_Omega

/*
 * The presburger interface is divided into the following parts.
 * These parts are all included together, but are in separate
 * files to keep things organized a bit.
 *
 * In many files, you can include just some of the following,
 * specifically: if you are building a presburger tree, just
 * include "pres_tree.h"; if you are querying it, include
 * "pres_dnf.d" and "pres_conj.h"; if you are doing relational
 * operations, include "Relation.h"
 *
 * Most of the function definitions are in the .c files with
 * the same name as the .h that declares them, except:
 *   the remap and push_exists functions are in pres_var.c
 *   the DNFize functions are in pres_dnf.c
 *   the functions involving printing are in pres_print.c
 *   the beautify functions are in pres_beaut.c
 *   the rearrange functions are in pres_rear.c
 *   the compression functions are in pres_cmpr.c
 */

#include <omega/omega_core/debugging.h>
#include <omega/pres_var.h>
#include <omega/pres_cnstr.h>
#include <omega/pres_subs.h>
#include <omega/pres_form.h>
#include <omega/pres_logic.h>
#include <omega/pres_decl.h>
#include <omega/pres_quant.h>
#include <omega/pres_conj.h>
#include <omega/pres_cmpr.h>
#include <omega/Relation.h>

#include <omega/Rel_map.h>
#include <omega/farkas.h>
#include <omega/hull.h>
#include <omega/closure.h>
#include <omega/lib_hack.h>  //included to provide correct 1-pass linking

#endif
