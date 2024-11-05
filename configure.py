#!/usr/bin/env python3

###
# Generates build files for the project.
# This file also includes the project configuration,
# such as compiler flags and the object matching status.
#
# Usage:
#   python3 configure.py
#   ninja
#
# Append --help to see available options.
###

import argparse
import sys
from pathlib import Path
from typing import Any, Dict, List

from tools.project import (
    Object,
    ProjectConfig,
    calculate_progress,
    generate_build,
    is_windows,
)

# Game versions
DEFAULT_VERSION = 0
VERSIONS = [
    "GP5E01",  # 0
]

parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "-v",
    "--version",
    choices=VERSIONS,
    type=str.upper,
    default=VERSIONS[DEFAULT_VERSION],
    help="version to build",
)
parser.add_argument(
    "--build-dir",
    metavar="DIR",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--binutils",
    metavar="BINARY",
    type=Path,
    help="path to binutils (optional)",
)
parser.add_argument(
    "--compilers",
    metavar="DIR",
    type=Path,
    help="path to compilers (optional)",
)
parser.add_argument(
    "--map",
    action="store_true",
    help="generate map file(s)",
)
parser.add_argument(
    "--no-asm",
    action="store_true",
    help="don't incorporate .s files from asm directory",
)
parser.add_argument(
    "--debug",
    action="store_true",
    help="build with debug info (non-matching)",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )
parser.add_argument(
    "--dtk",
    metavar="BINARY | DIR",
    type=Path,
    help="path to decomp-toolkit binary or source (optional)",
)
parser.add_argument(
    "--sjiswrap",
    metavar="EXE",
    type=Path,
    help="path to sjiswrap.exe (optional)",
)
parser.add_argument(
    "--verbose",
    action="store_true",
    help="print verbose output",
)
parser.add_argument(
    "--non-matching",
    dest="non_matching",
    action="store_true",
    help="builds equivalent (but non-matching) or modded objects",
)
args = parser.parse_args()

config = ProjectConfig()
config.version = str(args.version)
version_num = VERSIONS.index(config.version)

# Apply arguments
config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.debug = args.debug
config.generate_map = args.map
config.non_matching = args.non_matching
config.sjiswrap_path = args.sjiswrap
if not is_windows():
    config.wrapper = args.wrapper
if args.no_asm:
    config.asm_dir = None

# Tool versions
config.binutils_tag = "2.42-1"
config.compilers_tag = "20231018"
config.dtk_tag = "v0.9.2"
config.sjiswrap_tag = "v1.1.1"
config.wibo_tag = "0.6.11"

# Project
config.config_path = Path("config") / config.version / "config.yml"
config.check_sha_path = Path("config") / config.version / "build.sha1"
config.asflags = [
    "-mgekko",
    "--strip-local-absolute",
    "-I include",
    f"-I build/{config.version}/include",
    f"--defsym version={version_num}",
]
config.ldflags = [
    "-fp hardware",
    "-nodefaults",
    "-warn off",
    # "-listclosure", # Uncomment for Wii linkers
]
# Use for any additional files that should cause a re-configure when modified
config.reconfig_deps = []

# Base flags, common to most GC/Wii games.
# Generally leave untouched, with overrides added below.
cflags_base = [
    "-nodefaults",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-Cpp_exceptions off",
    # "-W all",
    "-O4,p",
    "-inline auto",
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    "-maxerrors 1",
    "-nosyspath",
    "-RTTI off",
    "-fp_contract on",
    "-str reuse",
    "-multibyte",  # For Wii compilers, replace with `-enc SJIS`
    "-i include",
    "-DMUSY_TARGET=MUSY_TARGET_DOLPHIN",
    f"-i build/{config.version}/include",
    f"-DVERSION={version_num}",
]

# Debug flags
if config.debug:
    cflags_base.extend(["-sym on", "-DDEBUG=1"])
else:
    cflags_base.append("-DNDEBUG=1")

# Metrowerks library flags
cflags_runtime = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-gccinc",
    "-common off",
    "-inline auto",
]
# Metrowerks library flags
cflags_msl = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-common off",
    "-inline auto,deferred",
]

# Metrowerks library flags
cflags_trk = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-common off",
    "-sdata 0",
    "-sdata2 0",
    "-inline auto,deferred",
]

cflags_odemuexi = [
    *cflags_base,
    "-inline auto,deferred",
]

cflags_amcstub = [
    *cflags_base,
    "-inline auto,deferred",
]

cflags_odenotstub = [
    *cflags_base,
    "-inline auto,deferred",
]

cflags_musyx = [
    "-proc gekko",
    "-nodefaults",
    "-nosyspath",
    "-i include",
    "-inline auto",
    "-O4,p",
    "-fp hard",
    "-enum int",
    "-Cpp_exceptions off",
    "-str reuse,pool,readonly",
    "-fp_contract off",
    "-DMUSY_TARGET=MUSY_TARGET_DOLPHIN",
]

# REL flags
cflags_rel = [
    *cflags_base,
    "-sdata 0",
    "-sdata2 0",
]

# Game flags
cflags_game = [
    *cflags_base,
    "-O0,p",
    "-char unsigned",
    "-fp_contract off",
]

# Zlib flags
cflags_zlib = [
    *cflags_base,
    "-O0,p",
    "-fp_contract off",
]

# Game flags
cflags_libhu = [
    *cflags_base,
    "-O0,p",
    "-char unsigned",
    "-fp_contract off",
]

# Game flags
cflags_msm = [
    *cflags_base,
]

config.linker_version = "GC/2.6"
config.rel_strip_partial = False
config.rel_empty_file = "REL/empty.c"

# Helper function for Dolphin libraries
def DolphinLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.2.5n",
        "cflags": cflags_base,
        "host": False,
        "objects": objects,
    }

def MusyX(objects, mw_version="GC/1.3.2", debug=False, major=2, minor=0, patch=4):
    cflags = cflags_musyx if not debug else cflags_musyx_debug
    return {
        "lib": "musyx",
        "mw_version": mw_version,
        "host": False,
        "cflags": [
            *cflags,
            f"-DMUSY_VERSION_MAJOR={major}",
            f"-DMUSY_VERSION_MINOR={minor}",
            f"-DMUSY_VERSION_PATCH={patch}",
        ],
        "objects": objects,
    }
   
