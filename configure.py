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
    "GP5E01_00",  # 0
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
    "--objdiff",
    metavar="BINARY | DIR",
    type=Path,
    help="path to objdiff-cli binary or source (optional)",
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
parser.add_argument(
    "--no-progress",
    dest="progress",
    action="store_false",
    help="disable progress calculation",
)
args = parser.parse_args()

config = ProjectConfig()
config.version = str(args.version)
version_num = VERSIONS.index(config.version)

# Apply arguments
config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.objdiff_path = args.objdiff
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.generate_map = args.map
config.non_matching = args.non_matching
config.sjiswrap_path = args.sjiswrap
config.progress = args.progress
if not is_windows():
    config.wrapper = args.wrapper
# Don't build asm unless we're --non-matching
if not config.non_matching:
    config.asm_dir = None

# Tool versions
config.binutils_tag = "2.42-1"
config.compilers_tag = "20240706"
config.dtk_tag = "v1.5.1"
config.objdiff_tag = "v2.7.1"
config.sjiswrap_tag = "v1.2.0"
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
    "-i src",
    "-DMUSY_TARGET=MUSY_TARGET_DOLPHIN",
    f"-i build/{config.version}/include",
    f"-DVERSION={version_num}",
]

# Debug flags
if args.debug:
    # Or -sym dwarf-2 for Wii compilers
    cflags_base.extend(["-sym on", "-DDEBUG=1"])
else:
    cflags_base.append("-DNDEBUG=1")

# Metrowerks library flags
cflags_runtime = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-str reuse,readonly",
    "-common off",
    "-inline auto,deferred",
]

# Dolphin library flags
cflags_dolphin = [
    *cflags_base,
    "-fp_contract off",
]

cflags_thp = [
    *cflags_base,
]

# Metrowerks library flags
cflags_msl = [
    *cflags_base,
    "-char signed",
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-common off",
    "-inline auto,deferred",
]

# Metrowerks library flags
cflags_trk = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-str reuse,readonly",
    "-common off",
    "-sdata 0",
    "-sdata2 0",
    "-inline auto,deferred",
    "-enum min",
    "-sdatathreshold 0"
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

cflags_musyx_debug = [
    "-proc gecko",
    "-fp hard",
    "-nodefaults",
    "-nosyspath",
    "-i include",
    "-i extern/musyx/include",
    "-i libc",
    "-g",
    "-sym on",
    "-D_DEBUG=1",
    "-fp hard",
    "-enum int",
    "-Cpp_exceptions off",
    "-DMUSY_TARGET=MUSY_TARGET_DOLPHIN",
]

# REL flags
cflags_rel = [
    *cflags_base,
    "-O0,p",
    "-char unsigned",
    "-fp_contract off",
    "-sym on",
    "-sdata 0",
    "-sdata2 0",
    "-pool off",
]

