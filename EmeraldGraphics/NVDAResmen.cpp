//
//  NVDAResmen.cpp
//  EmeraldGraphics
//
//  Created by yoridan on 3.02.26.
//

#include "NVDAResmen.hpp"
#include <Headers/kern_api.hpp>
#include <Headers/kern_util.hpp>
#include <Headers/plugin_start.hpp>

static const char *kextPath[] = {
    "/System/Library/Extensions/NVDAResman.kext/Contents/MacOS/NVDAResman"
};

static KernelPatcher::KextInfo kextInfo[] = {
    { "com.apple.nvidia.driver.NVDAResman", kextPath, arrsize(kextPath), {}, {}, KernelPatcher::KextInfo::Unloaded }
};

static mach_vm_address_t orig_enableController = 0;
static mach_vm_address_t orig_sub_57436 = 0;
constexpr size_t OFFSET_RESOURCE_MGR =0x300;


//static uint64_t returnRax(void *this_ptr) {
//    DBGLOG(NVDBG, "=== enableController called ===");
//
//    typedef uint64_t (*func_t)(void*);
//    uint64_t result = reinterpret_cast<func_t>(orig_enableController)(this_ptr);
//
//    // RAX contains the return value
//    SYSLOG(NVDBG, "RAX (return value) = 0x%llx", result);
//
//    return result;
//}
static uint64_t hooked_sub_57436(uint32_t param){
    DBGLOG(NVDBG,"sub_57436 called with: 0x%x",param);
    typedef uint64_t (*func_t)(uint32_t);
    uint64_t result= reinterpret_cast<func_t>(orig_sub_57436)(param);
    DBGLOG(NVDBG,"sub_057436 returned: 0x%llx",result);
    return result;
           };

static uint64_t hooked_enableController(void *this_ptr) {
    DBGLOG("NVDADebugger", "=== enableController called ===");
    DBGLOG("NVDADebugger", "this = %p", this_ptr);
    
    if (!this_ptr) {
        SYSLOG("NVDADebugger", "ERROR: this pointer is NULL!");
        return 0xE00002BC;  // kIOReturnError
    }
    
    uint8_t *obj = reinterpret_cast<uint8_t*>(this_ptr);
    void **resource_mgr_ptr = reinterpret_cast<void**>(obj + OFFSET_RESOURCE_MGR);
    void *resource_mgr = *resource_mgr_ptr;
    
    DBGLOG("NVDADebugger", "Resource manager pointer (this+0x300) = %p", resource_mgr);
    
    if (!resource_mgr) {
        SYSLOG("NVDADebugger", "WARNING: Resource manager at offset 0x300 is NULL!");
        SYSLOG("NVDADebugger", "This is the root cause of the kernel panic.");
        SYSLOG("NVDADebugger", "Returning kIOReturnNotReady (0xE00002CD) to prevent crash");
        
        // Dump surrounding memory for debugging
        DBGLOG("NVDADebugger", "Memory dump around this+0x300:");
        uint64_t *mem = reinterpret_cast<uint64_t*>(obj + OFFSET_RESOURCE_MGR - 0x20);
        for (int i = 0; i < 8; i++) {
            DBGLOG("NVDADebugger", "  [this+0x%03x] = 0x%016llx",
                   OFFSET_RESOURCE_MGR - 0x20 + (i * 8), mem[i]);
        }
    
    return 0xE00002CD;
        
    }
    DBGLOG("NVDADebugger", "Resource manager pointer is VALID, calling original...");
        
    
        typedef uint64_t (*enableController_func_t)(void*);
        enableController_func_t original = reinterpret_cast<enableController_func_t>(orig_enableController);
        uint64_t result = original(this_ptr);
        
        DBGLOG("NVDADebugger", "Original enableController returned: 0x%llx", result);
        DBGLOG("NVDADebugger", "=== enableController completed ===");
        
        return result;
}

static void processKext(void *user, KernelPatcher &patcher, size_t index, mach_vm_address_t address, size_t size) {
    if (index != 0) {
        SYSLOG("NVDADebugger", "Invalid kext index %zu", index);
        return;
    }
    
    DBGLOG("NVDADebugger", "NVDAResman loaded at 0x%llx, size 0x%zx", address, size);
    
    mach_vm_address_t enableControllerAddr = patcher.solveSymbol(index, "__ZN4NVDA16enableControllerEv");
    
    if (enableControllerAddr) {
        SYSLOG("NVDADebugger", "Found NVDA::enableController at 0x%llx", enableControllerAddr);
        

        orig_enableController = patcher.routeFunction(
            enableControllerAddr,
            reinterpret_cast<mach_vm_address_t>(hooked_enableController),
            true
        );
        
        if (patcher.getError() == KernelPatcher::Error::NoError) {
            SYSLOG("NVDADebugger", "Successfully hooked enableController!");
            SYSLOG("NVDADebugger", "Original function saved at 0x%llx", orig_enableController);
        } else {
            SYSLOG("NVDADebugger", "Failed to hook enableController: error %d", patcher.getError());
            patcher.clearError();
        }
    } else {
        SYSLOG("NVDADebugger", "Could not find enableController symbol");
        
            }
    mach_vm_address_t sub_57436_addr = address + 0x57436;
        
        SYSLOG(NVDBG, "Attempting to hook sub_57436 at 0x%llx", sub_57436_addr);
        
        orig_sub_57436 = patcher.routeFunction(
            sub_57436_addr,
            reinterpret_cast<mach_vm_address_t>(hooked_sub_57436),
            true
        );
        
        if (patcher.getError() == KernelPatcher::Error::NoError) {
            SYSLOG(NVDBG, "Successfully hooked sub_57436!");
            SYSLOG(NVDBG, "Original sub_57436 saved at 0x%llx", orig_sub_57436);
        } else {
            SYSLOG(NVDBG, "Failed to hook sub_57436: error %d", patcher.getError());
            patcher.clearError();
        }
    }
}

void NVDAResman::init() {
    DBGLOG("NVDADebugger", "Registering NVDAResman patcher...");
    lilu.onKextLoad(kextInfo, arrsize(kextInfo), processKext);
}