# Helper function for REL script objects
def Rel(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.3.2",
        "cflags": cflags_rel,
        "host": True,
        "objects": objects,
    }


Matching = True                   # Object matches and should be linked
NonMatching = False               # Object does not match and should not be linked
Equivalent = config.non_matching  # Object should be linked when configured with --non-matching

config.warn_missing_config = True
config.warn_missing_source = False
config.libs = [
    {
        "lib": "Game",
        "mw_version": config.linker_version,
        "cflags": cflags_game,
        "host": False,
        "objects": [
            Object(NonMatching, "game/main.c"),
            Object(Matching, "game/pad.c"),
            Object(Matching, "game/dvd.c"),
            Object(Matching, "game/data.c"),
            Object(Matching, "game/decode.c"),
            Object(Matching, "game/font.c"),
            Object(Matching, "game/init.c"),
            Object(Matching, "game/jmp.c"),
            Object(Matching, "game/malloc.c"),
            Object(Matching, "game/memory.c"),
            Object(Matching, "game/printfunc.c"),
            Object(Matching, "game/process.c"),
            Object(Matching, "game/sprman.c"),
            Object(Matching, "game/sprput.c"),
            Object(Matching, "game/hsfload.c"),
            Object(NonMatching, "game/hsfdraw.c"),
            Object(Matching, "game/hsfman.c"),
            Object(Matching, "game/hsfmotion.c"),
            Object(Matching, "game/hsfanim.c"),
            Object(NonMatching, "game/hsfex.c"),
            Object(Matching, "game/perf.c"),
            Object(Matching, "game/objmain.c"),
            Object(Matching, "game/fault.c"),
            Object(Matching, "game/gamework.c"),
            Object(Matching, "game/objsysobj.c"),
            Object(Matching, "game/objdll.c"),
            Object(Matching, "game/frand.c"),
            Object(Matching, "game/audio.c"),
            Object(Matching, "game/EnvelopeExec.c"),
            Object(NonMatching, "game/gamemes.c"),
            Object(Matching, "game/ovllist.c"),
            Object(Matching, "game/esprite.c"),
            Object(Matching, "game/ClusterExec.c"),
            Object(Matching, "game/ShapeExec.c"),
            Object(Matching, "game/wipe.c"),
            Object(Matching, "game/window.c"),
            Object(Matching, "game/messdata.c"),
            Object(Matching, "game/card.c"),
            Object(Matching, "game/armem.c"),
            Object(Matching, "game/charman.c"),
            Object(Matching, "game/mapspace.c"),
            Object(Matching, "game/THPSimple.c"),
            Object(Matching, "game/THPDraw.c"),
            Object(Matching, "game/thpmain.c"),
            Object(Matching, "game/objsub.c"),
            Object(Matching, "game/flag.c"),
            Object(NonMatching, "game/saveload.c"),
            Object(Matching, "game/sreset.c"),
            Object(NonMatching, "game/mg/timer.c"),
            Object(NonMatching, "game/mg/score.c"),
            Object(NonMatching, "game/mg/seqman.c"),
            Object(NonMatching, "game/mg/colman.c"),
            Object(NonMatching, "game/mg/actman.c"),
            Object(NonMatching, "game/mg/mess.c"),
            Object(Matching, "game/board/pausewatch.c"),
            Object(Matching, "game/board/main.c"),
            Object(NonMatching, "game/board/camera.c"),
            Object(NonMatching, "game/board/player.c"),
            Object(Matching, "game/board/model.c"),
            Object(NonMatching, "game/board/window.c"),
            Object(NonMatching, "game/board/audio.c"),
            Object(NonMatching, "game/board/com.c"),
            Object(NonMatching, "game/board/scroll.c"),
            Object(NonMatching, "game/board/masu.c"),
            Object(NonMatching, "game/board/coin.c"),
            Object(NonMatching, "game/board/star.c"),
            Object(NonMatching, "game/board/padall.c"),
            Object(NonMatching, "game/board/sai.c"),
            Object(NonMatching, "game/board/status.c"),
            Object(NonMatching, "game/board/opening.c"),
            Object(NonMatching, "game/board/pause.c"),
            Object(NonMatching, "game/board/tutorial.c"),
            Object(NonMatching, "game/board/roulette.c"),
            Object(NonMatching, "game/board/capmachine.c"),
            Object(NonMatching, "game/board/capsule.c"),
            Object(NonMatching, "game/board/capmain.c"),
            Object(NonMatching, "game/board/guide.c"),
            Object(NonMatching, "game/board/path.c"),
            Object(NonMatching, "game/board/mg.c"),
            Object(NonMatching, "game/board/effect.c"),
            Object(NonMatching, "game/board/pauseoption.c"),
            Object(NonMatching, "game/board/gate.c"),
            Object(NonMatching, "game/board/story.c"),
            Object(NonMatching, "game/board/telop.c"),
            Object(NonMatching, "game/board/mgcircuit.c"),
            
            Object(NonMatching, "game/code_801A7E90.c"),
        ],
    },
    {
        "lib": "zlib",
        "mw_version": config.linker_version,
        "cflags": cflags_zlib,
        "host": False,
        "objects": [
            Object(Matching, "zlib/adler32.c"),
            Object(Matching, "zlib/inflate.c"),
            Object(Matching, "zlib/infblock.c"),
            Object(Matching, "zlib/infcodes.c"),
            Object(Matching, "zlib/infutil.c"),
            Object(Matching, "zlib/inftrees.c"),
            Object(Matching, "zlib/inffast.c"),
            Object(Matching, "zlib/zutil.c"),
        ],
    },
    DolphinLib(
        "base",
        [
            Object(NonMatching, "dolphin/base/PPCArch.c"),
        ],
    ),
    DolphinLib(
        "os",
        [
            Object(NonMatching, "dolphin/os/OS.c"),
            Object(NonMatching, "dolphin/os/OSAlarm.c"),
            Object(NonMatching, "dolphin/os/OSAlloc.c"),
            Object(NonMatching, "dolphin/os/OSArena.c"),
            Object(NonMatching, "dolphin/os/OSAudioSystem.c"),
            Object(NonMatching, "dolphin/os/OSCache.c"),
            Object(NonMatching, "dolphin/os/OSContext.c"),
            Object(NonMatching, "dolphin/os/OSError.c"),
            Object(NonMatching, "dolphin/os/OSFont.c"),
            Object(NonMatching, "dolphin/os/OSInterrupt.c"),
            Object(NonMatching, "dolphin/os/OSLink.c"),
            Object(NonMatching, "dolphin/os/OSMessage.c"),
            Object(NonMatching, "dolphin/os/OSMemory.c"),
            Object(NonMatching, "dolphin/os/OSMutex.c"),
            Object(NonMatching, "dolphin/os/OSReboot.c"),
            Object(NonMatching, "dolphin/os/OSReset.c"),
            Object(NonMatching, "dolphin/os/OSResetSW.c"),
            Object(NonMatching, "dolphin/os/OSRtc.c"),
            Object(NonMatching, "dolphin/os/OSSemaphore.c"),
            Object(NonMatching, "dolphin/os/OSStopwatch.c"),
            Object(NonMatching, "dolphin/os/OSSync.c"),
            Object(NonMatching, "dolphin/os/OSThread.c"),
            Object(NonMatching, "dolphin/os/OSTime.c"),
            Object(NonMatching, "dolphin/os/__start.c"),
            Object(NonMatching, "dolphin/os/__ppc_eabi_init.c"),
        ],
    ),
    DolphinLib(
        "db",
        [
            Object(NonMatching, "dolphin/db/db.c"),
        ],
    ),
    DolphinLib(
        "mtx",
        [
            Object(NonMatching, "dolphin/mtx/mtx.c"),
            Object(NonMatching, "dolphin/mtx/mtxvec.c"),
            Object(NonMatching, "dolphin/mtx/mtx44.c"),
            Object(NonMatching, "dolphin/mtx/vec.c"),
            Object(NonMatching, "dolphin/mtx/quat.c"),
            Object(NonMatching, "dolphin/mtx/psmtx.c"),
        ],
    ),
    DolphinLib(
        "dvd",
        [
            Object(NonMatching, "dolphin/dvd/dvdlow.c"),
            Object(NonMatching, "dolphin/dvd/dvdfs.c"),
            Object(NonMatching, "dolphin/dvd/dvd.c"),
            Object(NonMatching, "dolphin/dvd/dvdqueue.c"),
            Object(NonMatching, "dolphin/dvd/dvderror.c"),
            Object(NonMatching, "dolphin/dvd/dvdidutils.c"),
            Object(NonMatching, "dolphin/dvd/dvdFatal.c"),
            Object(NonMatching, "dolphin/dvd/fstload.c"),
        ],
    ),
    DolphinLib(
        "vi",
        [
            Object(NonMatching, "dolphin/vi/vi.c"),
        ],
    ),
    DolphinLib(
        "demo",
        [
            Object(NonMatching, "dolphin/demo/DEMOInit.c"),
            Object(NonMatching, "dolphin/demo/DEMOFont.c"),
            Object(NonMatching, "dolphin/demo/DEMOPuts.c"),
            Object(NonMatching, "dolphin/demo/DEMOStats.c"),
        ],
    ),
    DolphinLib(
        "pad",
        [
            Object(NonMatching, "dolphin/pad/Padclamp.c"),
            Object(NonMatching, "dolphin/pad/Pad.c"),
        ],
    ),
    DolphinLib(
        "ai",
        [
            Object(NonMatching, "dolphin/ai/ai.c"),
        ],
    ),
    DolphinLib(
        "ar",
        [
            Object(NonMatching, "dolphin/ar/ar.c"),
            Object(NonMatching, "dolphin/ar/arq.c"),
        ],
    ),
    DolphinLib(
        "dsp",
        [
            Object(NonMatching, "dolphin/dsp/dsp.c"),
            Object(NonMatching, "dolphin/dsp/dsp_debug.c"),
            Object(NonMatching, "dolphin/dsp/dsp_task.c"),
        ],
    ),
    DolphinLib(
        "gx",
        [
            Object(NonMatching, "dolphin/gx/GXInit.c"),
            Object(NonMatching, "dolphin/gx/GXFifo.c"),
            Object(NonMatching, "dolphin/gx/GXAttr.c"),
            Object(NonMatching, "dolphin/gx/GXMisc.c"),
            Object(NonMatching, "dolphin/gx/GXGeometry.c"),
            Object(NonMatching, "dolphin/gx/GXFrameBuf.c"),
            Object(NonMatching, "dolphin/gx/GXLight.c"),
            Object(NonMatching, "dolphin/gx/GXTexture.c"),
            Object(NonMatching, "dolphin/gx/GXBump.c"),
            Object(NonMatching, "dolphin/gx/GXTev.c"),
            Object(NonMatching, "dolphin/gx/GXPixel.c"),
            Object(NonMatching, "dolphin/gx/GXStubs.c"),
            Object(NonMatching, "dolphin/gx/GXDisplayList.c"),
            Object(NonMatching, "dolphin/gx/GXTransform.c"),
            Object(NonMatching, "dolphin/gx/GXPerf.c"),
        ],
    ),
    DolphinLib(
        "card",
        [
            Object(NonMatching, "dolphin/card/CARDBios.c"),
            Object(NonMatching, "dolphin/card/CARDUnlock.c"),
            Object(NonMatching, "dolphin/card/CARDRdwr.c"),
            Object(NonMatching, "dolphin/card/CARDBlock.c"),
            Object(NonMatching, "dolphin/card/CARDDir.c"),
            Object(NonMatching, "dolphin/card/CARDCheck.c"),
            Object(NonMatching, "dolphin/card/CARDMount.c"),
            Object(NonMatching, "dolphin/card/CARDFormat.c"),
            Object(NonMatching, "dolphin/card/CARDOpen.c"),
            Object(NonMatching, "dolphin/card/CARDCreate.c"),
            Object(NonMatching, "dolphin/card/CARDRead.c"),
            Object(NonMatching, "dolphin/card/CARDWrite.c"),
            Object(NonMatching, "dolphin/card/CARDDelete.c"),
            Object(NonMatching, "dolphin/card/CARDStat.c"),
            Object(NonMatching, "dolphin/card/CARDNet.c"),
        ],
    ),
    DolphinLib(
        "exi",
        [
            Object(NonMatching, "dolphin/exi/EXIBios.c"),
            Object(NonMatching, "dolphin/exi/EXIUart.c"),
        ],
    ),
    DolphinLib(
        "si",
        [
            Object(NonMatching, "dolphin/si/SIBios.c"),
            Object(NonMatching, "dolphin/si/SISamplingRate.c"),
        ],
    ),
    DolphinLib(
        "thp",
        [
            Object(NonMatching, "dolphin/thp/THPDec.c"),
            Object(NonMatching, "dolphin/thp/THPAudio.c"),
        ],
    ),
    {
        "lib": "Runtime.PPCEABI.H",
        "mw_version": config.linker_version,
        "cflags": cflags_runtime,
        "host": False,
        "objects": [
            Object(NonMatching, "Runtime.PPCEABI.H/__va_arg.c"),
            Object(NonMatching, "Runtime.PPCEABI.H/global_destructor_chain.c"),
            Object(NonMatching, "Runtime.PPCEABI.H/__mem.c"),
            Object(NonMatching, "Runtime.PPCEABI.H/New.cp"),
            Object(NonMatching, "Runtime.PPCEABI.H/NewMore.cp"),
            Object(NonMatching, "Runtime.PPCEABI.H/NMWException.cp"),
            Object(NonMatching, "Runtime.PPCEABI.H/runtime.c"),
            Object(NonMatching, "Runtime.PPCEABI.H/__init_cpp_exceptions.cpp"),
            Object(NonMatching, "Runtime.PPCEABI.H/Gecko_ExceptionPPC.cp"),
            Object(NonMatching, "Runtime.PPCEABI.H/GCN_mem_alloc.c"),
        ],
    },
    {
        "lib": "MSL_C.PPCEABI.bare.H",
        "mw_version": config.linker_version,
        "cflags": cflags_msl,
        "host": False,
        "objects": [
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/abort_exit.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/alloc.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/errno.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/ansi_files.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/ansi_fp.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/arith.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/buffer_io.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/ctype.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/direct_io.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/file_io.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/FILE_POS.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/mbstring.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/mem.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/mem_funcs.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/misc_io.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/printf.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/float.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/signal.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/string.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/uart_console_io.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/wchar_io.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/e_acos.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/e_asin.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/e_atan2.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/e_fmod.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/e_pow.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/e_rem_pio2.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/k_cos.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/k_rem_pio2.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/k_sin.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/k_tan.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/s_atan.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/s_copysign.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/s_cos.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/s_floor.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/s_frexp.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/s_ldexp.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/s_modf.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/s_sin.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/s_tan.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/w_acos.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/w_asin.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/w_atan2.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/w_fmod.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/w_pow.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/math_ppc.c"),
        ],
    },
    {
        "lib": "TRK_MINNOW_DOLPHIN",
        "mw_version": config.linker_version,
        "cflags": cflags_trk,
        "host": False,
        "objects": [
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/mainloop.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/nubevent.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/nubinit.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/msg.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/msgbuf.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/serpoll.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/usrput.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/dispatch.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/msghndlr.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/support.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/mutex_TRK.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/notify.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/flush_cache.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/mem_TRK.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/targimpl.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/targsupp.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/__exception.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/dolphin_trk.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/mpc_7xx_603e.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/main_TRK.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/dolphin_trk_glue.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/targcont.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/target_options.c"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/mslsupp.c"),
        ],
    },
    MusyX(
        objects={
            Object(Matching, "musyx/runtime/seq.c"),
            Object(NonMatching, "musyx/runtime/synth.c"),
            Object(Matching, "musyx/runtime/seq_api.c"),
            Object(Matching, "musyx/runtime/snd_synthapi.c"),
            Object(NonMatching, "musyx/runtime/stream.c"),
            Object(NonMatching, "musyx/runtime/synthdata.c"),
            Object(NonMatching, "musyx/runtime/synthmacros.c"),
            Object(NonMatching, "musyx/runtime/synthvoice.c"),
            Object(Matching, "musyx/runtime/synth_ac.c"),
            Object(Matching, "musyx/runtime/synth_dbtab.c"),
            Object(Matching, "musyx/runtime/synth_adsr.c"),
            Object(Matching, "musyx/runtime/synth_vsamples.c"),
            Object(NonMatching, "musyx/runtime/s_data.c"),
            Object(NonMatching, "musyx/runtime/hw_dspctrl.c"),
            Object(Matching, "musyx/runtime/hw_volconv.c"),
            Object(NonMatching, "musyx/runtime/snd3d.c"),
            Object(NonMatching, "musyx/runtime/snd_init.c"),
            Object(Matching, "musyx/runtime/snd_math.c"),
            Object(NonMatching, "musyx/runtime/snd_midictrl.c"),
            Object(Matching, "musyx/runtime/snd_service.c"),
            Object(NonMatching, "musyx/runtime/hardware.c"),
            Object(NonMatching, "musyx/runtime/dsp_import.c"),
            Object(NonMatching, "musyx/runtime/hw_aramdma.c"),
            Object(Matching, "musyx/runtime/hw_dolphin.c"),
            Object(Matching, "musyx/runtime/hw_memory.c"),
            Object(Matching, "musyx/runtime/CheapReverb/creverb_fx.c"),
            Object(Matching, "musyx/runtime/CheapReverb/creverb.c"),
            Object(Matching, "musyx/runtime/StdReverb/reverb_fx.c"),
            Object(NonMatching, "musyx/runtime/StdReverb/reverb.c"),
            Object(Matching, "musyx/runtime/Delay/delay_fx.c"),
            Object(Matching, "musyx/runtime/Chorus/chorus_fx.c"),
        }
    ),
    {
        "lib": "OdemuExi2",
        "mw_version": config.linker_version,
        "cflags": cflags_odemuexi,
        "host": False,
        "objects": [
            Object(NonMatching, "OdemuExi2/DebuggerDriver.c"),
        ],
    },
    {
        "lib": "amcstubs",
        "mw_version": config.linker_version,
        "cflags": cflags_amcstub,
        "host": False,
        "objects": [
            Object(NonMatching, "amcstubs/AmcExi2Stubs.c"),
        ],
    },
    {
        "lib": "odenotstub",
        "mw_version": config.linker_version,
        "cflags": cflags_odenotstub,
        "host": False,
        "objects": [
            Object(NonMatching, "odenotstub/odenotstub.c"),
        ],
    },
    {
        "lib": "libhu",
        "mw_version": config.linker_version,
        "cflags": cflags_libhu,
        "host": False,
        "objects": [
            Object(NonMatching, "libhu/setvf.c"),
            Object(NonMatching, "libhu/subvf.c"),
        ],
    },
    {
        "lib": "msm",
        "mw_version": "GC/1.2.5n",
        "cflags": cflags_msm,
        "host": False,
        "objects": [
            Object(NonMatching, "msm/msmsys.c"),
            Object(NonMatching, "msm/msmmem.c"),
            Object(NonMatching, "msm/msmfio.c"),
            Object(NonMatching, "msm/msmmus.c"),
            Object(NonMatching, "msm/msmse.c"),
            Object(NonMatching, "msm/msmstream.c"),
        ],
    },
    {
        "lib": "REL",
        "mw_version": config.linker_version,
        "cflags": cflags_rel,
        "host": False,
        "objects": [
            Object(Matching, "REL/empty.c"),  # Must be marked as matching
            Object(NonMatching, "REL/global_destructor_chain.c"),
            Object(NonMatching, "REL/ptmf.c"),
        ],
    },
    {
        "lib": "nintendo_data",
        "mw_version": config.linker_version,
        "cflags": cflags_rel,
        "host": False,
        "objects": [
            Object(NonMatching, "REL/nintendo/data.c"),
        ],
    },
    Rel(
        "actmanDLL",
        objects={
            Object(NonMatching, "REL/actmanDLL/actman.c"),
        },
    ),
    Rel(
        "bootDll",
        objects={
            Object(NonMatching, "REL/bootDll/boot.c"),
        },
    ),
    Rel(
        "carddll",
        objects={
            Object(NonMatching, "REL/carddll/cardmain.c"),
            Object(NonMatching, "REL/carddll/card.c"),
            Object(NonMatching, "REL/carddll/player.c"),
            Object(NonMatching, "REL/carddll/camera.c"),
            Object(NonMatching, "REL/carddll/dice.c"),
            Object(NonMatching, "REL/carddll/status.c"),
        },
    ),
    Rel(
        "decathlonDll",
        objects={
            Object(NonMatching, "REL/decathlonDll/mgtournament.c"),
            Object(NonMatching, "REL/decathlonDll/score.c"),
            Object(NonMatching, "REL/decathlonDll/mess.c"),
        },
    ),
    Rel(
        "e3bootdll",
        objects={
            Object(NonMatching, "REL/e3bootdll/e3boot.c"),
        },
    ),
    Rel(
        "e3setupdll",
        objects={
            Object(NonMatching, "REL/e3setupdll/e3setup.c"),
            Object(NonMatching, "REL/e3setupdll/mgname.c"),
            Object(NonMatching, "REL/e3setupdll/toueidisp.c"),
            Object(NonMatching, "REL/e3setupdll/stage.c"),
        },
    ),
    Rel(
        "fileseldll",
        objects={
            Object(NonMatching, "REL/fileseldll/filesel.c"),
            Object(NonMatching, "REL/fileseldll/saveload.c"),
            Object(NonMatching, "REL/fileseldll/filename.c"),
            Object(NonMatching, "REL/fileseldll/toueidisp.c"),
        },
    ),
    Rel(
        "instDll",
        objects={
            Object(NonMatching, "REL/instDll/inst.c"),
            Object(NonMatching, "REL/instDll/mgname.c"),
            Object(NonMatching, "REL/instDll/toueidisp.c"),
            Object(NonMatching, "REL/instDll/stage.c"),
        },
    ),
    Rel(
        "m501Dll",
        objects={
            Object(NonMatching, "REL/m501Dll/m501.cpp"),
            Object(NonMatching, "REL/m501Dll/asiato.cpp"),
            Object(NonMatching, "REL/m501Dll/gamemaster.cpp"),
            Object(NonMatching, "REL/m501Dll/aisu.cpp"),
            Object(NonMatching, "REL/m501Dll/joint.cpp"),
            Object(NonMatching, "REL/m501Dll/objectsetup.cpp"),
            Object(NonMatching, "REL/m501Dll/camera.cpp"),
            Object(NonMatching, "REL/m501Dll/particle.cpp"),
            Object(NonMatching, "REL/m501Dll/stage.cpp"),
            Object(NonMatching, "REL/m501Dll/player.cpp"),
            Object(NonMatching, "REL/m501Dll/random.cpp"),
        },
    ),
    Rel(
        "m502dll",
        objects={
            Object(NonMatching, "REL/m502dll/m502.c"),
            Object(NonMatching, "REL/m502dll/camera.c"),
            Object(NonMatching, "REL/m502dll/stage.c"),
            Object(NonMatching, "REL/m502dll/player.c"),
            Object(NonMatching, "REL/m502dll/input.c"),
            Object(NonMatching, "REL/m502dll/block.c"),
            Object(NonMatching, "REL/m502dll/opening.c"),
            Object(NonMatching, "REL/m502dll/ending.c"),
            Object(NonMatching, "REL/m502dll/bat.c"),
        },
    ),
    Rel(
        "m503Dll",
        objects={
            Object(NonMatching, "REL/m503Dll/m503.c"),
            Object(NonMatching, "REL/m503Dll/opening.c"),
            Object(NonMatching, "REL/m503Dll/player.c"),
            Object(NonMatching, "REL/m503Dll/saru.c"),
            Object(NonMatching, "REL/m503Dll/stage.c"),
            Object(NonMatching, "REL/m503Dll/particle.c"),
            Object(NonMatching, "REL/m503Dll/score.c"),
            Object(NonMatching, "REL/m503Dll/actor.c"),
        },
    ),
    Rel(
        "m504dll",
        objects={
            Object(NonMatching, "REL/m504dll/m504.c"),
            Object(NonMatching, "REL/m504dll/stage.c"),
            Object(NonMatching, "REL/m504dll/player.c"),
            Object(NonMatching, "REL/m504dll/wanwan.c"),
            Object(NonMatching, "REL/m504dll/input.c"),
            Object(NonMatching, "REL/m504dll/com.c"),
            Object(NonMatching, "REL/m504dll/camera.c"),
            Object(NonMatching, "REL/m504dll/light.c"),
            Object(NonMatching, "REL/m504dll/particle.c"),
            Object(NonMatching, "REL/m504dll/opening.c"),
            Object(NonMatching, "REL/m504dll/ending.c"),
        },
    ),
    Rel(
        "m505Dll",
        objects={
            Object(NonMatching, "REL/m505Dll/m505.c"),
        },
    ),
    Rel(
        "m506Dll",
        objects={
            Object(NonMatching, "REL/m506Dll/m506.c"),
            Object(NonMatching, "REL/m506Dll/object.c"),
            Object(NonMatching, "REL/m506Dll/player.c"),
        },
    ),
    Rel(
        "m507dll",
        objects={
            Object(NonMatching, "REL/m507dll/m507.c"),
            Object(NonMatching, "REL/m507dll/effect.c"),
        },
    ),
    Rel(
        "m508Dll",
        objects={
            Object(NonMatching, "REL/m508Dll/m508.c"),
            Object(NonMatching, "REL/m508Dll/object.c"),
        },
    ),
    Rel(
        "m509Dll",
        objects={
            Object(NonMatching, "REL/m509Dll/m509.cpp"),
            Object(NonMatching, "REL/m509Dll/mazedata.cpp"),
            Object(NonMatching, "REL/m509Dll/maze.cpp"),
            Object(NonMatching, "REL/m509Dll/gamemaster.cpp"),
            Object(NonMatching, "REL/m509Dll/startspr.cpp"),
            Object(NonMatching, "REL/m509Dll/back.cpp"),
            Object(NonMatching, "REL/m509Dll/objectsetup.cpp"),
            Object(NonMatching, "REL/m509Dll/camera.cpp"),
            Object(NonMatching, "REL/m509Dll/player.cpp"),
            Object(NonMatching, "REL/m509Dll/random.cpp"),
        },
    ),
    Rel(
        "m510dll",
        objects={
            Object(NonMatching, "REL/m510dll/m510.c"),
            Object(NonMatching, "REL/m510dll/stage.c"),
            Object(NonMatching, "REL/m510dll/player.c"),
            Object(NonMatching, "REL/m510dll/playermodel.c"),
            Object(NonMatching, "REL/m510dll/pitch.c"),
            Object(NonMatching, "REL/m510dll/ball.c"),
            Object(NonMatching, "REL/m510dll/pad.c"),
            Object(NonMatching, "REL/m510dll/camera.c"),
            Object(NonMatching, "REL/m510dll/light.c"),
            Object(NonMatching, "REL/m510dll/opening.c"),
            Object(NonMatching, "REL/m510dll/ending.c"),
            Object(NonMatching, "REL/m510dll/ball_path.c"),
            Object(NonMatching, "REL/m510dll/score.c"),
        },
    ),
    Rel(
        "m511Dll",
        objects={
            Object(NonMatching, "REL/m511Dll/boat.cpp"),
            Object(NonMatching, "REL/m511Dll/math.cpp"),
            Object(NonMatching, "REL/m511Dll/m511.cpp"),
            Object(NonMatching, "REL/m511Dll/effring.cpp"),
            Object(NonMatching, "REL/m511Dll/gamemaster.cpp"),
            Object(NonMatching, "REL/m511Dll/particle.cpp"),
            Object(NonMatching, "REL/m511Dll/objectsetup.cpp"),
            Object(NonMatching, "REL/m511Dll/camera.cpp"),
            Object(NonMatching, "REL/m511Dll/boatshadow.cpp"),
            Object(NonMatching, "REL/m511Dll/stage.cpp"),
            Object(NonMatching, "REL/m511Dll/star.cpp"),
            Object(NonMatching, "REL/m511Dll/staryaji.cpp"),
            Object(NonMatching, "REL/m511Dll/player.cpp"),
            Object(NonMatching, "REL/m511Dll/score.cpp"),
            Object(NonMatching, "REL/m511Dll/boatsplash.cpp"),
            Object(NonMatching, "REL/m511Dll/kibako.cpp"),
            Object(NonMatching, "REL/m511Dll/water.cpp"),
        },
    ),
    Rel(
        "m512Dll",
        objects={
            Object(NonMatching, "REL/m512Dll/m512.cpp"),
            Object(NonMatching, "REL/m512Dll/gamemaster.cpp"),
            Object(NonMatching, "REL/m512Dll/objectsetup.cpp"),
            Object(NonMatching, "REL/m512Dll/camera.cpp"),
            Object(NonMatching, "REL/m512Dll/dust.cpp"),
            Object(NonMatching, "REL/m512Dll/stage.cpp"),
            Object(NonMatching, "REL/m512Dll/player.cpp"),
            Object(NonMatching, "REL/m512Dll/score.cpp"),
            Object(NonMatching, "REL/m512Dll/penguin.cpp"),
            Object(NonMatching, "REL/m512Dll/heiho.cpp"),
        },
    ),
    Rel(
        "m513Dll",
        objects={
            Object(NonMatching, "REL/m513Dll/m513.cpp"),
            Object(NonMatching, "REL/m513Dll/input.cpp"),
            Object(NonMatching, "REL/m513Dll/taimatsu.cpp"),
            Object(NonMatching, "REL/m513Dll/leaf.cpp"),
            Object(NonMatching, "REL/m513Dll/powereff.cpp"),
            Object(NonMatching, "REL/m513Dll/powerind.cpp"),
            Object(NonMatching, "REL/m513Dll/gamemaster.cpp"),
            Object(NonMatching, "REL/m513Dll/heiho.cpp"),
            Object(NonMatching, "REL/m513Dll/kuribo.cpp"),
            Object(NonMatching, "REL/m513Dll/objectsetup.cpp"),
            Object(NonMatching, "REL/m513Dll/camera.cpp"),
            Object(NonMatching, "REL/m513Dll/stage.cpp"),
            Object(NonMatching, "REL/m513Dll/misc.cpp"),
            Object(NonMatching, "REL/m513Dll/player.cpp"),
            Object(NonMatching, "REL/m513Dll/score.cpp"),
            Object(NonMatching, "REL/m513Dll/audio.cpp"),
        },
    ),
    Rel(
        "m514Dll",
        objects={
            Object(NonMatching, "REL/m514Dll/m514.c"),
            Object(NonMatching, "REL/m514Dll/player.c"),
            Object(NonMatching, "REL/m514Dll/object.c"),
            Object(NonMatching, "REL/m514Dll/stage.c"),
            Object(NonMatching, "REL/m514Dll/effect.c"),
            Object(NonMatching, "REL/m514Dll/score.c"),
        },
    ),
    Rel(
        "m515Dll",
        objects={
            Object(NonMatching, "REL/m515Dll/m515.c"),
        },
    ),
    Rel(
        "m516Dll",
        objects={
            Object(NonMatching, "REL/m516Dll/m516.c"),
            Object(NonMatching, "REL/m516Dll/camera.c"),
            Object(NonMatching, "REL/m516Dll/player.c"),
            Object(NonMatching, "REL/m516Dll/stage.c"),
            Object(NonMatching, "REL/m516Dll/effect.c"),
            Object(NonMatching, "REL/m516Dll/score.c"),
            Object(NonMatching, "REL/m516Dll/actor.c"),
        },
    ),
    Rel(
        "mdbeachDll",
        objects={
            Object(NonMatching, "REL/mdbeachDll/mdbeach.c"),
            Object(NonMatching, "REL/mdbeachDll/object.c"),
            Object(NonMatching, "REL/mdbeachDll/stage.c"),
        },
    ),
    Rel(
        "mdcardDll",
        objects={
            Object(NonMatching, "REL/mdcardDll/mdcard.c"),
            Object(NonMatching, "REL/mdcardDll/object.c"),
            Object(NonMatching, "REL/mdcardDll/stage.c"),
        },
    ),
    Rel(
        "mdminiDll",
        objects={
            Object(NonMatching, "REL/mdminiDll/mdmini.c"),
            Object(NonMatching, "REL/mdminiDll/object.c"),
            Object(NonMatching, "REL/mdminiDll/stage.c"),
        },
    ),
    Rel(
        "mdomakeDll",
        objects={
            Object(NonMatching, "REL/mdomakeDll/mdomake.c"),
            Object(NonMatching, "REL/mdomakeDll/object.c"),
            Object(NonMatching, "REL/mdomakeDll/stage.c"),
        },
    ),
    Rel(
        "mdoptionDll",
        objects={
            Object(NonMatching, "REL/mdoptionDll/mdoption.c"),
            Object(NonMatching, "REL/mdoptionDll/object.c"),
            Object(NonMatching, "REL/mdoptionDll/stage.c"),
            Object(NonMatching, "REL/mdoptionDll/record.c"),
        },
    ),
    Rel(
        "mdpartyDll",
        objects={
            Object(NonMatching, "REL/mdpartyDll/mdparty.c"),
            Object(NonMatching, "REL/mdpartyDll/object.c"),
            Object(NonMatching, "REL/mdpartyDll/stage.c"),
        },
    ),
    Rel(
        "mdselDll",
        objects={
            Object(NonMatching, "REL/mdselDll/mdsel.c"),
            Object(NonMatching, "REL/mdselDll/object.c"),
            Object(NonMatching, "REL/mdselDll/stage.c"),
        },
    ),
    Rel(
        "mdstoryDll",
        objects={
            Object(NonMatching, "REL/mdstoryDll/mdstory.c"),
            Object(NonMatching, "REL/mdstoryDll/object.c"),
            Object(NonMatching, "REL/mdstoryDll/stage.c"),
        },
    ),
    Rel(
        "meschkDll",
        objects={
            Object(NonMatching, "REL/meschkDll/meschk.c"),
        },
    ),
    Rel(
        "mgfreeDll",
        objects={
            Object(NonMatching, "REL/mgfreeDll/mgfree.c"),
            Object(NonMatching, "REL/mgfreeDll/decide.c"),
        },
    ),
    Rel(
        "mgmatchDll",
        objects={
            Object(NonMatching, "REL/mgmatchDll/mgmatch.c"),
            Object(NonMatching, "REL/mgmatchDll/score.c"),
            Object(NonMatching, "REL/mgmatchDll/mess.c"),
        },
    ),
    Rel(
        "mgtourDll",
        objects={
            Object(NonMatching, "REL/mgtourDll/mgtournament.c"),
            Object(NonMatching, "REL/mgtourDll/score.c"),
            Object(NonMatching, "REL/mgtourDll/mess.c"),
        },
    ),
    Rel(
        "mgwarsDll",
        objects={
            Object(NonMatching, "REL/mgwarsDll/mgwars.c"),
            Object(NonMatching, "REL/mgwarsDll/score.c"),
            Object(NonMatching, "REL/mgwarsDll/mess.c"),
        },
    ),
    Rel(
        "partyresultdll",
        objects={
            Object(NonMatching, "REL/partyresultdll/partyresult.c"),
            Object(NonMatching, "REL/partyresultdll/resultdisp.c"),
            Object(NonMatching, "REL/partyresultdll/result.c"),
            Object(NonMatching, "REL/partyresultdll/toueidisp.c"),
            Object(NonMatching, "REL/partyresultdll/util.c"),
        },
    ),
    Rel(
        "rescardDll",
        objects={
            Object(NonMatching, "REL/rescardDll/rescard.c"),
            Object(NonMatching, "REL/rescardDll/object.c"),
            Object(NonMatching, "REL/rescardDll/stage.c"),
        },
    ),
    Rel(
        "resultDll",
        objects={
            Object(NonMatching, "REL/resultDll/result.c"),
            Object(NonMatching, "REL/resultDll/stage.c"),
            Object(NonMatching, "REL/resultDll/toueidisp.c"),
            Object(NonMatching, "REL/resultDll/stagemodel.c"),
            Object(NonMatching, "REL/resultDll/battle.c"),
            Object(NonMatching, "REL/resultDll/team.c"),
        },
    ),
    Rel(
        "sd00dll",
        objects={
            Object(NonMatching, "REL/sd00dll/sd00.c"),
            Object(NonMatching, "REL/sd00dll/machine.c"),
            Object(NonMatching, "REL/sd00dll/hook.c"),
            Object(NonMatching, "REL/sd00dll/com.c"),
            Object(NonMatching, "REL/sd00dll/gun.c"),
            Object(NonMatching, "REL/sd00dll/stage.c"),
            Object(NonMatching, "REL/sd00dll/math.c"),
            Object(NonMatching, "REL/sd00dll/flag.c"),
            Object(NonMatching, "REL/sd00dll/rabbit.c"),
        },
    ),
    Rel(
        "sdroomdll",
        objects={
            Object(NonMatching, "REL/sdroomdll/sdroom.c"),
            Object(NonMatching, "REL/sdroomdll/machine.c"),
            Object(NonMatching, "REL/sdroomdll/garage.c"),
            Object(NonMatching, "REL/sdroomdll/garagesave.c"),
            Object(NonMatching, "REL/sdroomdll/filename.c"),
            Object(NonMatching, "REL/sdroomdll/picture.c"),
            Object(NonMatching, "REL/sdroomdll/tournament.c"),
            Object(NonMatching, "REL/sdroomdll/ending.c"),
            Object(NonMatching, "REL/sdroomdll/desk.c"),
        },
    ),
    Rel(
        "motchkDll",
        objects={
            Object(NonMatching, "REL/motchkDll/motchk.c"),
        },
    ),
    Rel(
        "safDll",
        objects={
            Object(NonMatching, "REL/safDll/safold.c"),
            Object(NonMatching, "REL/safDll/saf.c"),
            Object(NonMatching, "REL/safDll/saf2.c"),
        },
    ),
    Rel(
        "sequencedll",
        objects={
            Object(NonMatching, "REL/sequencedll/sequence.c"),
        },
    ),
    Rel(
        "selmenuDll",
        objects={
            Object(NonMatching, "REL/selmenuDll/selmenu.c"),
        },
    ),
    Rel(
        "staffDll",
        objects={
            Object(NonMatching, "REL/staffDll/staff.c"),
        },
    ),
    Rel(
        "storymodedll",
        objects={
            Object(NonMatching, "REL/storymodedll/storymode.c"),
            Object(NonMatching, "REL/storymodedll/opening.c"),
        },
    ),
    Rel(
        "storyresultdll",
        objects={
            Object(NonMatching, "REL/storyresultdll/storyresult.c"),
            Object(NonMatching, "REL/storyresultdll/rank.c"),
            Object(NonMatching, "REL/storyresultdll/effect.c"),
        },
    ),
    Rel(
        "systemDll",
        objects={
            Object(NonMatching, "REL/systemDll/nel.c"),
            Object(NonMatching, "REL/systemDll/object.c"),
            Object(NonMatching, "REL/systemDll/camera.c"),
            Object(NonMatching, "REL/systemDll/stage.c"),
            Object(NonMatching, "REL/systemDll/math.c"),
            Object(NonMatching, "REL/systemDll/window.c"),
        },
    ),
    Rel(
        "w01Dll",
        objects={
            Object(NonMatching, "REL/w01Dll/world01.c"),
        },
    ),
    Rel(
        "w01sDll",
        objects={
            Object(NonMatching, "REL/w01sDll/world01.c"),
        },
    ),
    Rel(
        "w02Dll",
        objects={
            Object(NonMatching, "REL/w02Dll/world02.c"),
        },
    ),
    Rel(
        "w02sDll",
        objects={
            Object(NonMatching, "REL/w02sDll/world02.c"),
        },
    ),
    Rel(
        "w03Dll",
        objects={
            Object(NonMatching, "REL/w03Dll/world03.c"),
        },
    ),
    Rel(
        "w03sDll",
        objects={
            Object(NonMatching, "REL/w03sDll/world03.c"),
        },
    ),
    Rel(
        "w04Dll",
        objects={
            Object(NonMatching, "REL/w04Dll/world04.c"),
        },
    ),
    Rel(
        "w04sDll",
        objects={
            Object(NonMatching, "REL/w04sDll/world04.c"),
        },
    ),
    Rel(
        "w05Dll",
        objects={
            Object(NonMatching, "REL/w05Dll/world05.c"),
        },
    ),
    Rel(
        "w05sDll",
        objects={
            Object(NonMatching, "REL/w05sDll/world05.c"),
        },
    ),
    Rel(
        "w06Dll",
        objects={
            Object(NonMatching, "REL/w06Dll/world06.c"),
        },
    ),
    Rel(
        "w06sDll",
        objects={
            Object(NonMatching, "REL/w06sDll/world06.c"),
        },
    ),
    Rel(
        "w07Dll",
        objects={
            Object(NonMatching, "REL/w07Dll/world07.c"),
        },
    ),
    Rel(
        "w07sDll",
        objects={
            Object(NonMatching, "REL/w07sDll/world07.c"),
        },
    ),
    Rel(
        "w10Dll",
        objects={
            Object(NonMatching, "REL/w10Dll/world10.c"),
        },
    ),
    Rel(
        "w20Dll",
        objects={
            Object(NonMatching, "REL/w20Dll/world20.c"),
        },
    ),
]

if args.mode == "configure":
    # Write build.ninja and objdiff.json
    generate_build(config)
elif args.mode == "progress":
    # Print progress and write progress.json
    config.progress_each_module = args.verbose
    calculate_progress(config)
else:
    sys.exit("Unknown mode: " + args.mode)
