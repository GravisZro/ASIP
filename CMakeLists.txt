### CMake config ###
cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0048 NEW)

### Project ###
project(Polygon-Approximation VERSION 0.1)

include_directories( BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/Core ${CMAKE_CURRENT_SOURCE_DIR}/Image-Vectorization )
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake ) 
if(NOT CMAKE_BUILD_TYPE)
	set( CMAKE_BUILD_TYPE Release )
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frounding-math")
if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)

SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O1 -g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g")

### External libs ###
add_definitions(-DCGAL_HAS_THREADS)

# GDAL
find_package(GDAL REQUIRED)
include_directories( BEFORE ${GDAL_INCLUDE_DIRS})

# OpenCV
find_package( OpenCV REQUIRED )

# CGAL
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
find_package(CGAL COMPONENTS Core Qt5 REQUIRED)
include( ${CGAL_USE_FILE} )

# Eigen
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# Boost
find_package(Boost 1.65.0 REQUIRED COMPONENTS filesystem)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREAD OFF)

### Complier flags ###
add_definitions( -Wall )


### Source files listing variables ###
set( HDRS Core/defs.h Core/comparators.h Core/geometry.h 
          Core/indexed_event.h Core/kinetic_model.h Core/line_item.h
          Core/lsd.h Core/lsd_interface.h Core/lsd_tuned.h
          Core/matrix.h Core/means.h Core/parameters.h Core/partition.h 
          Core/partition_elements.h Core/propagation.h
          Core/quadtree.h Core/quadtree_point.h Core/r_angle.h 
          Core/refine_partition.h Core/regularization_angles.h 
          Core/regularization_angles_ms.h 
          Core/regularization_ordinates.h Core/regularization_ordinates_ms.h 
          Core/r_ordinate.h 
          Core/segment_ray.h Core/segment_tree.h Core/svg.h Core/trace.h )

set( SRCS Core/geometry.cpp 
          Core/indexed_event.cpp Core/kinetic_model.cpp Core/line_item.cpp
          Core/lsd.cpp Core/lsd_interface.cpp Core/lsd_tuned.cpp
          Core/means.cpp Core/parameters.cpp Core/partition.cpp 
          Core/partition_elements.cpp Core/propagation.cpp 
          Core/quadtree_point.cpp Core/r_angle.cpp 
          Core/refine_partition.cpp Core/regularization_angles.cpp 
          Core/regularization_angles_ms.cpp 
          Core/regularization_ordinates.cpp Core/regularization_ordinates_ms.cpp 
          Core/r_ordinate.cpp 
          Core/segment_ray.cpp Core/segment_tree.cpp )

set( GHDRS Image-Vectorization/myqgraphicsview.cpp Image-Vectorization/interface.h 
           Image-Vectorization/partition_refine.h
           Image-Vectorization/thread_master.h
           Image-Vectorization/thread_worker.h )
set( GSRCS Image-Vectorization/main.cpp 
           Image-Vectorization/myqgraphicsview.cpp Image-Vectorization/interface.cpp 
           Image-Vectorization/partition_refine.cpp
           Image-Vectorization/thread_master.cpp
           Image-Vectorization/thread_worker.cpp )
set( MOCS moc_interface.cxx
          moc_myqgraphicsview.cxx
          moc_partition_refine.cxx 
          moc_thread_master.cxx 
          moc_thread_worker.cxx )

set( CHDRS Image-Vectorization-Console/args.h 
           Image-Vectorization-Console/image_vectorization_console.h )
set( CSRCS Image-Vectorization-Console/main.cpp
           Image-Vectorization-Console/args.cpp 
           Image-Vectorization-Console/image_vectorization_console.cpp )


if( CGAL_FOUND AND Qt5_FOUND AND GDAL_FOUND 
    AND Boost_FOUND AND Eigen3_FOUND AND OpenCV_FOUND )
          
          ### Library ###
          add_library( core ${HDRS} ${SRCS} )
          target_link_libraries( core ${Boost_LIBRARIES})
          target_link_libraries( core ${GDAL_LIBRARIES})
          target_link_libraries( core ${OpenCV_LIBS} )
          target_link_libraries( core ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES} )     
          set_target_properties( core PROPERTIES VERSION ${PROJECT_VERSION} )
          
          ### cmd-line executable ###
          add_executable( image_vectorization_console ${CHDRS} ${CSRCS})
          target_link_libraries( image_vectorization_console core)
          
          ### GUI ### 
          Qt5_wrap_ui( R_UI_FILES Image-Vectorization/interface.ui )
      
          # Qrc files (resources files, that contain icons, at least)
          Qt5_add_resources( Qt5_RESOURCE_FILES Image-Vectorization/interface.qrc )
      
          # Use the Qt MOC preprocessor on classes that derives from QObject
          Qt5_generate_moc( Image-Vectorization/interface.h moc_interface.cxx )
          Qt5_generate_moc( Image-Vectorization/myqgraphicsview.h moc_myqgraphicsview.cxx )
          Qt5_generate_moc( Image-Vectorization/partition_refine.h moc_partition_refine.cxx )
          Qt5_generate_moc( Image-Vectorization/thread_master.h moc_thread_master.cxx )
          Qt5_generate_moc( Image-Vectorization/thread_worker.h moc_thread_worker.cxx )

          # The graphical interface executable
          add_executable( image_vectorization_gui ${GHDRS} ${GSRCS} ${MOCS} ${R_UI_FILES} ${Qt5_RESOURCE_FILES} )
      
          qt5_use_modules( image_vectorization_gui Core Gui Widgets )
          target_link_libraries( image_vectorization_gui core)
          
### Error messages ###
else()
          message(STATUS "NOTICE: This demo requires ?? and will not be compiled.")
endif() 
