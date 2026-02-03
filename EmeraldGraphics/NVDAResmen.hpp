//
//  NVDAResmen.hpp
//  EmeraldGraphics
//
//  Created by yoridan on 3.02.26.
//

#ifndef NVDAResmen_hpp
#define NVDAResmen_hpp

#include <Headers/kern_patcher.hpp>
#include <Headers/kern_util.hpp>
#include <Headers/kern_api.hpp>
#include <libkern/c++/OSString.h>
#include <libkern/c++/OSObject.h>
#include <libkern/c++/OSData.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/IOService.h>
#include <mach/vm_map.h>
#include <IOKit/IOLib.h>
#include <sys/proc.h>
#include "kern_start.hpp"

// Define a class to manage the NVDA Patcher
class NVDAResman {
public:
    static void init();
};

#endif /* NVDAResmen_hpp */
