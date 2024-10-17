include(CMakeFindDependencyMacro)

find_dependency(esa)
find_dependency(esl)
find_dependency(CURL)

include("${CMAKE_CURRENT_LIST_DIR}/curl4eslTargets.cmake")