# Game flags
cflags_game = [
    *cflags_base,
    "-O0,p",
    "-sym on",
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
            Object(Matching, "game/main.c"),
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
            Object(Matching, "game/hsfdraw.c"),
            Object(Matching, "game/hsfman.c"),
            Object(Matching, "game/hsfmotion.c"),
            Object(Matching, "game/hsfanim.c"),
            Object(Matching, "game/hsfex.c"),
            Object(Matching, "game/perf.c"),
            Object(Matching, "game/objmain.c"),
            Object(Matching, "game/fault.c"),
            Object(Matching, "game/gamework.c"),
            Object(Matching, "game/objsysobj.c"),
            Object(Matching, "game/objdll.c"),
            Object(Matching, "game/frand.c"),
            Object(Matching, "game/audio.c"),
            Object(Matching, "game/EnvelopeExec.c"),
            Object(Matching, "game/gamemes.c"),
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
            Object(Matching, "game/mgdata.c"),
            Object(Matching, "game/objsub.c"),
            Object(Matching, "game/flag.c"),
            Object(Equivalent, "game/saveload.c"),
            Object(Matching, "game/sreset.c"),
            Object(Matching, "game/mg/timer.c"),
            Object(Matching, "game/mg/score.c"),
            Object(Matching, "game/mg/seqman.c"),
            Object(Matching, "game/mg/colman.c"),
            Object(Matching, "game/mg/actman.c"),
            Object(NonMatching, "game/mg/mess.c"),
            Object(Matching, "game/board/pausewatch.c"),
            Object(Matching, "game/board/main.c"),
            Object(Matching, "game/board/camera.c"),
            Object(Matching, "game/board/player.c"),
            Object(Matching, "game/board/model.c"),
            Object(Matching, "game/board/window.c"),
            Object(Matching, "game/board/audio.c"),
            Object(Matching, "game/board/com.c"),
            Object(Matching, "game/board/scroll.c"),
            Object(Matching, "game/board/masu.c"),
            Object(Matching, "game/board/coin.c"),
            Object(Matching, "game/board/star.c"),
            Object(Matching, "game/board/padall.c"),
            Object(Matching, "game/board/sai.c"),
            Object(Matching, "game/board/status.c"),
            Object(Matching, "game/board/opening.c"),
            Object(Matching, "game/board/pause.c"),
            Object(Matching, "game/board/tutorial.c"),
            Object(Matching, "game/board/roulette.c"),
            Object(Matching, "game/board/capmachine.c"),
            Object(Equivalent, "game/board/capsule.c"),
            Object(Equivalent, "game/board/capmain.c"),
            Object(Matching, "game/board/guide.c"),
            Object(Matching, "game/board/branch.c"),
            Object(Matching, "game/board/mgcall.c"),
            Object(Matching, "game/board/effect.c"),
            Object(Matching, "game/board/pauseoption.c"),
            Object(Matching, "game/board/gate.c"),
            Object(NonMatching, "game/board/story.c"),
            Object(Matching, "game/board/telop.c"),
            Object(Matching, "game/board/mgcircuit.c"),
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
    {
        "lib": "thp",
        "mw_version": "GC/1.2.5",
        "cflags": cflags_thp,
        "host": False,
        "objects": [
            Object(NonMatching, "dolphin/thp/THPDec.c"),
            Object(NonMatching, "dolphin/thp/THPAudio.c"),
        ],
    },
    {
        "lib": "Runtime.PPCEABI.H",
        "mw_version": config.linker_version,
        "cflags": cflags_runtime,
        "host": False,
        "objects": [
            Object(Matching, "Runtime.PPCEABI.H/__va_arg.c"),
            Object(NonMatching, "Runtime.PPCEABI.H/global_destructor_chain.c"),
            Object(Matching, "Runtime.PPCEABI.H/__mem.c"),
            Object(Matching, "Runtime.PPCEABI.H/New.cp", extra_cflags=["-Cpp_exceptions on"]),
            Object(Matching, "Runtime.PPCEABI.H/NewMore.cp", extra_cflags=["-Cpp_exceptions on", "-RTTI on"]),
            Object(NonMatching, "Runtime.PPCEABI.H/NMWException.cpp", extra_cflags=["-Cpp_exceptions on"]),
            Object(NonMatching, "Runtime.PPCEABI.H/runtime.c"),
            Object(Matching, "Runtime.PPCEABI.H/__init_cpp_exceptions.cpp"),
            Object(NonMatching, "Runtime.PPCEABI.H/Gecko_ExceptionPPC.cpp", extra_cflags=["-Cpp_exceptions on", "-RTTI on"]),
            Object(NonMatching, "Runtime.PPCEABI.H/GCN_mem_alloc.c"),
        ],
    },
    {
        "lib": "MSL_C.PPCEABI.bare.H",
        "mw_version": "GC/1.3",
        "cflags": cflags_msl,
        "host": False,
        "objects": [
            Object(Matching, "MSL_C.PPCEABI.bare.H/abort_exit.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/alloc.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/errno.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/ansi_files.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/ansi_fp.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/arith.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/buffer_io.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/ctype.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/direct_io.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/file_io.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/FILE_POS.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/mbstring.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/mem.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/mem_funcs.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/misc_io.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/printf.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/float.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/signal.c"),
            Object(NonMatching, "MSL_C.PPCEABI.bare.H/string.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/uart_console_io.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/wchar_io.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/e_acos.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/e_asin.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/e_atan2.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/e_fmod.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/e_pow.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/e_rem_pio2.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/k_cos.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/k_rem_pio2.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/k_sin.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/k_tan.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_atan.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_copysign.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_cos.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_floor.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_frexp.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_ldexp.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_modf.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_sin.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/s_tan.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/w_acos.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/w_asin.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/w_atan2.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/w_fmod.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/w_pow.c"),
            Object(Matching, "MSL_C.PPCEABI.bare.H/math_ppc.c"),
        ],
    },
    {
        "lib": "TRK_MINNOW_DOLPHIN",
        "mw_version": "GC/1.3",
        "cflags": cflags_trk,
        "host": False,
        "objects": [
            Object(Matching, "TRK_MINNOW_DOLPHIN/mainloop.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/nubevent.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/nubinit.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/msg.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/msgbuf.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/serpoll.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/usr_put.c", mw_version="GC/1.3.2"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/dispatch.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/msghndlr.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/support.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/mutex_TRK.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/notify.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/flush_cache.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/mem_TRK.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/targimpl.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/targsupp.s"),
            Object(NonMatching, "TRK_MINNOW_DOLPHIN/__exception.s"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/dolphin_trk.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/mpc_7xx_603e.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/main_TRK.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/dolphin_trk_glue.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/targcont.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/target_options.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/mslsupp.c"),
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
        "mw_version": "GC/1.2.5n",
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
            Object(Matching, "amcstubs/AmcExi2Stubs.c"),
        ],
    },
    {
        "lib": "odenotstub",
        "mw_version": config.linker_version,
        "cflags": cflags_odenotstub,
        "host": False,
        "objects": [
            Object(Matching, "odenotstub/odenotstub.c"),
        ],
    },
    {
        "lib": "libhu",
        "mw_version": config.linker_version,
        "cflags": cflags_libhu,
        "host": False,
        "objects": [
            Object(Matching, "libhu/setvf.c"),
            Object(Matching, "libhu/subvf.c"),
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
            Object(Matching, "REL/nintendo/data.c"),
        ],
    },
    Rel(
        "actmanDLL",
        objects={
            Object(Matching, "REL/actmanDLL/actman.c"),
        },
    ),
    Rel(
        "bootDll",
        objects={
            Object(Matching, "REL/bootDll/boot.c"),
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
            Object(Matching, "REL/decathlonDll/mgtournament.c"),
            Object(Matching, "REL/decathlonDll/score.c"),
            Object(Matching, "REL/decathlonDll/mess.c"),
        },
    ),
    Rel(
        "e3bootdll",
        objects={
            Object(Matching, "REL/e3bootdll/e3boot.c"),
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
            Object(Matching, "REL/fileseldll/filesel.c"),
            Object(Matching, "REL/fileseldll/saveload.c"),
            Object(Matching, "REL/fileseldll/filename.c"),
            Object(Matching, "REL/fileseldll/toueidisp.c"),
        },
    ),
    Rel(
        "instDll",
        objects={
            Object(Matching, "REL/instDll/inst.c"),
            Object(Matching, "REL/instDll/mgname.c"),
            Object(Matching, "REL/instDll/toueidisp.c"),
            Object(Matching, "REL/instDll/stage.c"),
        },
    ),
    Rel(
        "m433Dll",
        objects={
            Object(Matching, "REL/m433Dll/m433.c"),
            Object(Matching, "REL/m433Dll/stage.c"),
            Object(Matching, "REL/m433Dll/player.c"),
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
            Object(NonMatching, "REL/m512Dll/zako.cpp"),
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
        "m517Dll",
        objects={
            Object(NonMatching, "REL/m517Dll/effect.cpp"),
            Object(NonMatching, "REL/m517Dll/m517.cpp"),
            Object(NonMatching, "REL/m517Dll/input.cpp"),
            Object(NonMatching, "REL/m517Dll/gamemaster.cpp"),
            Object(NonMatching, "REL/m517Dll/dusteff.cpp"),
            Object(NonMatching, "REL/m517Dll/kuribo.cpp"),
            Object(NonMatching, "REL/m517Dll/objectsetup.cpp"),
            Object(NonMatching, "REL/m517Dll/stage.cpp"),
            Object(NonMatching, "REL/m517Dll/misc.cpp"),
            Object(NonMatching, "REL/m517Dll/choro.cpp"),
            Object(NonMatching, "REL/m517Dll/papereff.cpp"),
            Object(NonMatching, "REL/m517Dll/player.cpp"),
            Object(NonMatching, "REL/m517Dll/camera.cpp"),
            Object(NonMatching, "REL/m517Dll/dustcloud.cpp"),
            Object(NonMatching, "REL/m517Dll/audio.cpp"),
        },
    ),
    Rel(
        "m518dll",
        objects={
            Object(NonMatching, "REL/m518dll/m518.c"),
        },
    ),
    Rel(
        "m519dll",
        objects={
            Object(NonMatching, "REL/m519dll/m519.c"),
        },
    ),
    Rel(
        "m520Dll",
        objects={
            Object(NonMatching, "REL/m520Dll/m520.c"),
        },
    ),
    Rel(
        "m521Dll",
        objects={
            Object(NonMatching, "REL/m521Dll/clock.cpp"),
            Object(NonMatching, "REL/m521Dll/clockhand.cpp"),
            Object(NonMatching, "REL/m521Dll/m521.cpp"),
            Object(NonMatching, "REL/m521Dll/debugline.cpp"),
            Object(NonMatching, "REL/m521Dll/driver.cpp"),
            Object(NonMatching, "REL/m521Dll/gamemaster.cpp"),
            Object(NonMatching, "REL/m521Dll/objectsetup.cpp"),
            Object(NonMatching, "REL/m521Dll/camera.cpp"),
            Object(NonMatching, "REL/m521Dll/stage.cpp"),
            Object(NonMatching, "REL/m521Dll/player.cpp"),
            Object(NonMatching, "REL/m521Dll/score.cpp"),
        },
    ),
    Rel(
        "m522Dll",
        objects={
            Object(NonMatching, "REL/m522Dll/fire.cpp"),
            Object(NonMatching, "REL/m522Dll/bubble.cpp"),
            Object(NonMatching, "REL/m522Dll/m522.cpp"),
            Object(NonMatching, "REL/m522Dll/gamemaster.cpp"),
            Object(NonMatching, "REL/m522Dll/objectsetup.cpp"),
            Object(NonMatching, "REL/m522Dll/camera.cpp"),
            Object(NonMatching, "REL/m522Dll/sunfire.cpp"),
            Object(NonMatching, "REL/m522Dll/stage.cpp"),
            Object(NonMatching, "REL/m522Dll/player.cpp"),
        },
    ),
    Rel(
        "m523Dll",
        objects={
            Object(NonMatching, "REL/m523Dll/m523.c"),
            Object(NonMatching, "REL/m523Dll/player.c"),
        },
    ),
    Rel(
        "m524Dll",
        objects={
            Object(NonMatching, "REL/m524Dll/m524.c"),
        },
    ),
    Rel(
        "m525Dll",
        objects={
            Object(NonMatching, "REL/m525Dll/m525.c"),
        },
    ),
    Rel(
        "m526Dll",
        objects={
            Object(Matching, "REL/m526Dll/m526.c"),
            Object(Matching, "REL/m526Dll/math.c"),
        },
    ),
    Rel(
        "m527Dll",
        objects={
            Object(NonMatching, "REL/m527Dll/m527.c"),
        },
    ),
    Rel(
        "m528Dll",
        objects={
            Object(NonMatching, "REL/m528Dll/m528.c"),
        },
    ),
    Rel(
        "m529dll",
        objects={
            Object(NonMatching, "REL/m529dll/m529.c"),
        },
    ),
    Rel(
        "m530Dll",
        objects={
            Object(NonMatching, "REL/m530Dll/m530.c"),
            Object(NonMatching, "REL/m530Dll/kinomi.c"),
            Object(NonMatching, "REL/m530Dll/player.c"),
            Object(NonMatching, "REL/m530Dll/effect.c"),
        },
    ),
    Rel(
        "m531Dll",
        objects={
            Object(NonMatching, "REL/m531Dll/m531.cpp"),
            Object(NonMatching, "REL/m531Dll/effect.cpp"),
            Object(NonMatching, "REL/m531Dll/scorebase.cpp"),
            Object(NonMatching, "REL/m531Dll/flow.cpp"),
            Object(NonMatching, "REL/m531Dll/polish.cpp"),
            Object(NonMatching, "REL/m531Dll/score.cpp"),
            Object(NonMatching, "REL/m531Dll/object.cpp"),
            Object(NonMatching, "REL/m531Dll/player.cpp"),
            Object(NonMatching, "REL/m531Dll/objectsetup.cpp"),
            Object(NonMatching, "REL/m531Dll/gamemaster.cpp"),
            Object(NonMatching, "REL/m531Dll/stage.cpp"),
            Object(NonMatching, "REL/m531Dll/playerbase.cpp"),
            Object(NonMatching, "REL/m531Dll/audio.cpp"),
            Object(NonMatching, "REL/m531Dll/cleaneff.cpp"),
        },
    ),
    Rel(
        "m532Dll",
        objects={
            Object(NonMatching, "REL/m532Dll/ball.cpp"),
            Object(NonMatching, "REL/m532Dll/m532.cpp"),
            Object(NonMatching, "REL/m532Dll/rumble_warn.cpp"),
            Object(NonMatching, "REL/m532Dll/game_master.cpp"),
            Object(NonMatching, "REL/m532Dll/object_setup.cpp"),
            Object(NonMatching, "REL/m532Dll/camera.cpp"),
            Object(NonMatching, "REL/m532Dll/man_map.cpp"),
            Object(NonMatching, "REL/m532Dll/man_referee.cpp"),
            Object(NonMatching, "REL/m532Dll/score.cpp"),
            Object(NonMatching, "REL/m532Dll/unk.cpp"),
            Object(NonMatching, "REL/m532Dll/player.cpp"),
        },
    ),
    Rel(
        "m533dll",
        objects={
            Object(NonMatching, "REL/m533dll/m533.c"),
            Object(NonMatching, "REL/m533dll/pillar.c"),
        },
    ),
    Rel(
        "m534Dll",
        objects={
            Object(NonMatching, "REL/m534Dll/m534.c"),
            Object(NonMatching, "REL/m534Dll/camera.c"),
            Object(NonMatching, "REL/m534Dll/player.c"),
            Object(NonMatching, "REL/m534Dll/stage.c"),
            Object(NonMatching, "REL/m534Dll/effect.c"),
            Object(NonMatching, "REL/m534Dll/score.c"),
        },
    ),
    Rel(
        "m535Dll",
        objects={
            Object(NonMatching, "REL/m535Dll/m535.c"),
        },
    ),
    Rel(
        "m536Dll",
        objects={
            Object(NonMatching, "REL/m536Dll/m536.cpp"),
            Object(NonMatching, "REL/m536Dll/game_master.cpp"),
            Object(NonMatching, "REL/m536Dll/object_setup.cpp"),
            Object(NonMatching, "REL/m536Dll/camera.cpp"),
            Object(NonMatching, "REL/m536Dll/killereff.cpp"),
            Object(NonMatching, "REL/m536Dll/stage.cpp"),
            Object(NonMatching, "REL/m536Dll/player.cpp"),
            Object(NonMatching, "REL/m536Dll/score.cpp"),
            Object(NonMatching, "REL/m536Dll/killer.cpp"),
            Object(NonMatching, "REL/m536Dll/gun.cpp"),
        },
    ),
    Rel(
        "m537Dll",
        objects={
            Object(NonMatching, "REL/m537Dll/m537.cpp"),
            Object(NonMatching, "REL/m537Dll/dorrie.cpp"),
            Object(NonMatching, "REL/m537Dll/effect.cpp"),
            Object(NonMatching, "REL/m537Dll/game_master.cpp"),
            Object(NonMatching, "REL/m537Dll/object_setup.cpp"),
            Object(NonMatching, "REL/m537Dll/camera.cpp"),
            Object(NonMatching, "REL/m537Dll/stage.cpp"),
            Object(NonMatching, "REL/m537Dll/player.cpp"),
            Object(NonMatching, "REL/m537Dll/center_obj.cpp"),
            Object(NonMatching, "REL/m537Dll/water.cpp"),
        },
    ),
    Rel(
        "m538Dll",
        objects={
            Object(NonMatching, "REL/m538Dll/m538.c"),
        },
    ),
    Rel(
        "m539Dll",
        objects={
            Object(NonMatching, "REL/m539Dll/m539.cpp"),
            Object(NonMatching, "REL/m539Dll/input.cpp"),
            Object(NonMatching, "REL/m539Dll/drum.cpp"),
            Object(NonMatching, "REL/m539Dll/game_master.cpp"),
            Object(NonMatching, "REL/m539Dll/heiho.cpp"),
            Object(NonMatching, "REL/m539Dll/object_setup.cpp"),
            Object(NonMatching, "REL/m539Dll/camera.cpp"),
            Object(NonMatching, "REL/m539Dll/man_map.cpp"),
            Object(NonMatching, "REL/m539Dll/effect.cpp"),
            Object(NonMatching, "REL/m539Dll/player.cpp"),
            Object(NonMatching, "REL/m539Dll/scorespr.cpp"),
            Object(NonMatching, "REL/m539Dll/score.cpp"),
        },
    ),
    Rel(
        "m540Dll",
        objects={
            Object(NonMatching, "REL/m540Dll/m540.c"),
        },
    ),
    Rel(
        "m541Dll",
        objects={
            Object(NonMatching, "REL/m541Dll/m541.c"),
            Object(NonMatching, "REL/m541Dll/game.c"),
            Object(NonMatching, "REL/m541Dll/stage.c"),
        },
    ),
    Rel(
        "m542dll",
        objects={
            Object(NonMatching, "REL/m542dll/m542.c"),
            Object(NonMatching, "REL/m542dll/game.c"),
            Object(NonMatching, "REL/m542dll/stage.c"),
        },
    ),
    Rel(
        "m543Dll",
        objects={
            Object(NonMatching, "REL/m543Dll/m543.c"),
            Object(NonMatching, "REL/m543Dll/game.c"),
            Object(NonMatching, "REL/m543Dll/stage.c"),
        },
    ),
    Rel(
        "m544Dll",
        objects={
            Object(NonMatching, "REL/m544Dll/m544.c"),
            Object(NonMatching, "REL/m544Dll/game.c"),
            Object(NonMatching, "REL/m544Dll/stage.c"),
        },
    ),
    Rel(
        "m545Dll",
        objects={
            Object(NonMatching, "REL/m545Dll/m545.c"),
            Object(NonMatching, "REL/m545Dll/game.c"),
            Object(NonMatching, "REL/m545Dll/stage.c"),
        },
    ),
    Rel(
        "m546Dll",
        objects={
            Object(NonMatching, "REL/m546Dll/m546.c"),
            Object(NonMatching, "REL/m546Dll/game.c"),
            Object(NonMatching, "REL/m546Dll/stage.c"),
        },
    ),
    Rel(
        "m547dll",
        objects={
            Object(NonMatching, "REL/m547dll/m547.c"),
            Object(NonMatching, "REL/m547dll/game.c"),
            Object(NonMatching, "REL/m547dll/stage.c"),
        },
    ),
    Rel(
        "m548dll",
        objects={
            Object(NonMatching, "REL/m548dll/m548.c"),
            Object(NonMatching, "REL/m548dll/game.c"),
            Object(NonMatching, "REL/m548dll/stage.c"),
        },
    ),
    Rel(
        "m549Dll",
        objects={
            Object(NonMatching, "REL/m549Dll/m549.c"),
            Object(NonMatching, "REL/m549Dll/game.c"),
            Object(NonMatching, "REL/m549Dll/cloud.c"),
            Object(NonMatching, "REL/m549Dll/stage.c"),
        },
    ),
    Rel(
        "m550dll",
        objects={
            Object(NonMatching, "REL/m550dll/m550.c"),
            Object(NonMatching, "REL/m550dll/game.c"),
            Object(NonMatching, "REL/m550dll/stage.c"),
        },
    ),
    Rel(
        "m551Dll",
        objects={
            Object(NonMatching, "REL/m551Dll/m551.c"),
            Object(NonMatching, "REL/m551Dll/game.c"),
            Object(NonMatching, "REL/m551Dll/stage.c"),
        },
    ),
    Rel(
        "m552dll",
        objects={
            Object(NonMatching, "REL/m552dll/m552.c"),
            Object(NonMatching, "REL/m552dll/game.c"),
            Object(NonMatching, "REL/m552dll/cube.c"),
            Object(NonMatching, "REL/m552dll/com.c"),
            Object(NonMatching, "REL/m552dll/stage.c"),
        },
    ),
    Rel(
        "m553Dll",
        objects={
            Object(NonMatching, "REL/m553Dll/m553.c"),
            Object(NonMatching, "REL/m553Dll/game.c"),
            Object(NonMatching, "REL/m553Dll/stage.c"),
        },
    ),
    Rel(
        "m554dll",
        objects={
            Object(NonMatching, "REL/m554dll/m554.c"),
            Object(NonMatching, "REL/m554dll/game.c"),
            Object(NonMatching, "REL/m554dll/stage.c"),
        },
    ),
    Rel(
        "m555Dll",
        objects={
            Object(NonMatching, "REL/m555Dll/m555.c"),
            Object(NonMatching, "REL/m555Dll/game.c"),
            Object(NonMatching, "REL/m555Dll/stage.c"),
        },
    ),
    Rel(
        "m559dll",
        objects={
            Object(NonMatching, "REL/m559dll/m559.c"),
        },
    ),
    Rel(
        "m560dll",
        objects={
            Object(NonMatching, "REL/m560dll/m560.c"),
            Object(NonMatching, "REL/m560dll/effect.c"),
        },
    ),
    Rel(
        "m561dll",
        objects={
            Object(NonMatching, "REL/m561dll/m561.c"),
            Object(NonMatching, "REL/m561dll/effect.c"),
        },
    ),
    Rel(
        "m562dll",
        objects={
            Object(NonMatching, "REL/m562dll/m562.c"),
            Object(NonMatching, "REL/m562dll/main.c"),
            Object(NonMatching, "REL/m562dll/back.c"),
            Object(NonMatching, "REL/m562dll/stage.c"),
            Object(NonMatching, "REL/m562dll/player.c"),
            Object(NonMatching, "REL/m562dll/koopa.c"),
            Object(NonMatching, "REL/m562dll/koopaboss.c"),
            Object(NonMatching, "REL/m562dll/fire.c"),
            Object(NonMatching, "REL/m562dll/mini_mechakoopa.c"),
            Object(NonMatching, "REL/m562dll/firering.c"),
            Object(NonMatching, "REL/m562dll/sunbomb.c"),
            Object(NonMatching, "REL/m562dll/camera.c"),
            Object(NonMatching, "REL/m562dll/light.c"),
            Object(NonMatching, "REL/m562dll/sprite.c"),
            Object(NonMatching, "REL/m562dll/evt_opening.c"),
            Object(NonMatching, "REL/m562dll/evt_fire.c"),
            Object(NonMatching, "REL/m562dll/evt_fight.c"),
            Object(NonMatching, "REL/m562dll/evt_fall.c"),
            Object(NonMatching, "REL/m562dll/evt_koopagrow.c"),
            Object(NonMatching, "REL/m562dll/evt_ending.c"),
            Object(NonMatching, "REL/m562dll/evt_winner.c"),
            Object(NonMatching, "REL/m562dll/effect.c"),
        },
    ),
    Rel(
        "m563dll",
        objects={
            Object(NonMatching, "REL/m563dll/m563.c"),
            Object(NonMatching, "REL/m563dll/util.c"),
        },
    ),
    Rel(
        "m564Dll",
        objects={
            Object(NonMatching, "REL/m564Dll/m564.c"),
            Object(NonMatching, "REL/m564Dll/player.c"),
            Object(NonMatching, "REL/m564Dll/bomb.c"),
            Object(NonMatching, "REL/m564Dll/com.c"),
        },
    ),
    Rel(
        "m565dll",
        objects={
            Object(NonMatching, "REL/m565dll/m565.c"),
            Object(NonMatching, "REL/m565dll/main.c"),
            Object(NonMatching, "REL/m565dll/object.c"),
            Object(NonMatching, "REL/m565dll/back.c"),
            Object(NonMatching, "REL/m565dll/stage.c"),
            Object(NonMatching, "REL/m565dll/player.c"),
            Object(NonMatching, "REL/m565dll/puku2.c"),
            Object(NonMatching, "REL/m565dll/com.c"),
            Object(NonMatching, "REL/m565dll/opening.c"),
            Object(NonMatching, "REL/m565dll/ending.c"),
            Object(NonMatching, "REL/m565dll/light.c"),
        },
    ),
    Rel(
        "m566Dll",
        objects={
            Object(NonMatching, "REL/m566Dll/m566.c"),
        },
    ),
    Rel(
        "m567Dll",
        objects={
            Object(NonMatching, "REL/m567Dll/m567.c"),
        },
    ),
    Rel(
        "m568Dll",
        objects={
            Object(NonMatching, "REL/m568Dll/archive.cpp"),
            Object(NonMatching, "REL/m568Dll/m568.cpp"),
            Object(NonMatching, "REL/m568Dll/input.cpp"),
            Object(NonMatching, "REL/m568Dll/game_master.cpp"),
            Object(NonMatching, "REL/m568Dll/heiho.cpp"),
            Object(NonMatching, "REL/m568Dll/object_setup.cpp"),
            Object(NonMatching, "REL/m568Dll/camera.cpp"),
            Object(NonMatching, "REL/m568Dll/effect.cpp"),
            Object(NonMatching, "REL/m568Dll/stage.cpp"),
            Object(NonMatching, "REL/m568Dll/math.cpp"),
            Object(NonMatching, "REL/m568Dll/octtree.cpp"),
            Object(NonMatching, "REL/m568Dll/player.cpp"),
            Object(NonMatching, "REL/m568Dll/tube.cpp"),
        },
    ),
    Rel(
        "m569Dll",
        objects={
            Object(NonMatching, "REL/m569Dll/m569.c"),
            Object(NonMatching, "REL/m569Dll/player.c"),
            Object(NonMatching, "REL/m569Dll/kuribo.c"),
            Object(NonMatching, "REL/m569Dll/block.c"),
            Object(NonMatching, "REL/m569Dll/score.c"),
        },
    ),
    Rel(
        "m570Dll",
        objects={
            Object(NonMatching, "REL/m570Dll/m570.c"),
            Object(NonMatching, "REL/m570Dll/player.c"),
        },
    ),
    Rel(
        "m571dll",
        objects={
            Object(NonMatching, "REL/m571dll/m571.c"),
        },
    ),
    Rel(
        "m572dll",
        objects={
            Object(NonMatching, "REL/m572dll/m572.c"),
        },
    ),
    Rel(
        "m573dll",
        objects={
            Object(NonMatching, "REL/m573dll/m573.c"),
        },
    ),
    Rel(
        "m574Dll",
        objects={
            Object(NonMatching, "REL/m574Dll/m574.c"),
            Object(NonMatching, "REL/m574Dll/player.c"),
            Object(NonMatching, "REL/m574Dll/stage.c"),
            Object(NonMatching, "REL/m574Dll/ball.c"),
            Object(NonMatching, "REL/m574Dll/score.c"),
            Object(NonMatching, "REL/m574Dll/collide.c"),
        },
    ),
    Rel(
        "m575dll",
        objects={
            Object(NonMatching, "REL/m575dll/m575.c"),
            Object(NonMatching, "REL/m575dll/score.c"),
            Object(NonMatching, "REL/m575dll/stage.c"),
            Object(NonMatching, "REL/m575dll/ten.c"),
            Object(NonMatching, "REL/m575dll/player.c"),
            Object(NonMatching, "REL/m575dll/com.c"),
            Object(NonMatching, "REL/m575dll/camera.c"),
            Object(NonMatching, "REL/m575dll/light.c"),
            Object(NonMatching, "REL/m575dll/tuboeff.c"),
            Object(NonMatching, "REL/m575dll/tubo.c"),
            Object(NonMatching, "REL/m575dll/coin.c"),
            Object(NonMatching, "REL/m575dll/jugem.c"),
            Object(NonMatching, "REL/m575dll/opening.c"),
            Object(NonMatching, "REL/m575dll/effect.c"),
        },
    ),
    Rel(
        "m576Dll",
        objects={
            Object(NonMatching, "REL/m576Dll/m576.c"),
            Object(NonMatching, "REL/m576Dll/stage.c"),
            Object(NonMatching, "REL/m576Dll/score.c"),
        },
    ),
    Rel(
        "m577Dll",
        objects={
            Object(NonMatching, "REL/m577Dll/m577.c"),
            Object(NonMatching, "REL/m577Dll/stagedata.c"),
            Object(NonMatching, "REL/m577Dll/effect.c"),
        },
    ),
    Rel(
        "m579Dll",
        objects={
            Object(NonMatching, "REL/m579Dll/m579.cpp"),
            Object(NonMatching, "REL/m579Dll/puku2.cpp"),
            Object(NonMatching, "REL/m579Dll/game_master.cpp"),
            Object(NonMatching, "REL/m579Dll/fishman.cpp"),
            Object(NonMatching, "REL/m579Dll/object_setup.cpp"),
            Object(NonMatching, "REL/m579Dll/camera.cpp"),
            Object(NonMatching, "REL/m579Dll/water.cpp"),
            Object(NonMatching, "REL/m579Dll/stage.cpp"),
            Object(NonMatching, "REL/m579Dll/effect.cpp"),
            Object(NonMatching, "REL/m579Dll/player.cpp"),
            Object(NonMatching, "REL/m579Dll/fishline.cpp"),
            Object(NonMatching, "REL/m579Dll/score.cpp"),
            Object(NonMatching, "REL/m579Dll/watereff.cpp"),
            Object(NonMatching, "REL/m579Dll/zako.cpp"),
        },
    ),
    Rel(
        "m580Dll",
        objects={
            Object(NonMatching, "REL/m580Dll/m580.c"),
            Object(NonMatching, "REL/m580Dll/player.c"),
            Object(NonMatching, "REL/m580Dll/stage.c"),
            Object(NonMatching, "REL/m580Dll/stick.c"),
            Object(NonMatching, "REL/m580Dll/puck.c"),
            Object(NonMatching, "REL/m580Dll/effect.c"),
            Object(NonMatching, "REL/m580Dll/score.c"),
            Object(NonMatching, "REL/m580Dll/reflect.c"),
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
            Object(Matching, "REL/mdselDll/mdsel.c"),
            Object(Matching, "REL/mdselDll/object.c"),
            Object(Matching, "REL/mdselDll/stage.c"),
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
        "motchkDll",
        objects={
            Object(NonMatching, "REL/motchkDll/motchk.c"),
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
            Object(NonMatching, "REL/sd00dll/util.c"),
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
        "safDll",
        objects={
            Object(NonMatching, "REL/safDll/safanim.c"),
            Object(NonMatching, "REL/safDll/saf.c"),
            Object(NonMatching, "REL/safDll/safmotion.c"),
        },
    ),
    Rel(
        "sequencedll",
        objects={
            Object(Matching, "REL/sequencedll/sequence.c"),
        },
    ),
    Rel(
        "selmenuDll",
        objects={
            Object(Matching, "REL/selmenuDll/selmenu.c"),
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
            Object(Matching, "REL/w01Dll/world01.c"),
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
            Object(Matching, "REL/w06Dll/world06.c"),
        },
    ),
    Rel(
        "w06sDll",
        objects={
            Object(Matching, "REL/w06sDll/world06.c"),
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
