// 
//  kern_start.hpp
//  EmeraldGraphics
// 
//  Created by RoyalGraphX on 1/20/26.
//

#ifndef kern_start_h
#define kern_start_h

// Base Includes
#include <Headers/plugin_start.hpp>
#include <Headers/kern_patcher.hpp>
#include <Headers/kern_devinfo.hpp>
#include <IOKit/IORegistryEntry.h>
#include <Headers/kern_nvram.hpp>
#include <libkern/c++/OSString.h>
#include <Headers/kern_util.hpp>
#include <Headers/kern_mach.hpp>
#include <Headers/kern_efi.hpp>
#include <mach/i386/vm_types.h>
#include <Headers/kern_api.hpp>
#include <libkern/libkern.h>
#include <IOKit/IOLib.h>
#include <sys/sysctl.h>
#include <i386/cpuid.h>

// Logging Defs
#define MODULE_INIT "INIT"
#define MODULE_SHORT "EG"
#define MODULE_LONG "EmeraldGraphics"
#define MODULE_ERROR "ERR"
#define MODULE_WARN "WARN"
#define MODULE_INFO "INFO"
#define MODULE_CUTE "\u2665"

#define NVDBG "NVDBG"

// Root/Parent Class
class EMRLD {
public:
    
    /**
     * Standard functions
     */
    void init();
    void deinit();
    
    /**
    * Publicly accessible internal build flag
    */
    static const bool IS_INTERNAL;
    
    /**
     * Must be populated by EMRLD::init() before can read globally
     */
    static int darwinMajor;
    static int darwinMinor;
    
private:
    
    /**
     * Private self instance for callbacks
     */
    static EMRLD *callbackEMRLD;
	
};

#endif /* kern_start_h */

#ifndef EMRLD_VERSION /*EMRLD_VERSION Macro */
#define EMRLD_VERSION "Unknown"

#endif /* EMRLD_VERSION Macro */
