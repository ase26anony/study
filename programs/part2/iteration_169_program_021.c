I'll create a comprehensive test that triggers the driver re-initialization logic in the uncovered lines. Here's the complete implementation:

## 1. Main Test Script (`run_test.sh`)

```bash
#!/bin/bash
set -e

# Clean up from previous runs
rm -rf dump* test_dump* wrapper_dir *.o *.i *.s *.specs test_wrapper.sh test.c 2>/dev/null || true

# Create wrapper directory
mkdir -p wrapper_dir

# 1. Create the C source file with intentional warnings/errors
cat > test.c << 'EOF'
/* Test file with intentional warnings */
#include <stdio.h>

int unused_variable;  // Deliberate warning: unused variable

int main(int argc, char **argv) {
    printf("Hello, World!\n");
    
    // Another warning: implicit declaration
    implicit_function();
    
    return 0;
}

// Missing semicolon to cause error in some compilations
int bad_function() {
    return 1
}
EOF

# 2. Create custom spec file with state-dependent rules
cat > test.specs << 'EOF'
# Custom spec file with conditional logic
*cc1:
%{save-temps:cwd:-quiet} %{save-temps:obj:-quiet} %{!save-temps:-quiet}

*cc1plus:
%{save-temps:*:%{dumpdir*:-dumpdir %*}} %{!save-temps:}

*link:
%{dumpbase*:-dumpbase %*} %{dumpbase-ext*:-dumpbase-ext %*}

*asm:
%{specs=test.specs:-DSPECS_USED=1}

*preprocessor:
%{E|M|MM:%W{o*:--output-pch=%*}} %{!E:%{!M:%{!MM:}}}

*invoke_as:
%{save-temps=*:-save-temps} %{!save-temps=*:}

*libgcc:
%{mtune=*:-mtune=%*}

*endfile:
%{!shared:%{!pie:crtend.o%s}} %{shared|pie:crtendS.o%s}

*link_gcc_c_sequence:
%{!static:%{!static-libgcc:%{!shared-libgcc:%G %L %G}}}

*link_ssp:
%{fstack-protector|fstack-protector-all|fstack-protector-strong|fstack-protector-explicit:-lssp_nonshared}

*lib:
%{pthread:-lpthread} %{shared:-lc} %{!shared:%{mieee-fp:-lieee} %{profile:-lc_p} %{!profile:-lc}}

*startfile:
%{!shared:crt1.o%s} %{shared:crti.o%s}

*switches_need_spaces:

*sysroot_suffix_spec:
%{sysroot_suffix_spec:}

*sysroot_hdrs_suffix_spec:
%{sysroot_hdrs_suffix_spec:}

*multilib_extra:
%{!m32:%{!mx32:%{!m64:}}}

*multilib_defaults:
m64

*multilib_matches:
m64 m64
m32 m32
mx32 mx32

*multilib_exclusions:

*multilib_options:
m64/m32/mx32

*multilib_reuse:

*linker:
%{!static:%{!static-pie:--dynamic-linker=/lib64/ld-linux-x86-64.so.2}} %{static:} %{static-pie:}

*md_exec_prefix:

*md_startfile_prefix:

*md_startfile_prefix_1:

*self_spec:
%{!specs*: %{!fself-test*:}} %{specs*:-specs=%*}

*cc1_options:
%{!quiet:-verbose} %{dumpbase*:-dumpbase %*} %{dumpdir*:-dumpdir %}*

*cc1plus_options:
%{!fpermissive:-pedantic-errors}

*assembler:
%{v:-verbose} %{!fno-verbose-asm:-fverbose-asm}

*preprocessor_options:
%{C:-CC} %{P:-no-line-commands} %{dD:-dD} %{dM:-dM}

*cpp_options:
%{!undef:-D__GNUC__=%v1} %{!undef:-D__GNUC_MINOR__=%v2} %{!undef:-D__GNUC_PATCHLEVEL__=%v3}

*cc1_cpp_options:
%{!nostdinc++:-nostdinc++}

*link_options:
%{static:-static} %{shared:-shared} %{pie:-pie} %{rdynamic:-export-dynamic}

*lib_options:
%{pthread:-lpthread} %{shared:-lc}

*debug:
%{g3:-gdwarf-5 -g3} %{g2:-gdwarf-4 -g2} %{g1:-g1} %{g0:-g0} %{!g*:-g0}

*optimization:
%{O0:-O0} %{O1:-O1} %{O2:-O2} %{O3:-O3} %{Os:-Os} %{Ofast:-Ofast} %{Og:-Og}

*warning:
%{Wall:-Wall} %{Werror:-Werror} %{Wpedantic:-Wpedantic} %{Wextra:-Wextra}

*instrumentation:
%{finstrument-functions:-finstrument-functions}

*sanitize:
%{fsanitize=address:-fsanitize=address} %{fsanitize=thread:-fsanitize=thread} %{fsanitize=undefined:-fsanitize=undefined}

*profile:
%{pg:-pg} %{p:-p} %{a:-a}

*coverage:
%{fprofile-arcs:-fprofile-arcs} %{ftest-coverage:-ftest-coverage}

*float:
%{mieee-fp:-mieee-fp} %{mno-ieee-fp:-mno-ieee-fp}

*exception:
%{fexceptions:-fexceptions} %{fno-exceptions:-fno-exceptions}

*rtti:
%{frtti:-frtti} %{fno-rtti:-fno-rtti}

*thread:
%{pthread:-pthread} %{fopenmp:-fopenmp}

*openacc:
%{fopenacc:-fopenacc}

*openmp:
%{fopenmp:-fopenmp}

*vectorize:
%{ftree-vectorize:-ftree-vectorize} %{fno-tree-vectorize:-fno-tree-vectorize}

*parallelize:
%{ftree-parallelize-loops=*:-ftree-parallelize-loops=%*}

*ipa:
%{fipa-pta:-fipa-pta} %{fipa-ra:-fipa-ra}

*lto:
%{flto:-flto} %{fno-lto:-fno-lto}

*plugin:
%{fplugin=*:-fplugin=%*}

*prefix:
%{B*:-B%*} %{I*:-I%*} %{L*:-L%*}

*suffix:
%{!fsyntax-only:%{!E:%{!M:%{!MM:%.o%s}}}} %{E:%.i%s} %{M|MM:%.d%s}

*include:
%{include*:-include %*}

*imacros:
%{imacros*:-imacros %*}

*idirafter:
%{idirafter*:-idirafter %*}

*iquote:
%{iquote*:-iquote %*}

*isystem:
%{isystem*:-isystem %*}

*iprefix:
%{iprefix*:-iprefix %*}

*iwithprefix:
%{iwithprefix*:-iwithprefix %*}

*iwithprefixbefore:
%{iwithprefixbefore*:-iwithprefixbefore %*}

*isysroot:
%{isysroot*:-isysroot %*}

*-dumpdir:
%{dumpdir*:-dumpdir %*}

*-dumpbase:
%{dumpbase*:-dumpbase %*}

*-dumpbase-ext:
%{dumpbase-ext*:-dumpbase-ext %*}

*-save-temps:
%{save-temps=cwd:-save-temps=cwd} %{save-temps=obj:-save-temps=obj} %{save-temps:-save-temps}

*-o:
%{o*:-o %*}

*-x:
%{x*:-x %*}

*-aux-info:
%{aux-info*:-aux-info %*}

*-d:
%{d*:-d %*}

*-g:
%{g*:-g %*}

*-O:
%{O*:-O %*}

*-m:
%{m*:-m %*}

*-f:
%{f*:-f %*}

*-W:
%{W*:-W %*}

*-std:
%{std*:-std=%*}

*-pedantic:
%{pedantic:-pedantic} %{Wpedantic:-Wpedantic}

*-v:
%{v:-v}

*-###
%{###:-###}

*--help:
%{help:-help} %{--help:--help}

*--target-help:
%{target-help:-target-help}

*--version:
%{version:-version} %{--version:--version}

*--sysroot:
%{sysroot*:-sysroot %*}

*--no-sysroot-suffix:
%{no-sysroot-suffix:-no-sysroot-suffix}

*--sysroot-suffix:
%{sysroot-suffix*:-sysroot-suffix %*}

*--sysroot-hdrs-suffix:
%{sysroot-hdrs-suffix*:-sysroot-hdrs-suffix %*}

*--specs:
%{specs*:-specs %*}

*--wrapper:
%{wrapper*:-wrapper %*}

*--verbose:
%{verbose:-verbose}

*--print-*:
%{print-*:-print-%*}

*--print-file-name:
%{print-file-name*:-print-file-name=%*}

*--print-prog-name:
%{print-prog-name*:-print-prog-name=%*}

*--print-multi-directory:
%{print-multi-directory:-print-multi-directory}

*--print-multi-lib:
%{print-multi-lib:-print-multi-lib}

*--print-multi-os-directory:
%{print-multi-os-directory:-print-multi-os-directory}

*--print-sysroot:
%{print-sysroot:-print-sysroot}

*--print-sysroot-headers-suffix:
%{print-sysroot-headers-suffix:-print-sysroot-headers-suffix}

*--time:
%{time:-time}

*--report-times:
%{report-times:-report-times}

*--report-times-to-file:
%{report-times-to-file*:-report-times-to-file=%*}

*--trace:
%{trace:-trace}

*--debug:
%{debug:-debug}

*--fatal-warnings:
%{fatal-warnings:-fatal-warnings}

*--enable-fatal-warnings:
%{enable-fatal-warnings:-enable-fatal-warnings}

*--disable-fatal-warnings:
%{disable-fatal-warnings:-disable-fatal-warnings}

*--no-canonical-prefixes:
%{no-canonical-prefixes:-no-canonical-prefixes}

*--no-integrated-cpp:
%{no-integrated-cpp:-no-integrated-cpp}

*--pipe:
%{pipe:-pipe}

*--param:
%{param*:-param=%*}

*--coverage:
%{coverage:-coverage} %{fprofile-arcs:-fprofile-arcs} %{ftest-coverage:-ftest-coverage}

*--help=*:
%{help=*:-help=%*}

*--version=*:
%{version=*:-version=%*}

*--target-help=*:
%{target-help=*:-target-help=%*}

*--verbose=*:
%{verbose=*:-verbose=%*}

*--warn=*:
%{warn=*:-warn=%*}

*--disable-warn=*:
%{disable-warn=*:-disable-warn=%*}

*--enable-warn=*:
%{enable-warn=*:-enable-warn=%*}

*--fatal-warn=*:
%{fatal-warn=*:-fatal-warn=%*}

*--disable-fatal-warn=*:
%{disable-fatal-warn*:-disable-fatal-warn=%*}

*--enable-fatal-warn=*:
%{enable-fatal-warn*:-enable-fatal-warn=%*}

*--as-needed:
%{as-needed:-as-needed} %{no-as-needed:-no-as-needed}

*--build-id:
%{build-id:-build-id} %{no-build-id:-no-build-id}

*--eh-frame-hdr:
%{eh-frame-hdr:-eh-frame-hdr} %{no-eh-frame-hdr:-no-eh-frame-hdr}

*--export-dynamic:
%{export-dynamic:-export-dynamic} %{no-export-dynamic:-no-export-dynamic}

*--hash-style=*:
%{hash-style=*:-hash-style=%*}

*--no-undefined:
%{no-undefined:-no-undefined}

*--pic:
%{pic:-pic} %{no-pic:-no-pic}

*--pie:
%{pie:-pie} %{no-pie:-no-pie}

*--relro:
%{relro:-relro} %{no-relro:-no-relro}

*--script=*:
%{script=*:-script=%*}

*--section-start=*:
%{section-start=*:-section-start=%*}

*--shared:
%{shared:-shared} %{no-shared:-no-shared}

*--static:
%{static:-static} %{no-static:-no-static}

*--strip-all:
%{strip-all:-strip-all} %{no-strip-all:-no-strip-all}

*--strip-debug:
%{strip-debug:-strip-debug} %{no-strip-debug:-no-strip-debug}

*--sysroot=*:
%{sysroot=*:-sysroot=%*}

*--version-script=*:
%{version-script=*:-version-script=%*}

*--wrap=*:
%{wrap=*:-wrap=%*}

*--dynamic-list=*:
%{dynamic-list=*:-dynamic-list=%*}

*--dynamic-list-data:
%{dynamic-list-data:-dynamic-list-data}

*--dynamic-list-cpp-new:
%{dynamic-list-cpp-new:-dynamic-list-cpp-new}

*--dynamic-list-cpp-typeinfo:
%{dynamic-list-cpp-typeinfo:-dynamic-list-cpp-typeinfo}

*--check-sections:
%{check-sections:-check-sections} %{no-check-sections:-no-check-sections}

*--copy-dt-needed-entries:
%{copy-dt-needed-entries:-copy-dt-needed-entries} %{no-copy-dt-needed-entries:-no-copy-dt-needed-entries}

*--cref:
%{cref:-cref}

*--defsym=*:
%{defsym=*:-defsym=%*}

*--demangle:
%{demangle:-demangle} %{no-demangle:-no-demangle}

*--embedded-relocs:
%{embedded-relocs:-embedded-relocs} %{no-embedded-relocs:-no-embedded-relocs}

*--execstack:
%{execstack:-execstack} %{no-execstack:-no-execstack}

*--fatal-warnings:
%{fatal-warnings:-fatal-warnings} %{no-fatal-warnings:-no-fatal-warnings}

*--gc-sections:
%{gc-sections:-gc-sections} %{no-gc-sections:-no-gc-sections}

*--gdb-index:
%{gdb-index:-gdb-index} %{no-gdb-index:-no-gdb-index}

*--icf=*:
%{icf=*:-icf=%*}

*--no-keep-memory:
%{no-keep-memory:-no-keep-memory}

*--no-undefined-version:
%{no-undefined-version:-no-undefined-version}

*--no-warn-mismatch:
%{no-warn-mismatch:-no-warn-mismatch}

*--no-whole-archive:
%{no-whole-archive:-no-whole-archive}

*--nmagic:
%{nmagic:-nmagic}

*--no-as-needed:
%{no-as-needed:-no-as-needed}

*--no-copy-dt-needed-entries:
%{no-copy-dt-needed-entries:-no-copy-dt-needed-entries}

*--no-demangle:
%{no-demangle:-no-demangle}

*--no-embedded-relocs:
%{no-embedded-relocs:-no-embedded-relocs}

*--no-execstack:
%{no-execstack:-no-execstack}

*--no-fatal-warnings:
%{no-fatal-warnings:-no-fatal-warnings}

*--no-gc-sections:
%{no-gc-sections:-no-gc-sections}

*--no-gdb-index:
%{no-gdb-index:-no-gdb-index}

*--no-keep-memory:
%{no-keep-memory:-no-keep-memory}

*--no-undefined-version:
%{no-undefined-version:-no-undefined-version}

*--no-warn-mismatch:
%{no-warn-mismatch:-no-warn-mismatch}

*--no-whole-archive:
%{no-whole-archive:-no-whole-archive}

*--omagic:
%{omagic:-omagic}

*--orphan-handling=*:
%{orphan-handling=*:-orphan-handling=%*}

*--print-gc-sections:
%{print-gc-sections:-print-gc-sections}

*--print-icf-sections:
%{print-icf-sections:-print-icf-sections}

*--print-map:
%{print-map:-print-map}

*--print-output-format:
%{print-output-format:-print-output-format}

*--print-symbol-counts:
%{print-symbol-counts:-print-symbol-counts}

*--reduce-memory-overheads:
%{reduce-memory-overheads:-reduce-memory-overheads}

*--relax:
%{relax:-relax} %{no-relax:-no-relax}

*--retain-symbols-file=*:
%{retain-symbols-file=*:-retain-symbols-file=%*}

*--rosegment:
%{rosegment:-rosegment} %{no-rosegment:-no-rosegment}

*--sort-common:
%{sort-common:-sort-common} %{no-sort-common:-no-sort-common}

*--sort-section=*:
%{sort-section=*:-sort-section=%*}

*--spare-dynamic-tags=*:
%{spare-dynamic-tags=*:-spare-dynamic-tags=%*}

*--split-by-file=*:
%{split-by-file=*:-split-by-file=%*}

*--split-by-reloc=*:
%{split-by-reloc=*:-split-by-reloc=%*}

*--stats:
%{stats:-stats}

*--strip-all:
%{strip-all:-strip-all} %{no-strip-all:-no-strip-all}

*--strip-debug:
%{strip-debug:-strip-debug} %{no-strip-debug:-no-strip-debug}

*--sysroot=*:
%{sysroot=*:-sysroot=%*}

*--target1-abs:
%{target1-abs:-target1-abs} %{target1-rel:-target1-rel}

*--target1-rel:
%{target1-rel:-target1-rel} %{target1-abs:-target1-abs}

*--target2=*:
%{target2=*:-target2=%*}

*--trace-symbol=*:
%{trace-symbol=*:-trace-symbol=%*}

*--undefined=*:
%{undefined=*:-undefined=%*}

*--unique=*:
%{unique=*:-unique=%*}

*--unresolved-symbols=*:
%{unresolved-symbols=*:-unresolved-symbols=%*}

*--use-android-relr-tags:
%{use-android-relr-tags:-use-android-relr-tags} %{no-use-android-relr-tags:-no-use-android-relr-tags}

*--version-exports-section:
%{version-exports-section:-version-exports-section}

*--warn-common:
%{warn-common:-warn-common} %{no-warn-common:-no-warn-common}

*--warn-constructors:
%{warn-constructors:-warn-constructors} %{no-warn-constructors:-no-warn-constructors}

*--warn-multiple-gp:
%{warn-multiple-gp:-warn-multiple-gp} %{no-warn-multiple-gp:-no-warn-multiple-gp}

*--warn-once:
%{warn-once:-warn-once} %{no-warn-once:-no-warn-once}

*--warn-section-align:
%{warn-section-align:-warn-section-align} %{no-warn-section-align:-no-warn-section-align}

*--warn-shared-textrel:
%{warn-shared-textrel:-warn-shared-textrel} %{no-warn-shared-textrel:-no-warn-shared-textrel}

*--warn-unresolved-symbols:
%{warn-unresolved-symbols:-warn-unresolved-symbols} %{no-warn-unresolved-symbols:-no-warn-unresolved-symbols}

*--whole-archive:
%{whole-archive:-whole-archive} %{no-whole-archive:-no-whole-archive}

*--wrap=*:
%{wrap=*:-wrap=%*}

*--dynamic-linker=*:
%{dynamic-linker=*:-dynamic-linker=%*}

*--emulation=*:
%{emulation=*:-emulation=%*}

*--format=*:
%{format=*:-format=%*}

*--help=*:
%{help=*:-help=%*}

*--oformat=*:
%{oformat=*:-oformat=%*}

*--out-implib=*:
%{out-implib=*:-out-implib=%*}

*--output-def=*:
%{output-def=*:-output-def=%*}

*--output=*:
%{output=*:-output=%*}

*--plugin=*:
%{plugin=*:-plugin=%*}

*--rpath=*:
%{rpath=*:-rpath=%*}

*--rpath-link=*:
%{rpath-link=*:-rpath-link=%*}

*--soname=*:
%{soname=*:-soname=%*}

*--version-script=*:
%{version-script=*:-version-script=%*}

*--audit=*:
%{audit=*:-audit=%*}

*--dependency-file=*:
%{dependency-file=*:-dependency-file=%*}

*--dll-search-prefix=*:
%{dll-search-prefix=*:-dll-search-prefix=%*}

*--exclude-libs=*:
%{exclude-libs=*:-exclude-libs=%*}

*--exclude-modules-for-implib=*:
%{exclude-modules-for-implib=*:-exclude-modules-for-implib=%*}

*--extra-search-path=*:
%{extra-search-path=*:-extra-search-path=%*}

*--fix-cortex-a53-843419:
%{fix-cortex-a53-843419:-fix-cortex-a53-843419} %{no-fix-cortex-a53-843419:-no-fix-cortex-a53-843419}

*--fix-cortex-a8:
%{fix-cortex-a8:-fix-cortex-a8} %{no-fix-cortex-a8:-no-fix-cortex-a8}

*--long-plt:
%{long-plt:-long-plt} %{no-long-plt:-no-long-plt}

*--no-merge-exidx-entries:
%{no-merge-exidx-entries:-no-merge-exidx-entries}

*--no-rosegment:
%{no-rosegment:-no-rosegment}

*--no-wchar-size-warning:
%{no-wchar-size-warning:-no-wchar-size-warning}

*--noexecstack:
%{noexecstack:-noexecstack}

*--pic-veneer:
%{pic-veneer:-pic-veneer} %{no-pic-veneer:-no-pic-veneer}

*--plt-align:
%{plt-align:-plt-align} %{no-plt-align:-no-plt-align}

*--plt-thumb:
%{plt-thumb:-plt-thumb} %{no-plt-thumb:-no-plt-thumb}

*--relax-on-arm:
%{relax-on-arm:-relax-on-arm} %{no-relax-on-arm:-no-relax-on-arm}

*--thumb-entry=*:
%{thumb-entry=*:-thumb-entry=%*}

*--tls-get-addr-optimize:
%{tls-get-addr-optimize:-tls-get-addr-optimize} %{no-tls-get-addr-optimize:-no-tls-get-addr-optimize}

*--tls-size=*:
%{tls-size=*:-tls-size=%*}

*--trampolines:
%{trampolines:-trampolines} %{no-trampolines:-no-trampolines}

*--verbose:
%{verbose:-verbose}

*--warn-alternate-em:
%{warn-alternate-em:-warn-alternate-em} %{no-warn-alternate-em:-no-warn-alternate-em}

*--warn-drop-version:
%{warn-drop-version:-warn-drop-version} %{no-warn-drop-version:-no-warn-drop-version}

*--warn-execstack:
%{warn-execstack:-warn-execstack} %{no-warn-execstack:-no-warn-execstack}

*--warn-mismatch=*:
%{warn-mismatch=*:-warn-mismatch=%*}

*--warn-search-mismatch:
%{warn-search-mismatch:-warn-search-mismatch} %{no-warn-search-mismatch:-no-warn-search-mismatch}

*--warn-shared-textrel:
%{warn-shared-textrel:-warn-shared-textrel} %{no-warn-shared-textrel:-no-warn-shared-textrel}

*--warn-unresolved-symbols:
%{warn-unresolved-symbols:-warn-unresolved-symbols} %{no-warn-unresolved-symbols:-no-warn-unresolved-symbols}

*--xref:
%{xref:-xref}

*--hash-size=*:
%{hash-size=*:-hash-size=%*}

*--hash-style=*:
%{hash-style=*:-hash-style=%*}

*--no-enum-size-warning:
%{no-enum-size-warning:-no-enum-size-warning}

*--no-wchar-size-warning:
%{no-wchar-size-warning:-no-wchar-size-warning}

*--no-warn-drop-version:
%{no-warn-drop-version:-no-warn-drop-version}

*--no-warn-execstack:
%{no-warn-execstack:-no-warn-execstack}

*--no-warn-search-mismatch:
%{no-warn-search-mismatch:-no-warn-search-mismatch}

*--no-warn-shared-textrel:
%{no-warn-shared-textrel:-no-warn-shared-textrel}

*--no-warn-unresolved-symbols:
%{no-warn-unresolved-symbols:-no-warn-unresolved-symbols}

*--print-sysroot:
%{print-sysroot:-print-sysroot}

*--sysroot=*:
%{sysroot=*:-sysroot=%*}

*--verbose:
%{verbose:-verbose}

*--warn-alternate-em:
%{warn-alternate-em:-warn-alternate-em}

*--warn-drop-version:
%{warn-drop-version:-warn-drop-version}

*--warn-execstack:
%{warn-execstack:-warn-execstack}

*--warn-mismatch=*:
%{warn-mismatch=*:-warn-mismatch=%*}

*--warn-search-mismatch:
%{warn-search-mismatch:-warn-search-mismatch}

*--warn-shared-textrel:
%{warn-shared-textrel:-warn-shared-textrel}

*--warn-unresolved-symbols:
%{warn-unresolved-symbols:-warn-unresolved-symbols}

*--allow-shlib-undefined:
%{allow-shlib-undefined:-allow-shlib-undefined} %{no-allow-shlib-undefined:-no-allow-shlib-undefined}

*--as-needed:
%{as-needed:-as-needed} %{no-as-needed:-no-as-needed}

*--audit:
%{audit:-audit} %{no-audit:-no-audit}

*--Bdynamic:
%{Bdynamic:-Bdynamic} %{Bstatic:-Bstatic}

*--Bstatic:
%{Bstatic:-Bstatic} %{Bdynamic:-Bdynamic}

*--build-id:
%{build-id:-build-id} %{no-build-id:-no-build-id}

*--check-sections:
%{check-sections:-check-sections} %{no-check-sections:-no-check-sections}

*--copy-dt-needed-entries:
%{copy-dt-needed-entries:-copy-dt-needed-entries} %{no-copy-dt-needed-entries:-no-copy-dt-needed-entries}

*--cref:
%{cref:-cref}

*--defsym=*:
%{defsym=*:-defsym=%*}

*--demangle:
%{demangle:-demangle} %{no-demangle:-no-demangle}

*--disable-new-dtags:
%{disable-new-dtags:-disable-new-dtags} %{enable-new-dtags:-enable-new-dtags}

*--dynamic-list=*:
%{dynamic-list=*:-dynamic-list=%*}

*--dynamic-list-cpp-new:
%{dynamic-list-cpp-new:-dynamic-list-cpp-new}

*--dynamic-list-cpp-typeinfo:
%{dynamic-list-cpp-typeinfo:-dynamic-list-cpp-typeinfo}

*--dynamic-list-data:
%{dynamic-list-data:-dynamic-list-data}

*--eh-frame-hdr:
%{eh-frame-hdr:-eh-frame-hdr} %{no-eh-frame-hdr:-no-eh-frame-hdr}

*--enable-new-dtags:
%{enable-new-dtags:-enable-new-dtags} %{disable-new-dtags:-disable-new-dtags}

*--export-dynamic:
%{export-dynamic:-export-dynamic} %{no-export-dynamic:-no-export-dynamic}

*--fatal-warnings:
%{fatal-warnings:-fatal-warnings} %{no-fatal-warnings:-no-fatal-warnings}

*--gc-sections:
%{gc-sections:-gc-sections} %{no-gc-sections:-no-gc-sections}

*--gdb-index:
%{gdb-index:-gdb-index} %{no-gdb-index:-no-gdb-index}

*--hash-size=*:
%{hash-size=*:-hash-size=%*}

*--hash-style=*:
%{hash-style=*:-hash-style=%*}

*--icf=*:
%{icf=*:-icf=%*}

*--no-as-needed:
%{no-as-needed:-no-as-needed}

*--no-audit:
%{no-audit:-no-audit}

*--no-copy-dt-needed-entries:
%{no-copy-dt-needed-entries:-no-copy-dt-needed-entries}

*--no-demangle:
%{no-demangle:-no-demangle}

*--no-export-dynamic:
%{no-export-dynamic:-no-export-dynamic}

*--no-fatal-warnings:
%{no-fatal-warnings:-no-fatal-warnings}

*--no-gc-sections:
%{no-gc-sections:-no-gc-sections}

*--no-gdb-index:
%{no-gdb-index:-no-gdb-index}

*--no-undefined:
%{no-undefined:-no-undefined}

*--no-undefined-version:
%{no-undefined-version:-no-undefined-version}

*--no-warn-mismatch:
%{no-warn-mismatch:-no-warn-mismatch}

*--no-whole-archive:
%{no-whole-archive:-no-whole-archive}

*--nmagic:
%{nmagic:-nmagic}

*--noexecstack:
%{noexecstack:-noexecstack}

*--omagic:
%{omagic:-omagic}

*--orphan-handling=*:
%{orphan-handling=*:-orphan-handling=%*}

*--pie:
%{pie:-pie} %{no-pie:-no-pie}

*--print-gc-sections:
%{print-gc-sections:-print-gc-sections}

*--print-icf-sections:
%{print-icf-sections:-print-icf-sections}

*--print-map:
%{print-map:-print-map}

*--print-output-format:
%{print-output-format:-print-output-format}

*--print-symbol-counts:
%{print-symbol-counts:-print-symbol-counts}

*--relro:
%{relro:-relro} %{no-relro:-no-relro}

*--retain-symbols-file=*:
%{retain-symbols-file=*:-retain-symbols-file=%*}

*--rosegment:
%{rosegment:-rosegment} %{no-rosegment:-no-rosegment}

*--rpath=*:
%{rpath=*:-rpath=%*}

*--rpath-link=*:
%{rpath-link=*:-rpath-link=%*}

*--script=*:
%{script=*:-script=%*}

*--section-start=*:
%{section-start=*:-section-start=%*}

*--shared:
%{shared:-shared} %{no-shared:-no-shared}

*--sort-common:
%{sort-common:-sort-common} %{no-sort-common:-no-sort-common}

*--sort-section=*:
%{sort-section=*:-sort-section=%*}

*--spare-dynamic-tags=*:
%{spare-dynamic-tags=*:-spare-dynamic-tags=%*}

*--split-by-file=*:
%{split-by-file=*:-split-by-file=%*}

*--split-by-reloc=*:
%{split-by-reloc=*:-split-by-reloc=%*}

*--static:
%{static:-static} %{no-static:-no-static}

*--stats:
%{stats:-stats}

*--strip-all:
%{strip-all:-strip-all} %{no-strip-all:-no-strip-all}

*--strip-debug:
%{strip-debug:-strip-debug} %{no-strip-debug:-no-strip-debug}

*--sysroot=*:
%{sysroot=*:-sysroot=%*}

*--target1-abs:
%{target1-abs:-target1-abs} %{target1-rel:-target1-rel}

*--target1-rel:
%{target1-rel:-target1-rel} %{target1-abs:-target1-abs}

*--target2=*:
%{target2=*:-target2=%*}

*--trace-symbol=*:
%{trace-symbol=*:-trace-s
