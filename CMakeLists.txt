
# ==============================================

cmake_minimum_required(VERSION 3.22)

# ==============================================

set(NAME "SpectralRotator")
set(CODE "Sprt")
set(COMPANY "Kaixo")
set(COMPANY_CODE "Kaix")
set(WEBSITE "https://kaixo.me")
set(BUNDLE_IDENTIFIER "me.kaixo.spectralrotator")
set(INITIAL_SIZE "900, 600")
set(SOFTWARE_IDENTIFIER "4")
set(IS_SYNTH true)
set(USE_OPENGL false)
set(JUCE_VERSION "8.0.12")

# ==============================================

set(THEME "theme/theme.json")
set(THEME_SCHEMA "theme/schema.json")
set(PARAMETERS "resources/parameters.json")

# ==============================================

set (CMAKE_CXX_STANDARD 23)

if(NOT DEFINED VERSION)
  set(VERSION "1.1.1")
  set(VERSION_TYPE "SNAPSHOT")
  set(HAS_DEBUGGER true)
else()
  # When version is defined, it's a release/demo build
  if(NOT DEFINED VERSION)
    set(VERSION_TYPE "RELEASE")
  endif()
  set(HAS_DEBUGGER false)
endif()

project(${NAME} VERSION ${VERSION})

add_subdirectory(core)

# ==============================================
