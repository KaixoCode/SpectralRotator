
# ==============================================

cmake_minimum_required(VERSION 3.15)

# ==============================================

set(NAME "SpectralRotator")
set(CODE "Sprt")
set(COMPANY "Kaixo")
set(COMPANY_CODE "Kaix")
set(WEBSITE "https://kaixo.me")
set(BUNDLE_IDENTIFIER "me.kaixo.spectralrotator")
set(VERSION_TYPE "SNAPSHOT")
set(VERSION "1.0.3")
set(INITIAL_SIZE "500, 500")
set(IS_SYNTH true)

# ==============================================

set(THEME "theme/theme.json")
set(THEME_SCHEMA "theme/schema.json")
set(PARAMETERS "resources/Parameters.xml")

# ==============================================

set (CMAKE_CXX_STANDARD 23)

project(${NAME} VERSION ${VERSION})

add_subdirectory(core)

# ==============================================
