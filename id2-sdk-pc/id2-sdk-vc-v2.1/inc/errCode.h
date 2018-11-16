/**
 * Copyright (C) 2017 The YunOS IoT Project. All rights reserved.
 */

#ifndef _ERR_CODE_H
#define _ERR_CODE_H

#ifdef __cplusplus
extern "C" {
#endif

#define DS_STATUS_OK                 ( 0)
#define DS_STATUS_ERROR              (-1)    /* General error */
#define DS_CONFIG_LICENSE_INVALID    (-2)    /* use a wrong lincens in config */
#define DS_CONFIG_CHIPID_INVALID     (-3)    /* the chipid in license is invalid */
#define DS_CONFIG_CHIPNAME_INVALID   (-4)    /* the chipName in config is wrong */
#define DS_CONFIG_WRONG_PRIKEY       (-5)    /* provide wrong factprivatekey in config */
#define DS_SERVICE_UNAVAILABLE       (-6)    /* id2 server unvailable */
#define DS_ID2KEYS_USEUP             (-7)    /* id^2 key used up */
#define DS_LOCALDB_OPEN_FAILD        (-8)    /* create local db error */ 
#define DS_CREATE_SYNC_THREAD_ERROR  (-10)   /* create syncdata thread failed */
#define DS_DECRYPTE_ID2_ERROR        (-11)   /* id2 data decrypte error */

#ifdef __cplusplus
}
#endif

#endif /* _ERR_CODE_H */
