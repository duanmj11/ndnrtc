//
//  ndnrtc-library.cpp
//  ndnrtc
//
//  Copyright 2013 Regents of the University of California
//  For licensing details see the LICENSE file.
//
//  Author:  Peter Gusev
//

#include "ndnrtc-library.h"
#include "ndnrtc-manager.h"

using namespace ndnrtc;

INdnRtcLibrary& NdnRtcLibrary::getSharedInstance()
{
    return NdnRtcManager::getSharedInstance();
}
