cmake_minimum_required(VERSION 3.18)
project(doc)

# generate doc
find_package(Doxygen)
if(${DOXYGEN_FOUND})
    set(DOC_SOURCE_PATH  ${PROJECT_SOURCE_DIR}/..)
    set(DOXYGEN_OUTPUT_DIRECTORY ${DOC_SOURCE_PATH}/doc)
    message(STATUS "netpp: Doc generate supported ${DOC_SOURCE_PATH}")
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
    set(DOXYGEN_PROJECT_NAME netpp)
    doxygen_add_docs(
            doc
            ${DOC_SOURCE_PATH}/include ${DOC_SOURCE_PATH}/src ${DOC_SOURCE_PATH}/README.md
            WORKING_DIRECTORY ${DOC_SOURCE_PATH}
            COMMENT "Generate doc ${DOC_SOURCE_PATH}"
    )
endif()
