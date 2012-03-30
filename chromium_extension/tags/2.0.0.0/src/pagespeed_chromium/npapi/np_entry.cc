/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

//////////////////////////////////////////////////////////////
//
// Main plugin entry point implementation
//

#ifdef _WINDOWS
#include <windows.h>
#endif

#include <stdio.h>
#include "third_party/npapi/npapi.h"
#include "third_party/npapi/npfunctions.h"

#include "pagespeed_chromium/pagespeed_npapi.h"

extern "C" {

#ifndef HIBYTE
#define HIBYTE(x) ((static_cast<unsigned short>(x) & 0xff00) >> 8)
#endif

// Export our module so Chrome is able to load us.
// See http://gcc.gnu.org/wiki/Visibility for more information.
#if defined(__GNUC__)
#pragma GCC visibility push(default)
#endif

NPError OSCALL NP_GetEntryPoints(NPPluginFuncs* nppfuncs) {
  nppfuncs->version = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
  nppfuncs->newp = NPP_New;
  nppfuncs->destroy = NPP_Destroy;
  nppfuncs->getvalue = NPP_GetValue;
  nppfuncs->setwindow = NPP_SetWindow;
  nppfuncs->event = NPP_HandleEvent;
  nppfuncs->newstream = NPP_NewStream;
  nppfuncs->destroystream = NPP_DestroyStream;
  return NPERR_NO_ERROR;
}

NPError OSCALL NP_Initialize(NPNetscapeFuncs* npnf
#if !defined(_WINDOWS) && !defined(WEBKIT_DARWIN_SDK)
                             , NPPluginFuncs *nppfuncs
#endif
                             ) {
  if (npnf == NULL) {
    return NPERR_INVALID_FUNCTABLE_ERROR;
  }
  if (HIBYTE(npnf->version) > NP_VERSION_MAJOR) {
    return NPERR_INCOMPATIBLE_VERSION_ERROR;
  }
  npnfuncs = npnf;
#if !defined(_WINDOWS) && !defined(WEBKIT_DARWIN_SDK)
  NP_GetEntryPoints(nppfuncs);
#endif
  InitializePageSpeedPlugin();
  return NPERR_NO_ERROR;
}

NPError  OSCALL NP_Shutdown() {
  ShutDownPageSpeedPlugin();
  return NPERR_NO_ERROR;
}

const char* NP_GetMIMEDescription(void) {
  return const_cast<char*>("application/x-page-speed::Page Speed Plugin");
}

// Needs to be present for WebKit based browsers.
NPError OSCALL NP_GetValue(void* npp, NPPVariable variable, void* value) {
  return NPP_GetValue(static_cast<NPP>(npp), variable, value);
}

#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

}  // extern "C"
