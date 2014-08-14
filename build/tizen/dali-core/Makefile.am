# Copyright (c) 2014 Samsung Electronics Co., Ltd.

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Parse each file.list
# These files define <module>_src_files & <module>_header_files,
# where <module> is the subdirectory from core_src_dir

emoticons_dir = /usr/share/emoticons/

internal_src_dir = ../../../dali/internal
include ../../../dali/internal/file.list

public_api_src_dir = ../../../dali/public-api
include ../../../dali/public-api/file.list

platform_abstraction_src_dir = ../../../dali/integration-api
include ../../../dali/integration-api/file.list

# Build the Dali libtool library

src_files = $(internal_src_files) \
            $(effects_src_files) \
            $(platform_abstraction_src_files) \
            $(public_api_src_files)

if DYNAMICS_SUPPORT
src_files += $(internal_dynamics_src_files)
endif

lib_LTLIBRARIES = libdali-core.la

libdali_core_la_SOURCES = $(dali_shaders_src_file) \
                          $(src_files)

scripts_dir = ../../scripts
dali_shaders_script = $(scripts_dir)/generate-shader-strings.pl
dali_shaders_dir = ../../../dali/internal/render/shader-source

$(nodist_libdali_core_la_OBJECTS): $(dali_shaders_src_file)

dali_shaders_src_file = dali-shaders.cpp

dali-shaders.cpp: $(dali_shaders_script) $(dali_shaders_dir)/*.txt
	$< --shader-dir=$(dali_shaders_dir) --file-prefix=dali-shaders

dali_core_includes = \
        -I../../..

daliShaderbinDir  = ${dataReadWriteDir}/core/shaderbin/
daliGlyphCacheDir = ${dataReadWriteDir}/glyphcache/

libdali_core_la_CXXFLAGS = -DDALI_COMPILATION \
                           -DDALI_DATA_RW_DIR="\"${daliReadWriteDir}\"" \
                           -DDALI_DATA_RO_DIR="\"${daliReadOnlyDir}\"" \
                           -DDALI_SHADERBIN_DIR="\"${daliShaderbinDir}\"" \
                           -DDALI_GLYPH_CACHE_DIR="\"${daliGlyphCacheDir}\"" \
                           -DDALI_EMOTICON_DIR="\"${emoticons_dir}\"" \
                           -Werror -Wall -Wfloat-equal \
                           $(dali_core_includes) \
                           $(DALI_CFLAGS)

libdali_core_la_LIBADD = $(DALI_LDFLAGS) \
                         -lboost_thread

# Create an empty shaderbin dir
install-data-local:
	$(MKDIR_P) ${DESTDIR}/${daliShaderbinDir}

# Install headers under the correct subdirectories
platformabstractiondir = $(includedir)/dali/integration-api
platformabstractioneventsdir = $(includedir)/dali/integration-api/events
platformabstractiondynamicsdir = $(includedir)/dali/integration-api/dynamics

platformabstraction_HEADERS = $(platform_abstraction_header_files)
platformabstractionevents_HEADERS = $(platform_abstraction_events_header_files)
platformabstractiondynamics_HEADERS = $(platform_abstraction_dynamics_header_files)

publicapidir = $(devincludepath)/dali/public-api
publicapiactorsdir = $(publicapidir)/actors
publicapianimationdir = $(publicapidir)/animation
publicapicommondir = $(publicapidir)/common
publicapidynamicsdir = $(publicapidir)/dynamics
publicapieventsdir = $(publicapidir)/events
publicapigeometrydir = $(publicapidir)/geometry
publicapiimagesdir = $(publicapidir)/images
publicapimathdir = $(publicapidir)/math
publicapimodelingdir = $(publicapidir)/modeling
publicapiobjectdir = $(publicapidir)/object
publicapirendertasksdir = $(publicapidir)/render-tasks
publicapishadereffectsdir = $(publicapidir)/shader-effects
publicapisignalsdir = $(publicapidir)/signals
publicapitextdir = $(publicapidir)/text
publicapiscriptingdir = $(publicapidir)/scripting

publicapi_HEADERS = $(public_api_header_files)
publicapiactors_HEADERS = $(public_api_core_actors_header_files)
publicapianimation_HEADERS = $(public_api_core_animation_header_files)
publicapicommon_HEADERS = $(public_api_core_common_header_files)
publicapidynamics_HEADERS = $(public_api_core_dynamics_header_files)
publicapievents_HEADERS = $(public_api_core_events_header_files)
publicapigeometry_HEADERS = $(public_api_core_geometry_header_files)
publicapiimages_HEADERS = $(public_api_core_images_header_files)
publicapimath_HEADERS = $(public_api_core_math_header_files)
publicapimodeling_HEADERS = $(public_api_core_modeling_header_files)
publicapirendertasks_HEADERS = $(public_api_core_render_tasks_header_files)
publicapiobject_HEADERS = $(public_api_core_object_header_files)
publicapiscripting_HEADERS = $(public_api_core_scripting_header_files)
publicapishadereffects_HEADERS = $(public_api_core_shader_effects_header_files)
publicapisignals_HEADERS = $(public_api_core_signals_header_files)
publicapitext_HEADERS = $(public_api_core_text_header_files)

CLEANFILES = dali-shaders.cpp \
             dali-shaders.h

# linking test

noinst_PROGRAMS = linker.test

linker_test_SOURCES = linker-test.cpp  \
    ../../../automated-tests/src/dali/dali-test-suite-utils/test-application.cpp \
    ../../../automated-tests/src/dali/dali-test-suite-utils/test-platform-abstraction.cpp \
    ../../../automated-tests/src/dali/dali-test-suite-utils/test-render-controller.cpp \
    ../../../automated-tests/src/dali/dali-test-suite-utils/test-gl-abstraction.cpp \
    ../../../automated-tests/src/dali/dali-test-suite-utils/test-gesture-manager.cpp \
    ../../../automated-tests/src/dali/dali-test-suite-utils/test-gl-sync-abstraction.cpp \
    ../../../automated-tests/src/dali/dali-test-suite-utils/test-trace-call-stack.cpp


linker_test_CXXFLAGS = \
  -Werror -Wall -Wfloat-equal \
  -I../../../automated-tests/src/dali/dali-test-suite-utils \
  $(dali_core_includes) \
  $(DALI_CFLAGS)

linker_test_DEPENDENCIES = libdali-core.la
linker_test_LDADD = \
  libdali-core.la