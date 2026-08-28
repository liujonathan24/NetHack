find_program(FLEX_EXECUTABLE NAMES flex flex.exe)
find_program(BISON_EXECUTABLE NAMES bison bison.exe)

set(MAKEDEFS_SRC makedefs.c ${NLE_SRC}/monst.c ${NLE_SRC}/objects.c)

set(DGN_COMP_SRC dgn_main.c panic.c ${NLE_SRC}/alloc.c
                 ${NLE_UTIL_GEN}/dgn_parser.c ${NLE_UTIL_GEN}/dgn_scanner.c)

set(LEV_COMP_SRC
    lev_main.c
    panic.c
    ${NLE_SRC}/decl.c
    ${NLE_SRC}/monst.c
    ${NLE_SRC}/objects.c
    ${NLE_SRC}/alloc.c
    ${NLE_SRC}/drawing.c
    ${NLE_UTIL_GEN}/lev_parser.c
    ${NLE_UTIL_GEN}/lev_scanner.c)

set(MAKEDEFS_HEADERS ${NLE_INC_GEN}/date.h ${NLE_INC_GEN}/onames.h
                     ${NLE_INC_GEN}/pm.h)

set(DLB_SRC dlb_main.c ${NLE_SRC}/dlb.c panic.c ${NLE_SRC}/alloc.c)

file(MAKE_DIRECTORY ${NLE_INC_GEN} ${NLE_SRC_GEN})

add_executable(makedefs ${MAKEDEFS_SRC})
target_include_directories(makedefs PRIVATE ${NLE_INC} ${NLE_INC_GEN} ${nle_SOURCE_DIR}/third_party/deboost.context/include
                                            ${NLE_UTIL_GEN})
# Build-tool view of objects[]/obj_descr[] — extern writable arrays
# rather than nle_ctx_t-redirected macros (no runtime ctx here).
target_compile_definitions(makedefs PRIVATE NLE_OBJECTS_GLOBAL)
add_executable(dgn_comp ${DGN_COMP_SRC})
target_include_directories(dgn_comp PRIVATE ${NLE_INC} ${NLE_INC_GEN} ${nle_SOURCE_DIR}/third_party/deboost.context/include
                                            ${NLE_UTIL_GEN})
add_executable(lev_comp ${LEV_COMP_SRC})
target_include_directories(lev_comp PRIVATE ${NLE_INC} ${NLE_INC_GEN} ${nle_SOURCE_DIR}/third_party/deboost.context/include
                                            ${NLE_UTIL_GEN})
target_compile_definitions(lev_comp PRIVATE NLE_OBJECTS_GLOBAL)
add_executable(dlb ${DLB_SRC})
target_include_directories(dlb PRIVATE ${NLE_INC} ${NLE_INC_GEN} ${nle_SOURCE_DIR}/third_party/deboost.context/include
                                       ${NLE_UTIL_GEN})
add_executable(recover recover.c)
target_include_directories(recover PRIVATE ${NLE_INC} ${NLE_INC_GEN} ${nle_SOURCE_DIR}/third_party/deboost.context/include
                                           ${NLE_UTIL_GEN})

# date.h carries BUILD_DATE / BUILD_TIME, which makedefs -v stamps from the
# wall clock at build time. BUILD_DATE is printed on NetHack's intro screen, so
# it lands in the observation's tty_chars/tty_colors -- meaning ANY rebuild of
# even completely unmodified sources shows up as an observation diff against a
# reference build for the handful of steps before the splash scrolls away.
# That would make golden-parity comparisons unreadable (a real gameplay change
# and a mere relink look the same).
#
# If util/date.h.pinned exists, it is copied over the generated header, so a
# rebuilt engine is byte-identical in play to the build the pinned header came
# from. This is build metadata only: on UNIX, BUILD_TIME is read solely by
# comp_times(), which is #ifdef MICRO, so nothing else in the game reads either
# macro. Provenance of the actual binary is its mtime and ELF BuildID, not this
# string. Delete util/date.h.pinned to get the ordinary wall-clock behaviour.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/date.h.pinned)
  add_custom_command(
    DEPENDS makedefs ${CMAKE_CURRENT_SOURCE_DIR}/date.h.pinned
    OUTPUT ${NLE_INC_GEN}/date.h
    COMMAND $<TARGET_FILE:makedefs> ARGS -v
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/date.h.pinned ${NLE_INC_GEN}/date.h
    COMMENT "date.h: generated, then pinned from util/date.h.pinned")
