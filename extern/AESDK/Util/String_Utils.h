/*******************************************************************/
/*                                                                 */
/*                      ADOBE CONFIDENTIAL                         */
/*                   _ _ _ _ _ _ _ _ _ _ _ _ _                     */
/*                                                                 */
/* Copyright 2007 Adobe Systems Incorporated                       */
/* All Rights Reserved.                                            */
/*                                                                 */
/* NOTICE:  All information contained herein is, and remains the   */
/* property of Adobe Systems Incorporated and its suppliers, if    */
/* any.  The intellectual and technical concepts contained         */
/* herein are proprietary to Adobe Systems Incorporated and its    */
/* suppliers and may be covered by U.S. and Foreign Patents,       */
/* patents in process, and are protected by trade secret or        */
/* copyright law.  Dissemination of this information or            */
/* reproduction of this material is strictly forbidden unless      */
/* prior written permission is obtained from Adobe Systems         */
/* Incorporated.                                                   */
/*                                                                 */
/*******************************************************************/



/* String_Utils.h  */


#pragma once 

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>

#ifdef __cplusplus
extern "C" {
#endif
std::string	GetStringPtr(int strNum);
#ifdef __cplusplus
}
#endif

#define	STR(_foo) GetStringPtr(_foo).data()







#endif /* STRING_UTILS_H */