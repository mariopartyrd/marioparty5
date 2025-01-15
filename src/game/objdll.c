#include "game/objdll.h"
#include "game/dvd.h"
#include "game/memory.h"

typedef BOOL (*OMDLLPROLOG)(void);
typedef void (*OMDLLEPILOG)(void);

OMDLLINFO *omDLLinfoTbl[OMDLLINFO_MAX];

static OVLTBL *omDLLFileList;

void omDLLDBGOut(void)
{
	OSReport("DLL DBG OUT\n");
}

void omDLLInit(OVLTBL *ovlList)
{
	s32 i;
	OSReport("DLL DBG OUT\n");
	for(i=0; i<OMDLLINFO_MAX; i++) {
		omDLLinfoTbl[i] = NULL;
	}
	omDLLFileList = ovlList;
}

s32 omDLLStart(s16 dllno, s16 reloadF)
{
	s32 dll;
	OSReport("DLLStart %d %d\n", dllno, reloadF);
	dll = omDLLSearch(dllno);
	if(dll >= 0 && !reloadF) {
		OMDLLINFO *dllP = omDLLinfoTbl[dll];
		OSReport("objdll>Already Loaded %s(%08x %08x)\n", dllP->name, dllP->module, dllP->bss);
		
		omDLLInfoDump(&dllP->module->info);
		omDLLHeaderDump(dllP->module);
		memset(dllP->bss, 0, dllP->module->bssSize);
		HuMemDCFlushAll();
		dllP->ret = ((OMDLLPROLOG)dllP->module->prolog)();
		OSReport("objdll> %s prolog end\n", dllP->name);
		return dll;
	} else {
		for(dll=0; dll<OMDLLINFO_MAX; dll++) {
			if(omDLLinfoTbl[dll] == NULL) {
				break;
			}
		}
		if(dll == OMDLLINFO_MAX) {
			return -1;
		}
		omDLLLink(&omDLLinfoTbl[dll], dllno, TRUE);
		return dll;
	}
}

void omDLLNumEnd(s16 dllno, s16 reloadF)
{
	s16 dll;
	if(dllno < 0) {
		OSReport("objdll>omDLLNumEnd Invalid dllno %d\n", dllno);
		return;
	}
	OSReport("objdll>omDLLNumEnd %d %d\n", dllno, reloadF);
	dll = omDLLSearch(dllno);
	if(dll < 0) {
		OSReport("objdll>omDLLNumEnd not found DLL No%d\n", dllno);
		return;
	}
	omDLLEnd(dll, reloadF);
}

void omDLLEnd(s16 dll, s16 reloadF)
{
	OSReport("objdll>omDLLEnd %d %d\n", dll, reloadF);
	if(reloadF == TRUE) {
		OSReport("objdll>End DLL:%s\n", omDLLinfoTbl[dll]->name);
		omDLLUnlink(omDLLinfoTbl[dll], 1);
		omDLLinfoTbl[dll] = NULL;
	} else {
		OMDLLINFO *dllP = omDLLinfoTbl[dll];
		
		OSReport("objdll>Call Epilog\n");
		((OMDLLEPILOG)dllP->module->epilog)();
		OSReport("objdll>End DLL stayed:%s\n", omDLLinfoTbl[dll]->name);
	}
	OSReport("objdll>End DLL finish\n");
}

OMDLLINFO *omDLLLink(OMDLLINFO **dllInfoP, s16 dllno, s16 prologF)
{
	OMDLLINFO *dll;
	OVLTBL *dllFile = &omDLLFileList[dllno];
    static u8 ATTRIBUTE_ALIGN(32) strTable[1024]; //Needed for proper alignment of BSS in file
    
	OSReport("objdll>Link DLL:%s\n", dllFile->name);
	dll = HuMemDirectMalloc(HEAP_HEAP, sizeof(OMDLLINFO));
	*dllInfoP = dll;
	dll->name = dllFile->name;
	dll->module = HuDvdDataReadDirect(dllFile->name, HEAP_HEAP);
	dll->bss = HuMemDirectMalloc(HEAP_HEAP, dll->module->bssSize);
	if(OSLink(&dll->module->info, dll->bss) != TRUE) {
		OSReport("objdll>++++++++++++++++ DLL Link Failed\n");
	}
	omDLLInfoDump(&dll->module->info);
	omDLLHeaderDump(dll->module);
	OSReport("objdll>LinkOK %08x %08x\n", dll->module, dll->bss);
	if(prologF == TRUE) {
		OSReport("objdll> %s prolog start\n", dllFile->name);
		dll->ret = ((OMDLLPROLOG)dll->module->prolog)();
		OSReport("objdll> %s prolog end\n", dllFile->name);
	}
	return dll;
}

void omDLLUnlink(OMDLLINFO *dllInfo, s16 epilogF)
{
	OSReport("odjdll>Unlink DLL:%s\n", dllInfo->name);
	if(epilogF == TRUE) {
		OSReport("objdll>Unlink DLL epilog\n");
		((OMDLLEPILOG)dllInfo->module->epilog)();
		OSReport("objdll>Unlink DLL epilog finish\n");
	}
	if(OSUnlink(&dllInfo->module->info) != TRUE) {
		OSReport("objdll>+++++++++++++++++ DLL Unlink Failed\n");
	}
	HuMemDirectFree(dllInfo->bss);
	HuMemDirectFree(dllInfo->module);
	HuMemDirectFree(dllInfo);
}

s32 omDLLSearch(s16 dllno)
{
	s32 i;
	OVLTBL *dllFile = &omDLLFileList[dllno];
	OSReport("Search:%s\n", dllFile->name);
	for(i=0; i<OMDLLINFO_MAX; i++) {
		OMDLLINFO *dll = omDLLinfoTbl[i];
		if(dll != NULL && strcmp(dll->name, dllFile->name) == 0) {
			OSReport("+++++++++++ Find%d: %s\n", i, dll->name);
			return i;
		}
	}
	return -1;
}

void omDLLInfoDump(OSModuleInfo *module)
{
	OSReport("===== DLL Module Info dump ====\n");
	OSReport("                   ID:0x%08x\n", module->id);
	OSReport("             LinkPrev:0x%08x\n", module->link.prev);
	OSReport("             LinkNext:0x%08x\n", module->link.next);
	OSReport("          Section num:%d\n", module->numSections);
	OSReport("Section info tbl ofst:0x%08x\n", module->sectionInfoOffset);
	OSReport("           nameOffset:0x%08x\n", module->nameOffset);
	OSReport("             nameSize:%d\n", module->nameSize);
	OSReport("              version:0x%08x\n", module->version);
	OSReport("===============================\n");
}

void omDLLHeaderDump(OSModuleHeader *module)
{
	OSReport("==== DLL Module Header dump ====\n");
	OSReport("          bss Size:0x%08x\n", module->bssSize);
	OSReport("        rel Offset:0x%08x\n", module->relOffset);
	OSReport("        imp Offset:0x%08x\n", module->impOffset);
	OSReport("    prolog Section:%d\n", module->prologSection);
	OSReport("    epilog Section:%d\n", module->epilogSection);
	OSReport("unresolved Section:%d\n", module->unresolvedSection);
	OSReport("       prolog func:0x%08x\n", module->prolog);
	OSReport("       epilog func:0x%08x\n", module->epilog);
	OSReport("   unresolved func:0x%08x\n", module->unresolved);
	OSReport("================================\n");
}