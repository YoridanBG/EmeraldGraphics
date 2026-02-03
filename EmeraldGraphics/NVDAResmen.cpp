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

constexpr size_t OFFSET_RESOURCE_MGR = 0x300;  // this + 0x300

static uint64_t hooked_enableController(void *this_ptr) {
    DBGLOG("NVDADebugger", "=== enableController called ===");
    DBGLOG("NVDADebugger", "this = %p", this_ptr);
    
    if (!this_ptr) {
        SYSLOG("NVDADebugger", "ERROR: this pointer is NULL!");
        return 0xE00002BC;  // kIOReturnError
    }
    
    // Read the resource manager pointer at offset 0x300
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
        
        return 0xE00002CD;  // kIOReturnNotReady
    }
    
    DBGLOG("NVDADebugger", "Resource manager pointer is VALID, calling original...");
    
    // Call original function
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
    
    // Try to resolve the symbol
    mach_vm_address_t enableControllerAddr = patcher.solveSymbol(index, "__ZN4NVDA16enableControllerEv");
    
    if (enableControllerAddr) {
        SYSLOG("NVDADebugger", "Found NVDA::enableController at 0x%llx", enableControllerAddr);
        
        // Route (hook) the function
        orig_enableController = patcher.routeFunction(
            enableControllerAddr,
            reinterpret_cast<mach_vm_address_t>(hooked_enableController),
            true  // buildWrapper
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
        
        // Try pattern-based hooking as fallback
        DBGLOG("NVDADebugger", "Attempting pattern-based hooking...");
        
        // Look for the function prologue and known instruction sequence
        const uint8_t pattern[] = {
            0x55,                           // push rbp
            0x48, 0x89, 0xE5,              // mov rbp, rsp
            0x41, 0x57,                     // push r15
            0x41, 0x56,                     // push r14
            0x41, 0x55,                     // push r13
            0x41, 0x54,                     // push r12
            0x53,                           // push rbx
            0x50                            // push rax
        };
        
        mach_vm_address_t found = 0;
        for (size_t offset = 0; offset < size - sizeof(pattern); offset++) {
            if (memcmp(reinterpret_cast<void*>(address + offset), pattern, sizeof(pattern)) == 0) {
                // Verify this looks like enableController by checking nearby code
                found = address + offset;
                DBGLOG("NVDADebugger", "Found potential enableController at offset 0x%zx", offset);
                break;
            }
        }
        
        if (found) {
            orig_enableController = patcher.routeFunction(
                found,
                reinterpret_cast<mach_vm_address_t>(hooked_enableController),
                true
            );
            
            if (patcher.getError() == KernelPatcher::Error::NoError) {
                SYSLOG("NVDADebugger", "Successfully hooked via pattern matching!");
            }
        }
    }
}
void NVDAResman::init() {
    DBGLOG("NVDADebugger", "Registering NVDAResman patcher...");
    
    // Register the kext load callback with Lilu
    // kextInfo and processKext are the static variables/functions defined above
    lilu.onKextLoad(kextInfo, arrsize(kextInfo), processKext);
}