else()
  add_custom_command(
    DEPENDS makedefs
    OUTPUT ${NLE_INC_GEN}/date.h
    COMMAND $<TARGET_FILE:makedefs> ARGS -v)
endif()

add_custom_command(
  DEPENDS makedefs
  OUTPUT ${NLE_INC_GEN}/onames.h
  COMMAND $<TARGET_FILE:makedefs> ARGS -o)

add_custom_command(
  DEPENDS makedefs
  OUTPUT ${NLE_INC_GEN}/pm.h
  COMMAND $<TARGET_FILE:makedefs> ARGS -p)

add_custom_command(
  DEPENDS tilemap
  OUTPUT ${NLE_SRC_GEN}/tile.c
  COMMAND $<TARGET_FILE:tilemap>)

add_custom_command(
  OUTPUT ${NLE_UTIL_GEN}/dgn_parser.c ${NLE_UTIL_GEN}/dgn_comp.h
  COMMAND bison -y --defines=${NLE_UTIL_GEN}/dgn_comp.h -o
          ${NLE_UTIL_GEN}/dgn_parser.c ${NLE_UTIL}/dgn_comp.y
  DEPENDS ${NLE_UTIL}/dgn_comp.y)

add_custom_command(
  OUTPUT ${NLE_UTIL_GEN}/dgn_scanner.c
  COMMAND ${FLEX_EXECUTABLE} ARGS -o${NLE_UTIL_GEN}/dgn_scanner.c
          ${NLE_UTIL}/dgn_comp.l
  DEPENDS ${NLE_UTIL}/dgn_comp.l)

add_custom_command(
  OUTPUT ${NLE_UTIL_GEN}/lev_parser.c ${NLE_UTIL_GEN}/lev_comp.h
  COMMAND ${BISON_EXECUTABLE} ARGS -y --defines=${NLE_UTIL_GEN}/lev_comp.h -o
          ${NLE_UTIL_GEN}/lev_parser.c ${NLE_UTIL}/lev_comp.y
  DEPENDS ${NLE_UTIL}/lev_comp.y)

add_custom_command(
  OUTPUT ${NLE_UTIL_GEN}/lev_scanner.c
  COMMAND ${FLEX_EXECUTABLE} ARGS -o${NLE_UTIL_GEN}/lev_scanner.c
          ${NLE_UTIL}/lev_comp.l
  DEPENDS ${NLE_UTIL}/lev_comp.l)

add_custom_target(util DEPENDS ${MAKEDEFS_HEADERS} recover)

add_executable(tilemap ${NLE_WIN}/share/tilemap.c)
target_include_directories(tilemap PUBLIC ${NLE_INC} ${NLE_INC_GEN})
target_compile_definitions(tilemap PRIVATE NLE_OBJECTS_GLOBAL)
add_dependencies(tilemap util)

add_library(tile OBJECT ${NLE_SRC_GEN}/tile.c)
target_include_directories(tile PUBLIC ${NLE_INC} ${NLE_INC_GEN})
target_compile_definitions(tile PRIVATE NLE_OBJECTS_GLOBAL)

# NOTE: util is dependent on these two
add_dependencies(lev_comp util)
add_dependencies(dgn_comp util)

set(MAN_PAGES ${NLE_DOC}/makedefs.6 ${NLE_DOC}/dgn_comp.6 ${NLE_DOC}/lev_comp.6
              ${NLE_DOC}/dlb.6 ${NLE_DOC}/recover.6 ${NLE_DOC}/nethack.6)

set(TXT_DOCS
    ${NLE_DOC}/makedefs.txt ${NLE_DOC}/dgn_comp.txt ${NLE_DOC}/lev_comp.txt
    ${NLE_DOC}/dlb.txt ${NLE_DOC}/recover.txt ${NLE_DOC}/nethack.txt)

install(TARGETS makedefs dgn_comp lev_comp dlb recover DESTINATION ${INSTDIR})
install(FILES ${MAN_PAGES} DESTINATION ${INSTDIR}/man)
install(FILES ${TXT_DOCS} DESTINATION ${INSTDIR}/doc)
